#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <stack>
#include <deque>
#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include <map>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <cmath>

using namespace std;

typedef string simple_token;

map<string, const int> precedence = {
  {"sin", 1}, {"cos", 1}, {"exp", 1}, {"log", 1},
  {"pow", 1}, {"sqrt", 1}, {"fabs", 1},
  {"unary+", 2}, {"unary-", 2}, {"~", 2}, {"!", 2},
  {"*", 3}, {"/", 3}, {"%", 3},
  {"+", 4}, {"-", 4},
  {"<<", 5}, {">>", 5},
  {"&", 8}, {"^", 9}, {"|", 10},
  {"&&", 11}, {"||", 12},
  {"(", 20}
};

const string operators[] = {
  "sqrt", "fabs",
  "sin", "cos", "exp", "log", "pow", 
  "<<", ">>", 
  "&&", "||",
  "&", "|", 
  "(", ")", "~", "!", "*",
  "/", "%", "^", "+", "-", "," 
};

template<class T>
T fromString(const string& str)
{
  istringstream sstr(str);
  T temp;
  sstr >> temp;
  return temp;
}

inline bool isNumeric(char c)
{
  return (isdigit(c) || c == '.');
}

vector<simple_token> parseExpression(const string& rawInput)
{
  cout << "[Input Parsing] parse input into tokens" << endl;
  vector<simple_token> tokens;
  tokens.push_back("dummy");
  for(int i = 0; i < rawInput.size(); ++i){
    if(isNumeric(rawInput[i])){
      simple_token holder;
      while(i < rawInput.size() && isNumeric(rawInput[i])){
        holder += rawInput[i];
        ++i;
      }
      --i;
      tokens.push_back(holder);
    }else{
      for(auto& str : operators){
        if(i + str.size() <= rawInput.size() && rawInput.compare(i, str.size(), str) == 0){
          tokens.push_back(str);
          i += str.size()-1;
          break;
        }
      }
    }
  }
  for(int i = 1; i < tokens.size(); ++i){
    if(tokens[i] == "-" || tokens[i] == "+"){
      if(tokens[i-1] != ")" && !isNumeric(tokens[i-1][0])){
        tokens[i] = "unary" + tokens[i];
      }
    }
  }
  cout << "[Input Parsing] done" << endl;
  return tokens;
}

vector<simple_token> infix2postfix(const vector<simple_token>& tokens)
{
  cout << "[Infix to Postfix] start" << endl;
  deque<simple_token> S;
  vector<simple_token> express;
  for(int i = 1; i < tokens.size(); ++i){
    cout << "[Token Processing] encounter a `" << tokens[i] << "`" << endl;
    if(isNumeric(tokens[i][0])){
      cout << "\t[Push to Output] `" << tokens[i] << "`" << endl;
      express.push_back(tokens[i]);
    }else if(tokens[i] == "("){
      cout << "\t[Push to Stack] `(`" << endl;
      S.push_back(tokens[i]);
    }else if(tokens[i] == ","){
      cout << "\t[Flush Stack] flush stack until first `(`" << endl;
      while(S.back() != "("){
        cout << "\t\t[Push to Output] `" << S.back() << "`" << endl;
        express.push_back(S.back());
        S.pop_back();
      }
    }else if(tokens[i] == ")"){
      cout << "\t[Flush Stack] flush stack until first `(`" << endl;
      while(S.back() != "("){
        cout << "\t\t[Push to Output] `" << S.back() << "`" << endl;
        express.push_back(S.back());
        S.pop_back();
      }
      cout << "\[Flush Stack] pop `(` from stack" << endl;
      S.pop_back();
    }else{
      cout << "\t[Flush Stack] flush until the precedence of the top element is lesser then `" << tokens[i] << "`" << endl;
      while((!S.empty()) && (precedence[tokens[i]] <= 2 ? 
            precedence[S.back()] < precedence[tokens[i]] : precedence[S.back()] <= precedence[tokens[i]])){
        cout << "\t\t[Push to Output] `" << S.back() << "`" << endl;
        express.push_back(S.back());
        S.pop_back();
      }
      cout << "\t[Push to Stack] `" << tokens[i] <<  "`" << endl;
      S.push_back(tokens[i]);
    }
    cout << "[Output]";
    for(auto token : express){
      cout << ' ' << token;
    }
    cout << endl;
    cout << "[Stack]";
    for(auto token : S){
      cout << ' ' << token;
    }
    cout << endl;
    cout << "[Token Processing] done" << endl;
  }
  cout << "[Flush Stack] clear the whole stack" << endl;
  while(!S.empty()){
    cout << "\t[Push to Output] `" << S.back() << "`" << endl;
    express.push_back(S.back());
    S.pop_back();
  }
  cout << "[Infix to Postfix] done" << endl;
  return express;
}

template<typename T>
T evalPostfixExpression(const vector<simple_token>& express)
{
  stack<T> eval;
  for(auto token : express){
    if(isNumeric(token[0])){
      eval.push(fromString<T>(token));
    }else if(token == "+"){
      T b = eval.top(); eval.pop();
      T a = eval.top(); eval.pop();
      eval.push(a + b);
    }else if(token == "-"){
      T b = eval.top(); eval.pop();
      T a = eval.top(); eval.pop();
      eval.push(a - b);
    }else if(token == "unary+"){
      T a = eval.top(); eval.pop();
      eval.push(+a);
    }else if(token == "unary-"){
      T a = eval.top(); eval.pop();
      eval.push(-a);
    }else if(token == "*"){
      T b = eval.top(); eval.pop();
      T a = eval.top(); eval.pop();
      eval.push(a * b);
    }else if(token == "/"){
      T b = eval.top(); eval.pop();
      T a = eval.top(); eval.pop();
      eval.push(a / b);
    }else if(token == "pow"){
      T b = eval.top(); eval.pop();
      T a = eval.top(); eval.pop();
      eval.push(pow(a, b));
    }else if(token == "sin"){
      T a = eval.top(); eval.pop();
      eval.push(sin(a));
    }else if(token == "cos"){
      T a = eval.top(); eval.pop();
      eval.push(cos(a));
    }else if(token == "exp"){
      T a = eval.top(); eval.pop();
      eval.push(exp(a));
    }else if(token == "log"){
      T a = eval.top(); eval.pop();
      eval.push(log(a));
    }else if(token == "sqrt"){
      T a = eval.top(); eval.pop();
      eval.push(sqrt(a));
    }else if(token == "fabs"){
      T a = eval.top(); eval.pop();
      eval.push(fabs(a));
    }else if(token == "%"){
      int b = eval.top(); eval.pop();
      int a = eval.top(); eval.pop();
      eval.push(a % b);
    }else if(token == "<<"){
      int b = eval.top(); eval.pop();
      int a = eval.top(); eval.pop();
      eval.push(a << b);
    }else if(token == ">>"){
      int b = eval.top(); eval.pop();
      int a = eval.top(); eval.pop();
      eval.push(a >> b);
    }else if(token == "&"){
      int b = eval.top(); eval.pop();
      int a = eval.top(); eval.pop();
      eval.push(a & b);
    }else if(token == "^"){
      int b = eval.top(); eval.pop();
      int a = eval.top(); eval.pop();
      eval.push(a ^ b);
    }else if(token == "|"){
      int b = eval.top(); eval.pop();
      int a = eval.top(); eval.pop();
      eval.push(a | b);
    }else if(token == "&&"){
      int b = eval.top(); eval.pop();
      int a = eval.top(); eval.pop();
      eval.push(a && b);
    }else if(token == "||"){
      int b = eval.top(); eval.pop();
      int a = eval.top(); eval.pop();
      eval.push(a || b);
    }else if(token == "~"){
      int a = eval.top(); eval.pop();
      eval.push(~a);
    }else if(token == "!"){
      int a = eval.top(); eval.pop();
      eval.push(!a);
    }
  }
  return eval.top();
}

int main(int argc, char *argv[])
{
  string rawInput;
  while(getline(cin, rawInput)){
    auto tokens(parseExpression(rawInput));
    auto postfixExp(infix2postfix(tokens));
    cout << "Postfix Exp:";
    for(auto token : postfixExp){
      if(token[0] == 'u'){
        cout << ' ' << token[5];
      }else{
#ifndef TASK_3_4_2
        cout << ' ' << token;
#else
        if(isNumeric(token[0]))
          cout << ' ' << setprecision(6) << fixed << fromString<double>(token);
        else
          cout << ' ' << token;
#endif
      }
    }
    cout << endl;

    cout << "RESULT: ";
#ifndef TASK_3_4_2
    cout << evalPostfixExpression<int>(postfixExp) << endl;
#else
    cout << setprecision(6) << fixed
         << evalPostfixExpression<double>(postfixExp) << endl;
#endif

  }
  return 0;
}

