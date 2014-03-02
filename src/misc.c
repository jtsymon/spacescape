
#include "misc.h"

void end(int status) {
#ifdef wait_for_fucking_retarded_netbeans_terminal
    usleep(100000);
#endif
    exit(status);
}

void vprint_err(const char* format, va_list args) {
    fprintf(stderr, "\033[0;31m");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\033[0m\n");
}

void print_err(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprint_err(format, args);
    va_end(args);
}

void fail(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprint_err(format, args);
    va_end(args);
    end(1);
}

unsigned long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)1000000 * tv.tv_sec + tv.tv_usec;
}

int max(int a, int b) {
    return a > b ? a : b;
}
int min(int a, int b) {
    return a < b ? a : b;
}