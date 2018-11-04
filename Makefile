TARGET=aqcc_detail
TARGET_SELF=aqcc_self_detail
TARGET_SELFSELF=aqcc_selfself_detail
SRC=main.c vector.c utility.c map.c lex.c parse.c x86_64_gen.c type.c env.c ast.c analyze.c string_builder.c cpp.c token.c optimize.c assemble.c code.c link.c object.c stdlib.c
SRC_ASM=system.s
SELF_OBJS=$(SRC:.c=.self.o) $(SRC_ASM:.s=.self.o)
SELFSELF_OBJS=$(SRC:.c=.selfself.o) $(SRC_ASM:.s=.selfself.o)
DEBUG=-v

$(TARGET): $(SRC) $(SRC_ASM) test.inc aqcc.h
	gcc -o $@ $(SRC) $(SRC_ASM) -O0 -g3 -Wall -std=c11 -fno-builtin  -fno-stack-protector -static -nostdlib

test: $(TARGET)
	./test.sh

#$(TARGET_SELF): $(TARGET) $(SELF_OBJS)
$(TARGET_SELF): $(TARGET) $(SRC) $(SRC_ASM) test.inc aqcc.h
	##gcc $^ -o $@ -static -nostdlib
	#./$(TARGET) oe $(SELF_OBJS) $@
	#chmod +x $@
	AQCC_DETAIL=./$(TARGET) ./aqcc -o $@ $(SRC) $(SRC_ASM) $(DEBUG)

#$(TARGET_SELFSELF): $(TARGET_SELF) $(SELFSELF_OBJS)
$(TARGET_SELFSELF): $(TARGET_SELF) $(SRC) $(SRC_ASM) test.inc aqcc.h
	##gcc $^ -o $@ -static -nostdlib
	#./$(TARGET_SELF) oe $(SELFSELF_OBJS) $@
	#chmod +x $@
	AQCC_DETAIL=./$(TARGET_SELF) ./aqcc -o $@ $(SRC) $(SRC_ASM) $(DEBUG)

#%.self.s: %.c $(TARGET)
#	./$(TARGET) cs $< $@
#
#%.self.s: %.s
#	cp $< $@
#
#%.self.o: %.self.s $(TARGET)
#	./$(TARGET) so $< $@
#
#%.selfself.s: %.c $(TARGET_SELF)
#	./$(TARGET_SELF) cs $< $@
#
#%.selfself.s: %.s
#	cp $< $@
#
#%.selfself.o: %.selfself.s $(TARGET_SELF)
#	./$(TARGET_SELF) so $< $@
#

self_test: $(TARGET_SELF) _test_self_test.sh
	./_test_self_test.sh

selfself_test: $(TARGET_SELFSELF) _test_selfself_test.sh
	./_test_selfself_test.sh
	#cat $$(ls *.self.o | sort) > __self_sort.in
	#cat $$(ls *.selfself.o | sort) > __selfself_sort.in
	#cmp __self_sort.in __selfself_sort.in
	cmp aqcc_self_detail aqcc_selfself_detail

_test_self_test.sh: test.sh
	cp $^ $@
	sed -i -E "s#AQCC_DETAIL=\./aqcc_detail#AQCC_DETAIL=\./aqcc_self_detail#g" $@

_test_selfself_test.sh: test.sh
	cp $^ $@
	sed -i -E "s#AQCC_DETAIL=\./aqcc_detail#AQCC_DETAIL=\./aqcc_selfself_detail#g" $@

clean:
	rm -f $(SELF_OBJS) $(SELFSELF_OBJS)
	rm -f _test_self_test.sh _test_selfself_test.sh
	rm -f _test.c _test.s _test.o _test.in _test_exe.o
	rm -f _testutil.s _testutil.o
	rm -f _test_define.o  _test_define_exe.o
	rm -f _test_link_c.o  _test_link_s.o  test_link_c.o  test_link_exe.o
	rm -f _stdlib.s _stdlib.o
	rm -f _system.s _system.o
	rm -f $(TARGET) $(TARGET_SELF) $(TARGET_SELFSELF)
	rm -f __self_sort.in __selfself_sort.in
	rm -f test_link.o test_link.exe
	rm -f $(SELF_OBJS:.o=.s) $(SELFSELF_OBJS:.o=.s)

.PHONY: test self self_test selfself_test test clean
.PRECIOUS: $(SELF_OBJS:.o=.s) $(SELFSELF_OBJS:.o=.s)
