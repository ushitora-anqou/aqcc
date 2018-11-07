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

$AQCC test_link.c test_link2.c test_link.s test_link2.s -o _test_exe.o -v
[ $? -eq 0 ] || fail "$AQCC"
./_test_exe.o
[ $? -eq 1 ] || fail "./_test_exe.o (link)"

# tests for SIMPLE arch (only run on first generation)
function test_simple() {
    cfile=_test_simple.c
    sfile=_test_simple.s
    echo "$1" > $cfile
    $AQCC_CC $cfile $sfile --arch=SIMPLE
    cat $sfile | SIMPLE-arch-tools/macro | SIMPLE-arch-tools/assembler | SIMPLE-arch-tools/emulator
    res=$?
    [ $res -eq $2 ] || fail "[ERROR] \"$1\": expect $2 but got $res"
}

test_simple "int main() { return 10; }" 10
test_simple "int main() { return 10+20; }" 30
test_simple "int main() { return 20-10; }" 10
test_simple "int main() { return -10+20; }" 10
test_simple "int main() { return ~-1; }" 0
test_simple "int main() { return 1<2; }" 1
test_simple "int main() { return 2<1; }" 0
test_simple "int main() { return -1<2; }" 1
test_simple "int main() { return -2<1; }" 1
test_simple "int main() { return 1<=2; }" 1
test_simple "int main() { return 2<=1; }" 0
test_simple "int main() { return -1<=2; }" 1
test_simple "int main() { return -2<=1; }" 1
test_simple "int main() { return 1<=1; }" 1
test_simple "int main() { return 1>2; }" 0
test_simple "int main() { return 2>1; }" 1
test_simple "int main() { return -1>2; }" 0
test_simple "int main() { return -2>1; }" 0
test_simple "int main() { return 1>=2; }" 0
test_simple "int main() { return 2>=1; }" 1
test_simple "int main() { return -1>=2; }" 0
test_simple "int main() { return -2>=1; }" 0
test_simple "int main() { return 1>=1; }" 1
