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

    printf("answer: %ld\n", math(num1, num2, my_div));

    return 0;
}