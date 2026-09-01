#include <stdint.h>

// Maximum matrix dimension supported by the internal systolic array
#define MAX_SIZE 16

// Used only for HLS loop tripcount estimation
const unsigned int c_size = MAX_SIZE;

extern "C" {

void mmult(
    const int* a,   // Input Matrix A
    const int* b,   // Input Matrix B
    int* c,         // Output Matrix C

    int a_row,      // Number of rows in A
    int a_col,      // Number of columns in A
    int b_col       // Number of columns in B
)
{
    // ============================================================
    // HLS INTERFACES
    // ============================================================

    // AXI4 master interface for accessing matrices in PS DDR.
    //
    // A, B and C share one AXI master interface in this first
    // implementation. The accelerator reads A, then B, performs
    // computation, and finally writes C.
#pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem depth=256

#pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem depth=256

#pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem depth=256


    // AXI4-Lite control interface.
    //
    // Linux/PetaLinux will use these registers to provide:
    //   - physical address of A
    //   - physical address of B
    //   - physical address of C
    //   - matrix dimensions
    //   - start/done control
#pragma HLS INTERFACE s_axilite port=a bundle=control

#pragma HLS INTERFACE s_axilite port=b bundle=control

#pragma HLS INTERFACE s_axilite port=c bundle=control

#pragma HLS INTERFACE s_axilite port=a_row bundle=control

#pragma HLS INTERFACE s_axilite port=a_col bundle=control

#pragma HLS INTERFACE s_axilite port=b_col bundle=control

#pragma HLS INTERFACE s_axilite port=return bundle=control


    // ============================================================
    // MATRIX DIMENSIONS
    // ============================================================

    // Matrix multiplication:
    //
    // A: a_row x a_col
    // B: a_col x b_col
    // C: a_row x b_col

    int b_row = a_col;
    int c_row = a_row;
    int c_col = b_col;


    // ============================================================
    // LOCAL ON-CHIP MATRICES
    // ============================================================

    int localA[MAX_SIZE][MAX_SIZE];

    // Partition rows completely.
    //
    // Allows multiple rows of A to be accessed in parallel.
#pragma HLS ARRAY_PARTITION variable=localA dim=1 complete


    int localB[MAX_SIZE][MAX_SIZE];

    // Partition columns completely.
    //
    // Allows multiple columns of B to be accessed in parallel.
#pragma HLS ARRAY_PARTITION variable=localB dim=2 complete


    int localC[MAX_SIZE][MAX_SIZE];

    // Completely partition C.
    //
    // Each C[i][j] can effectively become an independent register.
#pragma HLS ARRAY_PARTITION variable=localC dim=0 complete


    // ============================================================
    // LOAD MATRIX A FROM DDR
    // ============================================================

readA:
    for (
        int loc = 0, i = 0, j = 0;
        loc < a_row * a_col;
        loc++, j++
    )
    {
#pragma HLS LOOP_TRIPCOUNT min=c_size*c_size max=c_size*c_size

        if (j == a_col)
        {
            i++;
            j = 0;
        }

        localA[i][j] = a[loc];
    }


    // ============================================================
    // LOAD MATRIX B FROM DDR
    // ============================================================

readB:
    for (
        int loc = 0, i = 0, j = 0;
        loc < b_row * b_col;
        loc++, j++
    )
    {
#pragma HLS LOOP_TRIPCOUNT min=c_size*c_size max=c_size*c_size

        if (j == b_col)
        {
            i++;
            j = 0;
        }

        localB[i][j] = b[loc];
    }


    // ============================================================
    // SYSTOLIC MATRIX MULTIPLICATION
    // ============================================================
    //
    // C[i][j] += A[i][k] * B[k][j]
    //
    // i and j are completely unrolled.
    //
    // For MAX_SIZE = 16 this creates up to:
    //
    //      16 x 16 = 256
    //
    // multiply-accumulate processing paths operating in parallel.
    //
    // k advances through the inner dimension of the matrices.
    // ============================================================

systolic1:
    for (int k = 0; k < a_col; k++)
    {
#pragma HLS LOOP_TRIPCOUNT min=c_size max=c_size

    systolic2:
        for (int i = 0; i < MAX_SIZE; i++)
        {
#pragma HLS UNROLL

        systolic3:
            for (int j = 0; j < MAX_SIZE; j++)
            {
#pragma HLS UNROLL

                // For k = 0, start accumulation at zero.
                // Otherwise reuse the previous partial sum.
                int last =
                    (k == 0)
                        ? 0
                        : localC[i][j];


                // Boundary protection.
                //
                // This allows dimensions smaller than MAX_SIZE.
                int a_val =
                    (i < a_row && k < a_col)
                        ? localA[i][k]
                        : 0;


                int b_val =
                    (k < b_row && j < b_col)
                        ? localB[k][j]
                        : 0;


                int result =
                    last + a_val * b_val;


                localC[i][j] = result;
            }
        }
    }


    // ============================================================
    // WRITE MATRIX C BACK TO DDR
    // ============================================================

writeC:
    for (
        int loc = 0, i = 0, j = 0;
        loc < c_row * c_col;
        loc++, j++
    )
    {
#pragma HLS LOOP_TRIPCOUNT min=c_size*c_size max=c_size*c_size

        if (j == c_col)
        {
            i++;
            j = 0;
        }

        c[loc] = localC[i][j];
    }
}

} // extern "C"
