#!/bin/sh

./aqcc test
[ $? -eq 0 ] || fail "./aqcc test"

cat test.c | ./aqcc  > _test.s
gcc _test.s -o _test.o testutil.o
./_test.o
