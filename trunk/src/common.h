#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define SHOW_ERRORS     1
#define SHOW_WARNINGS   1

#ifdef _DEBUG
    #define SHOW_DEBUG      1
#else
    #define SHOW_DEBUG      0
#endif

#undef bool
#undef true
#undef false
typedef enum __attribute__((packed)) {false, true} bool;

typedef uint8_t     byte;
typedef uint16_t    ushort;
typedef uint32_t    uint;
typedef uint64_t    ulong;

typedef union {
    struct {
        float x;
        float y;
    };
    float xy[2];
} vec2f_t;

typedef union {
    struct {
        int32_t x;
        int32_t y;
    };
    int32_t xy[2];
} vec2_t;

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884197
#endif

#if SHOW_ERRORS
    #define error(MSG, ...) {fprintf(stderr, "[ERROR] (%s:%i) "MSG"\n", __FILE__, __LINE__, ##__VA_ARGS__); exit(1);}
#else
    #define error(MSG,...) { }
#endif

#if SHOW_WARNINGS
    #define warn(fmt, ...) fprintf(stdout, "[WARNING] "fmt"\n", ##__VA_ARGS__)
#else
    #define warn(fmt, ...) { }
#endif

#if SHOW_DEBUG
    #define debug(fmt, ...) fprintf(stdout, "[DEBUG] "fmt"\n", ##__VA_ARGS__)
#else
    #define debug(fmt, ...) { }
#endif

bool check_little_endian(void);
int  endian_swap(int val);

#endif //_COMMON_H_
