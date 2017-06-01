#include "vm_gc.h"
#include "vm_util.h"

VM_FLAT int vm_dynamicmem_create(vm_root_t* root, size_t page_cnt){
	size_t data_size = page_cnt*VM_PAGE_SIZE;
	size_t heap_size = page_cnt*sizeof(uint8_t*);
	uint8_t* ptr = (uint8_t*)realloc(root->dmem.block,2*data_size + heap_size*root->process_cnt);
	if(ptr == NULL)
		return 0;
	root->dmem.size = data_size;
	root->dmem.block = ptr;
	root->dmem.data = ptr;
	ptr += 2*data_size;
	for(uint32_t i=0; i<root->process_cnt; i++){
		vm_heap_t* heap = &root->process[i].heap;
		heap->size = 0;
		heap->offset = VM_PAGE_SIZE;
		heap->data = NULL;
		heap->page = (uint8_t**)ptr;
		ptr += heap_size;
	}
	return 1;
}

static VM_INLINE int stack_add(vm_variable_t** top, vm_variable_t* src, vm_variable_t** dst){
	while(src->type == VM_VARIBLE_REF)
		src = src->value.v_varible;
	if(dst != NULL)
		dst[0] = src;
	if(src->clone || src->mutex_id == VM_NOCOPY_MUTEX)
		return 0;
	src->clone = 1;
	top[-1] = src;
	return -1;
}

static VM_FLAT void vm_gc_copy(vm_root_t* root){
	uint8_t* dst_ptr = root->dmem.block;
	if(root->dmem.data == root->dmem.block)
		dst_ptr += root->dmem.size;
	root->dmem.data = dst_ptr;
	vm_variable_t** stack_top = (vm_variable_t**)(dst_ptr + root->dmem.size);
	vm_variable_t** stack = stack_top;
	(--stack)[0] = root->queue.head;
	root->queue.head->clone = 1;
	while(stack != stack_top){
		vm_variable_t* src = (stack++)[0];
		vm_variable_t* dst = (vm_variable_t*)dst_ptr;
		dst->type = src->type;
		dst->clone = 0;
		dst->mutex_id = src->mutex_id;
		switch(src->type){
			case VM_VARIBLE_UNDEF:
				dst->value.v_varible = src->value.v_varible;
				if(src->value.v_varible != NULL){
					(--stack)[0] = src->value.v_varible;
					src->value.v_varible->clone = 1;
				}
				dst_ptr += 4+sizeof(vm_variable_t*);
				break;
			case VM_VARIBLE_INT:
				dst->value.v_int = src->value.v_int;
				dst_ptr += 4+sizeof(vm_int_t);
				break;
			case VM_VARIBLE_ATOM:
				dst->value.v_atom = src->value.v_atom;
				dst_ptr += 4+sizeof(vm_atom_t);
				break;
			case VM_VARIBLE_FLOAT:
				dst->value.v_float = src->value.v_float;
				dst_ptr += 4+sizeof(vm_float_t);
				break;
			case VM_VARIBLE_PAIR:
				stack += stack_add(stack,src->value.v_pair.head,&dst->value.v_pair.head);
				stack += stack_add(stack,src->value.v_pair.tail,&dst->value.v_pair.tail);
				dst_ptr += 4+sizeof(vm_pair_t);
				break;
			case VM_VARIBLE_TUPLE:
				dst->value.v_record.size = src->value.v_record.size;
				dst->value.v_record.id = src->value.v_record.id;
				for(uint32_t i=0; i<src->value.v_record.size; i++)
					stack += stack_add(stack,src->value.v_record.data.item[i],&dst->value.v_record.data.item[i]);
				dst_ptr += 4+sizeof(vm_record_t)+src->value.v_record.size*sizeof(vm_variable_t*);
				break;
			case VM_VARIBLE_RECORD:
				dst->value.v_record.size = src->value.v_record.size;
				dst->value.v_record.id = src->value.v_record.id;
				for(uint32_t i=0; i<src->value.v_record.size; i++){
					dst->value.v_record.data.pair[i].key = src->value.v_record.data.pair[i].key;
					stack += stack_add(stack,src->value.v_record.data.pair[i].value,&dst->value.v_record.data.pair[i].value);
				}
				dst_ptr += 4+sizeof(vm_record_t)+src->value.v_record.size*sizeof(vm_record_pair_t);
				break;
			case VM_VARIBLE_FUNCTION:
				dst->value.v_function.prototype = src->value.v_function.prototype;
				for(uint32_t i=0; i<src->value.v_function.prototype->closure_size; i++)
					stack += stack_add(stack,src->value.v_function.closure[i],dst->value.v_function.closure+i);
				dst_ptr += 4+sizeof(vm_function_instance_t)+src->value.v_function.prototype->closure_size*sizeof(vm_variable_t*);
				break;
			case VM_VARIBLE_THREAD:{
				dst->value.v_thread = src->value.v_thread;
				if(src->value.v_thread.next != NULL){
					(--stack)[0] = src->value.v_thread.next;
					src->value.v_thread.next->clone = 1;
				}
				stack += stack_add(stack,src->value.v_thread.env.function,NULL);
				for(uint32_t i=0; i<src->value.v_thread.env.sp; i++)
					stack += stack_add(stack,src->value.v_thread.stack[i],dst->value.v_thread.stack+i);
				for(uint32_t i=src->value.v_thread.env.rsp; i<src->value.v_thread.env.size; i+=3){
					stack += stack_add(stack,src->value.v_thread.stack[i],dst->value.v_thread.stack+i);
					dst->value.v_thread.stack[i+1] = src->value.v_thread.stack[i+1];
					dst->value.v_thread.stack[i+2] = src->value.v_thread.stack[i+2];
				}
				dst_ptr += 4+sizeof(vm_thread_t)+src->value.v_thread.env.size*sizeof(vm_variable_t*);
				break;
			}
			default:
				VM_FATAL("GC bad type");
		}
		src->type = VM_VARIBLE_GCREF;
		src->value.v_varible = dst;
	}
	root->dmem.offset = dst_ptr - root->dmem.data;
}

#define VM_GC_CDREF(x) if((x)->type==VM_VARIBLE_GCREF) VM_GC_DREF(x)
#define VM_GC_DREF(x) x = (x)->value.v_varible

static VM_FLAT void vm_gc_relax(vm_root_t* root){
	VM_GC_DREF(root->queue.head);
	VM_GC_DREF(root->queue.tail);
	uint8_t* dst_ptr = root->dmem.data;
	uint8_t* end = dst_ptr + root->dmem.offset;
	while(dst_ptr < end){
		vm_variable_t* dst = (vm_variable_t*)dst_ptr;
		switch(dst->type){
			case VM_VARIBLE_UNDEF:
				if(dst->value.v_varible != NULL)
					VM_GC_DREF(dst->value.v_varible);
				dst_ptr += 4+sizeof(vm_variable_t*);
				break;
			case VM_VARIBLE_INT:
				dst_ptr += 4+sizeof(vm_int_t);
				break;
			case VM_VARIBLE_ATOM:
				dst_ptr += 4+sizeof(vm_atom_t);
				break;
			case VM_VARIBLE_FLOAT:
				dst_ptr += 4+sizeof(vm_float_t);
				break;
			case VM_VARIBLE_PAIR:
				VM_GC_CDREF(dst->value.v_pair.head);
				VM_GC_CDREF(dst->value.v_pair.tail);
				dst_ptr += 4+sizeof(vm_pair_t);
				break;
			case VM_VARIBLE_TUPLE:
				for(uint32_t i=0; i<dst->value.v_record.size; i++)
					VM_GC_CDREF(dst->value.v_record.data.item[i]);
				dst_ptr += 4+sizeof(vm_record_t)+dst->value.v_record.size*sizeof(vm_variable_t);
				break;
			case VM_VARIBLE_RECORD:
				for(uint32_t i=0; i<dst->value.v_record.size; i++)
					VM_GC_CDREF(dst->value.v_record.data.pair[i].value);
				dst_ptr += 4+sizeof(vm_record_t)+dst->value.v_record.size*sizeof(vm_record_pair_t);
				break;
			case VM_VARIBLE_FUNCTION:
				for(uint32_t i=0; i<dst->value.v_function.prototype->closure_size; i++)
					VM_GC_CDREF(dst->value.v_function.closure[i]);
				dst_ptr += 4+sizeof(vm_function_instance_t)+dst->value.v_function.prototype->closure_size*sizeof(vm_variable_t*);
				break;
			case VM_VARIBLE_THREAD:{
				if(dst->value.v_thread.next != NULL)
					VM_GC_DREF(dst->value.v_thread.next);
				VM_GC_CDREF(dst->value.v_thread.env.function);
				for(uint32_t i=0; i<dst->value.v_thread.env.sp; i++)
					VM_GC_CDREF(dst->value.v_thread.stack[i]);
				for(uint32_t i=dst->value.v_thread.env.rsp; i<dst->value.v_thread.env.size; i+=3)
					VM_GC_CDREF(dst->value.v_thread.stack[i]);
				dst_ptr += 4+sizeof(vm_thread_t)+dst->value.v_thread.env.size*sizeof(vm_variable_t**);
				break;
			}
			default:
				VM_FATAL("GC bad type");
		}
	}
}

#define VM_GC_CCAST(x)	((uint8_t*)(x))
#define VM_GC_COFF(x)	\
	do{	\
		if(VM_GC_CCAST(x) >= old_data && VM_GC_CCAST(x) < old_end) \
			VM_GC_OFF(x); \
	}while(0)
#define VM_GC_OFF(x)	x = (vm_variable_t*)(((uint8_t*)(x))+offset)

static VM_FLAT void vm_gc_offset(vm_root_t* root, size_t offset){
	VM_GC_OFF(root->queue.head);
	VM_GC_OFF(root->queue.tail);
	uint8_t* dst_ptr = root->dmem.data;
	uint8_t* old_data = dst_ptr - offset;
	uint8_t* old_end = old_data + root->dmem.offset;
	uint8_t* end = dst_ptr + root->dmem.offset;
	while(dst_ptr < end){
		vm_variable_t* dst = (vm_variable_t*)dst_ptr;
		switch(dst->type){
			case VM_VARIBLE_UNDEF:
				if(dst->value.v_varible != NULL)
					VM_GC_OFF(dst->value.v_varible);
				dst_ptr += 4+sizeof(vm_variable_t*);
				break;
			case VM_VARIBLE_INT:
				dst_ptr += 4+sizeof(vm_int_t);
				break;
			case VM_VARIBLE_ATOM:
				dst_ptr += 4+sizeof(vm_atom_t);
				break;
			case VM_VARIBLE_FLOAT:
				dst_ptr += 4+sizeof(vm_float_t);
				break;
			case VM_VARIBLE_PAIR:
				VM_GC_COFF(dst->value.v_pair.head);
				VM_GC_COFF(dst->value.v_pair.tail);
				dst_ptr += 4+sizeof(vm_pair_t);
				break;
			case VM_VARIBLE_TUPLE:
				for(uint32_t i=0; i<dst->value.v_record.size; i++)
					VM_GC_COFF(dst->value.v_record.data.item[i]);
				dst_ptr += 4+sizeof(vm_record_t)+dst->value.v_record.size*sizeof(vm_variable_t*);
				break;
			case VM_VARIBLE_RECORD:
				for(uint32_t i=0; i<dst->value.v_record.size; i++)
					VM_GC_COFF(dst->value.v_record.data.pair[i].value);
				dst_ptr += 4+sizeof(vm_record_t)+dst->value.v_record.size*sizeof(vm_record_pair_t);
				break;
			case VM_VARIBLE_FUNCTION:
				for(uint32_t i=0; i<dst->value.v_function.prototype->closure_size; i++)
					VM_GC_COFF(dst->value.v_function.closure[i]);
				dst_ptr += 4+sizeof(vm_function_instance_t)+dst->value.v_function.prototype->closure_size*sizeof(vm_variable_t*);
				break;
			case VM_VARIBLE_THREAD:{
				if(dst->value.v_thread.next != NULL)
					VM_GC_OFF(dst->value.v_thread.next);
				VM_GC_COFF(dst->value.v_thread.env.function);
				for(uint32_t i=0; i<dst->value.v_thread.env.sp; i++)
					VM_GC_COFF(dst->value.v_thread.stack[i]);
				for(uint32_t i=dst->value.v_thread.env.rsp; i<dst->value.v_thread.env.size; i+=3)
					VM_GC_COFF(dst->value.v_thread.stack[i]);
				dst_ptr += 4+sizeof(vm_thread_t)+dst->value.v_thread.env.size*sizeof(vm_variable_t*);
				break;
			}
			default:
				VM_FATAL("GC bad type");
		}
	}
}

static VM_INLINE void vm_gc_realloc(vm_root_t* root, size_t page_cnt){
	uint8_t* block = root->dmem.block;
	if(!vm_dynamicmem_create(root,page_cnt) || root->dmem.block == block)
		return;
	if(root->dmem.block == NULL)
		VM_FATAL("GC null pointer");
	size_t offset = root->dmem.block-block;
	vm_gc_offset(root,offset);
}

void VM_FLAT vm_gc(vm_root_t* root){
	vm_gc_copy(root);
	vm_gc_relax(root);
	for(uint32_t i=0; i<root->process_cnt; i++){
		vm_heap_t* heap = &root->process[i].heap;
		heap->size = 0;
		heap->offset = VM_PAGE_SIZE;
		heap->data = NULL;
	}
	if(root->dmem.data == root->dmem.block){
		size_t page_cnt = root->dmem.size/VM_PAGE_SIZE;
		if(root->dmem.offset * VM_HEAP_TRESHOLD_HIGH > root->dmem.size){
			if(page_cnt != root->dmem.max_page){
				page_cnt *= VM_HEAP_RESIZE_HIGH;
				vm_gc_realloc(root,page_cnt<root->dmem.max_page?page_cnt:root->dmem.max_page);	
			}
		}else if(root->dmem.offset * VM_HEAP_TRESHOLD_LOW < root->dmem.size){
			if(page_cnt != root->dmem.min_page){
				page_cnt *= VM_HEAP_RESIZE_LOW;
				vm_gc_realloc(root,page_cnt>root->dmem.min_page?page_cnt:root->dmem.min_page);
			}
		}
	}
}
