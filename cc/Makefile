TARGET=cc
SRC=main.c vector.c utility.c map.c lex.c parse.c x86_64_gen.c type.c env.c ast.c analyze.c string_builder.c cpp.c token.c optimize.c assemble.c code.c link.c object.c stdlib.c
SRC_ASM=system.s
DEBUG=-v

$(TARGET): $(SRC) $(SRC_ASM) test.inc aqcc.h
	gcc -o $@ $(SRC) $(SRC_ASM) -O0 -g3 -Wall -std=c11 -fno-builtin  -fno-stack-protector -static -nostdlib

clean:
	rm -f $(TARGET)

.PHONY: clean
