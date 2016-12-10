#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " [Big5 to BoPoMoPho mapping]\n";
    return -1;
  }

  map<string,set<string>> ReverseMapping;
  vector<string> Chars;

  ifstream fin(argv[1]);
  string Big5, Bopomo;
  while(fin >> Big5 >> Bopomo) {
    string Initial(Bopomo.substr(0, 2));
    Big5 = Big5.substr(0, 2);
    ReverseMapping[Initial].insert(Big5);
    Chars.push_back(Big5);
    for (int i = 2; i < Bopomo.size(); ++i) {
      if (Bopomo[i] == '/') {
        ReverseMapping[Bopomo.substr(i+1, 2)].insert(Big5);
      }
    }
  }
  fin.close();

  for (auto It = ReverseMapping.begin(); It != ReverseMapping.end(); ++It) {
    cout << It->first;
    for (auto Char : It->second) {
      cout << " " << Char;
    }
    cout << endl;
  }

  for (auto Char : Chars) {
    cout << Char << " " << Char << endl;
  }

  return 0;
}
