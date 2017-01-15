#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
  string pathname_base("data/reut2-");
  vector<string> corpus;
  for (int i = 0; i <= 21; ++i) {
    char buff[20];
    snprintf(buff, 20, "%.3d.sgm", i);
    string pathname = pathname_base + buff;
    ifstream fin(pathname);
    string line;
    while (getline(fin, line)) {
      if (line.find("<TEXT>") != -1) {
        string doc;
        while (getline(fin, line)) {
          if (line.find("</TEXT>") != -1)
            break;
          const string drop_words[] = {
            "<TITLE>",
            "</TITLE>",
            "<DATELINE>",
            "</DATELINE>",
            "<AUTHOR>",
            "</AUTHOR>",
            "<BODY>",
            "</BODY>"
          };
          for (const string &dw : drop_words) {
            int pos = line.find(dw);
            if (pos != -1) {
              line.replace(pos, dw.size(), " ");
            }
          }

          doc.append(line);
          doc.append(" ");
        }
        if (!doc.empty())
          corpus.push_back(doc);
      }
    }
  }

  for (int i = 0; i < corpus.size(); ++i) {
    const string &doc = corpus[i];
    cout << i << " : {\n";
    cout << doc << "\n";
    cout << "}\n";
  }
  return 0;
}
