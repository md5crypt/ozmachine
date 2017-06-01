#pragma once

#include "vm_base.h"
#include "vm_util.h"
#include "vm_allocate.h"

static VM_INLINE vm_variable_t* vm_thread_get(vm_context_t* context){
	vm_queue_lock(context);
	volatile vm_pair_t* queue = &context->root->queue;
	if(context->root->halt)
		vm_throw(context,VM_EXCEPTION_HALT);
	while(queue->head == NULL){
		if(context->root->idle_cnt == 1)
			vm_terminate(context);
		context->root->idle_cnt--;
		vm_queue_wait(context);
		context->root->idle_cnt++;
		if(context->root->halt)
			vm_throw(context,VM_EXCEPTION_HALT);
	}
	vm_variable_t* v = queue->head;
	queue->head = v->value.v_thread.next;
	v->value.v_thread.next = NULL;
	vm_queue_unlock(context);
	return v;
}

static VM_INLINE void vm_thread_queue(vm_context_t* context, vm_variable_t* thread){
	volatile vm_pair_t* queue = &context->root->queue;
	if(queue->head == NULL){
		queue->head = thread;
	}else{
		queue->tail->value.v_thread.next = thread;
	}
	while(thread->value.v_thread.next != NULL)
		thread = thread->value.v_thread.next;
	queue->tail = thread;
}

static VM_INLINE void vm_thread_halt(vm_context_t* context){
	vm_queue_lock(context);
	context->thread->value.v_thread.env = context->env;
	vm_thread_queue(context,context->thread);
	vm_throw(context,VM_EXCEPTION_HALT);
}

static VM_INLINE void vm_thread_resume(vm_context_t* context, vm_variable_t* thread){
	vm_queue_lock(context);
	vm_thread_queue(context,thread);
	vm_queue_signal(context);
	vm_queue_unlock(context);
}

static VM_INLINE void vm_thread_wait(vm_variable_t* var, vm_variable_t* thread){
	if(var->value.v_varible == NULL){
		var->value.v_varible = thread;
	}else{
		vm_variable_t* tail = var->value.v_varible;
		while(tail->value.v_thread.next != NULL)
			tail = tail->value.v_thread.next;
		tail->value.v_thread.next = thread;
	}
}

static VM_INLINE void vm_thread_grow(vm_context_t* context){
	vm_variable_t* var = (vm_variable_t*)vm_allocate_thread(context,context->env.size*2);
	context->thread = var;
	vm_variable_t** stack = var->value.v_thread.stack;
	for(uint32_t i=0; i<context->env.sp; i++)
		stack[i] = context->stack[i];
	uint32_t old_top = context->env.size-1;
	uint32_t new_top = 2*context->env.size-1;
	while(old_top > context->env.rsp)
		stack[new_top--] = context->stack[old_top--];
	stack[new_top] = context->stack[old_top];
	context->stack = stack;
	context->env.size = context->env.size*2;
	context->env.rsp = new_top;
}
