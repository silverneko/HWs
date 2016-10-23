#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>
#include "decision_tree.h"

using namespace std;

void prettyPrintForest(const vector<tree*>& forest)
{
  for(int i = 0; i < forest.size(); ++i){
    cout << "int tree_predict_" << i << "(const double *attr)" << endl;
    cout << "{" << endl;
    prettyPrintTree(forest[i], 1);
    cout << "}" << endl;
    cout << endl;
  }
  cout << endl;
  cout << "int forest_predict(double *attr)" << endl;
  cout << "{" << endl;
  cout << "  int ticketbox = 0;" << endl;
  for(int i = 0; i < forest.size(); ++i)
    cout << "  ticketbox += tree_predict_" << i << "(attr);" << endl;
  cout << "  return (ticketbox >= 0 ? +1 : -1);" << endl;
  cout << "}" << endl;
}

int main(int argc, char *argv[])
{
  srand(time(NULL));

  if(argc < 3) return 0;
  ifstream fin(argv[1]);
  int ntree(cast2<int>(argv[2]));
  double eps(0.0);
 
  int nfeat = 0;
  vector<datum> data;
  string str;
  while(getline(fin, str)){
    for(char& v : str){
      if(v == ':')
        v = ' ';
    }
    
    istringstream sstr(str);
    datum dat;
    sstr >> dat.play;
    int feat;
    double value;
    while(sstr >> feat >> value){
      dat[feat] = value;
      nfeat = max(nfeat, feat);
    }

    data.push_back(dat);
  }
  for(auto& dat : data){
    dat.feature.resize(nfeat+1, 0.0);
  }
  
  vector<tree*> forest(ntree);

  for(auto& root : forest){
    random_shuffle(data.begin(), data.end());
    root = buildTree(data.begin(), data.begin() + data.size() / 3, nfeat, eps);
  }
  
  prettyPrintForest(forest);

  return 0;
}

