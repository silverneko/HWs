/*
 *  (C) 2010 by Computer System Laboratory, IIS, Academia Sinica, Taiwan.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdlib.h>
#include "exec-all.h"
#include "tcg-op.h"
#include "helper.h"
#define GEN_HELPER 1
#include "helper.h"
#include "optimization.h"

extern uint8_t *optimization_ret_addr;

// #define NOSHACK
#define NOIBTC

#ifdef NOSHACK

void shack_set_shadow(CPUState *env, target_ulong guest_eip, unsigned long *host_eip) {}
void push_shack(CPUState *env, TCGv_ptr cpu_env, target_ulong next_eip) {}
void pop_shack(TCGv_ptr cpu_env, TCGv next_eip) {}
void helper_shack_flush(CPUState *env) {}

#else

#define MEMORY_ARENA_CHUNK_SIZE (1 << 20)
typedef struct MemoryArena {
    uint8_t chunk[MEMORY_ARENA_CHUNK_SIZE];
    unsigned int offset;
    unsigned int bytes_left;
    struct MemoryArena *prev;
} MemoryArena;

MemoryArena *new_arena() {
    MemoryArena *arena = (MemoryArena*) malloc(sizeof(MemoryArena));
    arena->offset = 0;
    arena->bytes_left = MEMORY_ARENA_CHUNK_SIZE;
    arena->prev = NULL;
    return arena;
}

void *arena_alloc(MemoryArena **arena_p, unsigned int size) {
    MemoryArena *arena = *arena_p;
    if (arena->bytes_left < size) {
        MemoryArena *new_arena = (MemoryArena*) malloc(sizeof(MemoryArena));
        new_arena->offset = 0;
        new_arena->bytes_left = MEMORY_ARENA_CHUNK_SIZE;
        new_arena->prev = arena;
        arena = new_arena;
        *arena_p = arena;
    }
    arena->bytes_left -= size;
    void *addr = arena->chunk + arena->offset;
    arena->offset += size;
    return addr;
}

MemoryArena *shack_arena, *shadow_hash_slot;

/*
 * Shadow Stack
 */
#define SHADOW_HASH_SIZE (1 << 20)
#define SHADOW_HASH_MASK (SHADOW_HASH_SIZE - 1)

shadow_pair *shadow_hash_list[SHADOW_HASH_SIZE];

static inline void shack_init(CPUState *env)
{
    env->shack = malloc(sizeof(ShackSlot) * SHACK_SIZE);
    env->shack_top = env->shack;
    env->shack_end = ((ShackSlot*) env->shack) + SHACK_SIZE;
    memset(shadow_hash_list, 0, sizeof(shadow_pair*) * SHADOW_HASH_SIZE);
    shack_arena = new_arena();
    shadow_hash_slot = new_arena();
}

/*
 * shack_set_shadow()
 *  Insert a guest eip to host eip pair if it is not yet created.
 */
void shack_set_shadow(CPUState *env, target_ulong guest_eip, unsigned long *host_eip)
{
    // TODO (FIXME) Perhaps dropping this step altogether is better
    shadow_pair *it = shadow_hash_list[guest_eip & SHADOW_HASH_MASK];
    while (it != NULL) {
        if (it->guest_eip == guest_eip) {
            *it->shadow_slot = host_eip;
            if (it->prev)
                it->prev->next = it->next;
            if (it->next)
                it->next->prev = it->prev;
            break;
        }
        it = it->next;
    }
}

/*
 * helper_shack_flush()
 *  Reset shadow stack.
 */
void helper_shack_flush(CPUState *env)
{
    // NOT USED
}

static inline TranslationBlock *tb_find_slow(CPUState *env,
                                      target_ulong pc,
                                      target_ulong cs_base,
                                      uint64_t flags)
{
    TranslationBlock *tb, **ptb1;
    unsigned int h;
    tb_page_addr_t phys_pc, phys_page1, phys_page2;
    target_ulong virt_page2;

    tb_invalidated_flag = 0;

    /* find translated block using physical mappings */
    phys_pc = get_page_addr_code(env, pc);
    phys_page1 = phys_pc & TARGET_PAGE_MASK;
    phys_page2 = -1;
    h = tb_phys_hash_func(phys_pc);
    ptb1 = &tb_phys_hash[h];
    for(;;) {
        tb = *ptb1;
        if (!tb)
            goto not_found;
        if (tb->pc == pc &&
            tb->page_addr[0] == phys_page1 &&
            tb->cs_base == cs_base &&
            tb->flags == flags) {
            /* check next page if needed */
            if (tb->page_addr[1] != -1) {
                virt_page2 = (pc & TARGET_PAGE_MASK) +
                    TARGET_PAGE_SIZE;
                phys_page2 = get_page_addr_code(env, virt_page2);
                if (tb->page_addr[1] == phys_page2)
                    goto found;
            } else {
                goto found;
            }
        }
        ptb1 = &tb->phys_hash_next;
    }
 not_found:
    // /* if no translated code available, then translate it now */
    // tb = tb_gen_code(env, pc, cs_base, flags, 0);
    return NULL;
 found:
    /* we add the TB in the virtual pc hash table */
    env->tb_jmp_cache[tb_jmp_cache_hash_func(pc)] = tb;
    return tb;
}

static inline TranslationBlock *tb_find_fast(CPUState *env, target_ulong eip)
{
    TranslationBlock *tb;
    target_ulong cs_base, pc;
    int flags;

    /* we record a subset of the CPU state. It will
       always be the same before a given translated block
       is executed. */
    cpu_get_tb_cpu_state(env, &pc, &cs_base, &flags);
    pc = cs_base + eip;
    tb = env->tb_jmp_cache[tb_jmp_cache_hash_func(pc)];
    if (unlikely(!tb || tb->pc != pc || tb->cs_base != cs_base ||
                 tb->flags != flags)) {
        tb = tb_find_slow(env, pc, cs_base, flags);
    }
    return tb;
}

/*
 * push_shack()
 *  Push next guest eip into shadow stack.
 */
void push_shack(CPUState *env, TCGv_ptr cpu_env, target_ulong next_eip)
{
    // fprintf(stderr, "push_shack: %x\n", env->segs[R_CS].base + next_eip);
    TCGv_ptr shack_top_p = tcg_temp_local_new_ptr();
    TCGv_ptr shack_end_p = tcg_temp_local_new_ptr();
    tcg_gen_ld_ptr(shack_top_p, cpu_env, offsetof(CPUState, shack_top));
    tcg_gen_ld_ptr(shack_end_p, cpu_env, offsetof(CPUState, shack_end));
    int label = gen_new_label();
    tcg_gen_brcond_ptr(TCG_COND_NE, shack_top_p, shack_end_p, label);
    tcg_temp_free_ptr(shack_end_p);
    // shack top == end, so shack full and need to flush
    // shack_top = shack;
    tcg_gen_ld_ptr(shack_top_p, cpu_env, offsetof(CPUState, shack));
    gen_set_label(label);

    TCGv t0 = tcg_const_tl(next_eip);
    tcg_gen_st_tl(t0,
                  shack_top_p,
                  offsetof(ShackSlot, source_addr));
    tcg_temp_free_tl(t0);

    void **target_addr_slot = arena_alloc(&shack_arena, sizeof(void*));
    *target_addr_slot = NULL;
    TranslationBlock *tb = tb_find_fast(env, next_eip);
    if (tb) {
        // target block already translated
        *target_addr_slot = tb->tc_ptr;
    } else {
        // target block not yet translated, insert hash entry
        // FIXME
        // Let's just assume there don't exist entry of next_eip in hash
        // set already
        shadow_pair *new_node = arena_alloc(&shadow_hash_slot, sizeof(shadow_pair));
        new_node->guest_eip = next_eip;
        new_node->shadow_slot = target_addr_slot;
        new_node->prev = NULL;
        new_node->next = shadow_hash_list[next_eip & SHADOW_HASH_MASK];
        if (shadow_hash_list[next_eip & SHADOW_HASH_MASK])
            shadow_hash_list[next_eip & SHADOW_HASH_MASK]->prev = new_node;
        shadow_hash_list[next_eip & SHADOW_HASH_MASK] = new_node;
    }
    TCGv_ptr t1 = tcg_const_ptr(target_addr_slot);
    tcg_gen_st_ptr(t1,
                   shack_top_p,
                   offsetof(ShackSlot, target_addr));
    tcg_temp_free_ptr(t1);

    tcg_gen_addi_ptr(shack_top_p, shack_top_p, sizeof(ShackSlot));
    tcg_gen_st_ptr(shack_top_p, cpu_env, offsetof(CPUState, shack_top));
    tcg_temp_free_ptr(shack_top_p);
}

/*
 * pop_shack()
 *  Pop next host eip from shadow stack.
 */
void pop_shack(TCGv_ptr cpu_env, TCGv _next_eip)
{
    // fprintf(stderr, "pop_shack\n");
    TCGv next_eip = tcg_temp_local_new();
    tcg_gen_mov_tl(next_eip, _next_eip);
    tcg_temp_free(_next_eip);

    TCGv_ptr shack_top_p = tcg_temp_local_new_ptr();
    TCGv_ptr shack_p = tcg_temp_local_new_ptr();
    tcg_gen_ld_ptr(shack_top_p, cpu_env, offsetof(CPUState, shack_top));
    tcg_gen_ld_ptr(shack_p, cpu_env, offsetof(CPUState, shack));
    int end_label = gen_new_label();
    tcg_gen_brcond_ptr(TCG_COND_EQ, shack_top_p, shack_p, end_label);
    // if shack_top == shack, then shack is empty, do nothing
    tcg_temp_free_ptr(shack_p);

    tcg_gen_subi_tl(shack_top_p, shack_top_p, sizeof(ShackSlot));
    tcg_gen_st_ptr(shack_top_p, cpu_env, offsetof(CPUState, shack_top));
    TCGv source_addr = tcg_temp_local_new();
    tcg_gen_ld_tl(source_addr, shack_top_p, offsetof(ShackSlot, source_addr));

    tcg_gen_brcond_ptr(TCG_COND_NE, source_addr, next_eip, end_label);
    // Shack hit
    tcg_temp_free(source_addr);
    tcg_temp_free(next_eip);
    TCGv_ptr target_addr = tcg_temp_local_new_ptr();
    tcg_gen_ld_ptr(target_addr, shack_top_p, offsetof(ShackSlot, target_addr));
    tcg_gen_ld_ptr(target_addr, target_addr, 0);
    tcg_temp_free_ptr(shack_top_p);
    {
    TCGv zero = tcg_const_tl(0);
    tcg_gen_brcond_ptr(TCG_COND_EQ, target_addr, zero, end_label);
    tcg_temp_free(zero);
    }
    *gen_opc_ptr++ = INDEX_op_jmp;
    *gen_opparam_ptr++ = GET_TCGV_PTR(target_addr);
    tcg_temp_free_ptr(target_addr);

    gen_set_label(end_label);
}

#endif // NOSHACK

/*
 * Indirect Branch Target Cache
 */
__thread int update_ibtc;

#ifdef NOIBTC

void *helper_lookup_ibtc(target_ulong guest_eip) {
    return optimization_ret_addr;
}
void update_ibtc_entry(TranslationBlock *tb) {}
static inline void ibtc_init(CPUState *env) {}

#else

ibtc_table *ibtc;
target_ulong update_guest_eip;

/*
 * helper_lookup_ibtc()
 *  Look up IBTC. Return next host eip if cache hit or
 *  back-to-dispatcher stub address if cache miss.
 */
void *helper_lookup_ibtc(target_ulong guest_eip)
{
    struct jmp_pair jmpp = ibtc->htable[guest_eip & IBTC_CACHE_MASK];
    if (jmpp.guest_eip == guest_eip) {
        // fprintf(stderr, "ibtc hit\n");
        return jmpp.target_addr;
    } else {
        // fprintf(stderr, "ibtc miss: %u %u\n", jmpp.guest_eip, guest_eip);
        update_guest_eip = guest_eip;
        update_ibtc = 1;
        return optimization_ret_addr;
    }
}

/*
 * update_ibtc_entry()
 *  Populate eip and tb pair in IBTC entry.
 */
void update_ibtc_entry(TranslationBlock *tb)
{
    update_ibtc = 0;
    ibtc->htable[update_guest_eip & IBTC_CACHE_MASK] = (struct jmp_pair) {
        .guest_eip = update_guest_eip,
        .target_addr = tb->tc_ptr
    };
}

/*
 * ibtc_init()
 *  Create and initialize indirect branch target cache.
 */
static inline void ibtc_init(CPUState *env)
{
    ibtc = (ibtc_table *) malloc(sizeof(ibtc_table));
    memset(ibtc, 0, sizeof(ibtc_table));
    update_ibtc = 0;
}

#endif // NOIBTC

/*
 * init_optimizations()
 *  Initialize optimization subsystem.
 */
int init_optimizations(CPUState *env)
{
#ifndef NOSHACK
    shack_init(env);
#endif
#ifndef NOIBTC
    ibtc_init(env);
#endif

    return 0;
}

/*
 * vim: ts=8 sts=4 sw=4 expandtab
 */
