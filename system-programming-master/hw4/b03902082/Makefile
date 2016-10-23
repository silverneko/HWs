CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread
targets = merger Report.pdf

all: $(targets)

merger: src/merger.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

Report.pdf: src/Report.md
	pandoc $< -o Report.pdf --latex-engine=xelatex

clean:
	rm -rf $(targets)

