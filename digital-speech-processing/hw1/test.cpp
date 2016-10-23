#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "hmm.h"

using namespace std;

double viterbi(const HMM &, const vector<int> &);

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr,
            "Usage: %s [models list] [test data] [output result]\n",
            argv[0]);
    exit(1);
  }

  HMM Models[5];
  int ModelNum = load_models(argv[1], Models, 5);

  string Buffer;
  ifstream fin(argv[2]);
  ofstream fout(argv[3]);
  while (fin >> Buffer) {
    vector<int> Sequence;
    Sequence.reserve(Buffer.size());
    for (int i = 0; i < Buffer.size(); ++i) {
      Sequence.push_back(Buffer[i] - 'A');
    }

    double BestProb = viterbi(Models[0], Sequence);
    int BestModel = 0;
    for (int i = 1; i < ModelNum; ++i) {
      double Prob = viterbi(Models[i], Sequence);
      if (BestProb < Prob) {
        BestProb = Prob;
        BestModel = i;
      }
    }
    fout << Models[BestModel].model_name << ' ' << BestProb << '\n';
  }
  fin.close();
  fout.close();
  return 0;
}

double viterbi(const HMM &Model, const vector<int> &Sequence) {
  double Delta[MAX_SEQ][MAX_STATE] = {0.0};
  const int StateNum = Model.state_num;
  const int ObservationNum = Sequence.size();
  for (int i = 0; i < StateNum; ++i) {
    Delta[0][i] = Model.initial[i] * Model.observation[Sequence[0]][i];
  }
  for (int t = 1; t < ObservationNum; ++t) {
    for (int j = 0; j < StateNum; ++j) {
      double MaxProb = 0.0;
      for (int i = 0; i < StateNum; ++i) {
        MaxProb = max(MaxProb, Delta[t-1][i] * Model.transition[i][j]);
      }
      Delta[t][j] = MaxProb * Model.observation[Sequence[t]][j];
    }
  }
  double MaxProb = 0.0;
  for (int i = 0; i < StateNum; ++i) {
    MaxProb = max(MaxProb, Delta[ObservationNum-1][i]);
  }
  return MaxProb;
}
