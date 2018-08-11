#!/bin/sh

./aqcc test
[ $? -eq 0 ] || fail "./aqcc test"

cat test_define.c | ./aqcc  > _test.s
gcc _test.s -o _test.o testutil.o
./_test.o

gcc -E -P test.c -o _test.c
cat _test.c | ./aqcc  > _test.s
gcc _test.s -o _test.o testutil.o
./_test.o
