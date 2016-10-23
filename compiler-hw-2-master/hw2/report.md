HW2 - CSE
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

There are two files in the directory "test": `test.sh` and `a.cpp`

`test.sh` compiles `a.cpp` to `a.ll` and optimizes `a.ll` with `MyCSE`
 pass to produce `a.opt.ll` and finally compiles `a.opt.ll` to `a.out`.
I've written asserts in `a.cpp` so at the end when I execute `a.out`
 the asserts will check if the program's semantics are still correct after CSE.

## Compare between my pass enabled and disabled
The output of my testbench:
```
Run CSE
CSE count: 54
Verify the program by executing it
ok
All tests passed
```

After my CSE pass is enabled, 54 common subexpressions are detected and replaced.

