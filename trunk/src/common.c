#include "common.h"

bool check_little_endian(void) {
    int x = 1;
    return (*(char*)&x == 1);
}

int endian_swap(int val) {
    return
    ( (val & 0xFF) << 24 ) |
    ( (val & 0xFF00) << 8 ) |
    ( (val & 0xFF0000) >> 8) |
    ( val >> 24 );
}
