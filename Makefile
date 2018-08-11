TARGET=aqcc
SRC=main.c vector.c utility.c map.c lex.c parse.c gen.c type.c env.c ast.c analyze.c string_builder.c cpp.c token.c

$(TARGET): $(SRC) test.inc aqcc.h
	gcc -o $@ $(SRC) -O0 -g3 -Wall -lm -std=c11

test: $(TARGET) testutil.o
	./test.sh

testutil.o: testutil.c aqcc.h
	gcc -c -o testutil.o testutil.c

.PHONY: test
