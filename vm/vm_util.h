#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "vm_base.h"

#define VM_FATAL(x) do{printf("Fatal: %s\nline %d of file \"%s\"\n",x,__LINE__, __FILE__); exit(0);}while(0)

static VM_INLINE void vm_throw(vm_context_t* context, vm_exception_enum_t e){
	longjmp(context->interrupt,e);
}

static VM_INLINE void vm_mutex_lock(vm_context_t* context, int mutex){
	pthread_mutex_lock(context->mutex+mutex);
}

static VM_INLINE void vm_mutex_unlock(vm_context_t* context, int mutex){
	pthread_mutex_unlock(context->mutex+mutex);
}

static VM_INLINE void vm_alloc_lock(vm_context_t* context){
	pthread_mutex_lock(&context->root->synch.alloc_mutex);
}

static VM_INLINE void vm_alloc_unlock(vm_context_t* context){
	pthread_mutex_unlock(&context->root->synch.alloc_mutex);
}

static VM_INLINE void vm_queue_lock(vm_context_t* context){
	pthread_mutex_lock(&context->root->synch.queue_mutex);
}

static VM_INLINE void vm_queue_unlock(vm_context_t* context){
	pthread_mutex_unlock(&context->root->synch.queue_mutex);
}

static VM_INLINE void vm_queue_signal(vm_context_t* context){
	pthread_cond_signal(&context->root->synch.queue_cond);
}

static VM_INLINE void vm_queue_broadcast(vm_context_t* context){
	pthread_cond_broadcast(&context->root->synch.queue_cond);
}

static VM_INLINE void vm_queue_wait(vm_context_t* context){
	pthread_cond_wait(&context->root->synch.queue_cond,&context->root->synch.queue_mutex);
}

static VM_INLINE void vm_supervisor_signal(vm_context_t* context){
	pthread_cond_signal(&context->root->synch.supervisor_cond);
}

static VM_INLINE int vm_random_mutex(vm_context_t* context){
	context->rseed ^= context->rseed << 13;
	context->rseed ^= context->rseed >> 17;
	context->rseed ^= context->rseed << 5;
    return context->rseed&context->mutex_mask;
}

static VM_INLINE void vm_terminate(vm_context_t* context){
	context->root->supervisor_cmd = VM_SUPERVISOR_TERMINATE;
	context->root->halt = 1;
	vm_queue_broadcast(context);
	vm_throw(context,VM_EXCEPTION_HALT);
}

static VM_INLINE vm_bytecode_t* vm_bytecode_next(vm_bytecode_t* b, int l){
	return (vm_bytecode_t*)(((uint8_t*)b)+sizeof(vm_bytecode_t)+l*sizeof(uint32_t));
}

static VM_INLINE void vm_stack_push(vm_context_t* context, vm_variable_t* v){
	if(context->env.sp >= context->env.rsp)
		vm_throw(context,VM_EXCEPTION_THREAD_OOM);
	context->stack[context->env.sp++] = v;
}


static VM_INLINE void vm_stack_tpush(vm_context_t* context, vm_variable_t* v){
	if(context->spbuff >= context->env.rsp)
		vm_throw(context,VM_EXCEPTION_THREAD_OOM);
	context->stack[context->spbuff++] = v;
}

static VM_INLINE void vm_stack_commit(vm_context_t* context){
	context->env.sp = context->spbuff;
}

static VM_INLINE void vm_stack_transaction(vm_context_t* context){
	context->spbuff = context->env.sp;
}
