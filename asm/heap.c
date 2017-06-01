#include "heap.h"

#define SWAP(T, x, y) do { T temp; temp = x; x = y; y = temp; } while (0)
#define HEAP_GET_WIGHT(a)	(((size_t*)a)[0])

static inline void heap_repup(uint8_t** T, uint32_t size){
	uint32_t a = size-1;
	while(a > 0){
		uint32_t b = (a-1)>>1;
		if(HEAP_GET_WIGHT(T[a]) > HEAP_GET_WIGHT(T[b]))
			return;
		SWAP(uint8_t*,T[a],T[b]);
		a = b;
	}
}

static inline void heap_repdown(uint8_t** T, uint32_t size){
	uint32_t a = 0;
	while(1){
		if((a<<1)+1 >= size)
			break;
		uint32_t b;
		if((a<<1)+2 >= size)
			b = (a<<1)+1;
		else
			b = HEAP_GET_WIGHT(T[(a<<1)+1]) <= HEAP_GET_WIGHT(T[(a<<1)+2])?(a<<1)+1:(a<<1)+2;
		if(HEAP_GET_WIGHT(T[b]) > HEAP_GET_WIGHT(T[a]))
			break;
		SWAP(uint8_t*,T[a],T[b]);
		a = b;
	}
}

int heap_create(heap_t* heap){
	heap->size = 1;
	heap->pos = HEAP_BLOCK_COUNT*sizeof(uint8_t**) + sizeof(size_t);
	uint8_t* block = (uint8_t*)malloc(HEAP_BLOCK_SIZE);
	if(block == NULL)
		return 1;
	heap->mem = (uint8_t**)(block+sizeof(size_t));
	heap->mem[0] = block;
	heap->current = block;
	return 0;
}

void heap_destroy(heap_t* heap){
	for(uint32_t i=1; i<heap->size; i++){
		if(heap->mem[i] != ((uint8_t*)heap->mem)-sizeof(size_t))
			free(heap->mem[i]);
	}
	free(((uint8_t*)heap->mem)-sizeof(size_t));
}

void* heap_alloc(heap_t* heap, size_t n){
	if(heap->pos + n > HEAP_BLOCK_SIZE){
		HEAP_GET_WIGHT(heap->current) = heap->pos;
		heap_repdown(heap->mem, heap->size);
		heap->current = heap->mem[0];
		heap->pos = HEAP_GET_WIGHT(heap->current);
		if(heap->pos + n > HEAP_BLOCK_SIZE){
			uint8_t* block;
			if(n > HEAP_BLOCK_SIZE-sizeof(size_t))
				block = (uint8_t*)malloc(n+sizeof(size_t));
			else
				block = (uint8_t*)malloc(HEAP_BLOCK_SIZE);
			if(block == NULL)
				return NULL;
			HEAP_GET_WIGHT(block) = n+sizeof(size_t);
			heap->mem[heap->size++] = block;
			heap_repup(heap->mem, heap->size);
			heap->current = heap->mem[0];
			heap->pos = HEAP_GET_WIGHT(heap->current);
			return block+sizeof(size_t);
		}
	}
	uint8_t* block = heap->current+heap->pos;
	heap->pos += n;
	return block;
}