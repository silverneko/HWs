#!/bin/sh

make -C ../build

opt -load ../build/MyDCE/libMyDCE.so -myMagicDCE -S a.ll -o a.opt.ll
clang a.opt.ll -o a

