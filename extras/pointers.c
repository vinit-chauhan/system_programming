#include <stdio.h>

int main()
{

    int *a;
    int b = 100;

    // This statement will give an Segmentation fault Error.
    // Because there's no address associated with the pointer yet.
    // *a = 11231;

    a = &b;

    // This statement is valid because, a is associated with b
    *a = 11231;

    printf("\n");
    printf("a: %d, b:%d", *a, b);
    printf("\n");

    return 0;
}