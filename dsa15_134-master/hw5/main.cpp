#include <iostream>
#include <algorithm>
#include <cstdlib>
#include "binomial_heap.h"
#include <string>
#include <vector>
#include <tuple>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
  int ncomputer, workload;
  cin >> ncomputer >> workload;
  vector<BinomialHeap<tuple<int,int>>> computer(ncomputer);
  
  string str;
  while(cin >> str){
    if(str == "assign"){
      int i, id, p;
      cin >> i >> id >> p;
      computer[i].insert(make_tuple(p, -id));
      cout << "There are " << computer[i].size() << " tasks on computer " << i << "." << endl;
    }else if(str == "execute"){
      int i;
      cin >> i;
      if(computer[i].empty()) continue;
      int p = get<0>(computer[i].top());
      while((!computer[i].empty()) && get<0>(computer[i].top()) == p){
        cout << "Computer " << i << " executed task " << -get<1>(computer[i].top()) << "." << endl;
        computer[i].pop();
      }
    }else if(str == "merge"){
      int i, j;
      cin >> i >> j;
      if(computer[j].size() >= workload){
        computer[i].merge(computer[j]);
        cout << "The largest priority number is now " << (!computer[i].empty() ? get<0>(computer[i].top()) : 0)
             << " on " << i << "." << endl;
      }else{
        cout << "Merging request failed." << endl;
      }
    }
  }

  return 0;
}

