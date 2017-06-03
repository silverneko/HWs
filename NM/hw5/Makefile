MKLROOT = /home/yo/intel/mkl

all: mkl my_method

mkl: mkl.cpp
	g++ mkl.cpp -std=c++11 -O3 -o mkl  -Wl,--start-group $(MKLROOT)/lib/intel64/libmkl_intel_lp64.a $(MKLROOT)/lib/intel64/libmkl_intel_thread.a $(MKLROOT)/lib/intel64/libmkl_core.a -Wl,--end-group -liomp5 -lpthread -lm -ldl -L$(MKLROOT)/../compiler/lib/intel64  -m64 -I$(MKLROOT)/include -static

my_method: my_method.cpp
	g++ my_method.cpp -std=c++11 -O3 -static -o my_method
