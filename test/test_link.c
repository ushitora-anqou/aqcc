int fib(int n) { return n <= 1 ? n : fib(n - 1) + fib(n - 2); }

int test001();
int test002();
int test003();
int test004();

static int test003004var = 10;

int test003() { return test003004var; }

int main()
{
    return fib(5) == 5 && test001() == 10 && test002() == 20 &&
           test003() == 10 && test004() == 20;
}
