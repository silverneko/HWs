#include "MyDCE.h"
#include <queue>
#include <map>

/*
 * We initialize pass ID here. LLVM uses ID’s address to identify a pass,
 * so initialization value is not important.
 */
char MyDCE::ID(0);
static RegisterPass<MyDCE> X("myMagicDCE", "My DCE Pass", false, false);

using std::queue;
using std::map;

bool MyDCE::runOnFunction(Function &F) {
  bool changed = false;
  int ec = 0;
  queue<Instruction*> Q;
  map<Instruction*, bool> mark;
  for (auto b = F.begin(); b != F.end(); ++b) {
    BasicBlock &BB = *b;
    for (auto i = BB.begin(); i != BB.end(); ++i) {
      if ((i->isTerminator() || i->mayHaveSideEffects()) && !mark[i]) {
        mark[i] = true;
        Q.push(i);
      }
    }
  }
  while (!Q.empty()) {
    Instruction &Inst = *Q.front();
    Q.pop();
    for (auto op = Inst.op_begin(); op != Inst.op_end(); ++op) {
      if (isa<Instruction>(op->get())) {
        Instruction &Inst = static_cast<Instruction&>(*op->get());
        if (!mark[&Inst]) {
          mark[&Inst] = true;
          Q.push(&Inst);
        }
      }
    }
  }
  for (auto b = F.begin(); b != F.end(); ++b) {
    for (auto i = b->begin(); i != b->end(); ) {
      Instruction &Inst = *(i++);
      if (!mark[&Inst]) {
        Inst.eraseFromParent();
        ++ec;
        changed = true;
      }
    }
  }
  errs() << "DCE count: " << ec << "\n";
  return changed;
}

