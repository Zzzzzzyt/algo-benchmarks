#include <stdio.h>
#include <time.h>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include <cpuid.h>
#include <math.h>

#define MEMORY_BARRIER asm volatile("" : : : "memory")

typedef unsigned long long ull;

inline ull get_monotonic_time() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

int main() {
    if (__get_cpuid_max(0, 0) >= 0x15) {
        printf("Using CPUID to get TSC frequency\n");
        unsigned int regs[4] = {0, 0, 0, 0};
        __get_cpuid_count(0x15, 0, &regs[0], &regs[1], &regs[2], &regs[3]);
        printf("eax=%x ebx=%x ecx=%x edx=%x\n", regs[0], regs[1], regs[2], regs[3]);
        ull denom = regs[0];
        ull numer = regs[1];
        ull crystal_freq = regs[2];
        if (denom == 0 || numer == 0 || crystal_freq == 0) {
            printf("Invalid values from cpuid\n");
        } else {
            printf("denominator=%llu numerator=%llu crystal_freq=%llu\n", denom, numer, crystal_freq);
            double factor = crystal_freq * numer / denom / 1e9;
            printf("factor=%f\n", factor);
        }
    }

    printf("Measuring TSC frequency......\n");

    const int N = 1000;
    ull real_arr[N], tsc_arr[N];
    for (int i = 0; i < N; i++) {
        ull real_st = get_monotonic_time();
        MEMORY_BARRIER;
        ull tsc_st = __rdtsc();
        MEMORY_BARRIER;
        for (int j = 0; j < 10000000 + i * 10000; j++) {
            MEMORY_BARRIER;
        }
        MEMORY_BARRIER;
        ull real_ed = get_monotonic_time();
        MEMORY_BARRIER;
        ull tsc_ed = __rdtsc();
        real_arr[i] = real_ed - real_st;
        tsc_arr[i] = tsc_ed - tsc_st;
    }

    // Linear regression: tsc = a * real + b
    double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
    for (int i = 0; i < N; i++) {
        double x = (double)real_arr[i];
        double y = (double)tsc_arr[i];
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
    }
    double denom = N * sum_xx - sum_x * sum_x;
    double a = (N * sum_xy - sum_x * sum_y) / denom;
    double b = (sum_y * sum_xx - sum_x * sum_xy) / denom;

    printf("tsc = %.6f * real + %.6f\n", a, b);
    printf("Estimated TSC frequency: %.3f MHz\n", a * 1e3);

    FILE *fp = fopen("tsc_freq.txt", "w");
    if (fp) {
        fprintf(fp, "%.10f\n", a);
        fclose(fp);
    }

    return 0;
}