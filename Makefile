main: main.c vector.c utility.c aqcc.h test.c map.c
	gcc -o $@ main.c vector.c utility.c map.c -O0 -g -Wall -lm

test: main testutil.o
	./test.sh

testutil.o: testutil.c
	gcc -c -o testutil.o testutil.c

.PHONY: test
