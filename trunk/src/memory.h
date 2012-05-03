#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "common.h"

void*   memory_alloc(uint size);
void*   memory_calloc(uint size);
uint    memory_size(const void* ptr);

void    memory_free(void* ptr);
bool    memory_report();

#endif // _MEMORY_H_
