#include <vector>
#include <tuple>
#include <functional>
#include <algorithm>
#include <iostream>
#include "decision_tree.h"

using namespace std;

void swap(datum& a, datum& b)
{
  swap(a.play, b.play);
  swap(a.feature, b.feature);
}

double confusion(int a, int b)
{
  if(a*b == 0) return 0.0;
  return 2.0 * a * b / ((a + b)*(a + b));
}

double totalConfusion(int c, int d, int e, int f)
{
  if((c+d+e+f) == 0) return 0.0;
  return ((c + d) * confusion(c, d) + (e + f) * confusion(e, f)) / (c + d + e + f);
}

tuple<double, double> optimalThreshold(datait lb, datait rb, int nfeat)
{
  sort(lb, rb, [nfeat](const datum& a, const datum& b)
      { return a.feature[nfeat] < b.feature[nfeat];});

  vector<double> v(1, lb->feature[nfeat] - 1);
  for(auto it = lb; it+1 != rb; ++it){
    double a(it->feature[nfeat]), b((it+1)->feature[nfeat]);
    if(a != b)
      v.push_back((a + b) / 2.0 );
  }
  v.push_back((rb-1)->feature[nfeat] + 1);
  auto len = unique(v.begin(), v.end());
  v.resize(len - v.begin());

  int n = v.size() - 1;
  vector<int> psa(n+3, 0), psb(n+3, 0);
  int j = 1;
  for(int i = 0; i < rb - lb; ++i){
    while((lb+i)->feature[nfeat] > v[j]){
      psa[j+1] = psa[j];
      psb[j+1] = psb[j];
      ++j;
    }
    if((lb+i)->play == 1)
      ++psa[j];
    else
      ++psb[j];
  }
  while(j < n){
    psa[j+1] = psa[j];
    psb[j+1] = psb[j];
    ++j;
  }

  int ans = 0;
  double minConfusion = 1e20;
  for(int i = 0; i <= n; ++i){
    double conf = totalConfusion(psa[i], psb[i], psa[n] - psa[i], psb[n] - psb[i]);
    if(conf < minConfusion){
      minConfusion = conf;
      ans = i;
    }
  }

  return make_tuple(v[ans], minConfusion);
}

tree* buildTree(datait lb, datait rb, int nfeat, double eps)
{
  tree *root = new tree;
  root->lchild = NULL;
  root->rchild = NULL;
  if(rb - lb <= 0){
    root->decision = (rand() & 1 ? 1 : -1);
    return root;
  }

  int a = 0, b = 0;
  for(auto it = lb; it != rb; ++it){
    if(it->play == 1)
      ++a;
    else
      ++b;
  }
  if(rb - lb <= 1 || confusion(a, b) <= eps){
    root->decision = (a > b ? 1 : -1);
    return root;
  }

  double minConf = 1e20;
  int branch = 0;
  for(int i = 1; i <= nfeat; ++i){
    double _, conf;
    tie(_, conf) = optimalThreshold(lb, rb, i);
    if(conf < minConf){
      minConf = conf;
      branch = i;
    }
  }
  
  double thr, _;
  tie(thr, _) = optimalThreshold(lb, rb, branch);
  datait it = lb;
  while(it < rb && it->feature[branch] < thr)
    ++it;

  if(it-lb == 0 || rb-it == 0){
    root->decision = (a > b ? 1 : -1);
    return root;
  }

  root->feat = branch;
  root->thres = thr;
  root->lchild = buildTree(lb, it, nfeat, eps);
  root->rchild = buildTree(it, rb, nfeat, eps);
  return root;
}

void prettyPrintTree(const tree *root, int indent)
{
  if(root->decision == 0){
    cout << string(indent*2, ' ') << "if(attr[" << root->feat << "] < " << root->thres << "){" << endl;
    prettyPrintTree(root->lchild, indent + 1);
    cout << string(indent*2, ' ') << "}else{" << endl;
    prettyPrintTree(root->rchild, indent + 1);
    cout << string(indent*2, ' ') << "}" << endl;
  }else{
    cout << string(indent*2, ' ') << "return " << (root->decision == 1 ? "+1" : "-1") << ";" << endl;
  }
}

void prettyPrintTree(const tree *root)
{
  cout << "int tree_predict(double *attr)" << endl;
  cout << "{" << endl;
  prettyPrintTree(root, 1);
  cout << "}" << endl;
}

