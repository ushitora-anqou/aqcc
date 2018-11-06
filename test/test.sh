#!/bin/bash

function fail(){
    echo -ne "\e[1;31m[ERROR]\e[0m "
    echo "$1"
    exit 1
}

$AQCC_CC test
[ $? -eq 0 ] || fail "$AQCC_CC test"

$AQCC_AS test
[ $? -eq 0 ] || fail "$AQCC_AS test"

$AQCC_LD test
[ $? -eq 0 ] || fail "$AQCC_LD test"

AQCC=../aqcc

$AQCC test_define.c testutil.c stdlib.c system.s -o _test_define_exe.o -v
[ $? -eq 0 ] || fail "$AQCC"
./_test_define_exe.o
[ $? -eq 0 ] || fail "./_test_define_exe.o"

gcc -E -P test.c -o _test.c
$AQCC _test.c testutil.c stdlib.c system.s -o _test_exe.o -v
[ $? -eq 0 ] || fail "$AQCC"
./_test_exe.o
[ $? -eq 0 ] || fail "./_test_exe.o"

$AQCC test_link.c test_link.s test_link2.s -o _test_exe.o -v
[ $? -eq 0 ] || fail "$AQCC"
./_test_exe.o
[ $? -eq 1 ] || fail "./_test_exe.o (link)"
