TARGET=aqcc
TARGET_SELF=aqcc_self
TARGET_SELFSELF=aqcc_selfself
SRC=main.c vector.c utility.c map.c lex.c parse.c gen.c type.c env.c ast.c analyze.c string_builder.c cpp.c token.c
SELF_OBJS=$(SRC:.c=.self.o)
SELFSELF_OBJS=$(SRC:.c=.selfself.o)
SELF_ASSEMBLES=$(SRC:.c=.self.s)
SELFSELF_ASSEMBLES=$(SRC:.c=.selfself.s)

$(TARGET): $(SRC) test.inc aqcc.h
	gcc -o $@ $(SRC) -O0 -g3 -Wall -lm -std=c11

test: $(TARGET) testutil.o
	./test.sh

$(TARGET_SELF): $(SELF_OBJS)
	gcc $^ -o $@

$(TARGET_SELFSELF): $(SELFSELF_OBJS)
	gcc $^ -o $@

%.self.s: %.c $(TARGET)
	./$(TARGET) $< > $@

%.self.o: %.self.s
	gcc -c $^ -o $@

%.selfself.s: %.c $(TARGET_SELF)
	./$(TARGET_SELF) $< > $@

%.selfself.o: %.selfself.s
	gcc -c $^ -o $@


self_test: $(TARGET_SELF) _test_self_test.sh testutil.o
	./_test_self_test.sh

selfself_test: $(TARGET_SELFSELF) _test_selfself_test.sh testutil.o
	./_test_selfself_test.sh

_test_self_test.sh: test.sh
	cp $^ $@
	sed -i -E "s/\\.\\/aqcc/.\\/aqcc_self/g" $@

_test_selfself_test.sh: test.sh
	cp $^ $@
	sed -i -E "s/\\.\\/aqcc/.\\/aqcc_selfself/g" $@

testutil.o: testutil.c aqcc.h
	gcc -c testutil.c -o testutil.o

examples: $(TARGET)
	make -C examples

clean:
	make -C examples $@
	rm -f $(SELF_OBJS) $(SELFSELF_OBJS)
	rm -f $(SELF_ASSEMBLES) $(SELFSELF_ASSEMBLES)
	rm -f _test_self_test.sh
	rm -f $(TARGET) $(TARGET_SELF) $(TARGET_SELFSELF)

.PHONY: test self self_test test clean examples

.PRECIOUS: %.self.s %.selfself.s
