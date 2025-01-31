#include <stdio.h>
#include <stdarg.h>

int print(char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(fmt, args);
    va_end(args);
    return ret;
}

int println(char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(fmt, args);
    va_end(args);
    printf("\n");
    return ret + 1;
}

int read(char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vscanf(fmt, args);
    va_end(args);
    return ret;
}
