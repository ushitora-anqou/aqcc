SRC=main.c vector.c utility.c map.c lex.c parse.c gen.c type.c env.c ast.c


main: $(SRC) test.inc
	gcc -o $@ $(SRC) -O0 -g -Wall -lm

test: main testutil.o
	./test.sh

testutil.o: testutil.c
	gcc -c -o testutil.o testutil.c

.PHONY: test
