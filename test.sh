#!/bin/sh

function fail(){
    echo -e "\e[1;31m[ERROR]\e[0m $1"
    exit 1
}

function test_aqcc() {
    echo "$1" > _test.in
    cat _test.in | ./aqcc > _test.s
    [ $? -eq 0 ] || fail "test_aqcc \"$1\": ./aqcc > _test.s"
    gcc _test.s -o _test.o testutil.o -no-pie
    [ $? -eq 0 ] || fail "test_aqcc \"$1\": gcc _test.s -o _test.o"
    ./_test.o
    res=$?
    [ $res -eq $2 ] || fail "test_aqcc \"$1\" -> $res (expected $2)"
}

./aqcc test
[ $? -eq 0 ] || fail "./main test"

test_aqcc "int main(){return 2;}" 2
test_aqcc "int main(){return 22;}" 22
test_aqcc "int main(){return 2+2;}" 4
test_aqcc "int main(){return 11+11+11;}" 33
test_aqcc "int main(){return 5-3;}" 2
test_aqcc "int main(){return 35-22;}" 13
test_aqcc "int main(){return 35-22-11;}" 2
test_aqcc "int main(){return 199-23+300-475;}" 1
test_aqcc "int main(){return 1+4-3;}" 2
test_aqcc "int main(){return 1983+2-449-3123+1893-32+223-396;}" 101
test_aqcc "int main(){return 2*2;}" 4
test_aqcc "int main(){return 11*11;}" 121
test_aqcc "int main(){return 4/2;}" 2
test_aqcc "int main(){return 363/121;}" 3
test_aqcc "int main(){return 100/3;}" 33
test_aqcc "int main(){return 1+2*3;}" 7
test_aqcc "int main(){return 1+4*2-9/3;}" 6
test_aqcc "int main(){return 4%2;}" 0
test_aqcc "int main(){return 5%2;}" 1
test_aqcc "int main(){return 1935%10;}" 5
test_aqcc "int main(){return (1+2)*3;}" 9
test_aqcc "int main(){return (1+2)*(1+2);}" 9
test_aqcc "int main(){return (1+2)/(1+2);}" 1
test_aqcc "int main(){return 33*(1+2);}" 99
test_aqcc "int main(){return (33*(1+2))/3;}" 33
test_aqcc "int main(){return -3+5;}" 2
test_aqcc "int main(){return +4;}" 4
test_aqcc "int main(){return -(33*(1+2))/3+34;}" 1
test_aqcc "int main(){return 4 + 4;}" 8
test_aqcc "int main(){return - ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc "int main(){return 2 << 1;}" 4
test_aqcc "int main(){return 2 << 2 << 1;}" 16
test_aqcc "int main(){return 2 << (2 << 1);}" 32
test_aqcc "int main(){return (2 - 1) << 1;}" 2
test_aqcc "int main(){return 2 >> 1;}" 1
test_aqcc "int main(){return 4 >> 2 >> 1;}" 0
test_aqcc "int main(){return (2 - 1) >> 1;}" 0
test_aqcc "int main(){return 1<2;}" 1
test_aqcc "int main(){return 1 < 2;}" 1
test_aqcc "int main(){return 4 < 2;}" 0
test_aqcc "int main(){return 1>2;}" 0
test_aqcc "int main(){return 1 > 2;}" 0
test_aqcc "int main(){return 4 > 2;}" 1
test_aqcc "int main(){return 1<=2;}" 1
test_aqcc "int main(){return 1 <= 2;}" 1
test_aqcc "int main(){return 4 <= 2;}" 0
test_aqcc "int main(){return 2 <= 2;}" 1
test_aqcc "int main(){return 1>=2;}" 0
test_aqcc "int main(){return 1 >= 2;}" 0
test_aqcc "int main(){return 4 >= 2;}" 1
test_aqcc "int main(){return 2 >= 2;}" 1
test_aqcc "int main(){return (2 < 1) + 1;}" 1
test_aqcc "int main(){return 1==2;}" 0
test_aqcc "int main(){return 1 == 2;}" 0
test_aqcc "int main(){return 4 == 2;}" 0
test_aqcc "int main(){return 2 == 2;}" 1
test_aqcc "int main(){return 1!=2;}" 1
test_aqcc "int main(){return 1 != 2;}" 1
test_aqcc "int main(){return 4 != 2;}" 1
test_aqcc "int main(){return 2 != 2;}" 0
test_aqcc "int main(){return 0&0;}" 0
test_aqcc "int main(){return 0 & 0;}" 0
test_aqcc "int main(){return 1 & 0;}" 0
test_aqcc "int main(){return 0 & 1;}" 0
test_aqcc "int main(){return 1 & 1;}" 1
test_aqcc "int main(){return 1 & 2;}" 0
test_aqcc "int main(){return 2 & 2;}" 2
test_aqcc "int main(){return 3 & 5;}" 1
test_aqcc "int main(){return 0^0;}" 0
test_aqcc "int main(){return 0 ^ 0;}" 0
test_aqcc "int main(){return 1 ^ 0;}" 1
test_aqcc "int main(){return 0 ^ 1;}" 1
test_aqcc "int main(){return 1 ^ 1;}" 0
test_aqcc "int main(){return 1 ^ 2;}" 3
test_aqcc "int main(){return 2 ^ 2;}" 0
test_aqcc "int main(){return 3 ^ 5;}" 6
test_aqcc "int main(){return 0|0;}" 0
test_aqcc "int main(){return 0 | 0;}" 0
test_aqcc "int main(){return 1 | 0;}" 1
test_aqcc "int main(){return 0 | 1;}" 1
test_aqcc "int main(){return 1 | 1;}" 1
test_aqcc "int main(){return 1 | 2;}" 3
test_aqcc "int main(){return 2 | 2;}" 2
test_aqcc "int main(){return 3 | 5;}" 7
test_aqcc "int main(){return 1&&0;}" 0
test_aqcc "int main(){return 1 && 1;}" 1
test_aqcc "int main(){return 0 && 1;}" 0
test_aqcc "int main(){return 0 && 0;}" 0
test_aqcc "int main(){return 2 && 1;}" 1
test_aqcc "int main(){return -2 || 1;}" 1
test_aqcc "int main(){return 1||0;}" 1
test_aqcc "int main(){return 1 || 1;}" 1
test_aqcc "int main(){return 0 || 1;}" 1
test_aqcc "int main(){return 0 || 0;}" 0
test_aqcc "int main(){return 2 || 1;}" 1
test_aqcc "int main(){return -2 || 1;}" 1
test_aqcc "int main(){int x;return x=1;}" 1
test_aqcc "int main(){int xy;return xy = 100+100;}" 200
test_aqcc "int main(){int a_b;return a_b = - ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc "int main(){int _;return _ = (2 - 1) << 1;}" 2
test_aqcc "int main(){int x;int y;x = 1; return y = 2;}" 2
test_aqcc "int main(){int x;int y;int z;x = 1; y = 2; return z = x + y;}" 3
test_aqcc "int main(){int a0;int a1;int a2;int a3;a0 = 1; a1 = 1; a2 = a0 + a1; return a3 = a1 + a2;}" 3
test_aqcc "int main(){int x;int y;int z;x = y = 1; return z = x = x + y;}" 2
test_aqcc "int ret0();int main(){return ret0();}" 0
test_aqcc "int ret1();int ret0();int main(){return (ret0() + ret1()) * 2;}" 2
test_aqcc "int ret1();int ret0();int main(){return (ret0() * ret1()) + 2;}" 2
test_aqcc "int add1();int main(){return add1(1);}" 2
test_aqcc "int add_two();int main(){return add_two(1, 2);}" 3
test_aqcc "int add_all();int main(){return add_all(1, 2, 4, 8, 16, 32, 64, 128);}" 1
test_aqcc "int iret0(){return 0;}main(){return iret0();}" 0
test_aqcc "int iret0(){return 0;}iret1(){return 1;}main(){return iret0() + iret1();}" 1
test_aqcc "int main(){return;}" 0
test_aqcc "int main(){;}" 0
test_aqcc "int iadd(int a, int b) { return a + b; }main(){return iadd(1, 2);}" 3
test_aqcc "int iadd(int a, int b) { return a + b; }main(){return iadd(1, 2) * iadd(2, 3);}" 15
test_aqcc "int eighth(int a, int b, int c, int d, int e, int f, int g, int h){return h;}main(){return eighth(1, 2, 3, 4, 5, 6, 7, 8);}" 8
test_aqcc "int main(){return 0 == 0 ? 0 : 1;}" 0
test_aqcc "int fib(int n){return n == 0 ? 0 : n == 1 ? 1 : fib(n - 1) + fib(n - 2);}int main(){return fib(5);}" 5
test_aqcc "int main(){if(0 == 1)return 0;return 1;}" 1
test_aqcc "int main(){if(0 == 1)return 0;else return 1;}" 1
test_aqcc "int main(){if(0 == 1){return 0;}else{return 1;}}" 1
test_aqcc "int fib(int n){if(n<=1)return n;return fib(n-1)+fib(n-2);}int main(){return fib(0);}" 0
test_aqcc "int fib(int n){if(n<=1)return n;return fib(n-1)+fib(n-2);}int main(){return fib(1);}" 1
test_aqcc "int fib(int n){if(n<=1)return n;return fib(n-1)+fib(n-2);}int main(){return fib(5);}" 5
test_aqcc "int main(){int a;a=0;if(a == 0){a = 1;a = a + a;}if(a == 3){a = 3;}else{a = 4;}return a;}" 4
# thanks to @hsjoihs
test_aqcc "int foo(){return 2;}int bar(){return 7;}int main(){int a;a=3;int b;b=5;int c;c=2;if(a)if(0){b=foo();}else{c=bar();}return 162+b+c;}" 174
test_aqcc "int main(){int a;a=2;if(a==1)return 0;else if(a==2)return 1;}" 1
test_aqcc "int main(){int a;a=0;while(a!=10)a=a+1;return a;}" 10
test_aqcc "int main(){int a;a=0;while(a!=10){int b;b=1;a=a+1;}return a;}" 10
test_aqcc "int main(){int a;a=0;while(a!=10){if(a==5)break;a=a+1;}return a;}" 5
test_aqcc "int main(){int a;a=0;while(a<5){a=a+1;if(a==5)continue;a=a+1;}return a;}" 5
test_aqcc "int main(){int a;a=0;int i;for(i=0;i<=10;i=i+1){a=a+i;}return a;}" 55
test_aqcc "int main(){int a;a=0;for(;;){a=a+1;if(a>=10)break;}return a;}" 10
test_aqcc "int main(){int a;a=0;for(;;a=a+1)if(a<10)continue;else break;return a;}" 10;
test_aqcc "int main(){int a;a=0;a++;return a;}" 1
test_aqcc "int main(){int a;a=0;int i;for(i=0;i<=10;i++){a=a+i;}return a;}" 55
test_aqcc "int main(){int a;a=0;int b;b=(a++)+1;return b;}" 1
test_aqcc "int main(){int a;a=0;int b;b=(++a)+1;return b;}" 2
test_aqcc "int main(){int a;a=0;int i;for(i=0;i<=10;++i){a=a+i;}return a;}" 55
test_aqcc "int main(){int a;a=0;if(a==0){int a;a=1;}return a;}" 0
test_aqcc "int main(){int a;a=0;int i;for(i=0;i<10;i++){int a;a=1;}return a;}" 0
test_aqcc "int main(){int a;a=0;int i;for(i=0;i<10;i++){int i;for(i=0;i<10;i++){a=a+1;}}return a;}" 100
test_aqcc "main(){return 0;}" 0 # function without return value should be also valid.
test_aqcc "main(){int x;x=3;int *y;y=&x;return *y;}" 3
test_aqcc "main(){int x;x=3;int *y;y=&x;*y=10;return x;}" 10
test_aqcc "main(){int *x;int **y;int z;x=&z;y=&x;**y=1;return z;}" 1
test_aqcc "int *test(int *p) { *p = 1; return p; } int main() { int x; int *y; y = test(&x); return *y; }" 1
test_aqcc "int *alloc4(int **p);int main() { int *p; alloc4(&p); int *q; q = p + 2; return *q; }" 12
test_aqcc "int *alloc4(int **p);int main() { int *p; alloc4(&p); int *q; q = p + 3; return *q; }" 13
test_aqcc "int *alloc4(int **p);int main() { int *p; return *((1 + 1) + alloc4(&p)); }" 12
test_aqcc "int *alloc4(int **p);int main() { int *p; alloc4(&p); int *q; q = p + 2; return q - p; }" 2
test_aqcc "int *alloc4(int **p);int main() { int *p; alloc4(&p); int *q; q = p + 2; q = q - 1; return q - p; }" 1
test_aqcc "int *alloc4(int **p);int main() { int *p; alloc4(&p); int *q; q = p + 2; return ((q + 1) - p) == (q - p) + 1; }" 1
test_aqcc "int *alloc4(int **p);int main() { int *p; alloc4(&p); int *q; q = p + 2; return (q - p) + 1 == -(p - (q + 1)); }" 1
test_aqcc "int *alloc4(int **p);int main() { int *p; alloc4(&p); p++; return *p; }" 11
test_aqcc "int *alloc4(int **p);int main() { int *p; alloc4(&p); ++p; return *p; }" 11
test_aqcc "int main() { int n; n = n++; return n; }" 0
test_aqcc "int main() { int n; n = 0; return *&n; }" 0
test_aqcc "int main() { int n; n = 0; int i; int j; for(i = 0; i < 10; i++) for (j = 0; j < 10; j++) n++; return n; }" 100
test_aqcc "int main() { int ary[10]; *(ary + 5) = 4; return *(ary + 5); }" 4
test_aqcc "int main() { int ary[10][10]; *(*(ary + 4) + 5) = 9; return *(*(ary + 4) + 5); }" 9
test_aqcc "int main() { int ary[10]; int i; for (i = 0; i < 10; i++) *(i + ary) = i; return *(ary + 5); }" 5
test_aqcc "int main() { int ary[10][10]; int i; int j; for (i = 0; i < 10; i++) for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j; return *(*(ary + 6) + 4); }" 2
test_aqcc "int main() { int ary[10]; ary[5] = 10; return ary[5]; }" 10
test_aqcc "int main() { int ary[10][10]; ary[4][5] = 9; return ary[4][5]; }" 9
test_aqcc "int main() { int ary[10]; int i; for (i = 0; i < 10; i++) ary[i] = i; return ary[5]; }" 5
test_aqcc "int main() { int ary[10][10]; int i; int j; for (i = 0; i < 10; i++) for (j = 0; j < 10; j++) ary[i][j] = i - j; return ary[6][4]; }" 2
test_aqcc "int main() { int ary[10]; 3[ary] = 4; return 3[ary]; }" 4
test_aqcc "int fib(int n){if (n <= 1) return n; int p0; int p1; p0 = fib(n - 1); p1 = fib(n - 2); return p0 + p1; } int main(){return fib(5);}" 5
test_aqcc "int main() { int a; a = 2; a += 5; return a; }" 7
test_aqcc "int main() { int a; a = 8; a -= 5; return a; }" 3
test_aqcc "int main() { int a; a = 2; a *= 5; return a; }" 10
test_aqcc "int main() { int a; a = 10; a /= 5; return a; }" 2
test_aqcc "int main() { int a; a = 12; a &= 5; return a; }" 4
test_aqcc "int main() { int a; a = 12; a %= 5; return a; }" 2
test_aqcc "int main() { int a; a = 2; a |= 5; return a; }" 7
test_aqcc "int main() { int a; a = 2; a ^= 5; return a; }" 7
test_aqcc "int main() { int a; a = 2; a <<= 2; return a; }" 8
test_aqcc "int main() { int a; a = 4; a >>= 2; return a; }" 1
test_aqcc "int a; int main() { return a; }" 0
test_aqcc "int a; int main() { a = 4; return a; }" 4
test_aqcc "int a; int *p; int main() { p = &a; return a; }" 0
test_aqcc "int a; int *p; int main() { p = &a; *p = 3; return a; }" 3
test_aqcc "int a[20]; int main() { a[5] = 5; return a[5]; }" 5
test_aqcc "int a[20][10]; int main() { a[3][5] = 10; return a[3][5]; }" 10
test_aqcc "int main() { char c; c = 1; return c; }" 1
test_aqcc "int main(){char x;return x=1;}" 1
test_aqcc "int main(){char xy;return xy = 100+100;}" 200
test_aqcc "int main(){char a_b;return a_b = - ( 33 * ( 1 + 2 ) ) / 3 + 34;}" 1
test_aqcc "int main(){char _;return _ = (2 - 1) << 1;}" 2
test_aqcc "int main(){char x;char y;x = 1; return y = 2;}" 2
test_aqcc "int main(){char x;char y;char z;x = 1; y = 2; return z = x + y;}" 3
test_aqcc "int main(){char a0;char a1;char a2;char a3;a0 = 1; a1 = 1; a2 = a0 + a1; return a3 = a1 + a2;}" 3
test_aqcc "int main(){char x;char y;char z;x = y = 1; return z = x = x + y;}" 2
test_aqcc "char iadd(char a, char b) { return a + b; }main(){return iadd(1, 2);}" 3
test_aqcc "char iadd(char a, char b) { return a + b; }main(){return iadd(1, 2) * iadd(2, 3);}" 15
test_aqcc "char eighth(char a, char b, char c, char d, char e, char f, char g, char h){return h;}main(){return eighth(1, 2, 3, 4, 5, 6, 7, 8);}" 8
test_aqcc "int eighth(char a, char b, char c, char d, char e, char f, char g, int h){return h;}main(){return eighth(1, 2, 3, 4, 5, 6, 7, 255);}" 255
test_aqcc "int main() { char a; a = 2; a += 5; return a; }" 7
test_aqcc "int main() { char a; a = 8; a -= 5; return a; }" 3
test_aqcc "int main() { char a; a = 2; a *= 5; return a; }" 10
test_aqcc "int main() { char a; a = 10; a /= 5; return a; }" 2
test_aqcc "int main() { char a; a = 12; a &= 5; return a; }" 4
test_aqcc "int main() { char a; a = 12; a %= 5; return a; }" 2
test_aqcc "int main() { char a; a = 2; a |= 5; return a; }" 7
test_aqcc "int main() { char a; a = 2; a ^= 5; return a; }" 7
test_aqcc "int main() { char a; a = 2; a <<= 2; return a; }" 8
test_aqcc "int main() { char a; a = 4; a >>= 2; return a; }" 1
test_aqcc "int main() { char ary[10]; *(ary + 5) = 4; return *(ary + 5); }" 4
test_aqcc "int main() { char ary[10][10]; *(*(ary + 4) + 5) = 9; return *(*(ary + 4) + 5); }" 9
test_aqcc "int main() { char ary[10]; int i; for (i = 0; i < 10; i++) *(i + ary) = i; return *(ary + 5); }" 5
test_aqcc "int main() { char ary[10][10]; int i; int j; for (i = 0; i < 10; i++) for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j; return *(*(ary + 6) + 4); }" 2
test_aqcc "int main() { char ary[10]; ary[5] = 10; return ary[5]; }" 10
test_aqcc "int main() { char ary[10][10]; ary[4][5] = 9; return ary[4][5]; }" 9
test_aqcc "int main() { char ary[10]; int i; for (i = 0; i < 10; i++) ary[i] = i; return ary[5]; }" 5
test_aqcc "int main() { char ary[10][10]; int i; int j; for (i = 0; i < 10; i++) for (j = 0; j < 10; j++) ary[i][j] = i - j; return ary[6][4]; }" 2
test_aqcc "int main() { char ary[10]; 3[ary] = 4; return 3[ary]; }" 4
test_aqcc "char a; int main() { return a; }" 0
test_aqcc "char a; int main() { a = 4; return a; }" 4
test_aqcc "char a; int *p; int main() { p = &a; return a; }" 0
test_aqcc "char a; int *p; int main() { p = &a; *p = 3; return a; }" 3
test_aqcc "char a[20]; int main() { a[5] = 5; return a[5]; }" 5
test_aqcc "char a[20][10]; int main() { a[3][5] = 10; return a[3][5]; }" 10
test_aqcc "int main() { return sizeof(int); }" 4
test_aqcc "int main() { return sizeof(char); }" 1
test_aqcc "int main() { return sizeof(int*); }" 8
test_aqcc "int main() { return sizeof(char*); }" 8
test_aqcc "int main() { return sizeof(int***); }" 8
test_aqcc "int main() { return sizeof(char**); }" 8
test_aqcc "int main() { int a; return sizeof(a); }" 4
test_aqcc "int main() { char a; return sizeof(a); }" 1
test_aqcc "int main() { int *a; return sizeof(a); }" 8
test_aqcc "int main() { char *a; return sizeof(a); }" 8
test_aqcc "int main() { int a[20]; return sizeof(a); }" 80
test_aqcc "int main() { int a[5][6]; return sizeof(a); }" 120
test_aqcc "int main() { int a[5][3][2]; return sizeof(a); }" 120
test_aqcc "int main() { char a[20]; return sizeof(a); }" 20
test_aqcc "int main() { char a[5][6]; return sizeof(a); }" 30
test_aqcc "int main() { char a[5][6][4]; return sizeof(a); }" 120
test_aqcc "int main() { char a[2][4][3][5]; return sizeof(a); }" 120
test_aqcc "int a; int main() { return sizeof(a); }" 4
test_aqcc "char a; int main() { return sizeof(a); }" 1
test_aqcc "int *a; int main() { return sizeof(a); }" 8
test_aqcc "char *a; int main() { return sizeof(a); }" 8
test_aqcc "int a[20]; int main() { return sizeof(a); }" 80
test_aqcc "int a[5][6]; int main() { return sizeof(a); }" 120
test_aqcc "int a[5][3][2]; int main() { return sizeof(a); }" 120
test_aqcc "char a[20]; int main() { return sizeof(a); }" 20
test_aqcc "char a[5][6]; int main() { return sizeof(a); }" 30
test_aqcc "char a[5][6][4]; int main() { return sizeof(a); }" 120
test_aqcc 'int main() { char *str; str = "abc"; return str[0]; }' 97
test_aqcc 'int main() { return printf("%d", 0); }' 1
test_aqcc 'char *test() { char *str; str = "abc"; return str; } int main() { char *str; str = test(); return str[1]; }' 98
