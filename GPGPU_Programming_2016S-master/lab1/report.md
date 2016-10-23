lab1 report
===========

b03902082 Yi-Yo Chiang

## Part III
My procedure "flips" each word in the text.
For exxample: 
```
a abc abcd abcdefg
```
is to be transformed to
```
a cba dcba gfedcba
```
.

## How it's done
First I launch a kernel to calculate each character's new position after the flip (the `rpos[]`).
i.e.
```
id:   0123456789AB
text: a abc abcdef
pos:  101230123456
rpos: 004320BA9876
```
Next I launch another kernel to swap each character to it's new position using the values in the array `rpos`.

In fact, the two kernels can be merge into one kernel. But I want to keep my code simple and easy to debug so I split the two steps into different kernels.

