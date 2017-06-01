#pragma once

#include "vm_base.h"
#include "vm_util.h"
#include "vm_dereferance.h"
#include "vm_thread.h"
#include "vm_allocate.h"

void __attribute__((noreturn)) vm_exec(vm_context_t* context);

static VM_INLINE void vm_save_environment(vm_context_t* context){
	if(context->env.rsp-3 < context->env.sp)
		vm_throw(context,VM_EXCEPTION_THREAD_OOM);
#if INTPTR_MAX == INT64_MAX
	context->stack[--context->env.rsp] = (vm_variable_t*)((uint64_t)context->env.bp);
#else
	context->stack[--context->env.rsp] = (vm_variable_t*)context->env.bp;
#endif
	context->stack[--context->env.rsp] = (vm_variable_t*)context->env.pc;
	context->stack[--context->env.rsp] = context->env.function;
}

static VM_INLINE void vm_restore_environment(vm_context_t* context){
	if(context->env.rsp == context->env.size)
		vm_throw(context,VM_EXCEPTION_THREAD_EXIT);
	context->env.function = context->stack[context->env.rsp++];
	context->env.pc = (vm_bytecode_t*)context->stack[context->env.rsp++];
#if INTPTR_MAX == INT64_MAX
	context->env.bp = (uint64_t)context->stack[context->env.rsp++];
#else
	context->env.bp = (uint32_t)context->stack[context->env.rsp++];
#endif
}

static VM_INLINE void vm_return(vm_context_t* context, uint32_t n){
	uint32_t top = context->env.sp - n;
	for(uint32_t i=0; i<n; i++)
		context->stack[context->env.bp+i] = context->stack[top+i];
	context->env.sp = context->env.bp+n;
	vm_restore_environment(context);
	context->closure = context->env.function->value.v_function.closure;
}

static VM_INLINE void vm_call(vm_context_t* context, vm_variable_t* fun){
	fun = vm_dereferance(context, fun);
	if(fun->type != VM_VARIBLE_FUNCTION)
		vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
	vm_save_environment(context);
	vm_function_t* prototype = fun->value.v_function.prototype;
	context->env.bp = context->env.sp - prototype->arity;
	context->env.function = fun;
	if(prototype->type == VM_FUNCTION_DYNAMIC){
		context->env.pc = prototype->addr.v_dynamic;
		context->closure = fun->value.v_function.closure;
	}else{
		context->env.pc = (vm_bytecode_t*)context->smem.program;
		uint32_t n = prototype->addr.v_native(context);
		vm_return(context, n);
		context->env.pc = vm_bytecode_next(context->env.pc,1);
	}
}

static VM_INLINE void vm_rcall(vm_context_t* context, vm_variable_t* fun){
	fun = vm_dereferance(context, fun);
	if(fun->type != VM_VARIBLE_FUNCTION)
		vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
	vm_function_t* prototype = fun->value.v_function.prototype;
	
	uint32_t top = context->env.sp - prototype->arity;
	for(uint32_t i=0; i<prototype->arity; i++)
		context->stack[context->env.bp+i] = context->stack[top+i];
	context->env.sp = context->env.bp + prototype->arity;
	context->env.function = fun;
	context->env.pc = prototype->addr.v_dynamic;
	context->closure = fun->value.v_function.closure;
}

static VM_INLINE void vm_tcall(vm_context_t* context, vm_variable_t* fun){
	fun = vm_dereferance(context, fun);
	if(fun->type != VM_VARIBLE_FUNCTION)
		vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
	vm_function_t* prototype = fun->value.v_function.prototype;
	vm_variable_t* thread;
	if(prototype->arity > 0){
		size_t s = prototype->arity > VM_THREAD_STACK_SIZE ? 2*prototype->arity : VM_THREAD_STACK_SIZE;
		thread = vm_allocate_thread(context, s);
		thread->value.v_thread.env.sp = prototype->arity;
		thread->value.v_thread.env.rsp = s;
		thread->value.v_thread.env.size = s;
		for(uint32_t i=prototype->arity-1; i!=0xFFFFFFFF; i--)
			thread->value.v_thread.stack[i] = context->stack[--context->env.sp];
	}else{
		thread = vm_allocate_thread(context, VM_THREAD_STACK_SIZE);
		thread->value.v_thread.env.sp = 0;
		thread->value.v_thread.env.rsp = VM_THREAD_STACK_SIZE;
		thread->value.v_thread.env.size = VM_THREAD_STACK_SIZE;
	}
	thread->value.v_thread.env.bp = 0;
	thread->value.v_thread.env.pc = prototype->addr.v_dynamic;
	thread->value.v_thread.env.function = fun;
	vm_thread_resume(context, thread);
}
