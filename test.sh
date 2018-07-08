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
test_aqcc 5-3 2
test_aqcc 35-22 13
test_aqcc 35-22-11 2
test_aqcc 199-23+300-475 1
test_aqcc 1+4-3 2
test_aqcc 1983+2-449-3123+1893-32+223-396 101
