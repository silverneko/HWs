#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <functional>
#include <string>
#include <vector>
#include <tuple>
#include <ctime>
#include "decision_tree.h"

using namespace std;

int main(int argc, char *argv[])
{
  srand(time(NULL));

  if(argc < 3) return -1;
  ifstream fin(argv[1]);
  double eps(cast2<double>(argv[2]));

  string str;
  vector<datum> data; // vector of data
  int nfeat = 0;      // # of features
  while(getline(fin, str)){
    for(char& c : str){
      if(c == ':')
        c = ' ';
    }
    istringstream sstr(str);
    datum dat;
    sstr >> dat.play;
    int i;
    double f;
    while(sstr >> i >> f){
      dat[i] = f;
      nfeat = max(nfeat, i);
    }
    data.push_back(dat);
  }
  for(auto& d : data){
    d.feature.resize(nfeat+1, 0.0);
  }

  tree *root = buildTree(data.begin(), data.end(), nfeat, eps);

  prettyPrintTree(root);
  
  return 0;
}

