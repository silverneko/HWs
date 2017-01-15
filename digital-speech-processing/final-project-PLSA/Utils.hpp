#pragma once
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <random>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <sstream>
#include <iomanip>

namespace {
using namespace std;

int randint(int lb, int ub) {
  static std::mt19937 RNG(0x5EED);
  return std::uniform_int_distribution<int>(lb, ub)(RNG);
}

float randfloat(float lb, float ub) {
  static std::mt19937 RNG(0x5EED);
  return std::uniform_real_distribution<float>(lb, ub)(RNG);
}

string serializeFloat(float f) {
  stringstream stream;
  stream << setprecision(17) << f;
  return stream.str();
}

float deserializeFloat(string s) {
  stringstream stream(s);
  float f;
  stream >> f;
  return f;
}

static const char *StopWords[] = {"i", "me", "my", "myself", "we", "our", "ours", "ourselves", "you", "your", "yours", "yourself", "yourselves", "he", "him", "his", "himself", "she", "her", "hers", "herself", "it", "its", "itself", "they", "them", "their", "theirs", "themselves", "what", "which", "who", "whom", "this", "that", "these", "those", "am", "is", "are", "was", "were", "be", "been", "being", "have", "has", "had", "having", "do", "does", "did", "doing", "a", "an", "the", "and", "but", "if", "or", "because", "as", "until", "while", "of", "at", "by", "for", "with", "about", "against", "between", "into", "through", "during", "before", "after", "above", "below", "to", "from", "up", "down", "in", "out", "on", "off", "over", "under", "again", "further", "then", "once", "here", "there", "when", "where", "why", "how", "all", "any", "both", "each", "few", "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same", "so", "than", "too", "very", "s", "t", "can", "will", "just", "don", "should", "now", "d", "ll", "m", "o", "re", "ve", "y", "ain", "aren", "couldn", "didn", "doesn", "hadn", "hasn", "haven", "isn", "ma", "mightn", "mustn", "needn", "shan", "shouldn", "wasn", "weren", "won", "wouldn", "could", "would", "must"};

class Vectorizer {
public:
  unordered_map<string, int> w2i;
  vector<string> i2w;

  Vectorizer() {}
  unsigned int size() { return i2w.size();}
  int lookup(const string& s) {
    auto it = w2i.find(s);
    if (it != w2i.end())
      return it->second;
    return -1;
  }
  string lookup(int i) {
    if (i < i2w.size())
      return i2w[i];
    return "";
  }

  vector<int> vectorize(string s) {
    for (int i = 0; i < s.size(); ++i) {
      if (isalpha(s[i])) {
        s[i] = tolower(s[i]);
      } else {
        s[i] = ' ';
      }
    }
    vector<int> v;
    istringstream ssin(s);
    string token;
    while(ssin >> token) {
      if (token.size() > 1) {
        bool flag = true;
        for (const char * sw : StopWords) {
          if (token == sw) {
            flag = false;
            break;
          }
        }
        if (!flag) continue;
        if (lookup(token) == -1) {
          w2i.insert({token, size()});
          i2w.push_back(token);
        }
        v.push_back(w2i[token]);
      }
    }
    return v;
  }
};

vector<int> find_top_n(vector<float> v, int n) {
  vector<pair<float, int>> vi(v.size());
  for (int i = 0; i < v.size(); ++i)
    vi[i] = make_pair(v[i], i);
  sort(vi.begin(), vi.end(), greater<pair<float, int>>());
  vector<int> res(n);
  for (int i = 0; i < n; ++i)
    res[i] = vi[i].second;
  return res;
}

} // end of annonymous namespace
#endif
