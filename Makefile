main: main.c vector.c utility.c aqcc.h
	gcc -o $@ $^ -O0 -g -Wall

test: main
	./test.sh

.PHONY: test
