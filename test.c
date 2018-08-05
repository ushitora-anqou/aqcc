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

int printf(char *str, int a, int b, int c);

int expect_int(int no, int got, int expect)
{
    if (got != expect)
        printf("[ERROR] test%03d: expect %d, got %d\n", no, expect, got);
}

int test001() { return 2; }
int test002() { return 22; }
int test003() { return 2 + 2; }
int test004() { return 11 + 11 + 11; }
int test005() { return 5 - 3; }
int test006() { return 35 - 22; }
int test007() { return 35 - 22 - 11; }
int test008() { return 199 - 23 + 300 - 475; }
int test009() { return 1 + 4 - 3; }
int test010() { return 1983 + 2 - 449 - 3123 + 1893 - 32 + 223 - 396; }
int test011() { return 2 * 2; }
int test012() { return 11 * 11; }
int test013() { return 4 / 2; }
int test014() { return 363 / 121; }
int test015() { return 100 / 3; }
int test016() { return 1 + 2 * 3; }
int test017() { return 1 + 4 * 2 - 9 / 3; }
int test018() { return 4 % 2; }
int test019() { return 5 % 2; }
int test020() { return 1935 % 10; }
int test021() { return (1 + 2) * 3; }
int test022() { return (1 + 2) * (1 + 2); }
int test023() { return (1 + 2) / (1 + 2); }
int test024() { return 33 * (1 + 2); }
int test025() { return (33 * (1 + 2)) / 3; }
int test026() { return -3 + 5; }
int test027() { return +4; }
int test028() { return -(33 * (1 + 2)) / 3 + 34; }
int test029() { return 4 + 4; }
int test030() { return -(33 * (1 + 2)) / 3 + 34; }
int test031() { return 2 << 1; }
int test032() { return 2 << 2 << 1; }
int test033() { return 2 << (2 << 1); }
int test034() { return (2 - 1) << 1; }
int test035() { return 2 >> 1; }
int test036() { return 4 >> 2 >> 1; }
int test037() { return (2 - 1) >> 1; }
int test038() { return 1 < 2; }
int test039() { return 1 < 2; }
int test040() { return 4 < 2; }
int test041() { return 1 > 2; }
int test042() { return 1 > 2; }
int test043() { return 4 > 2; }
int test044() { return 1 <= 2; }
int test045() { return 1 <= 2; }
int test046() { return 4 <= 2; }
int test047() { return 2 <= 2; }
int test048() { return 1 >= 2; }
int test049() { return 1 >= 2; }
int test050() { return 4 >= 2; }
int test051() { return 2 >= 2; }
int test052() { return (2 < 1) + 1; }
int test053() { return 1 == 2; }
int test054() { return 1 == 2; }
int test055() { return 4 == 2; }
int test056() { return 2 == 2; }
int test057() { return 1 != 2; }
int test058() { return 1 != 2; }
int test059() { return 4 != 2; }
int test060() { return 2 != 2; }
int test061() { return 0 & 0; }
int test062() { return 0 & 0; }
int test063() { return 1 & 0; }
int test064() { return 0 & 1; }
int test065() { return 1 & 1; }
int test066() { return 1 & 2; }
int test067() { return 2 & 2; }
int test068() { return 3 & 5; }
int test069() { return 0 ^ 0; }
int test070() { return 0 ^ 0; }
int test071() { return 1 ^ 0; }
int test072() { return 0 ^ 1; }
int test073() { return 1 ^ 1; }
int test074() { return 1 ^ 2; }
int test075() { return 2 ^ 2; }
int test076() { return 3 ^ 5; }
int test077() { return 0 | 0; }
int test078() { return 0 | 0; }
int test079() { return 1 | 0; }
int test080() { return 0 | 1; }
int test081() { return 1 | 1; }
int test082() { return 1 | 2; }
int test083() { return 2 | 2; }
int test084() { return 3 | 5; }
int test085() { return 1 && 0; }
int test086() { return 1 && 1; }
int test087() { return 0 && 1; }
int test088() { return 0 && 0; }
int test089() { return 2 && 1; }
int test090() { return -2 || 1; }
int test091() { return 1 || 0; }
int test092() { return 1 || 1; }
int test093() { return 0 || 1; }
int test094() { return 0 || 0; }
int test095() { return 2 || 1; }
int test096() { return -2 || 1; }
int test097()
{
    int x;
    return x = 1;
}
int test098()
{
    int xy;
    return xy = 100 + 100;
}
int test099()
{
    int a_b;
    return a_b = -(33 * (1 + 2)) / 3 + 34;
}
int test100()
{
    int _;
    return _ = (2 - 1) << 1;
}
int test101()
{
    int x = 1;
    int y;
    return y = 2;
}
int test102()
{
    int x = 1;
    int y = 2;
    int z;
    return z = x + y;
}
int test103()
{
    int a0 = 1;
    int a1 = 1;
    int a2 = a0 + a1;
    int a3;
    return a3 = a1 + a2;
}
int test104()
{
    int x;
    int y;
    int z;
    x = y = 1;
    return z = x = x + y;
}
int test105() { return ret0(); }
int test106() { return (ret0() + ret1()) * 2; }
int test107() { return (ret0() * ret1()) + 2; }
int test108() { return add1(1); }
int test109() { return add_two(1, 2); }
int test110() { return add_all(1, 2, 4, 8, 16, 32, 64, 128); }
test111() { return iret0(); }
test112() { return iret0() + iret1(); }
int test113() { return; }
int test114() { ; }
test115() { return iadd(1, 2); }
test116() { return iadd(1, 2) * iadd(2, 3); }
test117() { return eighth(1, 2, 3, 4, 5, 6, 7, 8); }
int test118() { return 0 == 0 ? 0 : 1; }
int test119fib(int n)
{
    return n == 0 ? 0 : n == 1 ? 1 : test119fib(n - 1) + test119fib(n - 2);
}
int test119() { return test119fib(5); }
int test120()
{
    if (0 == 1) return 0;
    return 1;
}
int test121()
{
    if (0 == 1)
        return 0;
    else
        return 1;
}
int test122()
{
    if (0 == 1) {
        return 0;
    }
    else {
        return 1;
    }
}
int test123fib(int n)
{
    if (n <= 1) return n;
    return test123fib(n - 1) + test123fib(n - 2);
}
int test123() { return test123fib(0); }
int test124fib(int n)
{
    if (n <= 1) return n;
    return test124fib(n - 1) + test124fib(n - 2);
}
int test124() { return test124fib(1); }
int test125fib(int n)
{
    if (n <= 1) return n;
    return test125fib(n - 1) + test125fib(n - 2);
}
int test125() { return test125fib(5); }
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
    return a;
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
    return 162 + b + c;
}
int test129()
{
    int a = 2;
    if (a == 1)
        return 0;
    else if (a == 2)
        return 1;
}
int test130()
{
    int a = 0;
    while (a != 10) a = a + 1;
    return a;
}
int test131()
{
    int a = 0;
    while (a != 10) {
        int b;
        b = 1;
        a = a + 1;
    }
    return a;
}
int test132()
{
    int a = 0;
    while (a != 10) {
        if (a == 5) break;
        a = a + 1;
    }
    return a;
}
int test133()
{
    int a = 0;
    while (a < 5) {
        a = a + 1;
        if (a == 5) continue;
        a = a + 1;
    }
    return a;
}
int test134()
{
    int a = 0;
    int i;
    for (i = 0; i <= 10; i = i + 1) {
        a = a + i;
    }
    return a;
}
int test135()
{
    int a = 0;
    for (;;) {
        a = a + 1;
        if (a >= 10) break;
    }
    return a;
}
int test136()
{
    int a = 0;
    for (;; a = a + 1)
        if (a < 10)
            continue;
        else
            break;
    return a;
}
int test137()
{
    int a = 0;
    a++;
    return a;
}
int test138()
{
    int a = 0;
    int i;
    for (i = 0; i <= 10; i++) {
        a = a + i;
    }
    return a;
}
int test139()
{
    int a = 0;
    int b;
    b = (a++) + 1;
    return b;
}
int test140()
{
    int a = 0;
    int b;
    b = (++a) + 1;
    return b;
}
int test141()
{
    int a = 0;
    int i;
    for (i = 0; i <= 10; ++i) {
        a = a + i;
    }
    return a;
}
int test142()
{
    int a = 0;
    if (a == 0) {
        int a;
        a = 1;
    }
    return a;
}
int test143()
{
    int a = 0;
    int i;
    for (i = 0; i < 10; i++) {
        int a;
        a = 1;
    }
    return a;
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
    return a;
}
test145() { return 0; }
test146()
{
    int x = 3;
    int *y;
    y = &x;
    return *y;
}
test147()
{
    int x = 3;
    int *y;
    y = &x;
    *y = 10;
    return x;
}
test148()
{
    int **y;
    int z;
    int *x = &z;
    y = &x;
    **y = 1;
    return z;
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
    return *y;
}
int test150()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    return *q;
}
int test151()
{
    int *p;
    alloc4(&p);
    int *q = p + 3;
    return *q;
}
int test152()
{
    int *p;
    return *((1 + 1) + alloc4(&p));
}
int test153()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    return q - p;
}
int test154()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    q = q - 1;
    return q - p;
}
int test155()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    return ((q + 1) - p) == (q - p) + 1;
}
int test156()
{
    int *p;
    alloc4(&p);
    int *q = p + 2;
    return (q - p) + 1 == -(p - (q + 1));
}
int test157()
{
    int *p;
    alloc4(&p);
    p++;
    return *p;
}
int test158()
{
    int *p;
    alloc4(&p);
    ++p;
    return *p;
}
int test160()
{
    int n = 0;
    return *&n;
}
int test161()
{
    int n = 0;
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) n++;
    return n;
}
int test162()
{
    int ary[10];
    *(ary + 5) = 4;
    return *(ary + 5);
}
int test163()
{
    int ary[10][10];
    *(*(ary + 4) + 5) = 9;
    return *(*(ary + 4) + 5);
}
int test164()
{
    int ary[10];
    int i;
    for (i = 0; i < 10; i++) *(i + ary) = i;
    return *(ary + 5);
}
int test165()
{
    int ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j;
    return *(*(ary + 6) + 4);
}
int test166()
{
    int ary[10];
    ary[5] = 10;
    return ary[5];
}
int test167()
{
    int ary[10][10];
    ary[4][5] = 9;
    return ary[4][5];
}
int test168()
{
    int ary[10];
    int i;
    for (i = 0; i < 10; i++) ary[i] = i;
    return ary[5];
}
int test169()
{
    int ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) ary[i][j] = i - j;
    return ary[6][4];
}
int test170()
{
    int ary[10];
    3 [ary] = 4;
    return 3 [ary];
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
int test171() { return test171fib(5); }
int test172()
{
    int a = 2;
    a += 5;
    return a;
}
int test173()
{
    int a = 8;
    a -= 5;
    return a;
}
int test174()
{
    int a = 2;
    a *= 5;
    return a;
}
int test175()
{
    int a = 10;
    a /= 5;
    return a;
}
int test176()
{
    int a = 12;
    a &= 5;
    return a;
}
int test177()
{
    int a = 12;
    a %= 5;
    return a;
}
int test178()
{
    int a = 2;
    a |= 5;
    return a;
}
int test179()
{
    int a = 2;
    a ^= 5;
    return a;
}
int test180()
{
    int a = 2;
    a <<= 2;
    return a;
}
int test181()
{
    int a = 4;
    a >>= 2;
    return a;
}
int test182() { return a; }
int test183()
{
    a = 4;
    return a;
}
int test185()
{
    p = &a;
    *p = 3;
    return a;
}
int test186a[20];
int test186()
{
    test186a[5] = 5;
    return test186a[5];
}
int test187a[20][10];
int test187()
{
    test187a[3][5] = 10;
    return test187a[3][5];
}
int test188()
{
    char c = 1;
    return c;
}
int test189()
{
    char x;
    return x = 1;
}
int test190()
{
    char xy;
    return xy = 100 + 100;
}
int test191()
{
    char a_b;
    return a_b = -(33 * (1 + 2)) / 3 + 34;
}
int test192()
{
    char _;
    return _ = (2 - 1) << 1;
}
int test193()
{
    char x = 1;
    char y;
    return y = 2;
}
int test194()
{
    char x = 1;
    char y = 2;
    char z;
    return z = x + y;
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
    return a3 = a1 + a2;
}
int test196()
{
    char x;
    char y;
    char z;
    x = y = 1;
    return z = x = x + y;
}
test197() { return iadd(1, 2); }
test198() { return iadd(1, 2) * iadd(2, 3); }
test199() { return eighth(1, 2, 3, 4, 5, 6, 7, 8); }
test200() { return eighth(1, 2, 3, 4, 5, 6, 7, 255); }
int test201()
{
    char a = 2;
    a += 5;
    return a;
}
int test202()
{
    char a = 8;
    a -= 5;
    return a;
}
int test203()
{
    char a = 2;
    a *= 5;
    return a;
}
int test204()
{
    char a = 10;
    a /= 5;
    return a;
}
int test205()
{
    char a = 12;
    a &= 5;
    return a;
}
int test206()
{
    char a = 12;
    a %= 5;
    return a;
}
int test207()
{
    char a = 2;
    a |= 5;
    return a;
}
int test208()
{
    char a = 2;
    a ^= 5;
    return a;
}
int test209()
{
    char a = 2;
    a <<= 2;
    return a;
}
int test210()
{
    char a = 4;
    a >>= 2;
    return a;
}
int test211()
{
    char ary[10];
    *(ary + 5) = 4;
    return *(ary + 5);
}
int test212()
{
    char ary[10][10];
    *(*(ary + 4) + 5) = 9;
    return *(*(ary + 4) + 5);
}
int test213()
{
    char ary[10];
    int i;
    for (i = 0; i < 10; i++) *(i + ary) = i;
    return *(ary + 5);
}
int test214()
{
    char ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) *(*(ary + i) + j) = i - j;
    return *(*(ary + 6) + 4);
}
int test215()
{
    char ary[10];
    ary[5] = 10;
    return ary[5];
}
int test216()
{
    char ary[10][10];
    ary[4][5] = 9;
    return ary[4][5];
}
int test217()
{
    char ary[10];
    int i;
    for (i = 0; i < 10; i++) ary[i] = i;
    return ary[5];
}
int test218()
{
    char ary[10][10];
    int i;
    int j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++) ary[i][j] = i - j;
    return ary[6][4];
}
int test219()
{
    char ary[10];
    3 [ary] = 4;
    return 3 [ary];
}
int test220() { return b; }
int test221()
{
    b = 4;
    return b;
}
int test223()
{
    p = &b;
    *p = 3;
    return b;
}
char test224a[20];
int test224()
{
    test224a[5] = 5;
    return test224a[5];
}
char test225a[20][10];
int test225()
{
    test225a[3][5] = 10;
    return test225a[3][5];
}
int test226() { return sizeof(int); }
int test227() { return sizeof(char); }
int test228() { return sizeof(int *); }
int test229() { return sizeof(char *); }
int test230() { return sizeof(int ***); }
int test231() { return sizeof(char **); }
int test232()
{
    int a;
    return sizeof(a);
}
int test233()
{
    char a;
    return sizeof(a);
}
int test234()
{
    int *a;
    return sizeof(a);
}
int test235()
{
    char *a;
    return sizeof(a);
}
int test236()
{
    int a[20];
    return sizeof(a);
}
int test237()
{
    int a[5][6];
    return sizeof(a);
}
int test238()
{
    int a[5][3][2];
    return sizeof(a);
}
int test239()
{
    char a[20];
    return sizeof(a);
}
int test240()
{
    char a[5][6];
    return sizeof(a);
}
int test241()
{
    char a[5][6][4];
    return sizeof(a);
}
int test242()
{
    char a[2][4][3][5];
    return sizeof(a);
}
int test243() { return sizeof(a); }
int test244() { return sizeof(b); }
int test245() { return sizeof(p); }
int test246() { return sizeof(q); }
int test247a[20];
int test247() { return sizeof(test247a); }
int test248a[5][6];
int test248() { return sizeof(test248a); }
int test249a[5][3][2];
int test249() { return sizeof(test249a); }
char test250a[20];
int test250() { return sizeof(test250a); }
char test251a[5][6];
int test251() { return sizeof(test251a); }
char test252a[5][6][4];
int test252() { return sizeof(test252a); }
int test253()
{
    char *str;
    str = "abc";
    return str[0];
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
    return str[1];
}
int test255a = 4;
int test255() { return test255a; }
char test256a = 4;
int test256() { return test256a; }
int *test257a = 0;
int test257() { return test257a; }
char *test258a = 0;
int test258() { return test258a; }
int test259() { return sizeof("foo"); }
int test260()
{
    int a[4];
    a[0] = 1;
    a[1] = 3;
    a[2] = 4;
    return a[0] ? a[1] : a[2];
}
int test261detail(int *ary) { ary[0] = 5; }
int test261()
{
    int ary[2];
    ary[0] = 0;
    test261detail(ary);
    return ary[0];
}
int test262()
{
    int n = 0;
    return *&*&n;
}
int test263()
{
    char *str = "abc\0abc";
    return str[3];
}
int test264() { return sizeof("\t"); }
int test265() { return 'A'; }
int test266() { return 'a'; }
int test267() { return '\t'; }
int test268() { return '\0'; }
int test269() { return '\n'; }
int test270()
{
    /*** comment *****/
    return 1;
}
int test271()
{
    // comment
    return 1;
}
int test272() { return add_three(divdiv(100, 5), 2); }

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
    return ret;
}

int test274()
{
    int a = 4;
    switch (a) {
        case 1:
            return 1;
        case 4:
            return 4;
    }
}

int test275()
{
    int a = 4;
    switch (a) {
        case 1:
            return 1;
        default:
            return 4;
    }
}

int test276()
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

int test277()
{
    int a = 4;
    switch (a) {
        case 1:
            return a;
        case sizeof(int):
            return 4;
    }
}

int test278()
{
    int i = 0;
a:
    if (i > 5) return i;
    i++;
    goto a;
}

int test279()
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

int test280()
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

struct hoge {
    int piyo;
};

int test281() { return sizeof(struct hoge); }

int test282()
{
    struct hoge a;
    return sizeof(a);
}

int test283()
{
    struct hoge {
        int a;
        char b;
    } a;

    return sizeof(a);
}

int test284()
{
    struct hoge {
        char buf[25];
        int d;
    };
    return sizeof(struct hoge);
}

int main()
{
    expect_int(1, test001(), 2);
    expect_int(2, test002(), 22);
    expect_int(3, test003(), 4);
    expect_int(4, test004(), 33);
    expect_int(5, test005(), 2);
    expect_int(6, test006(), 13);
    expect_int(7, test007(), 2);
    expect_int(8, test008(), 1);
    expect_int(9, test009(), 2);
    expect_int(10, test010(), 101);
    expect_int(11, test011(), 4);
    expect_int(12, test012(), 121);
    expect_int(13, test013(), 2);
    expect_int(14, test014(), 3);
    expect_int(15, test015(), 33);
    expect_int(16, test016(), 7);
    expect_int(17, test017(), 6);
    expect_int(18, test018(), 0);
    expect_int(19, test019(), 1);
    expect_int(20, test020(), 5);
    expect_int(21, test021(), 9);
    expect_int(22, test022(), 9);
    expect_int(23, test023(), 1);
    expect_int(24, test024(), 99);
    expect_int(25, test025(), 33);
    expect_int(26, test026(), 2);
    expect_int(27, test027(), 4);
    expect_int(28, test028(), 1);
    expect_int(29, test029(), 8);
    expect_int(30, test030(), 1);
    expect_int(31, test031(), 4);
    expect_int(32, test032(), 16);
    expect_int(33, test033(), 32);
    expect_int(34, test034(), 2);
    expect_int(35, test035(), 1);
    expect_int(36, test036(), 0);
    expect_int(37, test037(), 0);
    expect_int(38, test038(), 1);
    expect_int(39, test039(), 1);
    expect_int(40, test040(), 0);
    expect_int(41, test041(), 0);
    expect_int(42, test042(), 0);
    expect_int(43, test043(), 1);
    expect_int(44, test044(), 1);
    expect_int(45, test045(), 1);
    expect_int(46, test046(), 0);
    expect_int(47, test047(), 1);
    expect_int(48, test048(), 0);
    expect_int(49, test049(), 0);
    expect_int(50, test050(), 1);
    expect_int(51, test051(), 1);
    expect_int(52, test052(), 1);
    expect_int(53, test053(), 0);
    expect_int(54, test054(), 0);
    expect_int(55, test055(), 0);
    expect_int(56, test056(), 1);
    expect_int(57, test057(), 1);
    expect_int(58, test058(), 1);
    expect_int(59, test059(), 1);
    expect_int(60, test060(), 0);
    expect_int(61, test061(), 0);
    expect_int(62, test062(), 0);
    expect_int(63, test063(), 0);
    expect_int(64, test064(), 0);
    expect_int(65, test065(), 1);
    expect_int(66, test066(), 0);
    expect_int(67, test067(), 2);
    expect_int(68, test068(), 1);
    expect_int(69, test069(), 0);
    expect_int(70, test070(), 0);
    expect_int(71, test071(), 1);
    expect_int(72, test072(), 1);
    expect_int(73, test073(), 0);
    expect_int(74, test074(), 3);
    expect_int(75, test075(), 0);
    expect_int(76, test076(), 6);
    expect_int(77, test077(), 0);
    expect_int(78, test078(), 0);
    expect_int(79, test079(), 1);
    expect_int(80, test080(), 1);
    expect_int(81, test081(), 1);
    expect_int(82, test082(), 3);
    expect_int(83, test083(), 2);
    expect_int(84, test084(), 7);
    expect_int(85, test085(), 0);
    expect_int(86, test086(), 1);
    expect_int(87, test087(), 0);
    expect_int(88, test088(), 0);
    expect_int(89, test089(), 1);
    expect_int(90, test090(), 1);
    expect_int(91, test091(), 1);
    expect_int(92, test092(), 1);
    expect_int(93, test093(), 1);
    expect_int(94, test094(), 0);
    expect_int(95, test095(), 1);
    expect_int(96, test096(), 1);
    expect_int(97, test097(), 1);
    expect_int(98, test098(), 200);
    expect_int(99, test099(), 1);
    expect_int(100, test100(), 2);
    expect_int(101, test101(), 2);
    expect_int(102, test102(), 3);
    expect_int(103, test103(), 3);
    expect_int(104, test104(), 2);
    expect_int(105, test105(), 0);
    expect_int(106, test106(), 2);
    expect_int(107, test107(), 2);
    expect_int(108, test108(), 2);
    expect_int(109, test109(), 3);
    expect_int(110, test110(), 1);
    expect_int(111, test111(), 0);
    expect_int(112, test112(), 1);
    expect_int(113, test113(), 0);
    expect_int(114, test114(), 0);
    expect_int(115, test115(), 3);
    expect_int(116, test116(), 15);
    expect_int(117, test117(), 8);
    expect_int(118, test118(), 0);
    expect_int(119, test119(), 5);
    expect_int(120, test120(), 1);
    expect_int(121, test121(), 1);
    expect_int(122, test122(), 1);
    expect_int(123, test123(), 0);
    expect_int(124, test124(), 1);
    expect_int(125, test125(), 5);
    expect_int(126, test126(), 4);
    expect_int(128, test128(), 174);
    expect_int(129, test129(), 1);
    expect_int(130, test130(), 10);
    expect_int(131, test131(), 10);
    expect_int(132, test132(), 5);
    expect_int(133, test133(), 5);
    expect_int(134, test134(), 55);
    expect_int(135, test135(), 10);
    expect_int(136, test136(), 10);
    expect_int(137, test137(), 1);
    expect_int(138, test138(), 55);
    expect_int(139, test139(), 1);
    expect_int(140, test140(), 2);
    expect_int(141, test141(), 55);
    expect_int(142, test142(), 0);
    expect_int(143, test143(), 0);
    expect_int(144, test144(), 100);
    expect_int(146, test146(), 3);
    expect_int(147, test147(), 10);
    expect_int(148, test148(), 1);
    expect_int(149, test149(), 1);
    expect_int(150, test150(), 12);
    expect_int(151, test151(), 13);
    expect_int(152, test152(), 12);
    expect_int(153, test153(), 2);
    expect_int(154, test154(), 1);
    expect_int(155, test155(), 1);
    expect_int(156, test156(), 1);
    expect_int(157, test157(), 11);
    expect_int(158, test158(), 11);
    expect_int(160, test160(), 0);
    expect_int(161, test161(), 100);
    expect_int(162, test162(), 4);
    expect_int(163, test163(), 9);
    expect_int(164, test164(), 5);
    expect_int(165, test165(), 2);
    expect_int(166, test166(), 10);
    expect_int(167, test167(), 9);
    expect_int(168, test168(), 5);
    expect_int(169, test169(), 2);
    expect_int(170, test170(), 4);
    expect_int(171, test171(), 5);
    expect_int(172, test172(), 7);
    expect_int(173, test173(), 3);
    expect_int(174, test174(), 10);
    expect_int(175, test175(), 2);
    expect_int(176, test176(), 4);
    expect_int(177, test177(), 2);
    expect_int(178, test178(), 7);
    expect_int(179, test179(), 7);
    expect_int(180, test180(), 8);
    expect_int(181, test181(), 1);
    expect_int(182, test182(), 0);
    expect_int(183, test183(), 4);
    expect_int(185, test185(), 3);
    expect_int(186, test186(), 5);
    expect_int(187, test187(), 10);
    expect_int(188, test188(), 1);
    expect_int(189, test189(), 1);
    expect_int(190, test190(), 200);
    expect_int(191, test191(), 1);
    expect_int(192, test192(), 2);
    expect_int(193, test193(), 2);
    expect_int(194, test194(), 3);
    expect_int(195, test195(), 3);
    expect_int(196, test196(), 2);
    expect_int(197, test197(), 3);
    expect_int(198, test198(), 15);
    expect_int(199, test199(), 8);
    expect_int(200, test200(), 255);
    expect_int(201, test201(), 7);
    expect_int(202, test202(), 3);
    expect_int(203, test203(), 10);
    expect_int(204, test204(), 2);
    expect_int(205, test205(), 4);
    expect_int(206, test206(), 2);
    expect_int(207, test207(), 7);
    expect_int(208, test208(), 7);
    expect_int(209, test209(), 8);
    expect_int(210, test210(), 1);
    expect_int(211, test211(), 4);
    expect_int(212, test212(), 9);
    expect_int(213, test213(), 5);
    expect_int(214, test214(), 2);
    expect_int(215, test215(), 10);
    expect_int(216, test216(), 9);
    expect_int(217, test217(), 5);
    expect_int(218, test218(), 2);
    expect_int(219, test219(), 4);
    expect_int(220, test220(), 0);
    expect_int(221, test221(), 4);
    expect_int(223, test223(), 3);
    expect_int(224, test224(), 5);
    expect_int(225, test225(), 10);
    expect_int(226, test226(), 4);
    expect_int(227, test227(), 1);
    expect_int(228, test228(), 8);
    expect_int(229, test229(), 8);
    expect_int(230, test230(), 8);
    expect_int(231, test231(), 8);
    expect_int(232, test232(), 4);
    expect_int(233, test233(), 1);
    expect_int(234, test234(), 8);
    expect_int(235, test235(), 8);
    expect_int(236, test236(), 80);
    expect_int(237, test237(), 120);
    expect_int(238, test238(), 120);
    expect_int(239, test239(), 20);
    expect_int(240, test240(), 30);
    expect_int(241, test241(), 120);
    expect_int(242, test242(), 120);
    expect_int(243, test243(), 4);
    expect_int(244, test244(), 1);
    expect_int(245, test245(), 8);
    expect_int(246, test246(), 8);
    expect_int(247, test247(), 80);
    expect_int(248, test248(), 120);
    expect_int(249, test249(), 120);
    expect_int(250, test250(), 20);
    expect_int(251, test251(), 30);
    expect_int(252, test252(), 120);
    expect_int(253, test253(), 97);
    expect_int(254, test254(), 98);
    expect_int(255, test255(), 4);
    expect_int(256, test256(), 4);
    expect_int(257, test257(), 0);
    expect_int(258, test258(), 0);
    expect_int(259, test259(), 4);
    expect_int(260, test260(), 3);
    expect_int(261, test261(), 5);
    expect_int(262, test262(), 0);
    expect_int(263, test263(), 0);
    expect_int(264, test264(), 2);
    expect_int(265, test265(), 65);
    expect_int(266, test266(), 97);
    expect_int(267, test267(), 9);
    expect_int(268, test268(), 0);
    expect_int(269, test269(), 10);
    expect_int(270, test270(), 1);
    expect_int(271, test271(), 1);
    expect_int(272, test272(), 25);
    expect_int(273, test273(), 4);
    expect_int(274, test274(), 4);
    expect_int(275, test275(), 4);
    expect_int(276, test276(), 4);
    expect_int(277, test277(), 4);
    expect_int(278, test278(), 6);
    expect_int(279, test279(), 0);
    expect_int(280, test280(), 5);
    expect_int(281, test281(), 4);
    expect_int(282, test282(), 4);
    expect_int(283, test283(), 8);
    expect_int(284, test284(), 32);
}
