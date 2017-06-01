#pragma once

#include "vm_util.h"

static VM_INLINE uint8_t* vm_dynamicmem_alloc(vm_context_t* context, size_t size){
	vm_alloc_lock(context);
	vm_dynamicmem_t* dmem = &context->root->dmem;
	if(dmem->size < dmem->offset+size){
		vm_alloc_unlock(context);
		vm_throw(context,VM_EXCEPTION_OOM);
	}
	uint8_t* ptr = dmem->data+dmem->offset;
	dmem->offset += size;
	vm_alloc_unlock(context);
	return ptr;
}

#define VM_SWAP(T, x, y) do { T temp; temp = x; x = y; y = temp; } while (0)
#define VM_PAGE_WEIGHT(a)	(((size_t*)a)[0])

static VM_INLINE void vm_heap_repup(vm_context_t* context){
	uint8_t** T = context->heap.page;
	size_t a = context->heap.size-1;
	while(a > 0){
		size_t b = (a-1)>>1;
		if(VM_PAGE_WEIGHT(T[a]) > VM_PAGE_WEIGHT(T[b]))
			return;
		VM_SWAP(uint8_t*,T[a],T[b]);
		a = b;
	}
}

static VM_INLINE void vm_heap_repdown(vm_context_t* context){
	uint8_t** T = context->heap.page;
	size_t size = context->heap.size;
	size_t a = 0;
	while(1){
		if((a<<1)+1 >= size)
			break;
		size_t b;
		if((a<<1)+2 >= size)
			b = (a<<1)+1;
		else
			b = VM_PAGE_WEIGHT(T[(a<<1)+1]) <= VM_PAGE_WEIGHT(T[(a<<1)+2])?(a<<1)+1:(a<<1)+2;
		if(VM_PAGE_WEIGHT(T[b]) > VM_PAGE_WEIGHT(T[a]))
			break;
		VM_SWAP(uint8_t*,T[a],T[b]);
		a = b;
	}
}

#undef VM_SWAP

static VM_INLINE uint8_t* vm_heap_alloc(vm_context_t* context, size_t size){
	if(context->heap.offset + size > VM_PAGE_SIZE){
		if(context->heap.data != NULL){
			VM_PAGE_WEIGHT(context->heap.data) = context->heap.offset;
			vm_heap_repdown(context);
			context->heap.data = context->heap.page[0];
			context->heap.offset = VM_PAGE_WEIGHT(context->heap.data);
		}
		if(context->heap.offset + size > VM_PAGE_SIZE){
			if(size > VM_PAGE_SIZE-sizeof(size_t))
				return vm_dynamicmem_alloc(context,size);
			uint8_t* page = vm_dynamicmem_alloc(context,VM_PAGE_SIZE);
			VM_PAGE_WEIGHT(page) = size+sizeof(size_t);
			context->heap.page[context->heap.size++] = page;
			vm_heap_repup(context);
			context->heap.data = context->heap.page[0];
			context->heap.offset = VM_PAGE_WEIGHT(context->heap.data);
			return page+sizeof(size_t);
		}
	}
	uint8_t* block = context->heap.data + context->heap.offset;
	context->heap.offset += size;
	return block;
}

#undef VM_PAGE_WEIGHT

static VM_INLINE vm_variable_t* vm_allocate(vm_context_t* context, size_t size){
	vm_variable_t* v = (vm_variable_t*)(vm_heap_alloc(context,size));
	v->clone = 0;
	v->mutex_id = 0;
	return v;
}

static VM_INLINE vm_variable_t* vm_allocate_undef(vm_context_t* context){
	vm_variable_t* v = vm_allocate(context, 4+sizeof(vm_variable_t*));
	v->type = VM_VARIBLE_UNDEF;
	v->value.v_varible = NULL;
	v->mutex_id = vm_random_mutex(context);
	return v;
}

static VM_INLINE vm_variable_t* vm_allocate_int(vm_context_t* context){
	vm_variable_t* v = vm_allocate(context, 4+sizeof(vm_int_t));
	v->type = VM_VARIBLE_INT;
	return v;
}

static VM_INLINE vm_variable_t* vm_allocate_float(vm_context_t* context){
	vm_variable_t* v = vm_allocate(context, 4+sizeof(vm_float_t));
	v->type = VM_VARIBLE_FLOAT;
	return v;
}

static VM_INLINE vm_variable_t* vm_allocate_atom(vm_context_t* context){
	vm_variable_t* v = vm_allocate(context, 4+sizeof(vm_atom_t));
	v->type = VM_VARIBLE_ATOM;
	return v;
}

static VM_INLINE vm_variable_t* vm_allocate_pair(vm_context_t* context){
	vm_variable_t* v = vm_allocate(context, 4+sizeof(vm_pair_t));
	v->type = VM_VARIBLE_PAIR;
	return v;
}

static VM_INLINE vm_variable_t* vm_allocate_tuple(vm_context_t* context, size_t size){
	vm_variable_t* v = vm_allocate(context, 4+sizeof(vm_record_t)+size*sizeof(vm_variable_t*));
	v->type = VM_VARIBLE_TUPLE;
	return v;
}

static VM_INLINE vm_variable_t* vm_allocate_record(vm_context_t* context, size_t size){
	vm_variable_t* v = vm_allocate(context, 4+sizeof(vm_record_t)+size*sizeof(vm_record_pair_t));
	v->type = VM_VARIBLE_RECORD;
	return v;
}

static VM_INLINE vm_variable_t* vm_allocate_function(vm_context_t* context, size_t size){
	vm_variable_t* v = vm_allocate(context, 4+sizeof(vm_function_instance_t)+size*sizeof(vm_variable_t*));
	v->type = VM_VARIBLE_FUNCTION;
	return v;
}

static VM_INLINE vm_variable_t* vm_allocate_thread(vm_context_t* context, size_t size){
	vm_variable_t* v = vm_allocate(context, 4+sizeof(vm_thread_t)+size*sizeof(vm_variable_t**));
	v->type = VM_VARIBLE_THREAD;
	v->value.v_thread.next = NULL;
	return v;
}
