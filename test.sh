#!/bin/sh

echo 2 | ./main > _test.s
gcc _test.s -o _test.o
./_test.o
[ $? -eq 2 ] || echo "ERROR"

