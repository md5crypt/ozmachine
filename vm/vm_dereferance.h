#pragma once

#include "vm_base.h"
#include "vm_util.h"
#include "vm_thread.h"

static VM_INLINE vm_variable_t* vm_dereferance_bytecode(vm_context_t* context, vm_bytecode_t* b, int n){
	switch(b->type[n]){
		case VM_BYTECODE_LOCAL:
			if(VM_CHECK_BOUNDS && context->env.bp+b->value[n] >= context->env.sp)
				vm_throw(context,VM_EXCEPTION_OOB);
			return context->stack[context->env.bp+b->value[n]];
		case VM_BYTECODE_CONSTANT:
			return context->smem.constants[b->value[n]];
		case VM_BYTECODE_CLOSURE:
			if(VM_CHECK_BOUNDS && b->value[n] >= context->env.function->value.v_function.prototype->closure_size)
				vm_throw(context,VM_EXCEPTION_OOB);
			return context->closure[b->value[n]];
		default:
			vm_throw(context,VM_EXCEPTION_BAD_OPCODE);
	}
	return NULL;
}

static VM_INLINE vm_variable_t* vm_dereferance_pattern(vm_context_t* context, vm_variable_t* v){
	return vm_dereferance_bytecode(context, &v->value.v_bytecode, 0);
}

static VM_INLINE vm_variable_t* vm_dereferance(vm_context_t* context, vm_variable_t* v){
	if(v->type == VM_VARIBLE_PATTERN)
		v = vm_dereferance_pattern(context, v);
	START:
	while(v->type == VM_VARIBLE_REF)
		v = v->value.v_varible;
	if(v->type == VM_VARIBLE_UNDEF){
		vm_mutex_lock(context, v->mutex_id);
		if(v->type != VM_VARIBLE_UNDEF){
			vm_mutex_unlock(context, v->mutex_id);
			goto START;
		}
		context->thread->value.v_thread.env = context->env;
		vm_thread_wait(v,context->thread);
		vm_mutex_unlock(context, v->mutex_id);
		vm_throw(context,VM_EXCEPTION_THREAD_STOP);
	}else if(v->type == VM_VARIBLE_REF){
		goto START;
	}
	return v;
}
