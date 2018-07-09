#!/bin/sh

function test_aqcc() {
    echo $1 | ./main > _test.s
    gcc _test.s -o _test.o
    ./_test.o
    res=$?
    [ $res -eq $2 ] || echo "ERROR: test_aqcc $1 -> $res (expected $2)"
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
test_aqcc 2*2 4
test_aqcc 11*11 121
test_aqcc 4/2 2
test_aqcc 363/121 3
test_aqcc 100/3 33
test_aqcc 1+2*3 7
test_aqcc 1+4*2-9/3 6

function test_vector() {
    gcc -o _test.o test_vector.c vector.c utility.c
    res=$(echo $1 | ./_test.o)
    [ "$res" = "$2" ] || echo "ERROR: test_vector $1 -> $res (expected $2)"
}

test_vector 1 1
test_vector 1234 1234
test_vector 12345 12345
