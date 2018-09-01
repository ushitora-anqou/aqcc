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

test_aqcc_experiment "int a = 10; int main() { return a; }" 10
test_aqcc_experiment "int a; int main() { a = 10; return a; }" 10
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
test_aqcc_experiment "int main() { int a = -10; return -a; }" 10
test_aqcc_experiment "int main() { int a = -1; return ~a; }" 0
test_aqcc_experiment "int main() { int a = 3; return (a << 3); }" 24
test_aqcc_experiment "int main() { int a = 11; return (a >> 3); }" 1
test_aqcc_experiment "int main() { int a = 1, b = 2; return a < b; }" 1
test_aqcc_experiment "int main() { int b = 2; return 3 < b; }" 0
test_aqcc_experiment "int main() { int a = 1, b = 2; return a <= b; }" 1
test_aqcc_experiment "int main() { int a = 2, b = 2; return a <= b; }" 1
test_aqcc_experiment "int main() { int b = 2; return 3 <= b; }" 0
test_aqcc_experiment "int main() { int a = 1, b = 2; return a == b; }" 0
test_aqcc_experiment "int main() { int b = 3; return 3 == b; }" 1
test_aqcc_experiment "int main() { int a = 2, b = 3; return a & b; }" 2
test_aqcc_experiment "int main() { int a = 2, b = 3; return a | b; }" 3
test_aqcc_experiment "int main() { int a = 2, b = 3; return a ^ b; }" 1
test_aqcc_experiment "int main() { int a[2]; a[1] = 5; return a[1]; }" 5
test_aqcc_experiment "int main() { int ary[10][10]; ary[4][5] = 9; return ary[4][5]; }" 9
test_aqcc_experiment "int main() { int ary[10][10]; ary[4][5] = ary[3][2] = 4; return ary[4][5] + 2 * ary[3][2]; }" 12
test_aqcc_experiment "int main() { int c = 0; if (c < 0) return 1; }" 0
test_aqcc_experiment "int main() { int c = 0; for(;c > 0;) ; return c; }" 0
test_aqcc_experiment "int main() { int c = 0; for(;c < 2;c+=1) ; return c; }" 2
test_aqcc_experiment "int main() { int c = 0; for(int i = 0;i < 2;i+=1) ; return c; }" 0
test_aqcc_experiment "int main() { int c = 0; for(int i = 0;i < 2;i+=1) c+=1; return c; }" 2
test_aqcc_experiment "int main() { int ary[5], *p; ary[1] = 10; p = ary; p++; return *p; }" 10
test_aqcc_experiment "int main() { int ary[5], *p; ary[1] = 10; p = ary; ++p; return *p; }" 10
test_aqcc_experiment "int main() { int ary[5], *p; ary[1] = 10; p = &ary[2]; p--; return *p; }" 10
test_aqcc_experiment "int main() { int ary[5], *p; ary[1] = 10; p = &ary[2]; --p; return *p; }" 10
test_aqcc_experiment "int main() { int a = -1; a++; return a; }" 0
test_aqcc_experiment "int main() { int a = -1; ++a; return a; }" 0
test_aqcc_experiment "int main() { int a = 1; a--; return a; }" 0
test_aqcc_experiment "int main() { int a = 1; a--; return a; }" 0
test_aqcc_experiment "int main(){return 2;}" 2
test_aqcc_experiment "int main(){return 22;}" 22
test_aqcc_experiment "int main(){return 2+2;}" 4
test_aqcc_experiment "int main(){return 11+11+11;}" 33
test_aqcc_experiment "int main(){return 5-3;}" 2
test_aqcc_experiment "int main(){return 35-22;}" 13
test_aqcc_experiment "int main(){return 35-22-11;}" 2
test_aqcc_experiment "int main(){return 199-23+300-475;}" 1
test_aqcc_experiment "int main(){return 1+4-3;}" 2
test_aqcc_experiment "int main(){return 1983+2-449-3123+1893-32+223-396;}" 101
test_aqcc_experiment "int main(){return 2*2;}" 4
test_aqcc_experiment "int main(){return 11*11;}" 121
test_aqcc_experiment "int main(){return 4/2;}" 2
test_aqcc_experiment "int main(){return 363/121;}" 3
test_aqcc_experiment "int main(){return 100/3;}" 33
test_aqcc_experiment "int main(){return 1+2*3;}" 7
test_aqcc_experiment "int main(){return 1+4*2-9/3;}" 6
test_aqcc_experiment "int main(){return 4%2;}" 0
test_aqcc_experiment "int main(){return 5%2;}" 1
test_aqcc_experiment "int main(){return 1935%10;}" 5
test_aqcc_experiment "int main(){return (1+2)*3;}" 9
test_aqcc_experiment "int main(){return (1+2)*(1+2);}" 9
test_aqcc_experiment "int main(){return (1+2)/(1+2);}" 1
test_aqcc_experiment "int main(){return 33*(1+2);}" 99
test_aqcc_experiment "int main(){return (33*(1+2))/3;}" 33
test_aqcc_experiment "int main(){return -3+5;}" 2
test_aqcc_experiment "int main(){return +4;}" 4
test_aqcc_experiment "int main(){return -(33*(1+2))/3+34;}" 1
test_aqcc_experiment "int main(){return 4 + 4;}" 8
test_aqcc_experiment "int main(){return - ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc_experiment "int main(){return 2 << 1;}" 4
test_aqcc_experiment "int main(){return 2 << 2 << 1;}" 16
test_aqcc_experiment "int main(){return 2 << (2 << 1);}" 32
test_aqcc_experiment "int main(){return (2 - 1) << 1;}" 2
test_aqcc_experiment "int main(){return 2 >> 1;}" 1
test_aqcc_experiment "int main(){return 4 >> 2 >> 1;}" 0
test_aqcc_experiment "int main(){return (2 - 1) >> 1;}" 0
test_aqcc_experiment "int main(){return 1<2;}" 1
test_aqcc_experiment "int main(){return 1 < 2;}" 1
test_aqcc_experiment "int main(){return 4 < 2;}" 0
test_aqcc_experiment "int main(){return 1>2;}" 0
test_aqcc_experiment "int main(){return 1 > 2;}" 0
test_aqcc_experiment "int main(){return 4 > 2;}" 1
test_aqcc_experiment "int main(){return 1<=2;}" 1
test_aqcc_experiment "int main(){return 1 <= 2;}" 1
test_aqcc_experiment "int main(){return 4 <= 2;}" 0
test_aqcc_experiment "int main(){return 2 <= 2;}" 1
test_aqcc_experiment "int main(){return 1>=2;}" 0
test_aqcc_experiment "int main(){return 1 >= 2;}" 0
test_aqcc_experiment "int main(){return 4 >= 2;}" 1
test_aqcc_experiment "int main(){return 2 >= 2;}" 1
test_aqcc_experiment "int main(){return (2 < 1) + 1;}" 1
test_aqcc_experiment "int main(){return 1==2;}" 0
test_aqcc_experiment "int main(){return 1 == 2;}" 0
test_aqcc_experiment "int main(){return 4 == 2;}" 0
test_aqcc_experiment "int main(){return 2 == 2;}" 1
test_aqcc_experiment "int main(){return 1!=2;}" 1
test_aqcc_experiment "int main(){return 1 != 2;}" 1
test_aqcc_experiment "int main(){return 4 != 2;}" 1
test_aqcc_experiment "int main(){return 2 != 2;}" 0
test_aqcc_experiment "int main(){return 0&0;}" 0
test_aqcc_experiment "int main(){return 0 & 0;}" 0
test_aqcc_experiment "int main(){return 1 & 0;}" 0
test_aqcc_experiment "int main(){return 0 & 1;}" 0
test_aqcc_experiment "int main(){return 1 & 1;}" 1
test_aqcc_experiment "int main(){return 1 & 2;}" 0
test_aqcc_experiment "int main(){return 2 & 2;}" 2
test_aqcc_experiment "int main(){return 3 & 5;}" 1
test_aqcc_experiment "int main(){return 0^0;}" 0
test_aqcc_experiment "int main(){return 0 ^ 0;}" 0
test_aqcc_experiment "int main(){return 1 ^ 0;}" 1
test_aqcc_experiment "int main(){return 0 ^ 1;}" 1
test_aqcc_experiment "int main(){return 1 ^ 1;}" 0
test_aqcc_experiment "int main(){return 1 ^ 2;}" 3
test_aqcc_experiment "int main(){return 2 ^ 2;}" 0
test_aqcc_experiment "int main(){return 3 ^ 5;}" 6
test_aqcc_experiment "int main(){return 0|0;}" 0
test_aqcc_experiment "int main(){return 0 | 0;}" 0
test_aqcc_experiment "int main(){return 1 | 0;}" 1
test_aqcc_experiment "int main(){return 0 | 1;}" 1
test_aqcc_experiment "int main(){return 1 | 1;}" 1
test_aqcc_experiment "int main(){return 1 | 2;}" 3
test_aqcc_experiment "int main(){return 2 | 2;}" 2
test_aqcc_experiment "int main(){return 3 | 5;}" 7
test_aqcc_experiment "int main(){return 1&&0;}" 0
test_aqcc_experiment "int main(){return 1 && 1;}" 1
test_aqcc_experiment "int main(){return 0 && 1;}" 0
test_aqcc_experiment "int main(){return 0 && 0;}" 0
test_aqcc_experiment "int main(){return 2 && 1;}" 1
test_aqcc_experiment "int main(){return -2 || 1;}" 1
test_aqcc_experiment "int main(){return 1||0;}" 1
test_aqcc_experiment "int main(){return 1 || 1;}" 1
test_aqcc_experiment "int main(){return 0 || 1;}" 1
test_aqcc_experiment "int main(){return 0 || 0;}" 0
test_aqcc_experiment "int main(){return 2 || 1;}" 1
test_aqcc_experiment "int main(){return -2 || 1;}" 1
test_aqcc_experiment "int main(){int x;return x=1;}" 1
test_aqcc_experiment "int main(){int xy;return xy = 100+100;}" 200
test_aqcc_experiment "int main(){int a_b;return a_b = - ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc_experiment "int main(){int _;return _ = (2 - 1) << 1;}" 2
test_aqcc_experiment "int main(){int x = 1;int y; return y = 2;}" 2
test_aqcc_experiment "int main(){int x = 1; int y = 2; int z; return z = x + y;}" 3
test_aqcc_experiment "int main(){int a0 = 1;int a1 = 1;int a2 = a0 + a1; int a3; return a3 = a1 + a2;}" 3
test_aqcc_experiment "int main(){int x;int y;int z;x = y = 1; return z = x = x + y;}" 2
test_aqcc_experiment "int main(){if(0 == 1)return 0;return 1;}" 1
test_aqcc_experiment "int main(){if(0 == 1)return 0;else return 1;}" 1
test_aqcc_experiment "int main(){if(0 == 1){return 0;}else{return 1;}}" 1
test_aqcc_experiment "int main(){int a=0;if(a == 0){a = 1;a = a + a;}if(a == 3){a = 3;}else{a = 4;}return a;}" 4
test_aqcc_experiment "int main(){int a=2;if(a==1)return 0;else if(a==2)return 1;}" 1
test_aqcc_experiment "int main(){int a=0;while(a!=10)a=a+1;return a;}" 10
test_aqcc_experiment "int main(){int a=0;while(a!=10){int b;b=1;a=a+1;}return a;}" 10
test_aqcc_experiment "int main(){int a=0;while(a!=10){if(a==5)break;a=a+1;}return a;}" 5
test_aqcc_experiment "int main(){int a=0;while(a<5){a=a+1;if(a==5)continue;a=a+1;}return a;}" 5
test_aqcc_experiment "int main(){int a=0;int i;for(i=0;i<=10;i=i+1){a=a+i;}return a;}" 55
test_aqcc_experiment "int main(){int a=0;for(;;){a=a+1;if(a>=10)break;}return a;}" 10
test_aqcc_experiment "int main(){int a=0;for(;;a=a+1)if(a<10)continue;else break;return a;}" 10;
test_aqcc_experiment "int main(){int a=0;a++;return a;}" 1
test_aqcc_experiment "int main(){int a=0;int i;for(i=0;i<=10;i++){a=a+i;}return a;}" 55
test_aqcc_experiment "int main(){int a=0;int b;b=(a++)+1;return b;}" 1
test_aqcc_experiment "int main(){int a=0;int b;b=(++a)+1;return b;}" 2
test_aqcc_experiment "int main(){int a=0;int i;for(i=0;i<=10;++i){a=a+i;}return a;}" 55
test_aqcc_experiment "int main(){int a=0;if(a==0){int a;a=1;}return a;}" 0
test_aqcc_experiment "int main(){int a=0;int i;for(i=0;i<10;i++){int a;a=1;}return a;}" 0
test_aqcc_experiment "int main(){int a=0;int i;for(i=0;i<10;i++){int i;for(i=0;i<10;i++){a=a+1;}}return a;}" 100
test_aqcc_experiment "int main() { int n = n++; return n; }" 0
test_aqcc_experiment "int main() { int n = 0; return *&n; }" 0
test_aqcc_experiment "int main() { int n = 0; int i; int j; for(i = 0; i < 10; i++) for (j = 0; j < 10; j++) n++; return n; }" 100
test_aqcc_experiment "int main() { int ary[10]; *(ary + 5) = 4; return *(ary + 5); }" 4
test_aqcc_experiment "int main() { int ary[10][10]; *(*(ary + 4) + 5) = 9; return *(*(ary + 4) + 5); }" 9
test_aqcc_experiment "int main() { int ary[10]; int i; for (i = 0; i < 10; i++) *(i + ary) = i; return *(ary + 5); }" 5
test_aqcc_experiment "int main() { int ary[10][10]; int i; int j; for (i = 0; i < 10; i++) for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j; return *(*(ary + 6) + 4); }" 2
test_aqcc_experiment "int main() { int ary[10]; ary[5] = 10; return ary[5]; }" 10
test_aqcc_experiment "int main() { int ary[10][10]; ary[4][5] = 9; return ary[4][5]; }" 9
test_aqcc_experiment "int main() { int ary[10]; int i; for (i = 0; i < 10; i++) ary[i] = i; return ary[5]; }" 5
test_aqcc_experiment "int main() { int ary[10][10]; int i; int j; for (i = 0; i < 10; i++) for (j = 0; j < 10; j++) ary[i][j] = i - j; return ary[6][4]; }" 2
test_aqcc_experiment "int main() { int ary[10]; 3[ary] = 4; return 3[ary]; }" 4
test_aqcc_experiment "int main() { int a = 2; a += 5; return a; }" 7
test_aqcc_experiment "int main() { int a = 8; a -= 5; return a; }" 3
test_aqcc_experiment "int main() { int a = 2; a *= 5; return a; }" 10
test_aqcc_experiment "int main() { int a = 10; a /= 5; return a; }" 2
test_aqcc_experiment "int main() { int a = 12; a &= 5; return a; }" 4
test_aqcc_experiment "int main() { int a = 12; a %= 5; return a; }" 2
test_aqcc_experiment "int main() { int a = 2; a |= 5; return a; }" 7
test_aqcc_experiment "int main() { int a = 2; a ^= 5; return a; }" 7
test_aqcc_experiment "int main() { int a = 2; a <<= 2; return a; }" 8
test_aqcc_experiment "int main() { int a = 4; a >>= 2; return a; }" 1
test_aqcc_experiment "int main() { char c = 1; return c; }" 1
test_aqcc_experiment "int main(){char x;return x=1;}" 1
test_aqcc_experiment "int main(){char xy;return xy = 100+100;}" 200
test_aqcc_experiment "int main(){char a_b;return a_b = - ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc_experiment "int main(){char _;return _ = (2 - 1) << 1;}" 2
test_aqcc_experiment "int main(){char x = 1;char y; return y = 2;}" 2
test_aqcc_experiment "int main(){char x = 1;char y = 2;char z; return z = x + y;}" 3
test_aqcc_experiment "int main(){char a0;char a1;char a2;char a3;a0 = 1; a1 = 1; a2 = a0 + a1; return a3 = a1 + a2;}" 3
test_aqcc_experiment "int main(){char x;char y;char z;x = y = 1; return z = x = x + y;}" 2
test_aqcc_experiment "int main() { char a = 2; a += 5; return a; }" 7
test_aqcc_experiment "int main() { char a = 8; a -= 5; return a; }" 3
test_aqcc_experiment "int main() { char a = 2; a *= 5; return a; }" 10
test_aqcc_experiment "int main() { char a = 10; a /= 5; return a; }" 2
test_aqcc_experiment "int main() { char a = 12; a &= 5; return a; }" 4
test_aqcc_experiment "int main() { char a = 12; a %= 5; return a; }" 2
test_aqcc_experiment "int main() { char a = 2; a |= 5; return a; }" 7
test_aqcc_experiment "int main() { char a = 2; a ^= 5; return a; }" 7
test_aqcc_experiment "int main() { char a = 2; a <<= 2; return a; }" 8
test_aqcc_experiment "int main() { char a = 4; a >>= 2; return a; }" 1
test_aqcc_experiment "int main() { char ary[10]; *(ary + 5) = 4; return *(ary + 5); }" 4
test_aqcc_experiment "int main() { char ary[10][10]; *(*(ary + 4) + 5) = 9; return *(*(ary + 4) + 5); }" 9
test_aqcc_experiment "int main() { char ary[10]; int i; for (i = 0; i < 10; i++) *(i + ary) = i; return *(ary + 5); }" 5
test_aqcc_experiment "int main() { char ary[10][10]; int i; int j; for (i = 0; i < 10; i++) for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j; return *(*(ary + 6) + 4); }" 2
test_aqcc_experiment "int main() { char ary[10]; ary[5] = 10; return ary[5]; }" 10
test_aqcc_experiment "int main() { char ary[10][10]; ary[4][5] = 9; return ary[4][5]; }" 9
test_aqcc_experiment "int main() { char ary[10]; int i; for (i = 0; i < 10; i++) ary[i] = i; return ary[5]; }" 5
test_aqcc_experiment "int main() { char ary[10][10]; int i; int j; for (i = 0; i < 10; i++) for (j = 0; j < 10; j++) ary[i][j] = i - j; return ary[6][4]; }" 2
test_aqcc_experiment "int main() { char ary[10]; 3[ary] = 4; return 3[ary]; }" 4
test_aqcc_experiment "int main() { return sizeof(int); }" 4
test_aqcc_experiment "int main() { return sizeof(char); }" 1
test_aqcc_experiment "int main() { return sizeof(int*); }" 8
test_aqcc_experiment "int main() { return sizeof(char*); }" 8
test_aqcc_experiment "int main() { return sizeof(int***); }" 8
test_aqcc_experiment "int main() { return sizeof(char**); }" 8
test_aqcc_experiment "int main() { int a; return sizeof(a); }" 4
test_aqcc_experiment "int main() { char a; return sizeof(a); }" 1
test_aqcc_experiment "int main() { int *a; return sizeof(a); }" 8
test_aqcc_experiment "int main() { char *a; return sizeof(a); }" 8
test_aqcc_experiment "int main() { int a[20]; return sizeof(a); }" 80
test_aqcc_experiment "int main() { int a[5][6]; return sizeof(a); }" 120
test_aqcc_experiment "int main() { int a[5][3][2]; return sizeof(a); }" 120
test_aqcc_experiment "int main() { char a[20]; return sizeof(a); }" 20
test_aqcc_experiment "int main() { char a[5][6]; return sizeof(a); }" 30
test_aqcc_experiment "int main() { char a[5][6][4]; return sizeof(a); }" 120
test_aqcc_experiment "int main() { char a[2][4][3][5]; return sizeof(a); }" 120
test_aqcc_experiment "int a; int main() { return a; }" 0
test_aqcc_experiment "int a; int main() { a = 4; return a; }" 4
test_aqcc_experiment "int a; int *p; int main() { p = &a; return a; }" 0
test_aqcc_experiment "int a; int *p; int main() { p = &a; *p = 3; return a; }" 3
test_aqcc_experiment "int a[20]; int main() { a[5] = 5; return a[5]; }" 5
test_aqcc_experiment "int a[20][10]; int main() { a[3][5] = 10; return a[3][5]; }" 10
test_aqcc_experiment "char a; int main() { return a; }" 0
test_aqcc_experiment "char a; int main() { a = 4; return a; }" 4
test_aqcc_experiment "char a; int *p; int main() { p = &a; return a; }" 0
test_aqcc_experiment "char a; int *p; int main() { p = &a; *p = 3; return a; }" 3
test_aqcc_experiment "char a[20]; int main() { a[5] = 5; return a[5]; }" 5
test_aqcc_experiment "char a[20][10]; int main() { a[3][5] = 10; return a[3][5]; }" 10
test_aqcc_experiment "int a; int main() { return sizeof(a); }" 4
test_aqcc_experiment "char a; int main() { return sizeof(a); }" 1
test_aqcc_experiment "int *a; int main() { return sizeof(a); }" 8
test_aqcc_experiment "char *a; int main() { return sizeof(a); }" 8
test_aqcc_experiment "int a[20]; int main() { return sizeof(a); }" 80
test_aqcc_experiment "int a[5][6]; int main() { return sizeof(a); }" 120
test_aqcc_experiment "int a[5][3][2]; int main() { return sizeof(a); }" 120
test_aqcc_experiment "char a[20]; int main() { return sizeof(a); }" 20
test_aqcc_experiment "char a[5][6]; int main() { return sizeof(a); }" 30
test_aqcc_experiment "char a[5][6][4]; int main() { return sizeof(a); }" 120
test_aqcc_experiment "int a = 4; int main() { return a; }" 4
test_aqcc_experiment "char a = 4; int main() { return a; }" 4
test_aqcc_experiment "int *a = 0; int main() { return a; }" 0
test_aqcc_experiment "char *a = 0; int main() { return a; }" 0
test_aqcc_experiment "char c, d; int a, b; char e; int main() { c = 4; d = 2; e = 10; a = 1; b = 2; return c * d - e + b; }" 0
test_aqcc_experiment 'int main() { char *str; str = "abc"; return str[0]; }' 97
test_aqcc_experiment 'int main() { return sizeof("foo"); }' 4
test_aqcc_experiment 'int main() { char *str = "abc\0abc"; return str[3]; }' 0
test_aqcc_experiment 'int main() { return sizeof("\t"); }' 2
test_aqcc_experiment "int main() { return 'A'; }" 65
test_aqcc_experiment "int main() { return 'a'; }" 97
test_aqcc_experiment "int main() { return '\t'; }" 9
test_aqcc_experiment "int main() { return '\0'; }" 0
test_aqcc_experiment "int main() { return '\n'; }" 10
test_aqcc_experiment "int main2() { return 0; } int main() { return '\n'; }" 10

