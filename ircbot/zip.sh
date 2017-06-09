#!/bin/sh

mkdir b03902082

files="config lex.yy.c main.py Makefile parser.tab.c Report.pdf"

for f in $files; do
  cp $f b03902082
done

zip -r b03902082.zip b03902082

rm -rf b03902082
