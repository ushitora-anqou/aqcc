#!/bin/sh

function fail(){
    echo -ne "\e[1;31m[ERROR]\e[0m "
    echo "$1"
    exit 1
}

./aqcc test
[ $? -eq 0 ] || fail "./aqcc test"

./aqcc test_define.c > _test.s
gcc _test.s -o _test.o testutil.o
./_test.o

gcc -E -P test.c -o _test.c
./aqcc _test.c > _test.s
gcc _test.s -o _test.o testutil.o
./_test.o

# test for assembler experiment
function fail(){
    echo -ne "\e[1;31m[ERROR]\e[0m "
    echo "$1"
    exit 1
}

function test_aqcc_experiment() {
    echo "$1" > _test.in
    ./aqcc _test.in > _test.s
    ./aqcc _test.in _test_main.o experiment
    [ $? -eq 0 ] || fail "test_aqcc \"$1\": ./aqcc"
    gcc _test_main.o testutil.o -o _test.o
    [ $? -eq 0 ] || fail "test_aqcc \"$1\": gcc _test_main.o -o _test.o"
    ./_test.o
    res=$?
    [ $res -eq $2 ] || fail "test_aqcc \"$1\" -> $res (expected $2)"
}

test_aqcc_experiment "int main() { return 100; }" 100
test_aqcc_experiment "int main() { return 10; }" 10
test_aqcc_experiment "int main() { int a; return 10; }" 10
test_aqcc_experiment "int main() { int a = 10; return a; }" 10
test_aqcc_experiment "int main() { int a = 10; int b = 5; return b; }" 5
test_aqcc_experiment "int main() { int a = 10; int b = 7; return a - b; }" 3
test_aqcc_experiment "int main() { int a = 10; return a - 7; }" 3
test_aqcc_experiment "int main() { int a = 10; int b = 7; return a + b; }" 17
test_aqcc_experiment "int main() { int a = 10; return a + 7; }" 17
test_aqcc_experiment "int main() { int a = 10; int b = 7; return a * b; }" 70
test_aqcc_experiment "int main() { int a = 10; return a * 7; }" 70
test_aqcc_experiment "int main() { int a = 10; int b = 7; return a / b; }" 1
test_aqcc_experiment "int main() { int a = 10; return a / 7; }" 1
test_aqcc_experiment "int main() { int a = 10; int b = 7; return a % b; }" 3
test_aqcc_experiment "int main() { int a = 10; return a % 7; }" 3

