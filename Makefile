main: main.c
	gcc -o $@ $^

test: main
	./test.sh

.PHONY: test
