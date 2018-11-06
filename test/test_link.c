int fib(int n) { return n <= 1 ? n : fib(n - 1) + fib(n - 2); }

int test001();
int test002();

int main() { return fib(5) == 5 && test001() == 10 && test002() == 20; }
