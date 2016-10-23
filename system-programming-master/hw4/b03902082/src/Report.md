HW4 Report
==========

b03902082 資工二 江懿友

##Measured time using random generated test data
segment size \\ n | 100 | 10000 | 1000000 | 10000000
--|--|--|--|--
n/100 | 0.11 / 0.00 | 0.15 / 0.04 | 3.41 / 3.07 | 29.13 / 30.91
n/25  | 0.07 / 0.00 | 0.12 / 0.03 | 2.87 / 2.65 | 27.33 / 27.06
n/10  | 0.09 / 0.00 | 0.08 / 0.02 | 3.03 / 2.39 | 26.14 / 24.51
n/5   | 0.07 / 0.00 | 0.05 / 0.02 | 3.11 / 2.15 | 24.59 / 22.04
n/2   | 0.03 / 0.00 | 0.02 / 0.02 | 2.60 / 1.81 | 22.20 / 19.09
n     | 0.03 / 0.00 | 0.05 / 0.01 | 2.19 / 1.74 | 25.54 / 17.51

Time format is (real time / user time), measured on csie workstation linux11 using linux's `time` utility.

##State my findings
Seems like time is shortest when there are only 1 or 2 segments.
There isn't much time gain when we increase the number of segments.
The reasons may be:
1. When we increase the number of segments, we increase the number of threads, too. So the overhead of creating and joining threads is increased, too.
2. Though the merging process of each thread can overlap, the output process cannot. So for each merging round, each thread still needs to output one by one. Thus the time cost of whole merging process is bound (or bottlenecked) by the output process (because the time spent on outputing cannot overlap).



