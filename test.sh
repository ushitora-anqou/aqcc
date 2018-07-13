#!/bin/sh

function fail(){
    echo -e "\e[1;31m[ERROR]\e[0m $1"
    exit 1
}

function test_aqcc() {
    echo "$1" | ./main > _test.s
    [ $? -eq 0 ] || fail "test_aqcc \"$1\": ./main > _test.s"
    gcc _test.s -o _test.o testutil.o
    [ $? -eq 0 ] || fail "test_aqcc \"$1\": gcc _test.s -o _test.o"
    ./_test.o
    res=$?
    [ $res -eq $2 ] || fail "test_aqcc \"$1\" -> $res (expected $2)"
}

./main test
[ $? -eq 0 ] || fail "./main test"

test_aqcc "main(){2;}" 2
test_aqcc "main(){22;}" 22
test_aqcc "main(){2+2;}" 4
test_aqcc "main(){11+11+11;}" 33
test_aqcc "main(){5-3;}" 2
test_aqcc "main(){35-22;}" 13
test_aqcc "main(){35-22-11;}" 2
test_aqcc "main(){199-23+300-475;}" 1
test_aqcc "main(){1+4-3;}" 2
test_aqcc "main(){1983+2-449-3123+1893-32+223-396;}" 101
test_aqcc "main(){2*2;}" 4
test_aqcc "main(){11*11;}" 121
test_aqcc "main(){4/2;}" 2
test_aqcc "main(){363/121;}" 3
test_aqcc "main(){100/3;}" 33
test_aqcc "main(){1+2*3;}" 7
test_aqcc "main(){1+4*2-9/3;}" 6
test_aqcc "main(){4%2;}" 0
test_aqcc "main(){5%2;}" 1
test_aqcc "main(){1935%10;}" 5
test_aqcc "main(){(1+2)*3;}" 9
test_aqcc "main(){(1+2)*(1+2);}" 9
test_aqcc "main(){(1+2)/(1+2);}" 1
test_aqcc "main(){33*(1+2);}" 99
test_aqcc "main(){(33*(1+2))/3;}" 33
test_aqcc "main(){-3+5;}" 2
test_aqcc "main(){+4;}" 4
test_aqcc "main(){-(33*(1+2))/3+34;}" 1
test_aqcc "main(){4 + 4;}" 8
test_aqcc "main(){- ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc "main(){2 << 1;}" 4
test_aqcc "main(){2 << 2 << 1;}" 16
test_aqcc "main(){2 << (2 << 1);}" 32
test_aqcc "main(){(2 - 1) << 1;}" 2
test_aqcc "main(){2 >> 1;}" 1
test_aqcc "main(){4 >> 2 >> 1;}" 0
test_aqcc "main(){(2 - 1) >> 1;}" 0
test_aqcc "main(){1<2;}" 1
test_aqcc "main(){1 < 2;}" 1
test_aqcc "main(){4 < 2;}" 0
test_aqcc "main(){1>2;}" 0
test_aqcc "main(){1 > 2;}" 0
test_aqcc "main(){4 > 2;}" 1
test_aqcc "main(){1<=2;}" 1
test_aqcc "main(){1 <= 2;}" 1
test_aqcc "main(){4 <= 2;}" 0
test_aqcc "main(){2 <= 2;}" 1
test_aqcc "main(){1>=2;}" 0
test_aqcc "main(){1 >= 2;}" 0
test_aqcc "main(){4 >= 2;}" 1
test_aqcc "main(){2 >= 2;}" 1
test_aqcc "main(){(2 < 1) + 1;}" 1
test_aqcc "main(){1==2;}" 0
test_aqcc "main(){1 == 2;}" 0
test_aqcc "main(){4 == 2;}" 0
test_aqcc "main(){2 == 2;}" 1
test_aqcc "main(){0&0;}" 0
test_aqcc "main(){0 & 0;}" 0
test_aqcc "main(){1 & 0;}" 0
test_aqcc "main(){0 & 1;}" 0
test_aqcc "main(){1 & 1;}" 1
test_aqcc "main(){1 & 2;}" 0
test_aqcc "main(){2 & 2;}" 2
test_aqcc "main(){3 & 5;}" 1
test_aqcc "main(){0^0;}" 0
test_aqcc "main(){0 ^ 0;}" 0
test_aqcc "main(){1 ^ 0;}" 1
test_aqcc "main(){0 ^ 1;}" 1
test_aqcc "main(){1 ^ 1;}" 0
test_aqcc "main(){1 ^ 2;}" 3
test_aqcc "main(){2 ^ 2;}" 0
test_aqcc "main(){3 ^ 5;}" 6
test_aqcc "main(){0|0;}" 0
test_aqcc "main(){0 | 0;}" 0
test_aqcc "main(){1 | 0;}" 1
test_aqcc "main(){0 | 1;}" 1
test_aqcc "main(){1 | 1;}" 1
test_aqcc "main(){1 | 2;}" 3
test_aqcc "main(){2 | 2;}" 2
test_aqcc "main(){3 | 5;}" 7
test_aqcc "main(){1&&0;}" 0
test_aqcc "main(){1 && 1;}" 1
test_aqcc "main(){0 && 1;}" 0
test_aqcc "main(){0 && 0;}" 0
test_aqcc "main(){2 && 1;}" 1
test_aqcc "main(){-2 || 1;}" 1
test_aqcc "main(){1||0;}" 1
test_aqcc "main(){1 || 1;}" 1
test_aqcc "main(){0 || 1;}" 1
test_aqcc "main(){0 || 0;}" 0
test_aqcc "main(){2 || 1;}" 1
test_aqcc "main(){-2 || 1;}" 1
test_aqcc "main(){x=1;}" 1
test_aqcc "main(){xy = 100+100;}" 200
test_aqcc "main(){a_b = - ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc "main(){_ = (2 - 1) << 1;}" 2
test_aqcc "main(){x = 1; y = 2;}" 2
test_aqcc "main(){x = 1; y = 2; z = x + y;}" 3
test_aqcc "main(){a0 = 1; a1 = 1; a2 = a0 + a1; a3 = a1 + a2;}" 3
test_aqcc "main(){x = y = 1; z = x = x + y;}" 2
test_aqcc "main(){ret0();}" 0
test_aqcc "main(){(ret0() + ret1()) * 2;}" 2
test_aqcc "main(){(ret0() * ret1()) + 2;}" 2
test_aqcc "main(){add1(1);}" 2
test_aqcc "main(){add_two(1, 2);}" 3
test_aqcc "main(){add_all(1, 2, 4, 8, 16, 32, 64, 128);}" 1
test_aqcc "iret0(){0;}main(){iret0();}" 0
test_aqcc "iret0(){0;}iret1(){1;}main(){iret0() + iret1();}" 1
