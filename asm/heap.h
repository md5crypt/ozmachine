#pragma once
#include <stdlib.h>
#include <inttypes.h>

#define HEAP_BLOCK_COUNT	1024*4
#define HEAP_BLOCK_SIZE		1024*16

typedef struct {
	size_t pos;
	uint32_t size;
	uint8_t* current;
	uint8_t** mem;
} heap_t;

int heap_create(heap_t* heap);
void heap_destroy(heap_t* heap);
void* heap_alloc(heap_t* heap, size_t n);
