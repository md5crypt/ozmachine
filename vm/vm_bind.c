#include <stdio.h>
#include <stdlib.h>

#include "vm_bind.h"

VM_FLAT void vm_bind(vm_context_t* context, vm_variable_t* dst, vm_variable_t* src){
	BEGIN:
	
	dst = vm_clone(context, dst);
	src = vm_clone(context, src);
		
	int dst_lock;
	int src_lock;
	
	DST_DREF:
	dst_lock = -1;
	while(dst->type == VM_VARIBLE_REF)
		dst = dst->value.v_varible;
	if(dst->type == VM_VARIBLE_UNDEF){
		dst_lock = dst->mutex_id;
		vm_mutex_lock(context, dst_lock);
		if(dst->type != VM_VARIBLE_UNDEF){
			vm_mutex_unlock(context, dst_lock);
			goto DST_DREF;
		}
	}else if(dst->type == VM_VARIBLE_REF){
		goto DST_DREF;
	}
	
	SRC_DREF:
	src_lock = -1;
	while(src->type == VM_VARIBLE_REF)
		src = src->value.v_varible;
	if(src->type == VM_VARIBLE_UNDEF){
		if(src->mutex_id != dst_lock){
			src_lock = src->mutex_id;
			vm_mutex_lock(context, src_lock);
			if(src->type != VM_VARIBLE_UNDEF){
				vm_mutex_unlock(context, src_lock);
				goto SRC_DREF;
			}
		}
	}else if(src->type == VM_VARIBLE_REF){
		goto SRC_DREF;
	}
		
	if(dst == src){
		if(dst_lock >= 0)
			vm_mutex_unlock(context, dst_lock);
		return;
	}
	
	if(dst->type != VM_VARIBLE_UNDEF){
		if(src->type != VM_VARIBLE_UNDEF){
			if(src->type != dst->type){
				if(dst->type == VM_VARIBLE_PLACEHOLDER)
					vm_stack_tpush(context, src);
				else if(src->type == VM_VARIBLE_PLACEHOLDER)
					vm_stack_tpush(context, dst);
				else if(src->type != VM_VARIBLE_WILDCHAR && dst->type != VM_VARIBLE_WILDCHAR)
					vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
				return;
			}
			switch(src->type){
				case VM_VARIBLE_INT:
					if(src->value.v_int != dst->value.v_int)
						vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
					break;
				case VM_VARIBLE_FLOAT:
					if(src->value.v_float != dst->value.v_float)
						vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
					break;
				case VM_VARIBLE_ATOM:
					if(src->value.v_atom != dst->value.v_atom)
						vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
					break;
				case VM_VARIBLE_PAIR:
					vm_bind(context, dst->value.v_pair.head,src->value.v_pair.head);
					dst = dst->value.v_pair.tail;
					src = src->value.v_pair.tail;
					goto BEGIN;
				case VM_VARIBLE_TUPLE:
					if(dst->value.v_record.size != src->value.v_record.size || dst->value.v_record.id != src->value.v_record.id)
						vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
					for(uint32_t i=0; i<dst->value.v_record.size; i++)
						vm_bind(context, dst->value.v_record.data.item[i],src->value.v_record.data.item[i]);
					break;
				case VM_VARIBLE_RECORD:
					if(dst->value.v_record.size != src->value.v_record.size || dst->value.v_record.id != src->value.v_record.id)
						vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
					for(uint32_t i=0; i<dst->value.v_record.size; i++){
						if(dst->value.v_record.data.pair[i].key != src->value.v_record.data.pair[i].key)
							vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
						vm_bind(context, dst->value.v_record.data.pair[i].value,src->value.v_record.data.pair[i].value);
					}
					break;
				default:
					vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
			}
			return;
		}
		if(dst->type == VM_VARIBLE_PLACEHOLDER){
			vm_stack_tpush(context, src);
			if(src_lock >= 0)
				vm_mutex_unlock(context, src_lock);
			return;
		}
		SWAP(dst,src);
		SWAP(dst_lock,src_lock);
	}else if(src->type == VM_VARIBLE_PLACEHOLDER){
		vm_stack_tpush(context, dst);
		if(dst_lock >= 0)
			vm_mutex_unlock(context, dst_lock);
		return;
	}

	if(dst->value.v_varible != NULL){ //if a thread is waiting on the destantion
		if(src->type == VM_VARIBLE_UNDEF){ //if the source is undefined
			vm_thread_wait(src,dst->value.v_varible); //add the thread to the source varible queue
		}else{
			vm_thread_resume(context, dst->value.v_varible); //push thread to global thread queue
		}
	}
	
	dst->value.v_varible = src;
	dst->type = VM_VARIBLE_REF;
	
	if(dst_lock >= 0)
		vm_mutex_unlock(context, dst_lock);
	if(src_lock >= 0)
		vm_mutex_unlock(context, src_lock);
}

VM_FLAT vm_variable_t* vm_clone_recursion(vm_context_t* context, vm_variable_t* pattern){
	vm_variable_t* v;
	switch(pattern->type){
		case VM_VARIBLE_WILDCHAR:
			return vm_allocate_undef(context);
		case VM_VARIBLE_PAIR:
			v = vm_allocate_pair(context);
			v->value.v_pair.head = vm_clone_resolve(context, pattern->value.v_pair.head);
			v->value.v_pair.tail = vm_clone_resolve(context, pattern->value.v_pair.tail);
			return v;
		case VM_VARIBLE_TUPLE:
			v = vm_allocate_tuple(context, pattern->value.v_record.size);
			v->value.v_record.size = pattern->value.v_record.size;
			v->value.v_record.id = pattern->value.v_record.id;
			for(uint32_t i=0; i<pattern->value.v_record.size; i++)
				v->value.v_record.data.item[i] = vm_clone_resolve(context, pattern->value.v_record.data.item[i]);
			return v;
		case VM_VARIBLE_RECORD:
			v = vm_allocate_record(context, pattern->value.v_record.size);
			v->value.v_record.size = pattern->value.v_record.size;
			v->value.v_record.id = pattern->value.v_record.id;
			for(uint32_t i=0; i<pattern->value.v_record.size; i++){
				v->value.v_record.data.pair[i].key = pattern->value.v_record.data.pair[i].key;
				v->value.v_record.data.pair[i].value = vm_clone_resolve(context, pattern->value.v_record.data.pair[i].value);
			}
			return v;
		default:
			vm_throw(context,VM_EXCEPTION_WTF);
	}
	return NULL;
}

static VM_FLAT void vm_match_recursion(vm_context_t* context, vm_variable_t* dst, vm_variable_t* src, jmp_buf abort){
	BEGIN:	
	if(dst->type == VM_VARIBLE_PLACEHOLDER){
		if(src->type == VM_VARIBLE_PATTERN)
			src = vm_dereferance_pattern(context, src);
		vm_stack_tpush(context,src);
		return;
	}
	if(dst->type == VM_VARIBLE_WILDCHAR)
		return;
	dst = vm_dereferance(context,dst);
	src = vm_dereferance(context,src);
	if(src == dst)
		return;
	if(src->type != dst->type)
		longjmp(abort,1);
	switch(src->type){
		case VM_VARIBLE_INT:
			if(src->value.v_int != dst->value.v_int)
				longjmp(abort,1);
			break;
		case VM_VARIBLE_FLOAT:
			if(src->value.v_float != dst->value.v_float)
				longjmp(abort,1);
			break;
		case VM_VARIBLE_ATOM:
			if(src->value.v_atom != dst->value.v_atom)
				longjmp(abort,1);
			break;
		case VM_VARIBLE_PAIR:
			vm_match_recursion(context,dst->value.v_pair.head,src->value.v_pair.head,abort);
			dst = dst->value.v_pair.tail;
			src = src->value.v_pair.tail;
			goto BEGIN;
		case VM_VARIBLE_TUPLE:
			if(dst->value.v_record.size != src->value.v_record.size || dst->value.v_record.id != src->value.v_record.id)
				longjmp(abort,1);
			for(uint32_t i=0; i<dst->value.v_record.size; i++)
				vm_match_recursion(context,dst->value.v_record.data.item[i],src->value.v_record.data.item[i],abort);
			break;
		case VM_VARIBLE_RECORD:
			if(dst->value.v_record.size != src->value.v_record.size || dst->value.v_record.id != src->value.v_record.id)
				longjmp(abort,1);
			for(uint32_t i=0; i<dst->value.v_record.size; i++){
				if(dst->value.v_record.data.pair[i].key != src->value.v_record.data.pair[i].key)
					longjmp(abort,1);
				vm_match_recursion(context,dst->value.v_record.data.pair[i].value,src->value.v_record.data.pair[i].value,abort);
			}
			break;
		default:
			longjmp(abort,1);
	}
}

VM_FLAT int vm_match(vm_context_t* context, vm_variable_t* dst, vm_variable_t* src){
	jmp_buf abort;
	if(setjmp(abort) > 0)
		return 0;
	vm_match_recursion(context,dst,src,abort);
	return 1;
}
