#!/bin/sh

set -e

make -C ../build

if [ -z $1 ]; then
  input="a.cpp"
else
  input=$1
fi
optflags="-load ../build/MyCSE/libMyCSE.so"

clang++ -O0 -S -emit-llvm -std=c++11 -o a.ll $input

echo "Run CSE"
opt $optflags -myCSE -S -o a.opt.ll a.ll

echo "Verify the program by executing it"
clang++ -o a.out a.opt.ll
./a.out

echo "All tests passed"

