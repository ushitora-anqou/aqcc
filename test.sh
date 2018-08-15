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
test_aqcc_experiment "int main() { return 4 * 3; }" 12
test_aqcc_experiment "int main() { return 3 + 4 * 5 - 6 + 7 * 8 + 9 - 10; }" 72
test_aqcc_experiment "int main() { return 12 / 4; }" 3
test_aqcc_experiment "int main() { return 1 + 3 * 4 / 12 - 1; }" 1
test_aqcc_experiment "int main() { return 11 % 4; }" 3
test_aqcc_experiment "int main() { return 1 + 6 * 4 / 12 % 2 - 1; }" 0
test_aqcc_experiment "int main() { return -10 + 20; }" 10
test_aqcc_experiment "int main() { return ~1 + 2; }" 0
test_aqcc_experiment "int main() { return ~0+1; }" 0
test_aqcc_experiment "int main() { return 1 << 0; }" 1
test_aqcc_experiment "int main() { return 1 << 1; }" 2
test_aqcc_experiment "int main() { return 1 << 2; }" 4
test_aqcc_experiment "int main() { return 3 << 0; }" 3
test_aqcc_experiment "int main() { return 3 << 1; }" 6
test_aqcc_experiment "int main() { return 3 << 2; }" 12
test_aqcc_experiment "int main() { return 6 >> 0; }" 6
test_aqcc_experiment "int main() { return 6 >> 1; }" 3
test_aqcc_experiment "int main() { return 6 >> 2; }" 1
test_aqcc_experiment "int main() { return 10 >> 0; }" 10
test_aqcc_experiment "int main() { return 10 >> 1; }" 5
test_aqcc_experiment "int main() { return 10 >> 2; }" 2
test_aqcc_experiment "int main() { return 1 < 2; }" 1
test_aqcc_experiment "int main() { return 1 < 2; }" 1
test_aqcc_experiment "int main() { return 4 < 2; }" 0
test_aqcc_experiment "int main() { return 1 > 2; }" 0
test_aqcc_experiment "int main() { return 1 > 2; }" 0
test_aqcc_experiment "int main() { return 4 > 2; }" 1
test_aqcc_experiment "int main() { return 1 <= 2; }" 1
test_aqcc_experiment "int main() { return 1 <= 2; }" 1
test_aqcc_experiment "int main() { return 4 <= 2; }" 0
test_aqcc_experiment "int main() { return 2 <= 2; }" 1
test_aqcc_experiment "int main() { return 1 >= 2; }" 0
test_aqcc_experiment "int main() { return 1 >= 2; }" 0
test_aqcc_experiment "int main() { return 4 >= 2; }" 1
test_aqcc_experiment "int main() { return 2 >= 2; }" 1
test_aqcc_experiment "int main() { return (2 < 1) + 1; }" 1
test_aqcc_experiment "int main() { return 1 == 2; }" 0
test_aqcc_experiment "int main() { return 1 == 2; }" 0
test_aqcc_experiment "int main() { return 4 == 2; }" 0
test_aqcc_experiment "int main() { return 2 == 2; }" 1
test_aqcc_experiment "int main() { return 1 != 2; }" 1
test_aqcc_experiment "int main() { return 1 != 2; }" 1
test_aqcc_experiment "int main() { return 4 != 2; }" 1
test_aqcc_experiment "int main() { return 2 != 2; }" 0
test_aqcc_experiment "int main() { return 0 & 0; }" 0
test_aqcc_experiment "int main() { return 0 & 0; }" 0
test_aqcc_experiment "int main() { return 1 & 0; }" 0
test_aqcc_experiment "int main() { return 0 & 1; }" 0
test_aqcc_experiment "int main() { return 1 & 1; }" 1
test_aqcc_experiment "int main() { return 1 & 2; }" 0
test_aqcc_experiment "int main() { return 2 & 2; }" 2
test_aqcc_experiment "int main() { return 3 & 5; }" 1
test_aqcc_experiment "int main() { return 0 ^ 0; }" 0
test_aqcc_experiment "int main() { return 0 ^ 0; }" 0
test_aqcc_experiment "int main() { return 1 ^ 0; }" 1
test_aqcc_experiment "int main() { return 0 ^ 1; }" 1
test_aqcc_experiment "int main() { return 1 ^ 1; }" 0
test_aqcc_experiment "int main() { return 1 ^ 2; }" 3
test_aqcc_experiment "int main() { return 2 ^ 2; }" 0
test_aqcc_experiment "int main() { return 3 ^ 5; }" 6
test_aqcc_experiment "int main() { return 0 | 0; }" 0
test_aqcc_experiment "int main() { return 0 | 0; }" 0
test_aqcc_experiment "int main() { return 1 | 0; }" 1
test_aqcc_experiment "int main() { return 0 | 1; }" 1
test_aqcc_experiment "int main() { return 1 | 1; }" 1
test_aqcc_experiment "int main() { return 1 | 2; }" 3
test_aqcc_experiment "int main() { return 2 | 2; }" 2
test_aqcc_experiment "int main() { return 3 | 5; }" 7
test_aqcc_experiment "int main() { return 1 && 0; }" 0
test_aqcc_experiment "int main() { return 1 && 1; }" 1
test_aqcc_experiment "int main() { return 0 && 1; }" 0
test_aqcc_experiment "int main() { return 0 && 0; }" 0
test_aqcc_experiment "int main() { return 2 && 1; }" 1
test_aqcc_experiment "int main() { return -2 || 1; }" 1
test_aqcc_experiment "int main() { return 1 || 0; }" 1
test_aqcc_experiment "int main() { return 1 || 1; }" 1
test_aqcc_experiment "int main() { return 0 || 1; }" 1
test_aqcc_experiment "int main() { return 0 || 0; }" 0
test_aqcc_experiment "int main() { return 2 || 1; }" 1
test_aqcc_experiment "int main() { return -2 || 1; }" 1
test_aqcc_experiment "int testasdgasdg; int main() { return testasdgasdg; }" 0
test_aqcc_experiment "int main() { int a = 1; return a; }" 1
test_aqcc_experiment "int main() { int a = 1; return a + 3 + 5; }" 9
test_aqcc_experiment "int main() { int a = 0; return a++; }" 0
test_aqcc_experiment "int main() { int a = 0; return ++a; }" 1
test_aqcc_experiment "int main() { int a = 1; return a--; }" 1
test_aqcc_experiment "int main() { int a = 1; return --a; }" 0
test_aqcc_experiment "int main() { int a = 54, *b = &a; return *b; }" 54
test_aqcc_experiment "int main() {
    int a = 43;
    if (a == 43) return 0;
    return 1;
}" 0
test_aqcc_experiment "int main() { int a = 43; return a != 42 ? 1 : 0; }" 1
test_aqcc_experiment "
int main()
{
    int a = 4, ret = 2;
    switch (a) {
        case 1:
            ret = 1;
            break;
        case 4:
            ret = 4;
            break;
    }
    return ret;
}" 4
test_aqcc_experiment "
int main()
{
    int a = 4, ret;
    switch (a) {
        case 1:
            ret = 1;
        default:
            ret = 4;
    }
    return ret;
}" 4
test_aqcc_experiment "
int main()
{
    int a = 1, ret;
    switch (a) {
        int i = 0;
        case 1:
            i = 4;
        default:
            ret = i;
    }
    return ret;
}" 4
test_aqcc_experiment "
int main() {
    int i = 0;
    do {
        i++;
    } while (i < 5);
    return i;
}" 5
test_aqcc_experiment "
int main(){
    int a = 0;
    int i;
    for (i = 0; i <= 10; i = i + 1) {
        a = a + i;
    }
    return a;
}" 55
test_aqcc_experiment "
int main(){
    int sum = 0, i = 0;
    do {
        for (int i = 0; i < 5; i++) {
            sum++;
            if (sum >= 3) break;
        }
        if (sum == 4) continue;
        i++;
        if (sum == 5) break;
    } while (1);
    return sum == 5 && i == 2;
}" 1
test_aqcc_experiment "
int main()
{
    int i;
    for (i = 0; i < 10; i++) {
        switch (i)
        case 5:
            goto end_loop;
    }
end_loop:
    return i;
}" 5
test_aqcc_experiment "
int main()
{
    struct hoge {
        int a;
    };
    struct hoge a;
    a.a = 1;
    return a.a;
}" 1
test_aqcc_experiment "
int main()
{
    char ary[5];
    ary[0] = ary[1] = 10;
    return ary[0] + ary[1];
}" 20
test_aqcc_experiment "
int main()
{
    char a[10];
    *(a + 4) = 10;
    return *(a + 4);
}" 10
test_aqcc_experiment "
int test119fib(int n)
{
    return n == 0 ? 0 : n == 1 ? 1 : test119fib(n - 1) + test119fib(n - 2);
}
int main()
{
    return test119fib(5);
}" 5
test_aqcc_experiment "
int eighth(int a, int b, int c, int d, int e, int f, int g, int h) { return h; }
int main()
{
    return eighth(0, 1, 2, 3, 4, 5, 6, 7);
}" 7
test_aqcc_experiment "int main() { int i; return i = 1, 2, 3; }" 3
test_aqcc_experiment "
typedef struct {
    int gp_offset;
    int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];

#define va_start __builtin_va_start
#define va_end __builtin_va_end

int vsprintf(char *str, const char *format, va_list ap);
void test346vsprintf(char *p, char *str, ...)
{
    va_list args;
    va_start(args, str);
    vsprintf(p, str, args);
    va_end(args);
}

int strcmp(char *lhs, char *rhs);
int main()
{
    char buf[256];
    test346vsprintf(buf, \"test%s\", \"abc\");
    if (strcmp(buf, \"testabc\") == 0)  return 1;
    return 0;
}" 1
