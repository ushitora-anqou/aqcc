int ret0() { return 0; }
int ret1() { return 1; }
int add1(int s) { return s + 1; }
int add_all(int a, int b, int c, int d, int e, int f, int g, int h)
{
    return h - (a + b + c + d + e + f + g);
}
int add_two(int a, int b) { return a + b; }

//#include <stdlib.h>
void *malloc(int size);
int *alloc4(int **p)
{
    *p = malloc(sizeof(int) * 4);
    (*p)[0] = 10;
    (*p)[1] = 11;
    (*p)[2] = 12;
    (*p)[3] = 13;

    return *p;
}
