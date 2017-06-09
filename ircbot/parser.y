%{
#include <cmath>
#include <cstdio>
#include <algorithm>

enum class ConstantType {
  Int,
  Float
};

class Constant {
public:
  ConstantType Type;
  union {
    int IntValue;
    double FloatValue;
  };
};

ConstantType getWiderType(ConstantType lhs, ConstantType rhs) {
  return std::max(lhs, rhs);
}

char output[512];

extern "C" int yylex();
void yyerror(char *msg);

%}

%union {
  Constant Const;
}

%token <Const> CONST
%token OP_PLUS
%token OP_MINUS
%token OP_TIMES
%token OP_DIV
%token OP_POW
%token LPAREN
%token RPAREN
%token ERROR

%type <Const> expression
%type <Const> term
%type <Const> factor
%type <Const> expr
%type <Const> start

%start start

%%
start       : expression
            {
              $$ = $1;
              if ($$.Type == ConstantType::Int) {
                sprintf(output, "%d", $$.IntValue);
              } else {
                sprintf(output, "%lf", $$.FloatValue);
              }
            }

expression  : expression OP_PLUS term
            {
              Constant Result;
              Result.Type = getWiderType($1.Type, $3.Type);
              if (Result.Type == ConstantType::Int) {
                Result.IntValue = $1.IntValue + $3.IntValue;
              } else {
                if ($1.Type == $3.Type) {
                  Result.FloatValue = $1.FloatValue + $3.FloatValue;
                } else if ($1.Type == ConstantType::Int) {
                  Result.FloatValue = (double)($1.IntValue) + $3.FloatValue;
                } else {
                  Result.FloatValue = $1.FloatValue + (double)($3.IntValue);
                }
              }
              $$ = Result;
            }
            | expression OP_MINUS term
            {
              Constant Result;
              Result.Type = getWiderType($1.Type, $3.Type);
              if (Result.Type == ConstantType::Int) {
                Result.IntValue = $1.IntValue - $3.IntValue;
              } else {
                if ($1.Type == $3.Type) {
                  Result.FloatValue = $1.FloatValue - $3.FloatValue;
                } else if ($1.Type == ConstantType::Int) {
                  Result.FloatValue = (double)($1.IntValue) - $3.FloatValue;
                } else {
                  Result.FloatValue = $1.FloatValue - (double)($3.IntValue);
                }
              }
              $$ = Result;
            }
            | term
            {
              $$ = $1;
            }

term        : term OP_TIMES factor
            {
              Constant Result;
              Result.Type = getWiderType($1.Type, $3.Type);
              if (Result.Type == ConstantType::Int) {
                Result.IntValue = $1.IntValue * $3.IntValue;
              } else {
                if ($1.Type == $3.Type) {
                  Result.FloatValue = $1.FloatValue * $3.FloatValue;
                } else if ($1.Type == ConstantType::Int) {
                  Result.FloatValue = (double)($1.IntValue) * $3.FloatValue;
                } else {
                  Result.FloatValue = $1.FloatValue * (double)($3.IntValue);
                }
              }
              $$ = Result;
            }
            | term OP_DIV factor
            {
              Constant Result;
              Result.Type = getWiderType($1.Type, $3.Type);
              if (Result.Type == ConstantType::Int) {
                if ($3.IntValue == 0) {
                  throw "Division by zero";
                } else {
                  Result.IntValue = $1.IntValue / $3.IntValue;
                }
              } else {
                if ($1.Type == $3.Type) {
                  Result.FloatValue = $1.FloatValue / $3.FloatValue;
                } else if ($1.Type == ConstantType::Int) {
                  Result.FloatValue = (double)($1.IntValue) / $3.FloatValue;
                } else {
                  Result.FloatValue = $1.FloatValue / (double)($3.IntValue);
                }
              }
              $$ = Result;
            }
            | factor
            {
              $$ = $1;
            }

factor      : expr OP_POW factor
              {
                double a, b;
                if ($1.Type == ConstantType::Int) {
                  a = (double)$1.IntValue;
                } else {
                  a = $1.FloatValue;
                }
                if ($3.Type == ConstantType::Int) {
                  b = (double)$3.IntValue;
                } else {
                  b = $3.FloatValue;
                }
                Constant Result;
                Result.Type = ConstantType::Float;
                Result.FloatValue = pow(a, b);
                $$ = Result;
              }
            | expr
              {
                $$ = $1;
              }

expr        : LPAREN expression RPAREN
              {
                $$ = $2;
              }
            | OP_PLUS expr
              {
                $$ = $2;
              }
            | OP_MINUS expr
              {
                $$ = $2;
                if ($2.Type == ConstantType::Int) {
                  $$.IntValue *= -1;
                } else {
                  $$.FloatValue *= -1;
                }
              }
            | CONST
              {
                $$ = $1;
              }

%%

#include "lex.yy.c"

int main(int argc, char *argv[]) {
  yyin = stdin;
  try {
    yyparse();
  } catch (...) {
    sprintf(output, "Error: Overflow or Division by zero");
  }
  printf("%s\n", output);
  return 0;
}

void yyerror(char *msg) {
  sprintf(output, "Error: Bad input format");
}
