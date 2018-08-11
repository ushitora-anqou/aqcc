#!/bin/sh

./aqcc test
[ $? -eq 0 ] || fail "./aqcc test"

./aqcc test_define.c > _test.s
gcc _test.s -o _test.o testutil.o
./_test.o

gcc -E -P test.c -o _test.c
./aqcc _test.c > _test.s
gcc _test.s -o _test.o testutil.o
./_test.o
