all: calc

calc: parser.tab.c lex.yy.c
	g++ parser.tab.c -lfl -o calc -std=c++11

clean:
	rm -rf calc
