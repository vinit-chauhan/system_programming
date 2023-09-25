#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

// Size of the array. It's defined by user at run time.
int SIZE = 0;

// Returns the maximum of two integer values.
int maximum(int *n1, int *n2)
{
    // dereference the pointers, compare their values and
    // returns whichever is maximum.
    return (*n1 > *n2) ? *n1 : *n2;
}

// Finds the maximum element from the array.
int maxarray(int *ar)
{
    // assumes the 1st element is max.
    int max = ar[0];

    // iterate through all elements to find the max.
    for (int i = 1; i < SIZE; i++)
    {
        if (ar[i] > max)
        {
            max = ar[i];
        }
    }
}

int prod(
    int (*function_maximum)(int *, int *),
    int (*function_max_array)(int *),
    int *parameter3,
    int *parameter4,
    int *parameter5)
{
    // Store the maximum value out of parameter3 and parameter4.
    int max_of_parameter3_and_parameter4 =
        function_maximum(parameter3, parameter4);

    // Store the maximum value from the array.
    int max_from_parameter5 = function_max_array(parameter5);

    // Computes the product of outputs of two functions.
    return max_of_parameter3_and_parameter4 * max_from_parameter5;
}

int main()
{
    // Create 2 static variables
    int num1 = 20, num2 = 30;

    // Get the size of the array from user.
    // Store the value in a global variable 'SIZE'
    printf("Enter number of elements that you want: ");
    scanf("%d", &SIZE);

    // Create an integer array of size 'SIZE'
    int *dynar = malloc(SIZE * sizeof(int));
    // Print error message and exit with exit code: 1 in error condition.
    if (dynar == NULL)
    {
        printf("[Error] Can't allocate the memory!");
        exit(1);
    }

    // Take user input for the elements of the array.
    printf("Enter values for the array:\n");
    for (int i = 0; i < SIZE; i++)
    {
        scanf("%d", (dynar + i));
    }

    // Print the array in the reverse order.
    printf("dynar: [");
    for (int i = SIZE - 1; i > -1; i--)
    {
        printf("%d, ", *(dynar + i));
    }
    printf("\b\b]\n");

    // calculate product of the given numbers using prod()
    int product = prod(maximum, maxarray, &num1, &num2, dynar);

    // print the output of prod()
    printf("Output of prod(): %d\n", product);

    return 0;
}