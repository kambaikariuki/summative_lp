#include <stdio.h>
#include <stdlib.h>

// Global variable
int global_counter = 0;

// Function 1: initialize array
void init_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i * 2;
    }
}

// Function 2: process array
int process_array(int *arr, int size) {
    int sum = 0;

    for (int i = 0; i < size; i++) {
        if (arr[i] % 4 == 0) {
            sum += arr[i];
        }
    }

    return sum;
}

// Function 3: print result
void print_result(int result) {
    printf("Result: %d\n", result);
}

int main() {
    int size = 5;

    int *arr = (int *)malloc(size * sizeof(int));

    if (!arr) {
        return 1;
    }

    init_array(arr, size);
    int result = process_array(arr, size);

    global_counter = result;

    print_result(global_counter);

    free(arr);
    return 0;
}
