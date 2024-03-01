#include "print.h"

#include <stdio.h>
#include <stdarg.h>

int print(const char *const fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = vprintf(fmt, args);
    va_end(args);
    return result;
}
