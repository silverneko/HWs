#ifndef __DECISION_TREE_H__
#define __DECISION_TREE_H__

#include <vector>
#include <tuple>
#include <string>
#include <sstream>

class datum{
  public:
    int play;
    std::vector<double> feature;
    datum() : play(0), feature() {}
    double& operator [] (int i)
    {
      if(i >= feature.size()){
        feature.resize(i+1, 0.0);
      }
      return feature[i];
    }
};

class tree{
  public:
    tree *lchild, *rchild;
    int feat;
    double thres;
    int decision;
    tree() : lchild(NULL), rchild(NULL), feat(0), thres(0.0), decision(0) {}
    tree(tree *lc, tree *rc, int fe, double thr) : lchild(lc)
                                                   , rchild(rc)
                                                   , feat(fe)
                                                   , thres(thr) {}
};

void swap(datum&, datum&);

double confusion(int, int);

double totalConfusion(int, int, int, int);

typedef std::vector<datum>::iterator datait;

std::tuple<double, double> optimalThreshold(datait, datait, int);

tree* buildTree(datait, datait, int, double);

template<typename T>
inline T cast2(const std::string& str)
{
  std::istringstream sstr(str);
  T res;
  sstr >> res;
  return res;
}

void prettyPrintTree(const tree*, int);

void prettyPrintTree(const tree*);

#endif
