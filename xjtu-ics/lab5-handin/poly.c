#include "poly.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <bits/time.h>
#include <math.h>
void poly_optim(const double a[], double x, long degree, double *result)
{
    const int N = 12;
    double r[N];
    double xx[12] = {1};

    for (int i = 0; i < N; i++)
    {
        r[i] = a[degree - i];
    }

    for (int i = 1; i < N; i++)
    {
        xx[i] = xx[i - 1] * x;
    }
    xx[12] = xx[10] * xx[2];

    for (int i = degree - N; i >= N - 1; i -= N)
    {

        r[0] = r[0] * xx[N] + a[i];
        r[1] = r[1] * xx[N] + a[i - 1];
        r[2] = r[2] * xx[N] + a[i - 2];
        r[3] = r[3] * xx[N] + a[i - 3];
        r[4] = r[4] * xx[N] + a[i - 4];
        r[5] = r[5] * xx[N] + a[i - 5];
        r[6] = r[6] * xx[N] + a[i - 6];
        r[7] = r[7] * xx[N] + a[i - 7];
        r[8] = r[8] * xx[N] + a[i - 8];
        r[9] = r[9] * xx[N] + a[i - 9];
        r[10] = r[10] * xx[N] + a[i - 10];
        r[11] = r[11] * xx[N] + a[i - 11];
    }
    double t1 = 0;
    for (int i = (degree + 1) % N; i > 0; i--)
    {
        t1 = t1 * x + a[i - 1];
    }
    double t2 = 0;
    for (int i = 0; i < N; i++)
    {
        t2 += r[i] * xx[N - 1 - i];
    }
    for (int i = 0; i < (degree + 1) % N; i++)
    {
        t2 *= x;
    }
    *result = t1 + t2;
}

void measure_time(poly_func_t poly, const double a[], double x, long degree, double *time)
{
    const int num_iterations = 50; // Number of iterations for averaging
    double result = -1;
    double total_time = 0;

    // Loop for multiple iterations to get average time
    for (int i = 0; i < num_iterations; i++)
    {
        // Warm-up cache
        poly(a, x, degree, &result);

        // Measure start time
        struct timespec start_time;
        clock_gettime(CLOCK_REALTIME, &start_time);

        // Perform the function to be measured
        poly(a, x, degree, &result);

        // Measure end time
        struct timespec end_time;
        clock_gettime(CLOCK_REALTIME, &end_time);

        // Calculate time difference in nanoseconds
        long long elapsed_ns = (end_time.tv_sec - start_time.tv_sec) * 1e9 +
                               (end_time.tv_nsec - start_time.tv_nsec);

        // Accumulate total time
        total_time += elapsed_ns;
    }

    // Calculate average time
    *time = total_time / num_iterations;
}