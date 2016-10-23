CC = clang++
CFLAG = -std=c++11 -O2
target = tree forest
depend = decision_tree.h decision_tree.cpp

all: $(target)

tree: tree.cpp $(depend)
	$(CC) $(CFLAG) tree.cpp decision_tree.cpp -o tree

forest: forest.cpp $(depend)
	$(CC) $(CFLAG) forest.cpp decision_tree.cpp -o forest

clean:
	rm -f $(target)
