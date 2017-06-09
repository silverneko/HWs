%{
#include <cstdlib>
%}
digit     [0-9]
int       {digit}+
float     {digit}+\.{digit}*|\.{digit}+
op_plus   "+"
op_minus  "-"
op_times  "*"
op_div    "/"
op_pow    "^"
lparen    "("
rparen    ")"
space     [ \t]+
error     .
%%
{int}       {
    Constant Const;
    Const.Type = ConstantType::Int;
    Const.IntValue = atoi(yytext);
    yylval.Const = Const;
    return CONST;
  }
{float}     {
    Constant Const;
    Const.Type = ConstantType::Float;
    Const.FloatValue = atof(yytext);
    yylval.Const = Const;
    return CONST;
  }
{op_plus}   return OP_PLUS;
{op_minus}  return OP_MINUS;
{op_times}  return OP_TIMES;
{op_div}    return OP_DIV;
{op_pow}    return OP_POW;
{lparen}    return LPAREN;
{rparen}    return RPAREN;
{space}     ;   // Do nothing
{error}     return ERROR;
%%
