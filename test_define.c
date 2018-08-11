int printf(char *str, ...);

#define test001int int
int test001()
{
    test001int a = 0;
    if (a != 0) printf("[ERROR] test001:1: a != 0\n");

#define test001test if (a != 0) printf("[ERROR] test001:1: a != 0\n");
    test001test;
}

int main() { test001(); }
