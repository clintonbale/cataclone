#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include "error.h"

#undef bool
#undef true
#undef false
typedef enum {false, true} bool;

typedef uint8_t     byte;
typedef uint16_t    ushort;
typedef uint32_t    uint;
typedef float       vec2f_t[2];
typedef int32_t     vec2_t[2];

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool check_little_endian(void);
int  endian_swap(int val);

#endif //_COMMON_H_
