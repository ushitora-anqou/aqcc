main: main.c vector.c utility.c aqcc.h test.c map.c
	gcc -o $@ main.c vector.c utility.c map.c -O0 -g -Wall

test: main
	./test.sh

.PHONY: test
