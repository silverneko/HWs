#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "hmm.h"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr,
            "Usage: %s [iterations] [init model] [train data] [output model]\n",
            argv[0]);
    exit(1);
  }

  int IterationsLeft = atoi(argv[1]);
  HMM Model;
  loadHMM(&Model, argv[2]);

  string Buffer;
  vector<vector<int>> Data;
  ifstream fin(argv[3]);
  while (fin >> Buffer) {
    vector<int> Seq;
    Seq.reserve(Buffer.size());
    for (int i = 0; i < Buffer.size(); ++i) {
      Seq.push_back(Buffer[i] - 'A');
    }
    Data.push_back(Seq);
  }
  fin.close();

  while (IterationsLeft--) {
    const int StateNum = Model.state_num;
    const int ObservationTypes = Model.observ_num;
    double AccuInit[MAX_STATE] = {0.0};
    double AccuGamma[MAX_STATE] = {0.0}; // P[state = i]
    double AccuEpsilon[MAX_STATE][MAX_STATE] = {0.0}; // P[q_t = i, q_t+1 = j]
    double AccuObservation[MAX_OBSERV][MAX_STATE] = {0.0};
    for (auto &Sequence : Data) {
      const int ObservationNum = Sequence.size();

      // Calculate forward variable (P[o'[0:t], q_t = i])
      double Alpha[MAX_SEQ][MAX_STATE] = {0.0};
      for (int i = 0; i < StateNum; ++i) {
        Alpha[0][i] = Model.initial[i] * Model.observation[Sequence[0]][i];
      }
      for (int t = 1; t < ObservationNum; ++t) {
        for (int j = 0; j < StateNum; ++j) {
          for (int i = 0; i < StateNum; ++i) {
            Alpha[t][j] += Alpha[t-1][i] * Model.transition[i][j];
          }
          Alpha[t][j] *= Model.observation[Sequence[t]][j];
        }
      }

      // Calculate PObserve (P[o'])
      double PObserve = 0.0;
      for (int i = 0; i < StateNum; ++i) {
        PObserve += Alpha[ObservationNum-1][i];
      }

      // Calculate backward variable (P[o'[t+1:T]| q_t = i])
      double Beta[MAX_SEQ][MAX_STATE] = {0.0};
      for (int i = 0; i < StateNum; ++i) {
        Beta[ObservationNum-1][i] = 1.0;
      }
      for (int t = ObservationNum - 2; t >= 0; --t) {
        for (int i = 0; i < StateNum; ++i) {
          for (int j = 0; j < StateNum; ++j) {
            Beta[t][i] += Model.transition[i][j] * Model.observation[Sequence[t+1]][j] * Beta[t+1][j];
          }
        }
      }

      // Calculate gamma (P[q_t = i | o'])
      double Gamma[MAX_SEQ][MAX_STATE] = {0.0};
      for (int t = 0; t < ObservationNum; ++t) {
        for (int i = 0; i < StateNum; ++i) {
          Gamma[t][i] = Alpha[t][i] * Beta[t][i] / PObserve;
        }
      }

      // Calculate epsilon (P[q_t = i, q_t+1 = j | o'])
      double Epsilon[MAX_SEQ][MAX_STATE][MAX_STATE] = {0.0};
      for (int t = 0; t < ObservationNum - 1; ++t) {
        for (int i = 0; i < StateNum; ++i) {
          for (int j = 0; j < StateNum; ++j) {
            Epsilon[t][i][j] = Alpha[t][i] * Model.transition[i][j] * Model.observation[Sequence[t+1]][j] * Beta[t+1][j] / PObserve;
          }
        }
      }

      // Accumulate results
      for (int i = 0; i < StateNum; ++i) {
        AccuInit[i] += Gamma[0][i];
      }
      for (int t = 0; t < ObservationNum; ++t) {
        for (int i = 0; i < StateNum; ++i) {
          AccuGamma[i] += Gamma[t][i];
        }
      }
      for (int t = 0; t < ObservationNum - 1; ++t) {
        for (int i = 0; i < StateNum; ++i) {
          for (int j = 0; j < StateNum; ++j) {
            AccuEpsilon[i][j] += Epsilon[t][i][j];
          }
        }
      }
      for (int t = 0; t < ObservationNum; ++t) {
        for (int i = 0; i < StateNum; ++i) {
          AccuObservation[Sequence[t]][i] += Gamma[t][i];
        }
      }
    }

    // Adjust model to fit test data
    for (int i = 0; i < StateNum; ++i) {
      Model.initial[i] = AccuInit[i] / Data.size();
    }
    for (int i = 0; i < StateNum; ++i) {
      for (int j = 0; j < StateNum; ++j) {
        Model.transition[i][j] = AccuEpsilon[i][j] / AccuGamma[i];
      }
    }
    for (int i = 0; i < ObservationTypes; ++i) {
      for (int j = 0; j < StateNum; ++j) {
        Model.observation[i][j] = AccuObservation[i][j] / AccuGamma[j];
      }
    }
  }

  FILE *OutputFile = fopen(argv[4], "w");
  dumpHMM(OutputFile, &Model);
  dumpHMM(stdout, &Model);
  fclose(OutputFile);
  return 0;
}
