CC:=gcc
CXX:=g++

CFLAGS:=-O2 -std=c++11 -Wall -Wextra

all: problem1
	./preprocess_trainingset raw/TrainingSet.raw S.raw > model
	./ocr model raw/sample1.raw
	./ocr model raw/sample2.raw

problem1: preprocess_trainingset.cpp ocr.cpp
	$(CXX) $(CFLAGS) preprocess_trainingset.cpp -o preprocess_trainingset
	$(CXX) $(CFLAGS) ocr.cpp -o ocr

