#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <tuple>
#include <functional>

using namespace std;
using namespace placeholders;

tuple<int,int> gcd_by_reverse_search(int a, int b)
{
  int it = 0;
  for(int i = min(a, b); i >= 1; --i){
    ++it;
    if((a % i == 0) && (b % i == 0)){
      return make_tuple(i, it);
    }
  }
  return make_tuple(1, it);
}

tuple<int,int> gcd_by_filter(int a, int b)
{
  int it = 0;
  for(int i = 2; i <= min(a, b); ++i){
    ++it;
    if((a % i == 0) && (b % i == 0)){
      auto t = gcd_by_filter(a/i, b/i);
      return make_tuple(i * get<0>(t), it + get<1>(t));
    }
  }
  return make_tuple(1, it);
}

tuple<int,int> gcd_by_filter_faster_internal(int a, int b, int s)
{
  int it = 0;
  for(int i = s; i <= min(a, b); ++i){
    ++it;
    if((a % i == 0) && (b % i == 0)){
      auto t = gcd_by_filter_faster_internal(a/i, b/i, i);
      return make_tuple(i * get<0>(t), it + get<1>(t));
    }
  }
  return make_tuple(1, it);
}

auto gcd_by_filter_faster = bind(gcd_by_filter_faster_internal, _1, _2, 2);

/*
tuple<int,int> gcd_by_filter_faster(int a, int b)
{
  return gcd_by_filter_faster_internal(a, b, s);
}
*/

tuple<int,int> gcd_by_binary(int a, int b)
{
  int n = min(a, b), m = max(a, b), ans = 1, it = 0;
  while((n != 0) && (m != 0)){
    ++it;
    if(!(n & 1) && !(m & 1)){
      ans *= 2;
    }
    if(!(n & 1)){
      n /= 2;
    }
    if(!(m & 1)){
      m /= 2;
    }
    if(n > m)
      swap(n, m);
    m -= n;
  }
  return make_tuple(n * ans, it);
}

tuple<int,int> gcd_by_euclid(int a, int b)
{
  int n = min(a, b), m = max(a, b), it = 0;
  while(m % n != 0){
    ++it;
    int tmp = n;
    n = m % n;
    m = tmp;
  }
  return make_tuple(n, it);
}

int main(int argc, char *argv[])
{
  function<tuple<int,int>(int,int)> gcds [] = {gcd_by_reverse_search, gcd_by_filter, gcd_by_filter_faster, gcd_by_binary, gcd_by_euclid};
  for(auto gcd : gcds){
    int a = 11260;
    int totalIt = 0;
    int maxIt = 0;
    int maxb = 0;
    for(int b = 52000; b <= 54260; ++b){
      totalIt += get<1>(gcd(a, b));
      int it = get<1>(gcd(a, b));
      if(it > maxIt){
        maxIt = it;
        maxb = b;
      }
    }
    printf("TotalIt: %d, Avg: %d\n", totalIt, totalIt / 2261);
    printf("MaxIt: %d, b: %d\n", maxIt, maxb);
  }
  return 0;
}

