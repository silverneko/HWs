#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <set>
#include <map>
#include <functional>
#include <tuple>
#include <vector>
#include <iterator>

using namespace std;

typedef unsigned long long ull;

template<class A, class B>
tuple<A,B> addTuple(const tuple<A,B>& _a, const tuple<A,B>& _b)
{
  return make_tuple(get<0>(_a) + get<0>(_b), get<1>(_a) + get<1>(_b));
}

template<class A, class B>
void printTuple(const tuple<A,B>& _a)
{
  cout << get<0>(_a) << ' ' << get<1>(_a);
}

void printSeparator()
{
  cout << "********************\n";
}

void get(vector<map<tuple<int,int,int>, tuple<ull,ull>>>& M, int u, int a, int q, int p, int d)
{
  printSeparator();
  printTuple(M[u][make_tuple(a, q, p * 4 + d)]);
  cout << '\n';
  printSeparator();
}

void clicked(const vector<set<tuple<int,int>>>& C, int u)
{
  printSeparator();
  auto& S = C[u];
  for(auto& t : S){
    cout << get<0>(t) << ' ' << get<1>(t) << '\n';
  }
  printSeparator();
}

void impressed(const vector<set<int>>& userImpressed, vector<map<int, set<tuple<ull,int,int,int,int>>>>& userImpression, int u1, int u2)
{
  printSeparator();
  vector<int> inter;
  set_intersection(userImpressed[u1].begin(), userImpressed[u1].end(), userImpressed[u2].begin(), userImpressed[u2].end(), back_inserter(inter));
  for(auto it : inter){
    cout << it << '\n';
    vector<tuple<ull,int,int,int,int>> mg;
    set_union(userImpression[u1][it].begin(), userImpression[u1][it].end(), userImpression[u2][it].begin(), userImpression[u2][it].end(), back_inserter(mg));
    for(auto& t : mg){
      cout << '\t' 
        << get<0>(t) << ' '
        << get<1>(t) << ' ' 
        << get<2>(t) << ' ' 
        << get<3>(t) << ' ' 
        << get<4>(t) << '\n';
    }
  }
  printSeparator();
}

void profit(const vector<map<int, tuple<ull,ull>>>& A, int a, double theta)
{
  printSeparator();
  for(auto it = A[a].begin(); it != A[a].end(); ++it){
    if( (get<1>(it->second) == 0 ? 0.0 : 1.0 * get<0>(it->second) / get<1>(it->second)) >= theta ){
      cout << it->first << '\n';
    }
  }
  printSeparator();
}

int main(int argc, char *argv[])
{
  ifstream fin(argv[1]);
  ull click, impression, url;
  int aid, adid, depth, position, qid, kid, tid, did, uid;

  vector<map<tuple<int,int,int>, tuple<ull,ull>>> M(24000000);
  vector<set<tuple<int,int>>> C(24000000);
  vector<set<int>> userImpressed(24000000);
  vector<map<int, set<tuple<ull,int,int,int,int>>>> userImpression(24000000);
  vector<map<int, tuple<ull,ull>>> A(24000000);
  while(fin >> click >> impression >> url >> aid >> adid >> depth >> position >> qid >> kid >> tid >> did >> uid){
    M[uid][make_tuple(aid, qid, position * 4 + depth)] = addTuple(M[uid][make_tuple(aid, qid, position * 4 + depth)], make_tuple(click, impression));
    if(click >= 1)
      C[uid].insert(make_tuple(aid, qid));
    userImpressed[uid].insert(aid);
    userImpression[uid][aid].insert(make_tuple(url, adid, kid, tid, did));
    A[aid][uid] = addTuple(A[aid][uid], make_tuple(click, impression));
  }

  while(true){
    string command;
    cin >> command;
    
    if(command == "get"){
      int u, a, q, p, d;
      cin >> u >> a >> q >> p >> d;
      get(M, u, a, q, p, d);
    }else if(command == "clicked"){
      int u;
      cin >> u;
      clicked(C, u);
    }else if(command == "impressed"){
      int u1, u2;
      cin >> u1 >> u2;
      impressed(userImpressed, userImpression, u1, u2);
    }else if(command == "profit"){
      int a;
      double theta;
      cin >> a >> theta;
      profit(A, a, theta);
    
    }else if(command == "quit"){
      break;
    }
  }

  return 0;
}

