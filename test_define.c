int printf(char *str, ...);

#include "test_define.h"

#ifndef test001nop
#define test001nop
#define test001int int
int test001()
{
    test001int a = 0;
    if (a != 0) printf("[ERROR] test001:1: a != 0\n");

#define test001test \
    if (a != 0) printf("[ERROR] test001:2: a != 0\n");
    test001test;

    if (test001header != 42) printf("[ERROR] test001:3: test001header != 42\n");
    if (test001iret(334) != 334)
        printf("[ERROR] test001:4: test001iret(334) != 334\n");
}
#ifndef test001nop
#define test001nop
#define test001int int
int test001()
{
    test001int a = 0;
    if (a != 0) printf("[ERROR] test001:1: a != 0\n");

#define test001test \
    if (a != 0) printf("[ERROR] test001:2: a != 0\n");
    test001test;

    if (test001header != 42) printf("[ERROR] test001:3: test001header != 42\n");
    if (test001iret(334) != 334)
        printf("[ERROR] test001:4: test001iret(334) != 334\n");
}
#endif
#endif

#ifndef test001nop
#define test001nop
#define test001int int
int test001()
{
    test001int a = 0;
    if (a != 0) printf("[ERROR] test001:1: a != 0\n");

#define test001test \
    if (a != 0) printf("[ERROR] test001:2: a != 0\n");
    test001test;

    if (test001header != 42) printf("[ERROR] test001:3: test001header != 42\n");
    if (test001iret(334) != 334)
        printf("[ERROR] test001:4: test001iret(334) != 334\n");
}
#endif

#define test002value
int test002()
{
    int a = 0;
#ifndef test002value
    printf("[ERROR] test002:1: #ifndef guard is out of order\n");
#else
    a = 1;
#ifdef test002value
    // Test nested #ifdef
    a = 2;
#else
    printf("[ERROR] test002:2: nested #ifdef guard is out of order\n");
#endif  // #ifdef test002value

#ifndef test002value
    printf(
        "[ERROR] test002:3: nested and multiple #else guard is out of order\n");
#else
    a = 3;
#endif
    if (a != 3) {
        printf(
            "[ERROR] test002:4: neither #ifdef nor #else is called. Expected "
            "'a': 3, got: %d\n",
            a);
    }
#endif
// #ifndef test002value

// DO NOT define test002unknown
#ifdef test002unknown
    printf("[ERROR] test002:5: #ifdef guard is out of order\n");
#else
    // ok
    a = 5;
#endif
    if (a != 5) {
        printf(
            "[ERROR] test002:6: neither #ifdef nor #else is called. Expected "
            "'a': 5, got: %d\n",
            a);
    }

#ifndef test002unknown
#ifdef test002unknown
    printf("[ERROR] test002:7: nested #ifdef guard is out of order\n");
#else
    a = 6;
#endif  // #ifdef test002unknown
#else
    printf("[ERROR] test002:8: nested #ifndef guard is out of order\n");
#endif
    if (a != 6) {
        printf(
            "[ERROR] test002:9: neither #ifdef nor #else is called. Expected "
            "'a': 6, got: %d\n",
            a);
    }
}

typedef struct {
    int gp_offset;
    int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];

#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

void test003allcorrect_va_arg(int a, int b, int c, int d, int e, int f, int g,
                              int h, ...)
{
    va_list args;
    va_start(args, h);
    if (va_arg(args, int) != a)
        printf("[ERROR] test003:1: va_arg(args, int) != a\n");
    if (va_arg(args, int) != b)
        printf("[ERROR] test003:2: va_arg(args, int) != b\n");
    if (va_arg(args, int) != c)
        printf("[ERROR] test003:3: va_arg(args, int) != c\n");
    if (va_arg(args, int) != d)
        printf("[ERROR] test003:4: va_arg(args, int) != d\n");
    if (va_arg(args, int) != e)
        printf("[ERROR] test003:5: va_arg(args, int) != e\n");
    if (va_arg(args, int) != f)
        printf("[ERROR] test003:6: va_arg(args, int) != f\n");
    if (va_arg(args, int) != g)
        printf("[ERROR] test003:7: va_arg(args, int) != g\n");
    if (va_arg(args, int) != h)
        printf("[ERROR] test003:8: va_arg(args, int) != h\n");
    va_end(args);
}

void test003charp(int a, ...)
{
    va_list args;
    va_start(args, a);
    if (va_arg(args, char *)[0] != a)
        printf("[ERROR] test003:9: va_arg(args, char *)[0] != a\n");
    va_end(args);
}

void test003vaarg_valist(char *a, char *b, va_list ap)
{
    if (va_arg(ap, char *)[0] != a[0])
        printf("[ERROR] test003:10: va_arg(ap, char *)[0] != a[0]\n");
}

void test003vaarg(char *a, char *b, ...)
{
    va_list args;
    va_start(args, b);
    test003vaarg_valist(a, b, args);
    va_end(args);
}

int test003()
{
    test003allcorrect_va_arg(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
    test003charp('a', "a");
    test003vaarg("a", "b", "a");
}

int main()
{
    test001();
    test002();
    test003();
}
