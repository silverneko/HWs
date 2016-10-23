HW1 - DCE
=========
B03902082

## How to compile and test on my benchmark
```bash
$ mkdir build
$ cd build
$ cmake ..
$ cd ../test
$ ./test.sh
```

There are two files in the directory "test": `test.sh` and `a.ll`

`a.ll` is the micro benchmark, I've written some instructions in `a.ll` that are "dead code". Executing `test.sh` compiles `MyDCE` pass and optimizes `a.ll` with it, the output is `a.opt.ll` and `a`.

## Compare between my pass enabled and disabled
This is the output of "`diff a.ll a.opt.ll`":
```
1c1
< ; ModuleID = 'a.ll.1'
---
> ; ModuleID = 'a.ll'
10d9
<   %3 = add nsw i32 %2, 13
13,15d11
<   %4 = load i32, i32* %x
<   %5 = add i32 %4, %2
<   %6 = mul i32 %5, %4
```
Four dead instructions are removed from `a.ll`.

