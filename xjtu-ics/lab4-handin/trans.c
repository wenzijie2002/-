/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_32x32(int M, int N, int A[N][M], int B[M][N])
{
    int a[8];
    for (int i = 0; i < N; i += 8)
    { // 枚举每八行
        for (int j = 0; j < M; j += 8)
        { // 枚举每八列
            for (int k = 0; k < 8; ++k, ++i)
            {                   // 枚举0~8中的每一行，一行八列
                a[0] = A[i][j]; // 这八个只会发生一次miss
                a[1] = A[i][j + 1];
                a[2] = A[i][j + 2];
                a[3] = A[i][j + 3];
                a[4] = A[i][j + 4];
                a[5] = A[i][j + 5];
                a[6] = A[i][j + 6];
                a[7] = A[i][j + 7];

                B[j][i] = a[0]; // 第一次 这八个都会 miss,后面就会命中，当然对角线有些例外
                B[j + 1][i] = a[1];
                B[j + 2][i] = a[2];
                B[j + 3][i] = a[3];
                B[j + 4][i] = a[4];
                B[j + 5][i] = a[5];
                B[j + 6][i] = a[6];
                B[j + 7][i] = a[7];
            }
            i -= 8;
        }
    }
}

void transpose_64x64(int M, int N, int A[N][M], int B[M][N])
{
    int a[8];
    for (int i = 0; i < N; i += 8)
    {
        for (int j = 0; j < M; j += 8)
        {
            for (int k = i; k < i + 4; k++)
            {
                for (int l = 0; l <= 7; l++)
                {
                    a[l] = A[k][j + l];
                }
                for (int l = 0; l <= 3; l++)
                {
                    B[j + l][k] = a[l];
                    B[j + l][k + 4] = a[l + 4];
                }
            }
            for (int k = j; k < j + 4; k++)
            {
                for (int l = 0; l <= 3; l++)
                {
                    a[l] = B[k][i + l + 4];
                    a[l + 4] = A[i + l + 4][k];
                }
                for (int l = 0; l <= 3; l++)
                {
                    B[k][i + l + 4] = a[l + 4];
                }
                for (int l = 0; l <= 3; l++)
                {
                    B[k + 4][i + l] = a[l];
                }
            }
            for (int k = i + 4; k < i + 8; k++)
            {
                for (int l = 4; l <= 7; l++)
                {
                    B[j + l][k] = A[k][j + l];
                }
            }
        }
    }
}
void transpose_61x67(int M, int N, int A[N][M], int B[M][N])
{
    for (int i = 0; i < N; i += 21)
        for (int j = 0; j < M; j += 21)
            for (int k = i; k < i + 21 && k < N; k++)
                for (int l = j; l < j + 21 && l < M; l++)
                    B[l][k] = A[k][l];
}
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    switch (M * 100 + N)
    {
    case 3232:
        transpose_32x32(M, N, A, B);
        break;
    case 6464:
        transpose_64x64(M, N, A, B);
        break;
    case 6167:
        transpose_61x67(M, N, A, B);
        break;
    default:
        break;
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
