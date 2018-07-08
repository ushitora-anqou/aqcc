#include <stdio.h>

int main()
{
    int input;

    scanf("%d", &input);
    puts(".global main");
    puts("main:");
    printf("\tmov $%d, %%eax\n", input);
    puts("\tret");

    return 0;
}
