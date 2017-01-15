#include <iostream>
#include <fstream>
#include <sstream>
#include "Utils.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  Vectorizer vectorizer;
  vector<string> RawCorpus;
  vector<vector<int>> Corpus;
  {
    ifstream fin("corpus");
    string line;
    while (getline(fin, line)) {
      // line = "id : {"
      string doc;
      while (getline(fin, line)) {
        if (line == "}")
          break;
        doc.append(line);
      }
      RawCorpus.push_back(doc);
    }
  }
  int nd, nw, nz;
  vector<vector<pair<int, int>>> Nw_d;
  vector<vector<float>> Pw_z, Pz_d;
  {
    ifstream fin("model.plsa");
    int nn;
    fin >> nn;
    vectorizer.i2w.resize(nn);
    for (int i = 0; i < nn; ++i) {
      string w;
      fin >> w;
      vectorizer.i2w[i] = w;
      vectorizer.w2i[w] = i;
    }
    fin >> nd >> nw >> nz;
    Nw_d.resize(nd);
    string line;
    getline(fin, line); // consume linebreak after nz
    for (int i = 0; i < nd; ++i) {
      getline(fin, line);
      istringstream ssin(line);
      int k, N;
      while(ssin >> k >> N) {
        Nw_d[i].push_back(make_pair(k, N));
      }
    }
    Pw_z.resize(nz);
    for (int i = 0; i < nz; ++i) {
      Pw_z[i].resize(nw);
      for (int j = 0; j < nw; ++j) {
        fin >> line;
        Pw_z[i][j] = deserializeFloat(line);
      }
    }
    Pz_d.resize(nd);
    for (int i = 0; i < nd; ++i) {
      Pz_d[i].resize(nz);
      for (int j = 0; j < nz; ++j) {
        fin >> line;
        Pz_d[i][j] = deserializeFloat(line);
      }
    }
  }
  cout << "Model loaded.\n";

  while (cin.good()) {
    string buffer;
    cin >> buffer;
    switch (buffer[0]) {
    case 'c':
      {
        int i;
        cin >> i;
        if (i < 0 || i >= nz) break;
        vector<int> indices = find_top_n(Pw_z[i], 10);
        for (int j = 0; j < 10; ++j) {
          cout << " " << vectorizer.lookup(indices[j]);
        }
        cout << "\n\n";
      }
      break;
    case 'w':
      {
        cin >> buffer;
        int id = vectorizer.lookup(buffer);
        if (id < 0) break;
        vector<pair<float, int>> vi;
        for (int j = 0; j < nz; ++j) {
          vi.push_back(make_pair(Pw_z[j][id], j));
        }
        sort(vi.begin(), vi.end());
        reverse(vi.begin(), vi.end());
        cout << "Rank by P(w | z)\n";
        for (int j = 0; j < 5; ++j) {
          cout << "Class" << "\t" << vi[j].second << "\t" << vi[j].first << "\n";
          vector<int> indices = find_top_n(Pw_z[vi[j].second], 10);
          for (int j = 0; j < 10; ++j) {
            cout << " " << vectorizer.lookup(indices[j]);
          }
          cout << "\n";
        }
        cout << "\n";
      }
      break;
    case 'q':
      {
        string query;
        getline(cin, query);

        for (int i = 0; i < query.size(); ++i) {
          if (isalpha(query[i])) {
            query[i] = tolower(query[i]);
          } else {
            query[i] = ' ';
          }
        }
        vector<int> v;
        istringstream ssin(query);
        string token;
        while(ssin >> token) {
          if (token.size() > 1) {
            int id = vectorizer.lookup(token);
            if (id != -1) {
              v.push_back(id);
            }
          }
        }
        if (v.empty()) break;
        sort(v.begin(), v.end());
        int nc = 0;
        vector<pair<int, int>> Nw_q;
        for (int j = 0; j < v.size(); ++j) {
          ++nc;
          if (j == v.size() - 1 || v[j] != v[j+1]) {
            Nw_q.push_back(make_pair(v[j], nc));
            nc = 0;
          }
        }
        vector<float> Pz_q(nz), Pz_q_(nz);
        for (int i = 0; i < nz; ++i) {
          Pz_q[i] = 1.f / nz;
        }
        for (int iter = 0; iter < 5; ++iter) {
          for (int j = 0; j < nz; ++j) {
            Pz_q_[j] = 0.f;
          }
          for (auto it = Nw_q.cbegin(), ed = Nw_q.cend(); it != ed; ++it) {
            int k = it->first, N = it->second;
            // Calculate p(z | d=q, w=k)
            float Y[nz], cdf = 0.f;
            for (int z = 0; z < nz; ++z) {
              Y[z] = Pw_z[z][k] * Pz_q[z];
              cdf += Y[z];
            }
            for (int z = 0; z < nz; ++z) {
              Pz_q_[z] += N * Y[z] / cdf;
            }
          }
          float cdf = 0.f;
          for (int j = 0; j < nz; ++j) {
            cdf += Pz_q_[j];
          }
          for (int j = 0; j < nz; ++j) {
            Pz_q[j] = Pz_q_[j] / cdf;
          }
        }

        vector<pair<float, int>> vi;
        for (int j = 0; j < nz; ++j) {
          vi.push_back(make_pair(Pz_q[j], j));
        }
        sort(vi.begin(), vi.end());
        reverse(vi.begin(), vi.end());
        cout << "Rank by P(z | q)\n";
        for (int j = 0; j < 5; ++j) {
          cout << "Class" << "\t" << vi[j].second << "\t" << vi[j].first << "\n";
          vector<int> indices = find_top_n(Pw_z[vi[j].second], 10);
          for (int j = 0; j < 10; ++j) {
            cout << " " << vectorizer.lookup(indices[j]);
          }
          cout << "\n";
        }
        cout << "\n";
      }
      break;
    }
  }

#if 0
  int id[10];
  id[0] = vectorizer.lookup("strike");
  id[1] = vectorizer.lookup("war");
  id[2] = vectorizer.lookup("taiwan");
  id[3] = vectorizer.lookup("campaign");
  id[4] = vectorizer.lookup("labour");
  id[5] = vectorizer.lookup("reserve");
  id[6] = vectorizer.lookup("oil");
  id[7] = vectorizer.lookup("export");
  id[8] = vectorizer.lookup("gdp");
  id[9] = vectorizer.lookup("deposit");
  for (int i = 0; i < 10; ++i) {
    vector<pair<float, int>> vi;
    for (int j = 0; j < nz; ++j) {
      vi.push_back(make_pair(Pw_z[j][id[i]], j));
    }
    sort(vi.begin(), vi.end());
    reverse(vi.begin(), vi.end());
    cout << vectorizer.lookup(id[i]) << endl;
    for (int j = 0; j < 6; ++j) {
      cout << "Class " << vi[j].second << " " << vi[j].first << endl;
    }
    cout << endl;
  }
#endif

  return 0;
}
