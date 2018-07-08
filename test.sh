#!/bin/sh

function test_aqcc() {
    echo $1 | ./main > _test.s
    gcc _test.s -o _test.o
    ./_test.o
    res=$?
    [ $res -eq $2 ] || echo "ERROR: $1 -> $res (expected $2)"
}

test_aqcc 2 2
test_aqcc 22 22
test_aqcc 2+2 4
test_aqcc 11+11+11 33
