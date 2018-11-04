#!/bin/bash

export AQCC_DETAIL=./aqcc_detail

function fail(){
    echo -ne "\e[1;31m[ERROR]\e[0m "
    echo "$1"
    exit 1
}

$AQCC_DETAIL test
[ $? -eq 0 ] || fail "./aqcc_detail test"

./aqcc test_define.c testutil.c stdlib.c system.s -o _test_define_exe.o -v
[ $? -eq 0 ] || fail "./aqcc"
./_test_define_exe.o
[ $? -eq 0 ] || fail "./_test_define_exe.o"

gcc -E -P test.c -o _test.c
./aqcc _test.c testutil.c stdlib.c system.s -o _test_exe.o -v
[ $? -eq 0 ] || fail "./aqcc"
./_test_exe.o
[ $? -eq 0 ] || fail "./_test_exe.o"

./aqcc test_link.c test_link.s -o _test_exe.o -v
[ $? -eq 0 ] || fail "./aqcc"
./_test_exe.o
[ $? -eq 5 ] || fail "./_test_exe.o (link)"
