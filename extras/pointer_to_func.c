#include <stdio.h>
#include <stdlib.h>

long math(long num1, long num2, long (*func)(long, long))
{
    return func(num1, num2);
}

long add(long num1, long num2)
{
    return (long)(num1 + num2);
}

long sub(long num1, long num2)
{
    return (long)(num1 - num2);
}

long mul(long num1, long num2)
{
    return (long)(num1 * num2);
}

long my_div(long num1, long num2)
{
    return (long)(num1 / num2);
}

long my_pow(long num1, long num2)
{
    long temp = num1;
    for (int i = 0; i < num2; i++)
    {
        temp = temp * num1;
    }

    return temp;
}

int main()
{
    long num1 = 10;
    long num2 = 5;

    // Anonymous function, but only supported in GNU C. for others it will give error.
    long (*my_func)(long, long) =
        ({
            long __fn__(long x, long y) { return x > y ? x : y; };
            __fn__;
        });

    printf("answer: %ld\n", math(num1, num2, my_div));

    return 0;
}