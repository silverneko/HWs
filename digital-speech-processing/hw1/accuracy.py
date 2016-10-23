#!/usr/bin/env python2

from sys import argv

File1 = open(argv[1], 'r')
File2 = open(argv[2], 'r')

TotalLines = 0
CorrectLines = 0

for x, y in zip(File1.readlines(), File2.readlines()):
    x = x.split()[0]
    y = y.split()[0]
    if x == y:
        CorrectLines += 1
    TotalLines += 1

Output = open('acc.txt', 'w')
Output.write('{}\n'.format(float(CorrectLines) / TotalLines))
