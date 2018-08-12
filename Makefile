TARGET=aqcc
SRC=main.c vector.c utility.c map.c lex.c parse.c gen.c type.c env.c ast.c analyze.c string_builder.c cpp.c token.c

$(TARGET): $(SRC) test.inc aqcc.h
	gcc -o $@ $(SRC) -O0 -g3 -Wall -lm -std=c11

test: $(TARGET) testutil.o
	./test.sh

aqcc_self: $(TARGET) main.c vector.c utility.c map.c lex.c parse.c gen.c type.c env.c ast.c analyze.c string_builder.c cpp.c token.c
	gcc -c main.c -o _self_main.o
	./aqcc vector.c > _self_vector.s
	gcc -c _self_vector.s -o _self_vector.o
	gcc -c utility.c -o _self_utility.o
	./aqcc map.c > _self_map.s
	gcc -c _self_map.s -o _self_map.o
	gcc -c lex.c -o _self_lex.o
	gcc -c parse.c -o _self_parse.o
	gcc -c gen.c -o _self_gen.o
	gcc -c type.c -o _self_type.o
	./aqcc env.c > _self_env.s
	gcc -c _self_env.s -o _self_env.o
	./aqcc ast.c > _self_ast.s
	gcc -c _self_ast.s -o _self_ast.o
	gcc -c analyze.c -o _self_analyze.o
	./aqcc string_builder.c > _self_string_builder.s
	gcc -c _self_string_builder.s -o _self_string_builder.o
	./aqcc cpp.c > _self_cpp.s
	gcc -c _self_cpp.s -o _self_cpp.o
	./aqcc token.c > _self_token.s
	gcc -c _self_token.s -o _self_token.o
	gcc _self_main.o _self_vector.o _self_utility.o _self_map.o _self_lex.o _self_parse.o _self_gen.o _self_type.o _self_env.o _self_ast.o _self_analyze.o _self_string_builder.o _self_cpp.o _self_token.o \
		-o $@

self_test: aqcc_self _test_self_test.sh testutil.o
	./_test_self_test.sh

_test_self_test.sh: test.sh
	cp $^ $@
	sed -i -E "s/\\.\\/aqcc/.\\/aqcc_self/g" $@

testutil.o: testutil.c aqcc.h
	gcc -c testutil.c -o testutil.o

clean:
	rm testutil.o _self_main.o _self_vector.o _self_utility.o _self_map.o _self_lex.o _self_parse.o _self_gen.o _self_type.o _self_env.o _self_ast.o _self_analyze.o _self_string_builder.o _self_cpp.o _self_token.o
	rm _test_self_test.sh
	rm aqcc aqcc_self

.PHONY: test self self_test test clean
