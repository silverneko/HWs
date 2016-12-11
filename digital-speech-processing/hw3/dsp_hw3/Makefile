# The following two variable will be commandline determined by TA
# For testing, you could uncomment them.
SRIPATH ?= /data/DSP_HW3/103_2/srilm-1.5.10
MACHINE_TYPE ?= i686-m64
LM ?= bigram.lm

CXX = g++
CXXFLAGS = -O3 -I$(SRIPATH)/include -std=c++11 -Wextra
vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)

TARGET = mydisambig
SRC = mydisambig.cpp
OBJ = $(SRC:.cpp=.o)
TO = ZhuYin-Big5.map
FROM = Big5-ZhuYin.map
.PHONY: all clean map run

all: $(TARGET)

$(TARGET): $(OBJ) -loolm -ldstruct -lmisc -lflm -llattice
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

run:
	@#TODO How to run your code toward different txt? 
	@for i in $(shell seq 1 10) ; do \
            echo "Running $$i.txt"; \
            ./mydisambig testdata/$$i.txt $(TO) $(LM) > result2/$$i.txt; \
        done;

map: mapping
	@echo "Mapping!"
	@./mapping $(FROM) > $(TO)
	@echo "Done!"

mapping: mapping.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	$(RM) $(OBJ) $(TARGET) mapping $(TO)
