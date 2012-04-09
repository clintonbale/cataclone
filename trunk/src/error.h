#pragma once
#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

//Enable and show errors?
#define ERRORS_ENABLED      1

//Enable and show warnings?
#define WARNINGS_ENABLED    1

static void error(const char* fmt, ...) {
#if ERRORS_ENABLED
    va_list argptr;

    fprintf(stderr, "[ERROR] ");
    va_start(argptr,  fmt);
    vfprintf(stderr, (char*)fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
#endif
}

static void warn(const char* fmt, ...) {
#if WARNINGS_ENABLED
    va_list argptr;

    fprintf(stderr, "[WARN] ");
    va_start(argptr,  fmt);
    vfprintf(stderr, (char*) fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");
#endif
}

static void debug(const char* fmt, ...) {
#if WARNINGS_ENABLED
    va_list argptr;

    fprintf(stderr, "[DEBUG] ");
    va_start(argptr,  fmt);
    vfprintf(stderr, (char*) fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");
#endif
}

#endif //_ERROR_H_
