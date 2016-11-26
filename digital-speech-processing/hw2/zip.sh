#!/bin/sh

dir="b03902082"
files="01_run_HCopy.sh 02_run_HCompV.sh 03_training.sh 04_testing.sh result/accuracy lib/proto lib/mix2_10.hed hw2-1_b03902082.pdf"

mkdir $dir

for f in $files; do
  cp $f $dir
done

zip -r "$dir.zip" $dir

rm -rf $dir
