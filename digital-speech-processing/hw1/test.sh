#!/bin/sh

./test modellist.txt testing_data1.txt result1.txt
./test modellist.txt testing_data2.txt result2.txt

./accuracy.py testing_answer.txt result1.txt

cat result1.txt
cat acc.txt
