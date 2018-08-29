int ret0();
int ret1();
int add1();
int add_two();
int add_all();
int iadd(int a, int b) { return a + b; }
iret1() { return 1; }
int iret0() { return 0; }
int *alloc4(int **p);
int eighth(int a, int b, int c, int d, int e, int f, int g, int h) { return h; }
int divdiv(int a, int b) { return a / b; }
int add_three(int a, int b) { return a + b + 3; }
int a;
char b;
int *p;
char *q;
typedef int Number;

struct hoge {
    int piyo;
};

union piyo {
    int *foo;
    int bar;
};

enum sugoienum {
    SE_A,
    SE_B = 1 + 2 - 2,
    SE_C,
    SE_D = 0,
    SE_E,
    SE_F,
};

int printf(char *str, ...);

#define EXPECT_INT(got, expect)                                          \
    do {                                                                 \
        int expect_int = (expect), got_int = (got);                      \
        int wrong_int = expect_int + 1;                                  \
        if (got_int != expect_int || wrong_int == got_int)               \
            printf("[ERROR] %d:%s: expect %d, got %d\n", __LINE__, #got, \
                   expect_int, got_int);                                 \
    } while (0)

void test097()
{
    int x;
    EXPECT_INT(x = 1, 1);
}
void test098()
{
    int xy;
    EXPECT_INT(xy = 100 + 100, 200);
}
void test099()
{
    int a_b;
    EXPECT_INT(a_b = -(33 * (1 + 2)) / 3 + 34, 1);
}
void test100()
{
    int _;
    EXPECT_INT(_ = (2 - 1) << 1, 2);
}
void test101()
{
    int x = 1;
    int y;
    EXPECT_INT(y = 2, 2);
}
void test102()
{
    int x = 1;
    int y = 2;
    int z;
    EXPECT_INT(z = x + y, 3);
}
void test103()
{
    int a0 = 1;
    int a1 = 1;
    int a2 = a0 + a1;
    int a3;
    EXPECT_INT(a3 = a1 + a2, 3);
}
void test104()
{
    int x;
    int y;
    int z;
    x = y = 1;
    EXPECT_INT(z = x = x + y, 2);
}
void test105() { EXPECT_INT(ret0(), 0); }
void test106() { EXPECT_INT((ret0() + ret1()) * 2, 2); }
void test107() { EXPECT_INT((ret0() * ret1()) + 2, 2); }
void test108() { EXPECT_INT(add1(1), 2); }
void test109() { EXPECT_INT(add_two(1, 2), 3); }
void test110() { EXPECT_INT(add_all(1, 2, 4, 8, 16, 32, 64, 128), 1); }
test111() { EXPECT_INT(iret0(), 0); }
test112() { EXPECT_INT(iret0() + iret1(), 1); }
int test113noreturn() {}
void test113() { EXPECT_INT(test113noreturn(), 0); }
test115() { EXPECT_INT(iadd(1, 2), 3); }
test116() { EXPECT_INT(iadd(1, 2) * iadd(2, 3), 15); }
test117() { EXPECT_INT(eighth(1, 2, 3, 4, 5, 6, 7, 8), 8); }
void test118() { EXPECT_INT(0 == 0 ? 0 : 1, 0); }
int test119fib(int n)
{
    return n == 0 ? 0 : n == 1 ? 1 : test119fib(n - 1) + test119fib(n - 2);
}
void test119() { EXPECT_INT(test119fib(5), 5); }
int test120detail()
{
    if (0 == 1) return 0;
    return 1;
}
void test120() { EXPECT_INT(test120detail(), 1); }
int test121detail()
{
    if (0 == 1)
        return 0;
    else
        return 1;
}
void test121() { EXPECT_INT(test121detail(), 1); }
int test122detail()
{
    if (0 == 1) {
        return 0;
    }
    else {
        return 1;
    }
}
void test122() { EXPECT_INT(test122detail(), 1); }
int test123fib(int n)
{
    if (n <= 1) return n;
    return test123fib(n - 1) + test123fib(n - 2);
}
void test123() { EXPECT_INT(test123fib(0), 0); }
int test124fib(int n)
{
    if (n <= 1) return n;
    return test124fib(n - 1) + test124fib(n - 2);
}
void test124() { EXPECT_INT(test124fib(1), 1); }
int test125fib(int n)
{
    if (n <= 1) return n;
    return test125fib(n - 1) + test125fib(n - 2);
}
void test125() { EXPECT_INT(test125fib(5), 5); }
void test126()
{
    int a = 0;
    if (a == 0) {
        a = 1;
        a = a + a;
    }
    if (a == 3) {
        a = 3;
    }
    else {
        a = 4;
    }
    EXPECT_INT(a, 4);
}
int test128foo() { return 2; }
int test128bar() { return 7; }
void test128()
{
    int a = 3;
    int b = 5;
    int c = 2;
    if (a)
        if (0) {
            b = test128foo();
        }
        else {
            c = test128bar();
        }
    EXPECT_INT(162 + b + c, 174);
}
int test129detail()
{
    int a = 2;
    if (a == 1)
        return 0;
    else if (a == 2)
        return 1;
}
void test129() { EXPECT_INT(test129detail(), 1); }
void test130()
{
    int a = 0;
    while (a != 10) a = a + 1;
    EXPECT_INT(a, 10);
}
void test131()
{
    int a = 0;
    while (a != 10) {
        int b;
        b = 1;
        a = a + 1;
    }
    EXPECT_INT(a, 10);
}
void test132()
{
    int a = 0;
    while (a != 10) {
        if (a == 5) break;
        a = a + 1;
    }
    EXPECT_INT(a, 5);
}
void test133()
{
    int a = 0;
    while (a < 5) {
        a = a + 1;
        if (a == 5) continue;
        a = a + 1;
    }
    EXPECT_INT(a, 5);
}
void test134()
{
    int a = 0;
    int i;
    for (i = 0; i <= 10; i = i + 1) {
        a = a + i;
    }
    EXPECT_INT(a, 55);
}
void test135()
{
    int a = 0;
    for (;;) {
        a = a + 1;
        if (a >= 10) break;
    }
    EXPECT_INT(a, 10);
}
void test136()
{
    int a = 0;
    for (;; a = a + 1)
        if (a < 10)
            continue;
        else
            break;
    EXPECT_INT(a, 10);
}
void test137()
{
    int a = 0;
    a++;
    EXPECT_INT(a, 1);
}
void test138()
{
    int a = 0;
    int i;
    for (i = 0; i <= 10; i++) {
        a = a + i;
    }
    EXPECT_INT(a, 55);
}
void test139()
{
    int a = 0;
    int b;
    b = (a++) + 1;
    EXPECT_INT(b, 1);
}
void test140()
{
    int a = 0;
    int b;
    b = (++a) + 1;
    EXPECT_INT(b, 2);
}
void test141()
{
    int a = 0;
    int i;
    for (i = 0; i <= 10; ++i) {
        a = a + i;
    }
    EXPECT_INT(a, 55);
}
void test142()
{
    int a = 0;
    if (a == 0) {
        int a;
        a = 1;
    }
    EXPECT_INT(a, 0);
}
void test143()
{
    int a = 0;
    int i;
    for (i = 0; i < 10; i++) {
        int a;
        a = 1;
    }
    EXPECT_INT(a, 0);
}
void test144()
{
    int a = 0;
    int i;
    for (i = 0; i < 10; i++) {
        int i;
        for (i = 0; i < 10; i++) {
            a = a + 1;
        }
    }
    EXPECT_INT(a, 100);
}
test146()
{
    int x = 3;
    int *y;
    y = &x;
    EXPECT_INT(*y, 3);
}
test147()
{
    int x = 3;
    int *y;
    y = &x;
    *y = 10;
    EXPECT_INT(x, 10);
}
test148()
{
    int **y;
    int z;
    int *x = &z;
    y = &x;
    **y = 1;
    EXPECT_INT(z, 1);
}
int *test149detail(int *p)
{
    *p = 1;
    return p;
}
void test149()
{
    int x;
    int *y = test149detail(&x);
    EXPECT_INT(*y, 1);
}
void test150()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    EXPECT_INT(*q, 12);
}
void test151()
{
    int *p;
    alloc4(&p);
    int *q = p + 3;
    EXPECT_INT(*q, 13);
}
void test152()
{
    int *p;
    EXPECT_INT(*((1 + 1) + alloc4(&p)), 12);
}
void test153()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    EXPECT_INT(q - p, 2);
}
void test154()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    q = q - 1;
    EXPECT_INT(q - p, 1);
}
void test155()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    EXPECT_INT(((q + 1) - p) == (q - p) + 1, 1);
}
void test156()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    int r = (q - p) + 1 == -(p - (q + 1));
    EXPECT_INT(r, 1);
}
void test157()
{
    int *p;
    alloc4(&p);
    p++;
    EXPECT_INT(*p, 11);
}
void test158()
{
    int *p;
    alloc4(&p);
    ++p;
    EXPECT_INT(*p, 11);
}
void test160()
{
    int n = 0;
    EXPECT_INT(*&n, 0);
}
void test161()
{
    int n = 0;
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) n++;
    EXPECT_INT(n, 100);
}
void test162()
{
    int ary[10];
    *(ary + 5) = 4;
    EXPECT_INT(*(ary + 5), 4);
}
void test163()
{
    int ary[10][10];
    *(*(ary + 4) + 5) = 9;
    EXPECT_INT(*(*(ary + 4) + 5), 9);
}
void test164()
{
    int ary[10];
    int i;
    for (i = 0; i < 10; i++) *(i + ary) = i;
    EXPECT_INT(*(ary + 5), 5);
}
void test165()
{
    int ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j;
    EXPECT_INT(*(*(ary + 6) + 4), 2);
}
void test166()
{
    int ary[10];
    ary[5] = 10;
    EXPECT_INT(ary[5], 10);

    *(&ary[5] - (-1)) = 10;
    EXPECT_INT(ary[6], 10);
    *(&ary[5] + (-1)) = 1;
    EXPECT_INT(ary[4], 1);
}
void test167()
{
    int ary[10][10];
    ary[4][5] = 9;
    EXPECT_INT(ary[4][5], 9);
}
void test168()
{
    int ary[10];
    int i;
    for (i = 0; i < 10; i++) ary[i] = i;
    EXPECT_INT(ary[5], 5);
}
void test169()
{
    int ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) ary[i][j] = i - j;
    EXPECT_INT(ary[6][4], 2);
}
void test170()
{
    int ary[10];
    3 [ary] = 4;
    EXPECT_INT(3 [ary], 4);
}
int test171fib(int n)
{
    if (n <= 1) return n;
    int p0;
    int p1;
    p0 = test171fib(n - 1);
    p1 = test171fib(n - 2);
    return p0 + p1;
}
void test171() { EXPECT_INT(test171fib(5), 5); }
void test172()
{
    int a = 2;
    a += 5;
    EXPECT_INT(a, 7);
}
void test173()
{
    int a = 8;
    a -= 5;
    EXPECT_INT(a, 3);
}
void test174()
{
    int a = 2;
    a *= 5;
    EXPECT_INT(a, 10);
}
void test175()
{
    int a = 10;
    a /= 5;
    EXPECT_INT(a, 2);
}
void test176()
{
    int a = 12;
    a &= 5;
    EXPECT_INT(a, 4);
}
void test177()
{
    int a = 12;
    a %= 5;
    EXPECT_INT(a, 2);
}
void test178()
{
    int a = 2;
    a |= 5;
    EXPECT_INT(a, 7);
}
void test179()
{
    int a = 2;
    a ^= 5;
    EXPECT_INT(a, 7);
}
void test180()
{
    int a = 2;
    a <<= 2;
    EXPECT_INT(a, 8);
}
void test181()
{
    int a = 4;
    a >>= 2;
    EXPECT_INT(a, 1);
}
void test182() { EXPECT_INT(a, 0); }
void test183()
{
    a = 4;
    EXPECT_INT(a, 4);
}
void test185()
{
    p = &a;
    *p = 3;
    EXPECT_INT(a, 3);
}
int test186a[20];
void test186()
{
    test186a[5] = 5;
    EXPECT_INT(test186a[5], 5);
}
int test187a[20][10];
void test187()
{
    test187a[3][5] = 10;
    EXPECT_INT(test187a[3][5], 10);
}
void test188()
{
    char c = 1;
    EXPECT_INT(c, 1);
}
void test189()
{
    char x;
    EXPECT_INT(x = 1, 1);
}
void test190()
{
    char xy;
    EXPECT_INT(xy = 100 + 100, 200);
}
void test191()
{
    char a_b;
    EXPECT_INT(a_b = -(33 * (1 + 2)) / 3 + 34, 1);
}
void test192()
{
    char _;
    EXPECT_INT(_ = (2 - 1) << 1, 2);
}
void test193()
{
    char x = 1;
    char y;
    EXPECT_INT(y = 2, 2);
}
void test194()
{
    char x = 1;
    char y = 2;
    char z;
    EXPECT_INT(z = x + y, 3);
}
void test195()
{
    char a0;
    char a1;
    char a2;
    char a3;
    a0 = 1;
    a1 = 1;
    a2 = a0 + a1;
    EXPECT_INT(a3 = a1 + a2, 3);
}
void test196()
{
    char x;
    char y;
    char z;
    x = y = 1;
    EXPECT_INT(z = x = x + y, 2);
}
test197() { EXPECT_INT(iadd(1, 2), 3); }
test198() { EXPECT_INT(iadd(1, 2) * iadd(2, 3), 15); }
test199() { EXPECT_INT(eighth(1, 2, 3, 4, 5, 6, 7, 8), 8); }
test200() { EXPECT_INT(eighth(1, 2, 3, 4, 5, 6, 7, 255), 255); }
void test201()
{
    char a = 2;
    a += 5;
    EXPECT_INT(a, 7);
}
void test202()
{
    char a = 8;
    a -= 5;
    EXPECT_INT(a, 3);
}
void test203()
{
    char a = 2;
    a *= 5;
    EXPECT_INT(a, 10);
}
void test204()
{
    char a = 10;
    a /= 5;
    EXPECT_INT(a, 2);
}
void test205()
{
    char a = 12;
    a &= 5;
    EXPECT_INT(a, 4);
}
void test206()
{
    char a = 12;
    a %= 5;
    EXPECT_INT(a, 2);
}
void test207()
{
    char a = 2;
    a |= 5;
    EXPECT_INT(a, 7);
}
void test208()
{
    char a = 2;
    a ^= 5;
    EXPECT_INT(a, 7);
}
void test209()
{
    char a = 2;
    a <<= 2;
    EXPECT_INT(a, 8);
}
void test210()
{
    char a = 4;
    a >>= 2;
    EXPECT_INT(a, 1);
}
void test211()
{
    char ary[10];
    *(ary + 5) = 4;
    EXPECT_INT(*(ary + 5), 4);
}
void test212()
{
    char ary[10][10];
    *(*(ary + 4) + 5) = 9;
    EXPECT_INT(*(*(ary + 4) + 5), 9);
}
void test213()
{
    char ary[10];
    int i;
    for (i = 0; i < 10; i++) *(i + ary) = i;
    EXPECT_INT(*(ary + 5), 5);
}
void test214()
{
    char ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j;
    EXPECT_INT(*(*(ary + 6) + 4), 2);
}
void test215()
{
    char ary[10];
    ary[5] = 10;
    EXPECT_INT(ary[5], 10);
}
void test216()
{
    char ary[10][10];
    ary[4][5] = 9;
    EXPECT_INT(ary[4][5], 9);
}
void test217()
{
    char ary[10];
    int i;
    for (i = 0; i < 10; i++) ary[i] = i;
    EXPECT_INT(ary[5], 5);
}
void test218()
{
    char ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) ary[i][j] = i - j;
    EXPECT_INT(ary[6][4], 2);
}
void test219()
{
    char ary[10];
    3 [ary] = 4;
    EXPECT_INT(3 [ary], 4);
}
void test220() { EXPECT_INT(b, 0); }
void test221()
{
    b = 4;
    EXPECT_INT(b, 4);
}
void test223()
{
    p = &b;
    *p = 3;
    EXPECT_INT(b, 3);
}
char test224a[20];
void test224()
{
    test224a[5] = 5;
    EXPECT_INT(test224a[5], 5);
}
char test225a[20][10];
void test225()
{
    test225a[3][5] = 10;
    EXPECT_INT(test225a[3][5], 10);
}
void test226() { EXPECT_INT(sizeof(int), 4); }
void test227() { EXPECT_INT(sizeof(char), 1); }
void test228() { EXPECT_INT(sizeof(int *), 8); }
void test229() { EXPECT_INT(sizeof(char *), 8); }
void test230() { EXPECT_INT(sizeof(int ***), 8); }
void test231() { EXPECT_INT(sizeof(char **), 8); }
void test232()
{
    int a;
    EXPECT_INT(sizeof(a), 4);
}
void test233()
{
    char a;
    EXPECT_INT(sizeof(a), 1);
}
void test234()
{
    int *a;
    EXPECT_INT(sizeof(a), 8);
}
void test235()
{
    char *a;
    EXPECT_INT(sizeof(a), 8);
}
void test236()
{
    int a[20];
    EXPECT_INT(sizeof(a), 80);
}
void test237()
{
    int a[5][6];
    EXPECT_INT(sizeof(a), 120);
}
void test238()
{
    int a[5][3][2];
    EXPECT_INT(sizeof(a), 120);
}
void test239()
{
    char a[20];
    EXPECT_INT(sizeof(a), 20);
}
void test240()
{
    char a[5][6];
    EXPECT_INT(sizeof(a), 30);
}
void test241()
{
    char a[5][6][4];
    EXPECT_INT(sizeof(a), 120);
}
void test242()
{
    char a[2][4][3][5];
    EXPECT_INT(sizeof(a), 120);
}
void test243() { EXPECT_INT(sizeof(a), 4); }
void test244() { EXPECT_INT(sizeof(b), 1); }
void test245() { EXPECT_INT(sizeof(p), 8); }
void test246() { EXPECT_INT(sizeof(q), 8); }
int test247a[20];
void test247() { EXPECT_INT(sizeof(test247a), 80); }
int test248a[5][6];
void test248() { EXPECT_INT(sizeof(test248a), 120); }
int test249a[5][3][2];
void test249() { EXPECT_INT(sizeof(test249a), 120); }
char test250a[20];
void test250() { EXPECT_INT(sizeof(test250a), 20); }
char test251a[5][6];
void test251() { EXPECT_INT(sizeof(test251a), 30); }
char test252a[5][6][4];
void test252() { EXPECT_INT(sizeof(test252a), 120); }
void test253()
{
    char *str;
    str = "abc";
    EXPECT_INT(str[0], 97);
}
char *test254detail()
{
    char *str;
    str = "abc";
    return str;
}
void test254()
{
    char *str;
    str = test254detail();
    EXPECT_INT(str[1], 98);
}
int test255a = 4;
void test255() { EXPECT_INT(test255a, 4); }
char test256a = 4;
void test256() { EXPECT_INT(test256a, 4); }
int *test257a = 0;
void test257() { EXPECT_INT(test257a, 0); }
char *test258a = 0;
void test258() { EXPECT_INT(test258a, 0); }
void test259() { EXPECT_INT(sizeof("foo"), 4); }
void test260()
{
    int a[4];
    a[0] = 1;
    a[1] = 3;
    a[2] = 4;
    EXPECT_INT(a[0] ? a[1] : a[2], 3);
}
int test261detail(int *ary) { ary[0] = 5; }
void test261()
{
    int ary[2];
    ary[0] = 0;
    test261detail(ary);
    EXPECT_INT(ary[0], 5);
}
void test262()
{
    int n = 0;
    EXPECT_INT(*&*&n, 0);
}
void test263()
{
    char *str = "abc\0abc";
    EXPECT_INT(str[3], 0);
}
void test264() { EXPECT_INT(sizeof("\t"), 2); }
void test265() { EXPECT_INT('A', 65); }
void test266() { EXPECT_INT('a', 97); }
void test267() { EXPECT_INT('\t', 9); }
void test268() { EXPECT_INT('\0', 0); }
void test269() { EXPECT_INT('\n', 10); }
void test270()
{
    /*** comment *****/
    EXPECT_INT(1, 1);
}
void test271()
{
    // comment
    EXPECT_INT(1, 1);
}
void test272() { EXPECT_INT(add_three(divdiv(100, 5), 2), 25); }

void test273()
{
    int a = 4;
    int ret = 2;
    switch (a) {
        case 1:
            ret = 1;
            break;
        case 4:
            ret = 4;
            break;
    }
    EXPECT_INT(ret, 4);
}

int test274detail()
{
    int a = 4;
    switch (a) {
        case 1:
            return 1;
        case 4:
            return 4;
    }
}

void test274() { EXPECT_INT(test274detail(), 4); }

int test275detail()
{
    int a = 4;
    switch (a) {
        case 1:
            return 1;
        default:
            return 4;
    }
}

void test275() { EXPECT_INT(test275detail(), 4); }

int test276detail()
{
    int a = 1;
    switch (a) {
        int i = 0;
        case 1:
            i = 4;
        default:
            return i;
    }
}

void test276() { EXPECT_INT(test276detail(), 4); }

int test277detail()
{
    int a = 4;
    switch (a) {
        case 1:
            return a;
        case sizeof(int):
            return 4;
    }
}

void test277() { EXPECT_INT(test277detail(), 4); }

int test278detail()
{
    int i = 0;
a:
    if (i > 5) return i;
    i++;
    goto a;
}

void test278() { EXPECT_INT(test278detail(), 6); }

int test279detail()
{
    int j = 0;
test279:
    1;
    int i = 0;
    if (j > 5) return i;
    j++;
    i++;
    goto test279;
}

void test279() { EXPECT_INT(test279detail(), 0); }

int test280detail()
{
    int i;
    for (i = 0; i < 10; i++) {
        switch (i)
        case 5:
            goto end_loop;
    }
end_loop:
    return i;
}

void test280() { EXPECT_INT(test280detail(), 5); }

void test281() { EXPECT_INT(sizeof(struct hoge), 4); }

void test282()
{
    struct hoge a;
    EXPECT_INT(sizeof(a), 4);
}

void test283()
{
    struct hoge {
        int a;
        char b;
    } a;

    EXPECT_INT(sizeof(a), 8);
}

void test284()
{
    struct hoge {
        char buf[25];
        int d;
    };
    EXPECT_INT(sizeof(struct hoge), 32);
}

void test285()
{
    struct hoge {
        int a;
    };
    struct hoge a;
    a.a = 1;
    EXPECT_INT(a.a, 1);
}

void test286()
{
    struct hoge {
        int a;
        char b[18];
    };
    struct hoge a;
    a.a = 1;
    a.b[10] = 2;
    EXPECT_INT(a.b[10], 2);
}

void test287()
{
    struct hoge {
        int *p;
    } a;
    int i = 34;
    a.p = &i;
    EXPECT_INT(*a.p, 34);
}

void test288()
{
    struct hoge {
        int i;
    } a;
    struct hoge *b = &a;
    (*b).i = 1;
    EXPECT_INT((*b).i, 1);
}

void test289()
{
    struct hoge {
        int i;
    } a;
    struct hoge *b = &a;
    b->i = 1;
    EXPECT_INT(b->i, 1);
}

void test290()
{
    struct hoge {
        int i;
        char ch[20];
        int j[10][2];
    } a;
    struct hoge *b = &a;
    b->j[5][1] = 20;
    b->j[3][0] = 10;
    EXPECT_INT(b->j[5][1], 20);
}

void test291()
{
    struct hoge {
        int a;
        struct piyo {
            int b;
        };
    };
    struct hoge h;
    EXPECT_INT(sizeof(h), 4);
}

void test292()
{
    struct hoge {
        int a;
        struct {
            int b;
        };
    };
    struct hoge h;
    EXPECT_INT(sizeof(h), 8);
}

void test293()
{
    struct hoge {
        int a;
        struct {
            int b;
        } c;
    };
    struct hoge h;
    EXPECT_INT(sizeof(h), 8);
}

void test294()
{
    struct hoge {
        int a;
        struct {
            int b;
            struct {
                int c;
                char d;
            } e;
        } f;
    };
    struct hoge h;
    EXPECT_INT(sizeof(h), 16);
}

void test295()
{
    struct hoge {
        int a;
        struct piyo {
            int b;
        } c;
    };
    struct hoge h;
    h.c.b = 4;
    EXPECT_INT(h.c.b, 4);
}

void test296()
{
    struct hoge {
        int a;
        struct {
            int b;
        };
    };
    struct hoge h;
    h.b = 2;
    h.a = 1;
    EXPECT_INT(h.b, 2);
}

void test297()
{
    struct hoge {
        int a;
        struct {
            int b;
        } c;
    };
    struct hoge h;
    h.c.b = 7;
    EXPECT_INT(h.c.b, 7);
}

void test298()
{
    struct hoge {
        int a;
        struct {
            int b;
            struct {
                int c;
                char d;
            } e;
        } f;
    };
    struct hoge h;
    h.f.b = 4;
    h.f.e.d = 5;
    h.a = h.f.b + h.f.e.d;
    EXPECT_INT(h.a, 9);
}

void test299()
{
    struct hoge {
        int a;
        struct {
            int b;
            struct {
                int c;
                char d;
            };
        };
    };
    struct hoge h;
    h.b = 4;
    h.d = 5;
    h.a = h.b + h.d;
    EXPECT_INT(h.a, 9);
}

void test301()
{
    int a, b;
    a = 1;
    b = 2;
    EXPECT_INT(a & b, 0);
}

void test302()
{
    int a = 1, b = 2;
    EXPECT_INT(a & b, 0);
}

void test303()
{
    int a = 1, b = 2, *c = &a, *d = &b;
    EXPECT_INT(*c & *d, 0);
}

void test304()
{
    int *a[2], b, c;
    a[0] = &b;
    a[1] = &c;
    *a[0] = 1;
    *a[1] = 2;
    EXPECT_INT(*a[0] & *a[1], 0);
}

void test305()
{
    struct hoge {
        int a;
        char b;
    };
    struct hoge *c[2], d, e;
    c[0] = &d;
    c[1] = &e;
    c[0]->a = 2;
    c[1]->a = 2;
    EXPECT_INT(c[0]->a & c[1]->a, 2);
}

int *test306retptr(int *p) { return p; }

void test306()
{
    int a[3];
    a[2] = 42;
    EXPECT_INT(test306retptr(a)[2], 42);
}

struct hoge *test307rethoge(struct hoge *p) { return p; }

void test307()
{
    struct hoge h;
    h.piyo = 43;
    EXPECT_INT(test307rethoge(&h)->piyo, 43);
}

void test308()
{
    int a = 1, b = 4;
    EXPECT_INT(!(a < b), 0);
}

void test309()
{
    int a = 4;
    EXPECT_INT(!a, 0);
}

void test310()
{
    int a = 0;
    EXPECT_INT(!!a, 0);
}

void test311()
{
    int a = 3;
    EXPECT_INT(!!a, 1);
}

int test312detail()
{
    int a = 0, *b = &a;
    if (*b) return 1;
    return 0;
}

void test312() { EXPECT_INT(test312detail(), 0); }

int test313detail()
{
    int a = 0, *b = &a;
    while (*b) return 1;
    return 0;
}

void test313() { EXPECT_INT(test313detail(), 0); }

int test314detail()
{
    int a = 0, *b = &a;
    for (; *b;) return 1;
    return 0;
}

void test314() { EXPECT_INT(test314detail(), 0); }

void test315()
{
    int i, j;
    EXPECT_INT((i = 4, j = 5), 5);
}

void test316()
{
    int i, j;
    for (i = 0, j = 0; i < 5 && j < 5; i++, j++)
        ;
    EXPECT_INT(i, 5);
    EXPECT_INT(j, 5);
}

void test317()
{
    int sum = 0;
    for (int i = 0; i < 5; i++) sum++;
    for (int i = 0; i < 5; i++) sum++;
    EXPECT_INT(sum, 10);
}

void test318()
{
    int sum = 0;
    for (int i = 0, j = 0; i < 5 && j < 6; i++, j++) sum++;
    for (int i = 0, j = 0; i < 6 || j < 3; i++, j++) sum++;
    EXPECT_INT(sum, 11);
}

void test319()
{
    int a = 3, b = a + 1;
    EXPECT_INT(b - a, 1);
}

struct test320struct *test320detail(struct test320struct *a) { return a; }

struct test320struct {
    int a;
};

void test320()
{
    struct test320struct st;
    test320detail(&st)->a = 1;
    EXPECT_INT(st.a, 1);
}

void test321()
{
    typedef int Ret;
    Ret ret;

    {
        typedef int Number;
        Number i = 0, j = 1;
        i += j;
        ret = i;
    }
    {
        typedef char Number;
        Number i = 3, j = 4;
        i -= j;
        ret -= i;
    }

    EXPECT_INT(ret, 2);
}

void test322()
{
    typedef struct hoge {
        int piyo;
        char foo;
        int bar;
    } hoge;

    hoge h;
    h.piyo = 3;
    h.foo = 4;
    h.bar = 5;
    EXPECT_INT(h.bar, 5);
}

void test323()
{
    struct test323struct {
        int piyo, foo, bar;
    };
    typedef struct test323struct hoge[5];
    hoge h;
    h[3].piyo = 3;
    h[3].foo = 4;
    h[3].bar = 5;
    EXPECT_INT(h[3].bar, 5);
}

void test324()
{
    typedef char *String;
    String str = "abc";
    EXPECT_INT(str[1], 'b');
}

void test325()
{
    typedef struct {
        int piyo;
        char foo;
        int bar;
    } hoge;

    hoge h;
    h.piyo = 3;
    h.foo = 4;
    h.bar = 5;
    EXPECT_INT(h.bar, 5);
}

void test326()
{
    struct A {
        int a;
    };
    struct A a;
    struct A *b = &a;
    (1, *b).a;
}

void test327()
{
    int a = 1;
    a--;
    --a;
    EXPECT_INT(a, -1);
}

void test328()
{
    int a = 1, b = a--, c = --a;
    EXPECT_INT(a, -1);
    EXPECT_INT(b, 1);
    EXPECT_INT(c, -1);
}

void test329()
{
    struct {
        int a;
    } st;
    (1, (1, st)).a = 4;
    EXPECT_INT(st.a, 4);
}

void test330()
{
    typedef struct {
        int a;
        char b, c;
        int d;
    } test330st1;
    EXPECT_INT(sizeof(test330st1), 12);

    typedef struct {
        int a;
        char b[2];
        int d;
    } test330st2;
    EXPECT_INT(sizeof(test330st2), 12);

    typedef struct {
        int a, b;
        char c, d, e;
        int f;
    } test330st3;
    EXPECT_INT(sizeof(test330st3), 16);

    typedef struct {
        int a;
        char b, c;
        int d;
        struct {
            int a;
            char b[2];
            int d;
        } test330st4;
    } test330st5;
    EXPECT_INT(sizeof(test330st5), 24);

    typedef struct {
        int a;
        char b, c;
        int d;
        struct {
            int a;
            char b[2];
            int d;
            struct {
                int a, b;
                char c, d, e;
                int f;
            } test330st6;
        } test330st7;
    } test330st8;
    EXPECT_INT(sizeof(test330st8), 40);

    typedef struct {
        int a;
        char b, c;
        int d;
        struct {
            int a2;
            char b2[2];
            int d2;
            struct {
                int a1, b1;
                char c1, d1, e;
                int f;
            };
        };
    } test330st9;
    EXPECT_INT(sizeof(test330st9), 40);

    typedef struct {
        int a;
        char b, c;
        int d;
        struct {
            int a;
            char b[2];
            int d;
            struct {
                int a, b;
                char c, d, e;
                int f;
            } test330st10[3];
        } test330st11[5];
    } test330st12;
    EXPECT_INT(sizeof(test330st12), 312);
}

void test331()
{
    int i = 0;
    do {
        i++;
    } while (i < 5);
    EXPECT_INT(i, 5);

    i = 0;
    do {
        i++;
    } while (i < 0);
    EXPECT_INT(i, 1);

    i = 0;
    do {
        i++;
        break;
    } while (1);
    EXPECT_INT(i, 1);

    i = 0;
    do {
        i++;
        if (i <= 5) continue;
        break;
    } while (1);
    EXPECT_INT(i, 6);

    int sum = 0;
    i = 0;
    do {
        for (int i = 0; i < 5; i++) {
            sum++;
            if (sum >= 3) break;
        }
        if (sum == 4) continue;
        i++;
        if (sum == 5) break;
    } while (1);
    EXPECT_INT(sum, 5);
    EXPECT_INT(i, 2);
}

void test332()
{
    int a = 257;
    char b = a;
    char c = (char)b;
    EXPECT_INT(b, 1);
    EXPECT_INT((int)c, 1);

    char d = 34;
    int e = d;
    int f = (char)d;
    EXPECT_INT(e, 34);
    EXPECT_INT(f, 34);

    struct hoge {
        int a;
    };
    struct piyo {
        char c[4];
    };

    struct hoge h;
    h.a = 67305985;  // 0x04030201
    struct piyo *p = &h;
    EXPECT_INT((int)((struct piyo *)&h)->c[0], 1);
    EXPECT_INT((int)((struct piyo *)&h)->c[1], 2);
    EXPECT_INT((int)((struct piyo *)&h)->c[2], 3);
    EXPECT_INT((int)((struct piyo *)&h)->c[3], 4);
}

void test333()
{
    struct hoge {
        int a;
    };
    struct piyo {
        char c[4];
    };

    struct hoge h;
    h.a = 67305985;  // 0x04030201
    void *ptr = (void *)&h;
    struct piyo *p = (struct piyo *)ptr;
    EXPECT_INT((int)((struct piyo *)&h)->c[0], 1);
    EXPECT_INT((int)((struct piyo *)&h)->c[1], 2);
    EXPECT_INT((int)((struct piyo *)&h)->c[2], 3);
    EXPECT_INT((int)((struct piyo *)&h)->c[3], 4);
}

void *test334detail(int flag)
{
    switch (flag) {
        case 0:
            return &a;
        default:
            return &b;
    }
}

void test334()
{
    a = 30;
    b = 42;

    void *ptr;
    ptr = test334detail(0);
    EXPECT_INT(*(int *)ptr, 30);
    ptr = test334detail(1);
    EXPECT_INT(*(char *)ptr, 42);
    ptr = test334detail(1);
    EXPECT_INT(*(char *)ptr, 42);
}

Number test335detail(Number n) { return n; }

void test335() { EXPECT_INT(test335detail(5), 5); }

void test336()
{
    union test336union {
        int a;
        char b;
        int *p;
    } u;

    EXPECT_INT(sizeof(union test336union), 8);

    u.a = 4;
    EXPECT_INT(u.a, 4);
    EXPECT_INT(u.b, 4);
    EXPECT_INT(u.p, 4);

    u.a = 67305985;  // 0x04030201
    EXPECT_INT(u.a, 67305985);
    EXPECT_INT(u.b, 1);

    union test336union2 {
        int a;

        struct {
            char b0, b1, b2, b3;
        };
    } u2;
    u2.a = 67305985;

    EXPECT_INT(u2.b0, 1);
    EXPECT_INT(u2.b1, 2);
    EXPECT_INT(u2.b2, 3);
    EXPECT_INT(u2.b3, 4);

    struct test336union3 {
        int a;

        union {
            char b0, b1, b2, b3;
        };
    } u3;
    u3.a = 67305985;
    u3.b0 = 1;

    EXPECT_INT(u3.b0, 1);
    EXPECT_INT(u3.b1, 1);
    EXPECT_INT(u3.b2, 1);
    EXPECT_INT(u3.b3, 1);
}

void test337()
{
    typedef union piyo piyo;
    piyo p;
    p.foo = 43;
    EXPECT_INT(p.foo, 43);
    EXPECT_INT(p.bar, 43);
}

void test338()
{
    const int a = 10;
    EXPECT_INT(a, 10);
    const int *p = &a;
    EXPECT_INT(*p, 10);
    int *const q = &a;
    EXPECT_INT(*q, 10);
    const int *const r = &a;
    EXPECT_INT(*r, 10);
}

void test339()
{
    enum ENUM { A, B, C, D, E };
    EXPECT_INT(A, 0);
    EXPECT_INT(B, 1);
    EXPECT_INT(C, 2);
    EXPECT_INT(D, 3);
    EXPECT_INT(E, 4);

    enum ENUM2 { A2, B2 = 10, C2, D2 = 100, E2 };
    EXPECT_INT(A2, 0);
    EXPECT_INT(B2, 10);
    EXPECT_INT(C2, 11);
    EXPECT_INT(D2, 100);
    EXPECT_INT(E2, 101);

    EXPECT_INT(B + D2, 101);
    EXPECT_INT(C - E2, -99);

    EXPECT_INT(SE_A, 0);
    EXPECT_INT(SE_B, 1);
    EXPECT_INT(SE_C, 2);
    EXPECT_INT(SE_D, 0);
    EXPECT_INT(SE_E, 1);
    EXPECT_INT(SE_F, 2);

    enum ENUM a = B;
    EXPECT_INT(a, B);
    typedef enum ENUM typedefedenum;
    typedefedenum ta = C;
    EXPECT_INT(ta, C);
    enum ENUM *pa = &a;
    EXPECT_INT(*pa, B);
    enum ENUM3 *pb = 0;
    EXPECT_INT(sizeof(pb), 8);
}

int test340detail(int a, ...) { return a; }

void test340() { EXPECT_INT(test340detail(1, 2, 3), 1); }

void exit(int status);
_Noreturn int reallyexit() { exit(0); }

void test341(int ival)
{
    static int a = 1;
    a++;
    EXPECT_INT(a, ival);

    static int *p = 0;
    p = &a;
    EXPECT_INT(*p, ival);

    static int ary[10];
    ary[5]++;
    EXPECT_INT(ary[5], ival - 1);
}

void test342()
{
    typedef struct Tree Tree;
    struct Tree {
        Tree *lhs, *rhs;
        int val;
    };
    /*
           a
         /   \
        b     c
            /   \
           d     e
   */

    Tree a, b, c, d, e;
    a.val = 10;
    b.val = 11;
    c.val = 12;
    d.val = 13;
    e.val = 14;
    a.lhs = &b;
    a.rhs = &c;
    b.lhs = b.rhs = 0;
    c.lhs = &d;
    c.rhs = &e;
    d.lhs = d.rhs = e.lhs = e.rhs = 0;

    Tree *root = &a;
    EXPECT_INT(root->val, 10);
    EXPECT_INT(root->lhs->val, 11);
    EXPECT_INT(root->rhs->val, 12);
    EXPECT_INT(root->rhs->lhs->val, 13);
    EXPECT_INT(root->rhs->rhs->val, 14);
}

void test343inc();
void test343()
{
    extern Number test343inc_var;
    EXPECT_INT(test343inc_var, 343);
    test343inc();
    EXPECT_INT(test343inc_var, 344);
}

void test344inc();
void test344()
{
    extern Number test344inc_var;
    EXPECT_INT(test344inc_var, 344);
    test344inc();
    EXPECT_INT(test344inc_var, 345);
}

void test345()
{
    char *str =
        "abc"
        "def";
    EXPECT_INT(str[2], 'c');
    EXPECT_INT(str[3], 'd');
}

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

void test346()
{
    char buf[256];
    test346vsprintf(buf, "%d%d%d%d%d%d%d%d", 0, 1, 2, 3, 4, 5, 6, 7);
    EXPECT_INT(buf[0], '0');
    EXPECT_INT(buf[1], '1');
    EXPECT_INT(buf[2], '2');
    EXPECT_INT(buf[3], '3');
    EXPECT_INT(buf[4], '4');
    EXPECT_INT(buf[5], '5');
    EXPECT_INT(buf[6], '6');
    EXPECT_INT(buf[7], '7');
    EXPECT_INT(buf[8], '\0');
}

int test347detail(int ary[4]) { return ary[2]; }

int test347()
{
    int ary[4];
    ary[2] = 100;
    EXPECT_INT(test347detail(ary), 100);
}

int test348a = -1;
int test348b = 5 + 6 - 7 / 2;
int test348()
{
    static int a = 5 + 6 - 7 / 2;
    EXPECT_INT(a, 8);

    static int b = -1;
    EXPECT_INT(b, -1);

    EXPECT_INT(test348a, -1);
    EXPECT_INT(test348b, 8);

    int c = 6;
    switch (c) {
        case 1 + 2:
            c = 3;
            break;
        case 3 * 2 + 2 / 3:
            c = 4;
            break;
    }
    EXPECT_INT(c, 4);
}

int test349()
{
    int a = 5;
    int b = (int)a + 4;
    EXPECT_INT(b, 9);
}

int main()
{
    EXPECT_INT(2, 2);
    EXPECT_INT(22, 22);
    EXPECT_INT(2 + 2, 4);
    EXPECT_INT(11 + 11 + 11, 33);
    EXPECT_INT(5 - 3, 2);
    EXPECT_INT(35 - 22, 13);
    EXPECT_INT(35 - 22 - 11, 2);
    EXPECT_INT(199 - 23 + 300 - 475, 1);
    EXPECT_INT(1 + 4 - 3, 2);
    EXPECT_INT(1983 + 2 - 449 - 3123 + 1893 - 32 + 223 - 396, 101);
    EXPECT_INT(2 * 2, 4);
    EXPECT_INT(11 * 11, 121);
    EXPECT_INT(4 / 2, 2);
    EXPECT_INT(363 / 121, 3);
    EXPECT_INT(100 / 3, 33);
    EXPECT_INT(1 + 2 * 3, 7);
    EXPECT_INT(1 + 4 * 2 - 9 / 3, 6);
    EXPECT_INT(4 % 2, 0);
    EXPECT_INT(5 % 2, 1);
    EXPECT_INT(1935 % 10, 5);
    EXPECT_INT((1 + 2) * 3, 9);
    EXPECT_INT((1 + 2) * (1 + 2), 9);
    EXPECT_INT((1 + 2) / (1 + 2), 1);
    EXPECT_INT(33 * (1 + 2), 99);
    EXPECT_INT((33 * (1 + 2)) / 3, 33);
    EXPECT_INT(-3 + 5, 2);
    EXPECT_INT(+4, 4);
    EXPECT_INT(-(33 * (1 + 2)) / 3 + 34, 1);
    EXPECT_INT(4 + 4, 8);
    EXPECT_INT(-(33 * (1 + 2)) / 3 + 34, 1);
    EXPECT_INT(2 << 1, 4);
    EXPECT_INT(2 << 2 << 1, 16);
    EXPECT_INT(2 << (2 << 1), 32);
    EXPECT_INT((2 - 1) << 1, 2);
    EXPECT_INT(2 >> 1, 1);
    EXPECT_INT(4 >> 2 >> 1, 0);
    EXPECT_INT((2 - 1) >> 1, 0);
    EXPECT_INT(1 < 2, 1);
    EXPECT_INT(1 < 2, 1);
    EXPECT_INT(4 < 2, 0);
    EXPECT_INT(1 > 2, 0);
    EXPECT_INT(1 > 2, 0);
    EXPECT_INT(4 > 2, 1);
    EXPECT_INT(1 <= 2, 1);
    EXPECT_INT(1 <= 2, 1);
    EXPECT_INT(4 <= 2, 0);
    EXPECT_INT(2 <= 2, 1);
    EXPECT_INT(1 >= 2, 0);
    EXPECT_INT(1 >= 2, 0);
    EXPECT_INT(4 >= 2, 1);
    EXPECT_INT(2 >= 2, 1);
    EXPECT_INT((2 < 1) + 1, 1);
    EXPECT_INT(1 == 2, 0);
    EXPECT_INT(1 == 2, 0);
    EXPECT_INT(4 == 2, 0);
    EXPECT_INT(2 == 2, 1);
    EXPECT_INT(1 != 2, 1);
    EXPECT_INT(1 != 2, 1);
    EXPECT_INT(4 != 2, 1);
    EXPECT_INT(2 != 2, 0);
    EXPECT_INT(0 & 0, 0);
    EXPECT_INT(0 & 0, 0);
    EXPECT_INT(1 & 0, 0);
    EXPECT_INT(0 & 1, 0);
    EXPECT_INT(1 & 1, 1);
    EXPECT_INT(1 & 2, 0);
    EXPECT_INT(2 & 2, 2);
    EXPECT_INT(3 & 5, 1);
    EXPECT_INT(0 ^ 0, 0);
    EXPECT_INT(0 ^ 0, 0);
    EXPECT_INT(1 ^ 0, 1);
    EXPECT_INT(0 ^ 1, 1);
    EXPECT_INT(1 ^ 1, 0);
    EXPECT_INT(1 ^ 2, 3);
    EXPECT_INT(2 ^ 2, 0);
    EXPECT_INT(3 ^ 5, 6);
    EXPECT_INT(0 | 0, 0);
    EXPECT_INT(0 | 0, 0);
    EXPECT_INT(1 | 0, 1);
    EXPECT_INT(0 | 1, 1);
    EXPECT_INT(1 | 1, 1);
    EXPECT_INT(1 | 2, 3);
    EXPECT_INT(2 | 2, 2);
    EXPECT_INT(3 | 5, 7);
    EXPECT_INT(1 && 0, 0);
    EXPECT_INT(1 && 1, 1);
    EXPECT_INT(0 && 1, 0);
    EXPECT_INT(0 && 0, 0);
    EXPECT_INT(2 && 1, 1);
    EXPECT_INT(-2 || 1, 1);
    EXPECT_INT(1 || 0, 1);
    EXPECT_INT(1 || 1, 1);
    EXPECT_INT(0 || 1, 1);
    EXPECT_INT(0 || 0, 0);
    EXPECT_INT(2 || 1, 1);
    EXPECT_INT(-2 || 1, 1);
    EXPECT_INT(~0, -1);
    EXPECT_INT(~1, -2);
    EXPECT_INT(~5, -6);
    EXPECT_INT(0x11, 17);
    EXPECT_INT(011, 9);

    test341(2);
    test097();
    test098();
    test099();
    test100();
    test101();
    test102();
    test103();
    test104();
    test105();
    test106();
    test107();
    test108();
    test109();
    test110();
    test111();
    test112();
    test113();
    test115();
    test116();
    test117();
    test118();
    test119();
    test120();
    test121();
    test122();
    test123();
    test124();
    test125();
    test126();
    test128();
    test129();
    test130();
    test131();
    test132();
    test133();
    test134();
    test135();
    test136();
    test137();
    test138();
    test139();
    test140();
    test141();
    test142();
    test143();
    test144();
    test146();
    test147();
    test148();
    test149();
    test150();
    test151();
    test152();
    test153();
    test154();
    test155();
    test156();
    test157();
    test158();
    test160();
    test161();
    test162();
    test163();
    test164();
    test165();
    test166();
    test167();
    test168();
    test169();
    test170();
    test171();
    test172();
    test173();
    test174();
    test175();
    test176();
    test177();
    test178();
    test179();
    test180();
    test181();
    test182();
    test183();
    test185();
    test186();
    test187();
    test188();
    test189();
    test190();
    test191();
    test192();
    test193();
    test194();
    test195();
    test196();
    test197();
    test198();
    test199();
    test200();
    test201();
    test202();
    test203();
    test204();
    test205();
    test206();
    test207();
    test208();
    test209();
    test210();
    test211();
    test212();
    test213();
    test214();
    test215();
    test216();
    test217();
    test218();
    test219();
    test220();
    test221();
    test223();
    test224();
    test225();
    test226();
    test227();
    test228();
    test229();
    test230();
    test231();
    test232();
    test233();
    test234();
    test235();
    test236();
    test237();
    test238();
    test239();
    test240();
    test241();
    test242();
    test243();
    test244();
    test245();
    test246();
    test247();
    test248();
    test249();
    test250();
    test251();
    test252();
    test253();
    test254();
    test255();
    test256();
    test257();
    test258();
    test259();
    test260();
    test261();
    test262();
    test263();
    test264();
    test265();
    test266();
    test267();
    test268();
    test269();
    test270();
    test271();
    test272();
    test273();
    test274();
    test275();
    test276();
    test277();
    test278();
    test279();
    test280();
    test281();
    test282();
    test283();
    test284();
    test285();
    test286();
    test287();
    test288();
    test289();
    test290();
    test291();
    test292();
    test293();
    test294();
    test295();
    test296();
    test297();
    test298();
    test299();
    test301();
    test302();
    test303();
    test304();
    test305();
    test306();
    test307();
    test308();
    test309();
    test310();
    test311();
    test312();
    test313();
    test314();
    test315();
    test316();
    test317();
    test341(3);
    test318();
    test319();
    test320();
    test321();
    test322();
    test323();
    test324();
    test325();
    test326();
    test327();
    test328();
    test329();
    test330();
    test331();
    test332();
    test333();
    test334();
    test335();
    test336();
    test337();
    test338();
    test339();
    test340();
    test341(4);
    test342();
    test343();
    test344();
    test345();
    test346();
    test347();
    test348();
}
