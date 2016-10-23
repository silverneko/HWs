#include <list>
#include <tuple>
#include <functional>
#include <algorithm>

template <typename T>
class BinomialHeap{
  private:
    class BinomialTree{
      public:
        int _size;
        T _cargo;
        std::list<BinomialTree*> _children;
        BinomialTree(const T &elem) : _size(1), _cargo(elem) {}
    };

    BinomialTree* _merge2(BinomialTree *a, BinomialTree *b)
    {
      if(a == nullptr) return b;
      if(b == nullptr) return a;
      if(b->_cargo > a->_cargo) std::swap(a, b);
      a->_children.push_back(b);
      a->_size += b->_size;
      return a;
    }

    std::tuple<BinomialTree*, BinomialTree*> _merge(BinomialTree *a, BinomialTree *b, BinomialTree *c)
    {
      BinomialTree *ptr[] = {a, b, c};
      std::sort(ptr, ptr + 3);
      if(ptr[1] == nullptr){
        return std::make_tuple(ptr[2], nullptr);
      }else{
        return std::make_tuple(ptr[0], _merge2(ptr[1], ptr[2]));
      }
    }

    int _size;
    BinomialTree *trees[64];
    BinomialTree *ptmax;
  public:
    BinomialHeap() : _size(0), trees {nullptr}, ptmax(nullptr) {}
    BinomialHeap(const T &elem) : _size(1), trees {nullptr}
    {
      ptmax = trees[0] = new BinomialTree(elem);
    }

    void merge(BinomialHeap<T>&);
    
    void insert(const T &elem)
    {
      //merge(BinomialHeap<T>(elem));
      BinomialHeap<T> node(elem);
      merge(node);
    }

    T top() { return ptmax->_cargo;}
    void pop();
    int size() { return _size;}
    bool empty() { return _size == 0;}
};

template <typename T>
void BinomialHeap<T>::merge(BinomialHeap<T> &b)
{
  BinomialTree *carryout = nullptr;
  ptmax = nullptr;
  for(int i = 0; i < 64; ++i){
    std::tie(trees[i], carryout) = _merge(trees[i], b.trees[i], carryout);
    if(ptmax == nullptr){
      ptmax = trees[i];
    }else if(trees[i] != nullptr){
      if(ptmax->_cargo < trees[i]->_cargo)
        ptmax = trees[i];
    }
  }
  _size += b._size;
  //clear b
  for(auto &it : b.trees)
    it = nullptr;
  b.ptmax = nullptr;
  b._size = 0;
}

template <typename T>
void BinomialHeap<T>::pop()
{
  BinomialHeap tmp;
  int i = 0;
  for(auto it = ptmax->_children.begin(); it != ptmax->_children.end(); ++it){
    tmp.trees[i++] = *it;
  }

  for(int i = 0; i < 64; ++i){
    if(trees[i] == ptmax){
      trees[i] = nullptr;
      break;
    }
  }

  delete ptmax;

  merge(tmp);
  --_size;
}
