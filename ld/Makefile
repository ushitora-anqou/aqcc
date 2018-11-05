TARGET=ld
SRC=main.c vector.c utility.c map.c link.c object.c stdlib.c string_builder.c
SRC_ASM=system.s
CC=gcc
FLAGS=-O0 -g3 -Wall -std=c11 -fno-builtin  -fno-stack-protector -static -nostdlib

$(TARGET): $(SRC) $(SRC_ASM) test.inc ld.h
	$(CC) -o $@ $(SRC) $(SRC_ASM) $(FLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean
