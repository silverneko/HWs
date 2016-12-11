#include <iostream>
#include <vector>
#include <map>
#include <tuple>

#include "File.h"
#include "Ngram.h"
#include "Vocab.h"
#include "VocabMap.h"

using namespace std;

LogP getBigram(Ngram &lm, VocabIndex w2, VocabIndex w1) {
  VocabIndex context[] = {w1, Vocab_None};
  return lm.wordProb(w2, context);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    cerr << "Usage: " << argv[0] <<
      " [text file] [vocabulary mapping] [language model]\n";
    return -1;
  }

  File TextFile(argv[1], "r");
  File MapFile(argv[2], "r");
  File LmFile(argv[3], "r");

  Vocab vocab;
  Vocab hiddenVocab;
  VocabMap vocabMap(vocab, hiddenVocab);

  // Keep <unk>
  hiddenVocab.unkIsWord() = true;
  if (vocabMap.read(MapFile) != true) {
    cerr << "Error reading map file.\n";
    return -1;
  }

  const int Order = 2;
  Ngram lm(hiddenVocab, Order);
  lm.read(LmFile, true);

  VocabString sentence[maxWordsPerLine];
  while (char *line = TextFile.getline()) {
    unsigned NumWords = Vocab::parseWords(line, sentence, maxWordsPerLine);
    if (NumWords == maxWordsPerLine) {
      TextFile.position() << "Too many words per sentence\n";
      continue;
    }

    VocabIndex wids[maxWordsPerLine];
    vocab.getIndices(sentence, wids, maxWordsPerLine, vocab.unkIndex());

    vector<map<VocabIndex, tuple<LogP, VocabIndex>>> ViterbiProb(NumWords + 1);
    ViterbiProb[0][Vocab_None] = make_tuple(LogP_One, Vocab_None);
    for (int i = 0; i < NumWords; ++i) {
      if (wids[i] == vocab.unkIndex()) {
        // OOV
        LogP logP = LogP_Zero;
        VocabIndex wid2 = hiddenVocab.unkIndex(), BestFrom = Vocab_None;
        for (auto It : ViterbiProb[i]) {
          VocabIndex wid1 = It.first;
          LogP AccuLogP = get<0>(It.second);
          LogP TranLogP = getBigram(lm, wid2, wid1);
          if (AccuLogP + TranLogP > logP) {
            logP = AccuLogP + TranLogP;
            BestFrom = wid1;
          }
        }
        ViterbiProb[i+1][wid2] = make_tuple(logP, BestFrom);
        continue;
      }
      VocabMapIter Iter(vocabMap, wids[i]);
      VocabIndex wid2;
      Prob prob;
      while (Iter.next(wid2, prob)) {
        LogP logP = LogP_Zero;
        VocabIndex BestFrom = Vocab_None;
        for (auto It : ViterbiProb[i]) {
          VocabIndex wid1 = It.first;
          LogP AccuLogP = get<0>(It.second);
          LogP TranLogP = getBigram(lm, wid2, wid1);
          if (AccuLogP + TranLogP > logP) {
            logP = AccuLogP + TranLogP;
            BestFrom = wid1;
          }
        }
        ViterbiProb[i+1][wid2] = make_tuple(logP, BestFrom);
      }
    }

    VocabIndex hiddenWids[maxWordsPerLine + 1];
    VocabIndex wid, widPrev;
    LogP logP = LogP_Zero;
    for (auto It : ViterbiProb[NumWords]) {
      VocabIndex wid1 = It.first;
      LogP logP1 = get<0>(It.second);
      if (logP1 > logP) {
        logP = logP1;
        wid = wid1;
        widPrev = get<1>(It.second);
      }
    }
    hiddenWids[NumWords] = Vocab_None;
    hiddenWids[NumWords-1] = wid;
    for (int i = NumWords-1; i > 0; --i) {
      hiddenWids[i-1] = widPrev;
      widPrev = get<1>(ViterbiProb[i][widPrev]);
    }

    VocabString hiddenWords[maxWordsPerLine + 1];
    hiddenVocab.getWords(hiddenWids, hiddenWords, maxWordsPerLine + 1);
    for (int i = 0; i < NumWords; ++i) {
      if (hiddenWids[i] == hiddenVocab.unkIndex()) {
        hiddenWords[i] = sentence[i];
      }
    }
    cout << Vocab_SentStart << ' '
         << hiddenWords << ' ' << Vocab_SentEnd << endl;
  }

  return 0;
}
