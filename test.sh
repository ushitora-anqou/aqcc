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

# test for register machine experiment
function fail(){
    echo -ne "\e[1;31m[ERROR]\e[0m "
    echo "$1"
    exit 1
}

function test_aqcc_experiment() {
    echo "$1" > _test.in
    ./aqcc _test.in experiment > _test.s
    [ $? -eq 0 ] || fail "test_aqcc \"$1\": ./aqcc > _test.s"
    gcc _test.s -o _test.o testutil.o
    [ $? -eq 0 ] || fail "test_aqcc \"$1\": gcc _test.s -o _test.o"
    ./_test.o
    res=$?
    [ $res -eq $2 ] || fail "test_aqcc \"$1\" -> $res (expected $2)"
}

test_aqcc_experiment "int main() { return 5; }" 5
test_aqcc_experiment "int main() { return 0; }" 0
test_aqcc_experiment "int main() { return 3 + 4; }" 7
test_aqcc_experiment "int main() { return 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10; }" 52
test_aqcc_experiment "int main() { return 4 - 3; }" 1
test_aqcc_experiment "int main() { return 3 + 4 + 5 - 6 + 7 - 8 + 9 - 10; }" 4

