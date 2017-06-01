#pragma once

#define VM_FLAT __attribute__((flatten))
#define VM_INLINE __attribute__((always_inline, flatten))
#define SWAP(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)

#define VM_NOCOPY_MUTEX 0xFFFF
#define VM_ATOM_FALSE 0
#define VM_ATOM_TRUE 1
#define VM_ATOM_NIL 2
#define VM_VECTOR_MAIN 3
#define VM_PAGE_SIZE (16*1024)
#define VM_THREAD_STACK_SIZE 16

#define VM_HEAP_TRESHOLD_HIGH	8.00
#define VM_HEAP_TRESHOLD_LOW	16.00
#define VM_HEAP_RESIZE_HIGH		8.00
#define VM_HEAP_RESIZE_LOW		0.50

#define VM_CHECK_BOUNDS 1