#include "memory.h"
#include <string.h>

#define HEADER_MAGIC  0xCBCBCBCB
#define TRAILER_MAGIC 0xCFB00CFB

//
// Slightly inspired by John Carmacks Zone Memory Allocator
//

typedef struct {
    uint size;
    uint magic;
} memblock_t;

static uint     memory_total_allocs= 0;
static uint     memory_total_frees = 0;
static ulong    memory_total_size  = 0;

/*
===============
memory_check_block

Checks to make sure a block is valid.
===============
*/
static bool memory_check_block(const memblock_t* block) {
    if(block == NULL) {
        error("memory_check_block: Block is NULL.");
        return false;
    }
    if(block->size == 0 || block->magic != HEADER_MAGIC) {
        error("memory_check_block: Corrupt memory block.");
        return false;
    }
    if(*(uint*)((byte*)block + sizeof(memblock_t) + block->size) != TRAILER_MAGIC) {
        error("memory_check_block: Memory block wrote past end.");
        return false;
    }
    return true;
}


/*
===============
memory_alloc
===============
*/
void* memory_alloc(uint size) {
    memblock_t* base;
    uint        aligned_size;

    if(size == 0)
        return NULL;

    //
    // Account for size of block header
    //
    size += sizeof(memblock_t);
    //
    // Request extra space for trash tester
    //
    size += sizeof(uint);
    //
    // Align to 32bit boundary
    //
    aligned_size = (size + 3) & ~3;

    base = (memblock_t*)malloc(aligned_size);
    if(base == NULL) {
        error("memory_alloc: Failed on allocation of %d bytes.", aligned_size);
        return NULL;
    }
    memory_total_allocs++;
    memory_total_size  += aligned_size;

    base->size = size - sizeof(memblock_t) - sizeof(uint);
    base->magic = HEADER_MAGIC;

    *(uint*)((byte*)base + sizeof(memblock_t) + base->size) = TRAILER_MAGIC;

    return (void*)((byte*)base + sizeof(memblock_t));
}


/*
===============
memory_calloc
===============
*/
void* memory_calloc(uint size) {
    void* block;

    block = memory_alloc(size);
    memset(block, 0, size);

    return block;
}


/*
===============
memory_size

The size (in bytes) available in this block.
===============
*/
uint memory_size(const void* ptr) {
    memblock_t* base;

    if(ptr == NULL) return 0;

    base = (memblock_t*)((byte*)ptr - sizeof(memblock_t));
    if(!memory_check_block(base))
        return 0;

    return base->size;
}


/*
===============
memory_free
===============
*/
void memory_free(void* ptr) {
    memblock_t* base;

    if(ptr == NULL) return;

    base = (memblock_t*)((byte*)ptr - sizeof(memblock_t));
    if(!memory_check_block(base))
        return;

    base->magic = 0;
    base->size = 0;

    free(base);
    base = NULL;
    ptr = NULL;

    memory_total_frees++;
}


/*
===============
memory_report

Outputs a report on how many allocs, how many frees,
the size of all the allocs and whether or not there is a memory leak.
===============
*/
bool memory_report() {
    fprintf(stdout,
            "==================================\n"
            "          Memory Report\n"
            "==================================\n"
            "Allocs : %d\n"
            "Frees  : %d\n"
            "Size   : %llu B \n"
            "Size   : %.4f MB\n"
            "Leak?  : %s\n"
            "==================================\n",
            memory_total_allocs,
            memory_total_frees,
            memory_total_size,
            (double)memory_total_size/1024.0f/1024.0f,
            memory_total_allocs!=memory_total_frees?"YES":"NO");
    //
    // Return true if there is a memory leak.
    //
    return memory_total_allocs!=memory_total_frees;
}
