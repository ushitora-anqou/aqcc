main: main.c vector.c utility.c aqcc.h test.c
	gcc -o $@ main.c vector.c utility.c -O0 -g -Wall

test: main
	./test.sh

.PHONY: test
