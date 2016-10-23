#include <algorithm>
#include <iomanip>
#include "biginteger.h"

using namespace std;

BigInteger::BigInteger() : digit {0}
{
}

BigInteger::BigInteger(int a) : digit {0}
{
  for(int i = 0; a > 0 && i < MAXDIGIT; ++i){
    digit[i] = a % 10;
    a /= 10;
  }
}

BigInteger::BigInteger(const string& s) : digit {0}
{
  for(int i = 0; i < s.size(); ++i){
    digit[i] = s[ s.size()-i-1 ] - '0';
  }
}

BigInteger::BigInteger(const BigInteger& b) : digit {0}
{
  for(int i = 0; i < MAXDIGIT; ++i)
    digit[i] = b.digit[i];
}

BigInteger::~BigInteger()
{
  // nothing to be done
}

bool BigInteger::operator < (const BigInteger& b) const
{
  for(int i = MAXDIGIT-1; i >= 0; --i){
    if(digit[i] != b.digit[i]){
      return digit[i] < b.digit[i];
    }
  }
  return false;
}

bool BigInteger::operator > (const BigInteger& b) const
{
  return b < (*this);
}

BigInteger BigInteger::operator + (const BigInteger& b) const
{
  BigInteger res;
  for(int i = 0; i < MAXDIGIT; ++i){
    res.digit[i] = digit[i] + b.digit[i];
  }
  for(int i = 0; i < MAXDIGIT-1; ++i){
    res.digit[i+1] += res.digit[i] / 10;
    res.digit[i] %= 10;
  }
  return res;
}

BigInteger BigInteger::operator - (const BigInteger& b) const
{
  BigInteger res;
  int borrow = 0;
  for(int i = 0; i < MAXDIGIT; ++i){
    res.digit[i] = digit[i] - b.digit[i] - borrow;
    borrow = 0;
    if(res.digit[i] < 0){
      res.digit[i] += 10;
      borrow = 1;
    }
  }
  return res;
}

BigInteger BigInteger::operator * (const BigInteger& b) const
{
  BigInteger a;
  int is = MAXDIGIT-1, js = MAXDIGIT-1;
  while(is >= 0 && digit[is] == 0) --is;
  while(js >= 0 && b.digit[js] == 0) --js;
  for(int i = is; i >= 0; --i){
    for(int j = js; j >= 0; --j){
      a.digit[i+j] += digit[i] * b.digit[j];
    }
  }
/*
  for(int i = 0; i < MAXDIGIT; ++i){
    for(int j = 0; j <= i; ++j){
      a.digit[i] += digit[j] * b.digit[i-j];
    }
  }
  */
  for(int i = 0; i < MAXDIGIT-1; ++i){
    a.digit[i+1] += a.digit[i] / 10;
    a.digit[i] %= 10;
  }
  return a;
}

BigInteger& BigInteger::operator *= (int a)
{
  for(int i = 0; i < MAXDIGIT; ++i){
    digit[i] *= a;
  }
  for(int i = 0; i < MAXDIGIT-1; ++i){
    digit[i+1] += digit[i] / 10;
    digit[i] %= 10;
  }
  return *this;
}

BigInteger BigInteger::__half() const
{
  BigInteger res(*this);
  for(int i = MAXDIGIT-1; i >= 0; --i){
    if(i >= 1)
      res.digit[i-1] += (res.digit[i] % 2) * 10;
    res.digit[i] /= 2;
  }
  return res;
}

BigInteger BigInteger::operator / (const BigInteger& b) const
{
  BigInteger lb(0), rb(*this + 1);
  while(!(rb-lb-1).iszero()){
    BigInteger mid((lb+rb).__half());
    if(mid * b > (*this)){
      rb = mid;
    }else{
      lb = mid;
    }
  }
  return lb;
}

BigInteger& BigInteger::operator /= (int a)
{
  *this = (*this) / a;
  return *this;
}

BigInteger BigInteger::operator % (const BigInteger& b) const
{
  auto q = (*this) / b;
  return (*this) - b * q;
}

BigInteger& BigInteger::operator = (const BigInteger& b)
{
  for(int i = 0; i < MAXDIGIT; ++i)
    digit[i] = b.digit[i];
  return *this;
}

bool BigInteger::operator == (const BigInteger& b) const
{
  for(int i = 0; i < MAXDIGIT; ++i)
    if(digit[i] != b.digit[i])
      return false;
  return true;
}

bool BigInteger::isodd() const
{
  return digit[0] & 1;
}

bool BigInteger::iseven() const
{
  return !isodd();
}

bool BigInteger::iszero() const
{
  for(auto d : digit){
    if(d != 0)
      return false;
  }
  return true;
}

ostream& operator << (ostream& sout, const BigInteger& b)
{
  if(b.iszero()){
    sout << 0;
  }else{
    int i = MAXDIGIT-1;
    while(i >= 0 && b.digit[i] == 0) --i;
    /*
    sout << b.digit[i++];
    sout << setfill('0') << setw(4);
    */
    for(; i >= 0; --i){
      sout << b.digit[i];
    }
    sout << setw(0);
  }
  return sout;
}


