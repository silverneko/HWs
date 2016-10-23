#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <string>

using namespace std;

class Treap{
public:
  int pri;
  long long key;
  long long keysum;
  int size;
  Treap *lchild, *rchild;
  Treap() : pri(rand()), key(0), keysum(0), size(0), lchild(0), rchild(0) {}
  Treap(long long _key) : pri(rand()), key(_key), keysum(_key), size(1), lchild(0), rchild(0) {}
};

template <class T>
class MyAllocator{
public:
  T *head;
  int quota;
  long long prevQuota;
  MyAllocator() : head(new T), quota(1), prevQuota(1) {}
  MyAllocator(int quota_) : head(new T [quota_]), quota(quota_), prevQuota(quota_) {}
  T* operator () ()
  {
    if(quota <= 0){
      prevQuota *= 2;
      quota = prevQuota;
      head = new T [quota];
    }
    --quota;
    return head++;
  }
  T* operator () (const T &initializer)
  {
    T *holder = (*this)();
    *holder = initializer;
    return holder;
  }
};

MyAllocator<Treap> memAllocator;

int key(Treap *t)
{
  if(t == nullptr) return 0;
  return t->key;
}

long long keysum(Treap *t)
{
  if(t == nullptr) return 0;
  return t->keysum;
}

int size(Treap *t)
{
  if(t == nullptr) return 0;
  return t->size;
}

void push(Treap *t)
{
  return ;
}

void pull(Treap *t)
{
  if(t == nullptr) return ;
  t->keysum = t->key;
  t->size = 1;
  t->keysum += keysum(t->lchild);
  t->size += size(t->lchild);
  t->keysum += keysum(t->rchild);
  t->size += size(t->rchild);
  return ;
}

Treap* merge(Treap *ta, Treap *tb)
{
  if(ta == nullptr) return tb;
  if(tb == nullptr) return ta;
  if(ta->pri < tb->pri){
    push(ta);
    //ta = memAllocator(*ta);
    ta->rchild = merge(ta->rchild, tb);
    pull(ta);
    return ta;
  }else{
    push(tb);
    //tb = memAllocator(*tb);
    tb->lchild = merge(ta, tb->lchild);
    pull(tb);
    return tb;
  }
}

tuple<Treap*, Treap*> splitK(Treap *t, int k)
{
  if(t == nullptr) return make_tuple(nullptr, nullptr);
  push(t);
  Treap *lt, *rt;
  if(key(t) < k){
    //lt = memAllocator(*t);
    lt = t;
    tie(lt->rchild, rt) = splitK(t->rchild, k);
    pull(lt);
  }else{
    //rt = memAllocator(*t);
    rt = t;
    tie(lt, rt->lchild) = splitK(t->lchild, k);
    pull(rt);
  }
  return make_tuple(lt, rt);
}

Treap* insert(Treap *t, int x)
{
  Treap *lt, *rt;
  tie(lt, rt) = splitK(t, x);
  Treap *mt = memAllocator(Treap(x));
  return merge(merge(lt, mt), rt);
}

Treap* heuristicMerge(Treap *ta, Treap *tb)
{
  if(ta == nullptr) return tb;
  if(tb == nullptr) return ta;
  if(size(ta) < size(tb)) swap(ta, tb);
  ta = insert(ta, tb->key);
  ta = heuristicMerge(ta, tb->lchild);
  ta = heuristicMerge(ta, tb->rchild);
  return ta;
}

//split by keysum
tuple<Treap*, Treap*> split(Treap *t, long long k)
{
  if(t == nullptr) return make_tuple(nullptr, nullptr);
  push(t);
  Treap *lt, *rt;
  if(keysum(t->lchild) + t->key <= k){
    lt = memAllocator(*t);
    tie(lt->rchild, rt) = split(t->rchild, k - keysum(t->lchild) - t->key);
    pull(lt);
  }else{
    rt = memAllocator(*t);
    tie(lt, rt->lchild) = split(t->lchild, k);
    pull(rt);
  }
  return make_tuple(lt, rt);
}

int ksum(Treap *t, long long k)
{
  if(t == nullptr) return 0;
  push(t);
  if(keysum(t->lchild) + t->key <= k){
    return size(t->lchild) + 1 + ksum(t->rchild, k - keysum(t->lchild) - t->key);
  }else{
    return ksum(t->lchild, k);
  }
}

int owner[100001];
int parent[100001];

void init()
{
  for(int i = 0; i < 100001; ++i){
    parent[i] = i;
    owner[i] = i;
  }
}

int find(int x)
{
  return parent[x] == x ? x : parent[x] = find(parent[x]);
}

void unite(int x, int y)
{
  x = find(x);
  y = find(y);
  parent[x] = y;
}

int main(int argc, char *argv[])
{
  srand(1257376);
  init();

  int n, m;
  cin >> n >> m;
  int price[100001] = {0};
  for(int i = 1; i <= n; ++i)
    cin >> price[i];
  
  Treap *treap[100001] = {nullptr};
  for(int i = 1; i <= n; ++i){
    treap[i] = memAllocator(Treap(price[i]));
  }

  while(m--){
    int q;
    cin >> q;
    if(q == 1){
      int i, j;
      cin >> i >> j;
      int u1 = owner[find(i)], u2 = owner[find(j)];
      unite(i, j);
      owner[find(i)] = u1;
      if(u1 != u2){
        treap[u1] = heuristicMerge(treap[u1], treap[u2]);
        treap[u2] = nullptr;
      }
    }else{
      int i;
      long long s;
      cin >> i >> s;
      int u = owner[find(i)];
      /*
      Treap *lt, *rt;
      tie(lt, rt) = split(treap[u], s);
      int k = size(lt);
      treap[u] = merge(lt, rt);
      */
      //int k = size( get<0>(split(treap[u], s)) );
      int k = ksum(treap[u], s);
      cout << u << " " << k << endl;
    }
  }

  return 0;
}

