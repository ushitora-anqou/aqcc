int fib(int n) { return n <= 1 ? n : fib(n - 1) + fib(n - 2); }

int test001();
int test002();
int test004();
int test007();

static int test003004var = 10;

int test003() { return test003004var; }

static int test005() { return 10; }

int test006() { return test005(); }

int main()
{
    return fib(5) == 5 && test001() == 10 && test002() == 20 &&
           test003() == 10 && test004() == 20 && test006() == 10 &&
           test007() == 20;
}
