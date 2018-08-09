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

struct hoge {
    int piyo;
};

int printf(char *str, int a, int b, int c);

#define EXPECT_INT(got, expect)                                          \
    {                                                                    \
        int expect_int = (expect), got_int = (got);                      \
        int wrong_int = expect_int + 1;                                  \
        if (got_int != expect_int || wrong_int == got_int)               \
            printf("[ERROR] %d:%s: expect %d, got %d\n", __LINE__, #got, \
                   expect_int, got_int);                                 \
    }

int test097()
{
    int x;
    EXPECT_INT(x = 1, 1);
}
int test098()
{
    int xy;
    EXPECT_INT(xy = 100 + 100, 200);
}
int test099()
{
    int a_b;
    EXPECT_INT(a_b = -(33 * (1 + 2)) / 3 + 34, 1);
}
int test100()
{
    int _;
    EXPECT_INT(_ = (2 - 1) << 1, 2);
}
int test101()
{
    int x = 1;
    int y;
    EXPECT_INT(y = 2, 2);
}
int test102()
{
    int x = 1;
    int y = 2;
    int z;
    EXPECT_INT(z = x + y, 3);
}
int test103()
{
    int a0 = 1;
    int a1 = 1;
    int a2 = a0 + a1;
    int a3;
    EXPECT_INT(a3 = a1 + a2, 3);
}
int test104()
{
    int x;
    int y;
    int z;
    x = y = 1;
    EXPECT_INT(z = x = x + y, 2);
}
int test105() { EXPECT_INT(ret0(), 0); }
int test106() { EXPECT_INT((ret0() + ret1()) * 2, 2); }
int test107() { EXPECT_INT((ret0() * ret1()) + 2, 2); }
int test108() { EXPECT_INT(add1(1), 2); }
int test109() { EXPECT_INT(add_two(1, 2), 3); }
int test110() { EXPECT_INT(add_all(1, 2, 4, 8, 16, 32, 64, 128), 1); }
test111() { EXPECT_INT(iret0(), 0); }
test112() { EXPECT_INT(iret0() + iret1(), 1); }
int test113noreturn() {}
int test113() { EXPECT_INT(test113noreturn(), 0); }
test115() { EXPECT_INT(iadd(1, 2), 3); }
test116() { EXPECT_INT(iadd(1, 2) * iadd(2, 3), 15); }
test117() { EXPECT_INT(eighth(1, 2, 3, 4, 5, 6, 7, 8), 8); }
int test118() { EXPECT_INT(0 == 0 ? 0 : 1, 0); }
int test119fib(int n)
{
    return n == 0 ? 0 : n == 1 ? 1 : test119fib(n - 1) + test119fib(n - 2);
}
int test119() { EXPECT_INT(test119fib(5), 5); }
int test120detail()
{
    if (0 == 1) return 0;
    return 1;
}
int test120() { EXPECT_INT(test120detail(), 1); }
int test121detail()
{
    if (0 == 1)
        return 0;
    else
        return 1;
}
int test121() { EXPECT_INT(test121detail(), 1); }
int test122detail()
{
    if (0 == 1) {
        return 0;
    }
    else {
        return 1;
    }
}
int test122() { EXPECT_INT(test122detail(), 1); }
int test123fib(int n)
{
    if (n <= 1) return n;
    return test123fib(n - 1) + test123fib(n - 2);
}
int test123() { EXPECT_INT(test123fib(0), 0); }
int test124fib(int n)
{
    if (n <= 1) return n;
    return test124fib(n - 1) + test124fib(n - 2);
}
int test124() { EXPECT_INT(test124fib(1), 1); }
int test125fib(int n)
{
    if (n <= 1) return n;
    return test125fib(n - 1) + test125fib(n - 2);
}
int test125() { EXPECT_INT(test125fib(5), 5); }
int test126()
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
int test128()
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
int test129() { EXPECT_INT(test129detail(), 1); }
int test130()
{
    int a = 0;
    while (a != 10) a = a + 1;
    EXPECT_INT(a, 10);
}
int test131()
{
    int a = 0;
    while (a != 10) {
        int b;
        b = 1;
        a = a + 1;
    }
    EXPECT_INT(a, 10);
}
int test132()
{
    int a = 0;
    while (a != 10) {
        if (a == 5) break;
        a = a + 1;
    }
    EXPECT_INT(a, 5);
}
int test133()
{
    int a = 0;
    while (a < 5) {
        a = a + 1;
        if (a == 5) continue;
        a = a + 1;
    }
    EXPECT_INT(a, 5);
}
int test134()
{
    int a = 0;
    int i;
    for (i = 0; i <= 10; i = i + 1) {
        a = a + i;
    }
    EXPECT_INT(a, 55);
}
int test135()
{
    int a = 0;
    for (;;) {
        a = a + 1;
        if (a >= 10) break;
    }
    EXPECT_INT(a, 10);
}
int test136()
{
    int a = 0;
    for (;; a = a + 1)
        if (a < 10)
            continue;
        else
            break;
    EXPECT_INT(a, 10);
}
int test137()
{
    int a = 0;
    a++;
    EXPECT_INT(a, 1);
}
int test138()
{
    int a = 0;
    int i;
    for (i = 0; i <= 10; i++) {
        a = a + i;
    }
    EXPECT_INT(a, 55);
}
int test139()
{
    int a = 0;
    int b;
    b = (a++) + 1;
    EXPECT_INT(b, 1);
}
int test140()
{
    int a = 0;
    int b;
    b = (++a) + 1;
    EXPECT_INT(b, 2);
}
int test141()
{
    int a = 0;
    int i;
    for (i = 0; i <= 10; ++i) {
        a = a + i;
    }
    EXPECT_INT(a, 55);
}
int test142()
{
    int a = 0;
    if (a == 0) {
        int a;
        a = 1;
    }
    EXPECT_INT(a, 0);
}
int test143()
{
    int a = 0;
    int i;
    for (i = 0; i < 10; i++) {
        int a;
        a = 1;
    }
    EXPECT_INT(a, 0);
}
int test144()
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
int test149()
{
    int x;
    int *y = test149detail(&x);
    EXPECT_INT(*y, 1);
}
int test150()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    EXPECT_INT(*q, 12);
}
int test151()
{
    int *p;
    alloc4(&p);
    int *q = p + 3;
    EXPECT_INT(*q, 13);
}
int test152()
{
    int *p;
    EXPECT_INT(*((1 + 1) + alloc4(&p)), 12);
}
int test153()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    EXPECT_INT(q - p, 2);
}
int test154()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    q = q - 1;
    EXPECT_INT(q - p, 1);
}
int test155()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    EXPECT_INT(((q + 1) - p) == (q - p) + 1, 1);
}
int test156()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    EXPECT_INT((q - p) + 1 == -(p - (q + 1)), 1);
}
int test157()
{
    int *p;
    alloc4(&p);
    p++;
    EXPECT_INT(*p, 11);
}
int test158()
{
    int *p;
    alloc4(&p);
    ++p;
    EXPECT_INT(*p, 11);
}
int test160()
{
    int n = 0;
    EXPECT_INT(*&n, 0);
}
int test161()
{
    int n = 0;
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) n++;
    EXPECT_INT(n, 100);
}
int test162()
{
    int ary[10];
    *(ary + 5) = 4;
    EXPECT_INT(*(ary + 5), 4);
}
int test163()
{
    int ary[10][10];
    *(*(ary + 4) + 5) = 9;
    EXPECT_INT(*(*(ary + 4) + 5), 9);
}
int test164()
{
    int ary[10];
    int i;
    for (i = 0; i < 10; i++) *(i + ary) = i;
    EXPECT_INT(*(ary + 5), 5);
}
int test165()
{
    int ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j;
    EXPECT_INT(*(*(ary + 6) + 4), 2);
}
int test166()
{
    int ary[10];
    ary[5] = 10;
    EXPECT_INT(ary[5], 10);
}
int test167()
{
    int ary[10][10];
    ary[4][5] = 9;
    EXPECT_INT(ary[4][5], 9);
}
int test168()
{
    int ary[10];
    int i;
    for (i = 0; i < 10; i++) ary[i] = i;
    EXPECT_INT(ary[5], 5);
}
int test169()
{
    int ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) ary[i][j] = i - j;
    EXPECT_INT(ary[6][4], 2);
}
int test170()
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
int test171() { EXPECT_INT(test171fib(5), 5); }
int test172()
{
    int a = 2;
    a += 5;
    EXPECT_INT(a, 7);
}
int test173()
{
    int a = 8;
    a -= 5;
    EXPECT_INT(a, 3);
}
int test174()
{
    int a = 2;
    a *= 5;
    EXPECT_INT(a, 10);
}
int test175()
{
    int a = 10;
    a /= 5;
    EXPECT_INT(a, 2);
}
int test176()
{
    int a = 12;
    a &= 5;
    EXPECT_INT(a, 4);
}
int test177()
{
    int a = 12;
    a %= 5;
    EXPECT_INT(a, 2);
}
int test178()
{
    int a = 2;
    a |= 5;
    EXPECT_INT(a, 7);
}
int test179()
{
    int a = 2;
    a ^= 5;
    EXPECT_INT(a, 7);
}
int test180()
{
    int a = 2;
    a <<= 2;
    EXPECT_INT(a, 8);
}
int test181()
{
    int a = 4;
    a >>= 2;
    EXPECT_INT(a, 1);
}
int test182() { EXPECT_INT(a, 0); }
int test183()
{
    a = 4;
    EXPECT_INT(a, 4);
}
int test185()
{
    p = &a;
    *p = 3;
    EXPECT_INT(a, 3);
}
int test186a[20];
int test186()
{
    test186a[5] = 5;
    EXPECT_INT(test186a[5], 5);
}
int test187a[20][10];
int test187()
{
    test187a[3][5] = 10;
    EXPECT_INT(test187a[3][5], 10);
}
int test188()
{
    char c = 1;
    EXPECT_INT(c, 1);
}
int test189()
{
    char x;
    EXPECT_INT(x = 1, 1);
}
int test190()
{
    char xy;
    EXPECT_INT(xy = 100 + 100, 200);
}
int test191()
{
    char a_b;
    EXPECT_INT(a_b = -(33 * (1 + 2)) / 3 + 34, 1);
}
int test192()
{
    char _;
    EXPECT_INT(_ = (2 - 1) << 1, 2);
}
int test193()
{
    char x = 1;
    char y;
    EXPECT_INT(y = 2, 2);
}
int test194()
{
    char x = 1;
    char y = 2;
    char z;
    EXPECT_INT(z = x + y, 3);
}
int test195()
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
int test196()
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
int test201()
{
    char a = 2;
    a += 5;
    EXPECT_INT(a, 7);
}
int test202()
{
    char a = 8;
    a -= 5;
    EXPECT_INT(a, 3);
}
int test203()
{
    char a = 2;
    a *= 5;
    EXPECT_INT(a, 10);
}
int test204()
{
    char a = 10;
    a /= 5;
    EXPECT_INT(a, 2);
}
int test205()
{
    char a = 12;
    a &= 5;
    EXPECT_INT(a, 4);
}
int test206()
{
    char a = 12;
    a %= 5;
    EXPECT_INT(a, 2);
}
int test207()
{
    char a = 2;
    a |= 5;
    EXPECT_INT(a, 7);
}
int test208()
{
    char a = 2;
    a ^= 5;
    EXPECT_INT(a, 7);
}
int test209()
{
    char a = 2;
    a <<= 2;
    EXPECT_INT(a, 8);
}
int test210()
{
    char a = 4;
    a >>= 2;
    EXPECT_INT(a, 1);
}
int test211()
{
    char ary[10];
    *(ary + 5) = 4;
    EXPECT_INT(*(ary + 5), 4);
}
int test212()
{
    char ary[10][10];
    *(*(ary + 4) + 5) = 9;
    EXPECT_INT(*(*(ary + 4) + 5), 9);
}
int test213()
{
    char ary[10];
    int i;
    for (i = 0; i < 10; i++) *(i + ary) = i;
    EXPECT_INT(*(ary + 5), 5);
}
int test214()
{
    char ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j;
    EXPECT_INT(*(*(ary + 6) + 4), 2);
}
int test215()
{
    char ary[10];
    ary[5] = 10;
    EXPECT_INT(ary[5], 10);
}
int test216()
{
    char ary[10][10];
    ary[4][5] = 9;
    EXPECT_INT(ary[4][5], 9);
}
int test217()
{
    char ary[10];
    int i;
    for (i = 0; i < 10; i++) ary[i] = i;
    EXPECT_INT(ary[5], 5);
}
int test218()
{
    char ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) ary[i][j] = i - j;
    EXPECT_INT(ary[6][4], 2);
}
int test219()
{
    char ary[10];
    3 [ary] = 4;
    EXPECT_INT(3 [ary], 4);
}
int test220() { EXPECT_INT(b, 0); }
int test221()
{
    b = 4;
    EXPECT_INT(b, 4);
}
int test223()
{
    p = &b;
    *p = 3;
    EXPECT_INT(b, 3);
}
char test224a[20];
int test224()
{
    test224a[5] = 5;
    EXPECT_INT(test224a[5], 5);
}
char test225a[20][10];
int test225()
{
    test225a[3][5] = 10;
    EXPECT_INT(test225a[3][5], 10);
}
int test226() { EXPECT_INT(sizeof(int), 4); }
int test227() { EXPECT_INT(sizeof(char), 1); }
int test228() { EXPECT_INT(sizeof(int *), 8); }
int test229() { EXPECT_INT(sizeof(char *), 8); }
int test230() { EXPECT_INT(sizeof(int ***), 8); }
int test231() { EXPECT_INT(sizeof(char **), 8); }
int test232()
{
    int a;
    EXPECT_INT(sizeof(a), 4);
}
int test233()
{
    char a;
    EXPECT_INT(sizeof(a), 1);
}
int test234()
{
    int *a;
    EXPECT_INT(sizeof(a), 8);
}
int test235()
{
    char *a;
    EXPECT_INT(sizeof(a), 8);
}
int test236()
{
    int a[20];
    EXPECT_INT(sizeof(a), 80);
}
int test237()
{
    int a[5][6];
    EXPECT_INT(sizeof(a), 120);
}
int test238()
{
    int a[5][3][2];
    EXPECT_INT(sizeof(a), 120);
}
int test239()
{
    char a[20];
    EXPECT_INT(sizeof(a), 20);
}
int test240()
{
    char a[5][6];
    EXPECT_INT(sizeof(a), 30);
}
int test241()
{
    char a[5][6][4];
    EXPECT_INT(sizeof(a), 120);
}
int test242()
{
    char a[2][4][3][5];
    EXPECT_INT(sizeof(a), 120);
}
int test243() { EXPECT_INT(sizeof(a), 4); }
int test244() { EXPECT_INT(sizeof(b), 1); }
int test245() { EXPECT_INT(sizeof(p), 8); }
int test246() { EXPECT_INT(sizeof(q), 8); }
int test247a[20];
int test247() { EXPECT_INT(sizeof(test247a), 80); }
int test248a[5][6];
int test248() { EXPECT_INT(sizeof(test248a), 120); }
int test249a[5][3][2];
int test249() { EXPECT_INT(sizeof(test249a), 120); }
char test250a[20];
int test250() { EXPECT_INT(sizeof(test250a), 20); }
char test251a[5][6];
int test251() { EXPECT_INT(sizeof(test251a), 30); }
char test252a[5][6][4];
int test252() { EXPECT_INT(sizeof(test252a), 120); }
int test253()
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
int test254()
{
    char *str;
    str = test254detail();
    EXPECT_INT(str[1], 98);
}
int test255a = 4;
int test255() { EXPECT_INT(test255a, 4); }
char test256a = 4;
int test256() { EXPECT_INT(test256a, 4); }
int *test257a = 0;
int test257() { EXPECT_INT(test257a, 0); }
char *test258a = 0;
int test258() { EXPECT_INT(test258a, 0); }
int test259() { EXPECT_INT(sizeof("foo"), 4); }
int test260()
{
    int a[4];
    a[0] = 1;
    a[1] = 3;
    a[2] = 4;
    EXPECT_INT(a[0] ? a[1] : a[2], 3);
}
int test261detail(int *ary) { ary[0] = 5; }
int test261()
{
    int ary[2];
    ary[0] = 0;
    test261detail(ary);
    EXPECT_INT(ary[0], 5);
}
int test262()
{
    int n = 0;
    EXPECT_INT(*&*&n, 0);
}
int test263()
{
    char *str = "abc\0abc";
    EXPECT_INT(str[3], 0);
}
int test264() { EXPECT_INT(sizeof("\t"), 2); }
int test265() { EXPECT_INT('A', 65); }
int test266() { EXPECT_INT('a', 97); }
int test267() { EXPECT_INT('\t', 9); }
int test268() { EXPECT_INT('\0', 0); }
int test269() { EXPECT_INT('\n', 10); }
int test270()
{
    /*** comment *****/
    EXPECT_INT(1, 1);
}
int test271()
{
    // comment
    EXPECT_INT(1, 1);
}
int test272() { EXPECT_INT(add_three(divdiv(100, 5), 2), 25); }

int test273()
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

int test274() { EXPECT_INT(test274detail(), 4); }

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

int test275() { EXPECT_INT(test275detail(), 4); }

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

int test276() { EXPECT_INT(test276detail(), 4); }

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

int test277() { EXPECT_INT(test277detail(), 4); }

int test278detail()
{
    int i = 0;
a:
    if (i > 5) return i;
    i++;
    goto a;
}

int test278() { EXPECT_INT(test278detail(), 6); }

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

int test279() { EXPECT_INT(test279detail(), 0); }

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

int test280() { EXPECT_INT(test280detail(), 5); }

int test281() { EXPECT_INT(sizeof(struct hoge), 4); }

int test282()
{
    struct hoge a;
    EXPECT_INT(sizeof(a), 4);
}

int test283()
{
    struct hoge {
        int a;
        char b;
    } a;

    EXPECT_INT(sizeof(a), 8);
}

int test284()
{
    struct hoge {
        char buf[25];
        int d;
    };
    EXPECT_INT(sizeof(struct hoge), 32);
}

int test285()
{
    struct hoge {
        int a;
    };
    struct hoge a;
    a.a = 1;
    EXPECT_INT(a.a, 1);
}

int test286()
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

int test287()
{
    struct hoge {
        int *p;
    } a;
    int i = 34;
    a.p = &i;
    EXPECT_INT(*a.p, 34);
}

int test288()
{
    struct hoge {
        int i;
    } a;
    struct hoge *b = &a;
    (*b).i = 1;
    EXPECT_INT((*b).i, 1);
}

int test289()
{
    struct hoge {
        int i;
    } a;
    struct hoge *b = &a;
    b->i = 1;
    EXPECT_INT(b->i, 1);
}

int test290()
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

int test291()
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

int test292()
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

int test293()
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

int test294()
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

int test295()
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

int test296()
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

int test297()
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

int test298()
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

int test299()
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

int test301()
{
    int a, b;
    a = 1;
    b = 2;
    EXPECT_INT(a & b, 0);
}

int test302()
{
    int a = 1, b = 2;
    EXPECT_INT(a & b, 0);
}

int test303()
{
    int a = 1, b = 2, *c = &a, *d = &b;
    EXPECT_INT(*c & *d, 0);
}

int test304()
{
    int *a[2], b, c;
    a[0] = &b;
    a[1] = &c;
    *a[0] = 1;
    *a[1] = 2;
    EXPECT_INT(*a[0] & *a[1], 0);
}

int test305()
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

int test306()
{
    int a[3];
    a[2] = 42;
    EXPECT_INT(test306retptr(a)[2], 42);
}

struct hoge *test307rethoge(struct hoge *p) { return p; }

int test307()
{
    struct hoge h;
    h.piyo = 43;
    EXPECT_INT(test307rethoge(&h)->piyo, 43);
}

int test308()
{
    int a = 1, b = 4;
    EXPECT_INT(!(a < b), 0);
}

int test309()
{
    int a = 4;
    EXPECT_INT(!a, 0);
}

int test310()
{
    int a = 0;
    EXPECT_INT(!!a, 0);
}

int test311()
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

int test312() { EXPECT_INT(test312detail(), 0); }

int test313detail()
{
    int a = 0, *b = &a;
    while (*b) return 1;
    return 0;
}

int test313() { EXPECT_INT(test313detail(), 0); }

int test314detail()
{
    int a = 0, *b = &a;
    for (; *b;) return 1;
    return 0;
}

int test314() { EXPECT_INT(test314detail(), 0); }

int test315()
{
    int i, j;
    EXPECT_INT((i = 4, j = 5), 5);
}

int test316()
{
    int i, j;
    for (i = 0, j = 0; i < 5 && j < 5; i++, j++)
        ;
    EXPECT_INT(i, 5);
    EXPECT_INT(j, 5);
}

int test317()
{
    int sum = 0;
    for (int i = 0; i < 5; i++) sum++;
    for (int i = 0; i < 5; i++) sum++;
    EXPECT_INT(sum, 10);
}

int test318()
{
    int sum = 0;
    for (int i = 0, j = 0; i < 5 && j < 6; i++, j++) sum++;
    for (int i = 0, j = 0; i < 6 || j < 3; i++, j++) sum++;
    EXPECT_INT(sum, 11);
}

int test319()
{
    int a = 3, b = a + 1;
    EXPECT_INT(b - a, 1);
}

struct test320struct *test320detail(struct test320struct *a) { return a; }

struct test320struct {
    int a;
};

int test320()
{
    struct test320struct st;
    test320detail(&st)->a = 1;
    EXPECT_INT(st.a, 1);
}

int test321()
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

int test322()
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

int test323()
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

int test324()
{
    typedef char *String;
    String str = "abc";
    EXPECT_INT(str[1], 'b');
}

int test325()
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

int test326()
{
    struct A {
        int a;
    };
    struct A a;
    struct A *b = &a;
    (1, *b).a;
}

int test327()
{
    int a = 1;
    a--;
    --a;
    EXPECT_INT(a, -1);
}

int test328()
{
    int a = 1, b = a--, c = --a;
    EXPECT_INT(a, -1);
    EXPECT_INT(b, 1);
    EXPECT_INT(c, -1);
}

int test329()
{
    struct {
        int a;
    } st;
    (1, (1, st)).a = 4;
    EXPECT_INT(st.a, 4);
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
}
