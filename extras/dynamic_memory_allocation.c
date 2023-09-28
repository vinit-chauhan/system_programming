#include <stdio.h>
#include <stdlib.h>

int
main() {
    int n;

    scanf("%d", &n);
    int* ptr;

    ptr = malloc(n * sizeof(int)); // n is the number of elements in the array

    if (ptr == NULL) {
        printf("Memory not allocated.\n");
        exit(0);
    }

    printf("Enter elements: ");
    for (int i = 0; i < n; i++) {
        scanf("%d", ptr + i);
    }

    printf("Elements of array are: [");
    for (int i = 0; i < n; i++) {
        if (i == n - 1) {
            printf("%d]\n", *(ptr + i));
        } else {
            printf("%d, ", *(ptr + i));
        }
    }

    return 0;
}