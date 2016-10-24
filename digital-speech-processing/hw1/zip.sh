#!/bin/sh

mkdir hw1_b03902082

files="acc.txt hmm.h Makefile train.cpp test.cpp Document.pdf"

for f in $files; do
  cp $f hw1_b03902082
done

cp model_0* hw1_b03902082
cp result* hw1_b03902082

zip -r hw1_b03902082.zip hw1_b03902082

rm -rf hw1_b03902082
