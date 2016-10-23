#!/usr/bin/env bash
set -e

clang -std=c99 -fno-inline -S -emit-llvm -o a.ll "$1"
opt -O3 -disable-inlining -S -o a.1.ll a.ll
llc -load ../llvm-3.8.0.src/build/lib/LLVMX86RenameRegister.so -O3 -o a.s a.1.ll
clang -o a.out a.s
./a.out
