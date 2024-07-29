#include <time.h>
#include <stdio.h>
#include "native.c"
void* mmalloc(size_t size) {
    void* result = malloc(size);
    if (result == NULL) {
        printf("bad_alloc\n");
        exit(1);
    }
    return result;
}
void throw_RangeError(const char* msg) {
    printf("Error: %s\n", msg);
    exit(1);
};
#define IT 1000
uint32_t testarr[81];
typedef void (*void_func_t)(void);

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

typedef LARGE_INTEGER TimeType;
typedef double (*perfomance_unit)(TimeType start, TimeType end, TimeType frequency);
double perfomance_microseconds(TimeType start, TimeType end, TimeType frequency) {
    return (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1e6;
}

double perfomance_milliseconds(TimeType start, TimeType end, TimeType frequency) {
    return (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1e3;
}

double perfomance_seconds(TimeType start, TimeType end, TimeType frequency) {
    return (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
}

void get_time(TimeType *time) {
    QueryPerformanceCounter(time);
}

void get_frequency(TimeType *frequency) {
    QueryPerformanceFrequency(frequency);
}

#else
#define _POSIX_C_SOURCE 199309L
#include <time.h>

typedef struct timespec TimeType;
typedef double (*perfomance_unit)(struct timespec start, struct timespec end);
double perfomance_microseconds(TimeType start, TimeType end) {
    double start_us = (double)start.tv_sec * 1e6 + (double)start.tv_nsec / 1e3;
    double end_us = (double)end.tv_sec * 1e6 + (double)end.tv_nsec / 1e3;
    return end_us - start_us;
}

double perfomance_milliseconds(TimeType start, TimeType end) {
    double start_ms = (double)start.tv_sec * 1e3 + (double)start.tv_nsec / 1e6;
    double end_ms = (double)end.tv_sec * 1e3 + (double)end.tv_nsec / 1e6;
    return end_ms - start_ms;
}

double perfomance_seconds(TimeType start, TimeType end) {
    double start_s = (double)start.tv_sec + (double)start.tv_nsec / 1e9;
    double end_s = (double)end.tv_sec + (double)end.tv_nsec / 1e9;
    return end_s - start_s;
}

void get_time(TimeType *time) {
    clock_gettime(CLOCK_MONOTONIC, time);
}

#endif
const char* unit_str(perfomance_unit unit) {

    if (unit == perfomance_microseconds)
        return "microseconds";
    if (unit == perfomance_milliseconds)
        return "milliseconds";
    else
        return "seconds";
}
double testf(size_t iterations, int print, const char* name, perfomance_unit unit, void (*fun)()) {
    TimeType start_time, end_time, frequency;
    
    get_frequency(&frequency);
    get_time(&start_time);
    
    for (size_t i = 0; i < iterations; ++i) {
        fun();
    }
    
    get_time(&end_time);
    
    double elapsed_time = unit(start_time, end_time, frequency);
    if (print) {
        printf("Time taken");
        if (name != NULL) {
            printf(" for %s", name);
        }
        printf(": %.2f %s\n", elapsed_time, unit_str(unit));
    }
    return elapsed_time;
}

void test_all(size_t iterations, int print, int count, perfomance_unit unit, void_func_t test1, ...) {
    if (count == 0) {
        printf("zero count\n");
        exit(0);
    }
    va_list args;
    va_start(args, test1);
    for (int i = 0; i < count; ++i)
        testf(print, iterations, NULL, unit, va_arg(args, void_func_t));
}
void stack_test() {
    test_stack(testarr, sizeof(testarr));
}
void heap_test() {
    test_heap(testarr, sizeof(testarr));
}
int main(void) {
    for (int i = 0; i < sizeof(testarr); ++i) {
        testarr[i] = i;
    }
    size_t it = 1000; // Number of iterations
    
    // Measure the execution time of the functions
    testf(it, 1, "stack", perfomance_milliseconds, stack_test); // 83-244
    testf(it, 1, "heap", perfomance_milliseconds, heap_test);  // 593 - 1525
    // stack optimization is usually faster for around 1000 microseconds (if it = 1000) - 1 millisecond
    
    return 0;
}