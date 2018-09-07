TARGET=aqcc
TARGET_SELF=aqcc_self
TARGET_SELFSELF=aqcc_selfself
SRC=main.c vector.c utility.c map.c lex.c parse.c gen.c type.c env.c ast.c analyze.c string_builder.c cpp.c token.c optimize.c assemble.c code.c system.c
SRC_ASM=system.s
SELF_OBJS=$(SRC:.c=.self.o)
SELFSELF_OBJS=$(SRC:.c=.selfself.o)

$(TARGET): $(SRC) $(SRC_ASM) test.inc aqcc.h
	gcc -o $@ $(SRC) $(SRC_ASM) -O0 -g3 -Wall -std=c11 -static

test: $(TARGET) testutil.o
	./test.sh

$(TARGET_SELF): $(SELF_OBJS)
	gcc $^ -o $@ -static

$(TARGET_SELFSELF): $(SELFSELF_OBJS)
	gcc $^ -o $@ -static

%.self.o: %.c $(TARGET)
	./$(TARGET) $< $@

%.selfself.o: %.c $(TARGET_SELF)
	./$(TARGET_SELF) $< $@

self_test: $(TARGET_SELF) _test_self_test.sh testutil.o
	./_test_self_test.sh

selfself_test: $(TARGET_SELFSELF) _test_selfself_test.sh testutil.o
	./_test_selfself_test.sh
	cat $$(ls *.self.o | sort) > __self_sort.in
	cat $$(ls *.selfself.o | sort) > __selfself_sort.in
	cmp __self_sort.in __selfself_sort.in

_test_self_test.sh: test.sh
	cp $^ $@
	sed -i -E "s/\\.\\/aqcc/.\\/aqcc_self/g" $@

_test_selfself_test.sh: test.sh
	cp $^ $@
	sed -i -E "s/\\.\\/aqcc/.\\/aqcc_selfself/g" $@

testutil.o: testutil.c aqcc.h
	gcc -c testutil.c -o $@

examples: $(TARGET)
	make -C examples

clean:
	make -C examples $@
	rm -f $(SELF_OBJS) $(SELFSELF_OBJS)
	rm -f _test_self_test.sh _test_selfself_test.sh
	rm -f _test.s _test.o testutil.o _test.in
	rm -f $(TARGET) $(TARGET_SELF) $(TARGET_SELFSELF)
	rm -f __self_sort.in __selfself_sort.in

.PHONY: test self self_test test clean examples
