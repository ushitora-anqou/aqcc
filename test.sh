#!/bin/sh

function fail(){
    echo -ne "\e[1;31m[ERROR]\e[0m "
    echo "$1"
    exit 1
}

./aqcc test
[ $? -eq 0 ] || fail "./aqcc test"

./aqcc test_define.c _test.s || fail "./aqcc test_define.c _test.s"
gcc _test.s -o _test.o testutil.o -static
./_test.o
[ $? -eq 0 ] || fail "./aqcc test_define.c _test.s && ./_test.o"

gcc -E -P test.c -o _test.c
./aqcc _test.c _test.s || fail "./aqcc _test.c _test.s"
gcc _test.s -o _test.o testutil.o -static
./_test.o
[ $? -eq 0 ] || fail "./aqcc _test.c _test.s && ./_test.o"

./aqcc test_define.c _test_main.o || fail "./aqcc test_define.c _test_main.o"
gcc _test_main.o -o _test.o testutil.o -static
./_test.o
[ $? -eq 0 ] || fail "./aqcc test_define.c _test_main.o && ./_test.o"

gcc -E -P test.c -o _test.c
./aqcc _test.c _test_main.o || fail "./aqcc _test.c _test_main.o"
gcc _test_main.o -o _test.o testutil.o -static
./_test.o
[ $? -eq 0 ] || fail "./aqcc _test.c _test_main.o && ./_test.o"

# test for asm asm
function fail(){
    echo -ne "\e[1;31m[ERROR]\e[0m "
    echo "$1"
    exit 1
}

function test_aqcc_asm_asm() {
    echo "$1" > _test.s
    ./aqcc _test.s _test_.s
    diff _test.s _test_.s
    res=$?
    [ $res -eq 0 ] || fail "error"
}

test_aqcc_asm_asm "\
.text
.global main
main:
mov %eax, %edx
mov \$0, %r11
mov (%rdi), %r8d
mov %r10w, (%r12)
mov (%rip), %edx
mov %r8d, -9(%rip)
lea -4(%rdi), %r8d
lea (%rip), %edx
or -4(%rip), %edx
push %rax
.data
.zero 4
.long 3
.ascii \"asdg\\0\"
nop\
"

ls *.c | grep -Ev "aqcc_src2?.c|test" | xargs cat > _test.c
./aqcc _test.c _test.s
./aqcc _test.s _test_main.o
gcc _test_main.o -o _test.o testutil.o -static

# test for linker
./aqcc test_link.s test_link.o
./aqcc test_link.o test_link.exe
./test_link.exe
[ $? -eq 0 ] || fail "./aqcc test_link.s test_link.o & ./aqcc test_link.o test_link.exe & ./test_link.exe"
