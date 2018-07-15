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

test_aqcc "main(){return 2;}" 2
test_aqcc "main(){return 22;}" 22
test_aqcc "main(){return 2+2;}" 4
test_aqcc "main(){return 11+11+11;}" 33
test_aqcc "main(){return 5-3;}" 2
test_aqcc "main(){return 35-22;}" 13
test_aqcc "main(){return 35-22-11;}" 2
test_aqcc "main(){return 199-23+300-475;}" 1
test_aqcc "main(){return 1+4-3;}" 2
test_aqcc "main(){return 1983+2-449-3123+1893-32+223-396;}" 101
test_aqcc "main(){return 2*2;}" 4
test_aqcc "main(){return 11*11;}" 121
test_aqcc "main(){return 4/2;}" 2
test_aqcc "main(){return 363/121;}" 3
test_aqcc "main(){return 100/3;}" 33
test_aqcc "main(){return 1+2*3;}" 7
test_aqcc "main(){return 1+4*2-9/3;}" 6
test_aqcc "main(){return 4%2;}" 0
test_aqcc "main(){return 5%2;}" 1
test_aqcc "main(){return 1935%10;}" 5
test_aqcc "main(){return (1+2)*3;}" 9
test_aqcc "main(){return (1+2)*(1+2);}" 9
test_aqcc "main(){return (1+2)/(1+2);}" 1
test_aqcc "main(){return 33*(1+2);}" 99
test_aqcc "main(){return (33*(1+2))/3;}" 33
test_aqcc "main(){return -3+5;}" 2
test_aqcc "main(){return +4;}" 4
test_aqcc "main(){return -(33*(1+2))/3+34;}" 1
test_aqcc "main(){return 4 + 4;}" 8
test_aqcc "main(){return - ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc "main(){return 2 << 1;}" 4
test_aqcc "main(){return 2 << 2 << 1;}" 16
test_aqcc "main(){return 2 << (2 << 1);}" 32
test_aqcc "main(){return (2 - 1) << 1;}" 2
test_aqcc "main(){return 2 >> 1;}" 1
test_aqcc "main(){return 4 >> 2 >> 1;}" 0
test_aqcc "main(){return (2 - 1) >> 1;}" 0
test_aqcc "main(){return 1<2;}" 1
test_aqcc "main(){return 1 < 2;}" 1
test_aqcc "main(){return 4 < 2;}" 0
test_aqcc "main(){return 1>2;}" 0
test_aqcc "main(){return 1 > 2;}" 0
test_aqcc "main(){return 4 > 2;}" 1
test_aqcc "main(){return 1<=2;}" 1
test_aqcc "main(){return 1 <= 2;}" 1
test_aqcc "main(){return 4 <= 2;}" 0
test_aqcc "main(){return 2 <= 2;}" 1
test_aqcc "main(){return 1>=2;}" 0
test_aqcc "main(){return 1 >= 2;}" 0
test_aqcc "main(){return 4 >= 2;}" 1
test_aqcc "main(){return 2 >= 2;}" 1
test_aqcc "main(){return (2 < 1) + 1;}" 1
test_aqcc "main(){return 1==2;}" 0
test_aqcc "main(){return 1 == 2;}" 0
test_aqcc "main(){return 4 == 2;}" 0
test_aqcc "main(){return 2 == 2;}" 1
test_aqcc "main(){return 1!=2;}" 1
test_aqcc "main(){return 1 != 2;}" 1
test_aqcc "main(){return 4 != 2;}" 1
test_aqcc "main(){return 2 != 2;}" 0
test_aqcc "main(){return 0&0;}" 0
test_aqcc "main(){return 0 & 0;}" 0
test_aqcc "main(){return 1 & 0;}" 0
test_aqcc "main(){return 0 & 1;}" 0
test_aqcc "main(){return 1 & 1;}" 1
test_aqcc "main(){return 1 & 2;}" 0
test_aqcc "main(){return 2 & 2;}" 2
test_aqcc "main(){return 3 & 5;}" 1
test_aqcc "main(){return 0^0;}" 0
test_aqcc "main(){return 0 ^ 0;}" 0
test_aqcc "main(){return 1 ^ 0;}" 1
test_aqcc "main(){return 0 ^ 1;}" 1
test_aqcc "main(){return 1 ^ 1;}" 0
test_aqcc "main(){return 1 ^ 2;}" 3
test_aqcc "main(){return 2 ^ 2;}" 0
test_aqcc "main(){return 3 ^ 5;}" 6
test_aqcc "main(){return 0|0;}" 0
test_aqcc "main(){return 0 | 0;}" 0
test_aqcc "main(){return 1 | 0;}" 1
test_aqcc "main(){return 0 | 1;}" 1
test_aqcc "main(){return 1 | 1;}" 1
test_aqcc "main(){return 1 | 2;}" 3
test_aqcc "main(){return 2 | 2;}" 2
test_aqcc "main(){return 3 | 5;}" 7
test_aqcc "main(){return 1&&0;}" 0
test_aqcc "main(){return 1 && 1;}" 1
test_aqcc "main(){return 0 && 1;}" 0
test_aqcc "main(){return 0 && 0;}" 0
test_aqcc "main(){return 2 && 1;}" 1
test_aqcc "main(){return -2 || 1;}" 1
test_aqcc "main(){return 1||0;}" 1
test_aqcc "main(){return 1 || 1;}" 1
test_aqcc "main(){return 0 || 1;}" 1
test_aqcc "main(){return 0 || 0;}" 0
test_aqcc "main(){return 2 || 1;}" 1
test_aqcc "main(){return -2 || 1;}" 1
test_aqcc "main(){return x=1;}" 1
test_aqcc "main(){return xy = 100+100;}" 200
test_aqcc "main(){return a_b = - ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc "main(){return _ = (2 - 1) << 1;}" 2
test_aqcc "main(){x = 1; return y = 2;}" 2
test_aqcc "main(){x = 1; y = 2; return z = x + y;}" 3
test_aqcc "main(){a0 = 1; a1 = 1; a2 = a0 + a1; return a3 = a1 + a2;}" 3
test_aqcc "main(){x = y = 1; return z = x = x + y;}" 2
test_aqcc "main(){return ret0();}" 0
test_aqcc "main(){return (ret0() + ret1()) * 2;}" 2
test_aqcc "main(){return (ret0() * ret1()) + 2;}" 2
test_aqcc "main(){return add1(1);}" 2
test_aqcc "main(){return add_two(1, 2);}" 3
test_aqcc "main(){return add_all(1, 2, 4, 8, 16, 32, 64, 128);}" 1
test_aqcc "iret0(){return 0;}main(){return iret0();}" 0
test_aqcc "iret0(){return 0;}iret1(){return 1;}main(){return iret0() + iret1();}" 1
test_aqcc "main(){return;}" 0
test_aqcc "main(){;}" 0
test_aqcc "iadd(a, b) { return a + b; }main(){return iadd(1, 2);}" 3
test_aqcc "iadd(a, b) { return a + b; }main(){return iadd(1, 2) * iadd(2, 3);}" 15
test_aqcc "eighth(a, b, c, d, e, f, g, h){return h;}main(){return eighth(1, 2, 3, 4, 5, 6, 7, 8);}" 8
test_aqcc "main(){return 0 == 0 ? 0 : 1;}" 0
test_aqcc "fib(n){return n == 0 ? 0 : n == 1 ? 1 : fib(n - 1) + fib(n - 2);}main(){return fib(5);}" 5
test_aqcc "main(){if(0 == 1)return 0;return 1;}" 1
test_aqcc "main(){if(0 == 1)return 0;else return 1;}" 1
test_aqcc "main(){if(0 == 1){return 0;}else{return 1;}}" 1
test_aqcc "fib(n){if(n<=1)return n;return fib(n-1)+fib(n-2);}main(){return fib(0);}" 0
test_aqcc "fib(n){if(n<=1)return n;return fib(n-1)+fib(n-2);}main(){return fib(1);}" 1
test_aqcc "fib(n){if(n<=1)return n;return fib(n-1)+fib(n-2);}main(){return fib(5);}" 5

