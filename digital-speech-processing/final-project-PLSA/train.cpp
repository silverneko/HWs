#include <iostream>
#include <fstream>
#include <sstream>
#include "Utils.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " [EM iterations] [number of classes]\n";
    exit(1);
  }
  const int MaxIterations = atoi(argv[1]);
  const int nz = atoi(argv[2]);

  cout << "Train for " << MaxIterations << " iterations,"
    " classify to " << nz << " classes.\n";

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
      Corpus.push_back(vectorizer.vectorize(doc));
    }
  }

  const int nd = Corpus.size(), nw = vectorizer.size();
  vector<vector<pair<int, int>>> Nw_d(nd);
  // Calculate document term-frequency
  for (int i = 0; i < nd; ++i) {
    vector<int> d = Corpus[i];
    sort(d.begin(), d.end());
    int nc = 0;
    for (int j = 0; j < d.size(); ++j) {
      ++nc;
      if (j == d.size() - 1 || d[j] != d[j+1]) {
        Nw_d[i].push_back(make_pair(d[j], nc));
        nc = 0;
      }
    }
  }
  // Remove terms that has high document-frequency
  int nwremoved = 0;
  {
    vector<int> df(nw);
    for (int i = 0; i < nd; ++i) {
      for (auto it = Nw_d[i].cbegin(), ed = Nw_d[i].cend(); it != ed; ++it) {
        int j = it->first;
        df[j] += 1;
      }
    }
    cout << "Terms removed (document-frequency > 20%):\n";
    bool flag = true;
    for (int i = 0; i < nw; ++i) {
      if (df[i] > nd / 5) {
        df[i] = 0;
        ++nwremoved;
        if (flag) {
          flag = false;
        } else {
          cout << ", ";
        }
        cout << vectorizer.lookup(i);
      }
    }
    cout << endl;
    for (int i = 0; i < nd; ++i) {
      for (auto it = Nw_d[i].begin(), ed = Nw_d[i].end(); it != ed; ++it) {
        int j = it->first;
        if (df[j] == 0) {
          it->second = 0;
        }
      }
    }
  }
  cout << "Corpus has " << nd << " documents, " << nw - nwremoved << " terms.\n";
  cout << "Start training ...\n";

  vector<vector<float>> Pw_z(nz, vector<float>(nw));
  vector<vector<float>> Pz_d(nd, vector<float>(nz));
  vector<vector<float>> Pw_z_(Pw_z), Pz_d_(Pz_d);
  // Randomly assign probabilities
  for (int i = 0; i < nz; ++i) {
    float cdf = 0.f;
    for (int j = 0; j < nw; ++j) {
      Pw_z[i][j] = randfloat(1e-10, 1);  // avoid divide by zero error
      cdf += Pw_z[i][j];
    }
    // Normalize to [0, 1]
    for (int j = 0; j < nw; ++j) {
      Pw_z[i][j] /= cdf;
    }
  }
  for (int i = 0; i < nd; ++i) {
    float cdf = 0.f;
    for (int j = 0; j < nz; ++j) {
      Pz_d[i][j] = randfloat(1e-10, 1);
      cdf += Pz_d[i][j];
    }
    // Normalize to [0, 1]
    for (int j = 0; j < nz; ++j) {
      Pz_d[i][j] /= cdf;
    }
  }

  for (int iter = 0; iter < MaxIterations; ++iter) {
    cout << "\r" << "Iteration " << iter + 1 << " / " << MaxIterations << flush;
    for (int i = 0; i < nz; ++i) {
      for (int j = 0; j < nw; ++j) {
        Pw_z_[i][j] = 0.f;
      }
    }
    for (int i = 0; i < nd; ++i) {
      for (int j = 0; j < nz; ++j) {
        Pz_d_[i][j] = 0.f;
      }
    }

    for (int j = 0; j < nd; ++j) {
      for (auto it = Nw_d[j].cbegin(), ed = Nw_d[j].cend(); it != ed; ++it) {
        int k = it->first, N = it->second;
        if (N == 0) continue;
        // Calculate p(z | d, w)
        float Y[nz], cdf = 0.f;
        for (int z = 0; z < nz; ++z) {
          Y[z] = Pw_z[z][k] * Pz_d[j][z];
          cdf += Y[z];
        }
        for (int z = 0; z < nz; ++z) {
          Y[z] /= cdf;
          Pz_d_[j][z] += N * Y[z];
          Pw_z_[z][k] += N * Y[z];
        }
      }
    }
    for (int i = 0; i < nz; ++i) {
      float cdf = 0.f;
      for (int j = 0; j < nw; ++j) {
        cdf += Pw_z_[i][j];
      }
      for (int j = 0; j < nw; ++j) {
        Pw_z[i][j] = Pw_z_[i][j] / cdf;
      }
    }
    for (int i = 0; i < nd; ++i) {
      float cdf = 0.f;
      for (int j = 0; j < nz; ++j) {
        cdf += Pz_d_[i][j];
      }
      for (int j = 0; j < nz; ++j) {
        Pz_d[i][j] = Pz_d_[i][j] / cdf;
      }
    }
  }

  cout << "\r" << "Complete training, output model.\n";
  // Output model
  {
    ofstream fout("model.plsa");
    fout << vectorizer.size() << "\n";
    for (int i = 0; i < vectorizer.i2w.size(); ++i) {
      fout << vectorizer.i2w[i] << "\n";
    }
    fout << nd << " " << nw << " " << nz << "\n";
    for (int j = 0; j < nd; ++j) {
      for (auto it = Nw_d[j].cbegin(), ed = Nw_d[j].cend(); it != ed; ++it) {
        int k = it->first, N = it->second;
        if (N == 0) continue;
        fout << k << " " << N << " ";
      }
      fout << "\n";
    }
    for (int i = 0; i < nz; ++i) {
      for (int j = 0; j < nw; ++j) {
        fout << serializeFloat(Pw_z[i][j]) << " ";
      }
      fout << "\n";
    }
    for (int i = 0; i < nd; ++i) {
      for (int j = 0; j < nz; ++j) {
        fout << serializeFloat(Pz_d[i][j]) << " ";
      }
      fout << "\n";
    }
  }

  vector<vector<int>> cat(nz);
  for (int i = 0; i < nd; ++i) {
    float p = 0;
    int c = 0;
    for (int j = 0; j < nz; ++j) {
      if (Pz_d[i][j] > p) {
        p = Pz_d[i][j];
        c = j;
      }
    }
    cat[c].push_back(i);
  }
  for (int i = 0; i < nz; ++i) {
    char pathname[50];
    snprintf(pathname, 50, "class%d.dump", i);
    ofstream fout(pathname);
    vector<pair<float, int>> vi(cat[i].size());
    for (int j = 0; j < cat[i].size(); ++j) {
      int d = cat[i][j];
      vi[j] = make_pair(Pz_d[d][i], d);
    }
    sort(vi.begin(), vi.end());
    reverse(vi.begin(), vi.end());
    for (int j = 0; j < vi.size(); ++j) {
      int d = vi[j].second;
      fout << d << " ";
    }
    fout << "\n";
    for (int j = 0; j < vi.size(); ++j) {
      int d = vi[j].second;
      fout << RawCorpus[d].substr(0, 80) << "\n";
    }
    fout << "\n========================================\n";
    for (int j = 0; j < vi.size(); ++j) {
      int d = vi[j].second;
      fout << RawCorpus[d] << "\n";
    }
  }

  cout << "\n";
  cout << "Output top ten terms for each class.\n";
  // Output each class' top ten terms
  for (int i = 0; i < nz; ++i) {
    vector<int> indices = find_top_n(Pw_z[i], 10);
    cout << "Class " << i << ":";
    for (int j = 0; j < 10; ++j) {
      cout << " " << vectorizer.lookup(indices[j]);
    }
    cout << "\n";
  }
  return 0;
}
