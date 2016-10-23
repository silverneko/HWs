#ifndef _BIGINTEGER

#define _BIGINTEGER

#include <string>
#include <iostream>

#define MAXDIGIT (520)

class BigInteger{
  private:
    long long digit[MAXDIGIT];
  public:
    BigInteger();
    BigInteger(int);
    BigInteger(const std::string&);
    BigInteger(const BigInteger&);
    ~BigInteger();

    bool operator<(const BigInteger&) const;
    bool operator>(const BigInteger&) const;

    BigInteger operator+(const BigInteger&) const;
    BigInteger operator-(const BigInteger&) const;
    BigInteger operator*(const BigInteger&) const;
    BigInteger operator/(const BigInteger&) const;
    BigInteger operator%(const BigInteger&) const;
    BigInteger& operator*=(int);
    BigInteger& operator/=(int);
    BigInteger& operator=(const BigInteger&);
    bool operator == (const BigInteger&) const;
    bool isodd() const;
    bool iseven() const;
    bool iszero() const;
    BigInteger __half() const;

    friend std::ostream& operator<<(std::ostream&, const BigInteger&);

};


#endif
