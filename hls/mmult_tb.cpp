#include <iostream>

#define SIZE 16

extern "C" void mmult(
    const int* a,
    const int* b,
    int* c,
    int a_row,
    int a_col,
    int b_col);

int main()
{
    int A[SIZE * SIZE];
    int B[SIZE * SIZE];
    int C[SIZE * SIZE];
    int gold[SIZE * SIZE];

    for (int i = 0; i < SIZE * SIZE; i++) {
        A[i] = i % 10;
        B[i] = i % 10;
        C[i] = 0;
        gold[i] = 0;
    }

    // CPU reference
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            for (int k = 0; k < SIZE; k++) {
                gold[i * SIZE + j] +=
                    A[i * SIZE + k] *
                    B[k * SIZE + j];
            }
        }
    }

    // HLS design
    mmult(
        A,
        B,
        C,
        SIZE,
        SIZE,
        SIZE);

    for (int i = 0; i < SIZE * SIZE; i++) {
        if (C[i] != gold[i]) {
            std::cout
                << "Mismatch at " << i
                << ": HW=" << C[i]
                << " SW=" << gold[i]
                << std::endl;

            return 1;
        }
    }

    std::cout << "TEST PASSED" << std::endl;

    return 0;
}
