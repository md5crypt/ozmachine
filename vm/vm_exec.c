#include <stdio.h>
#include <stdlib.h>

#include "vm_exec.h"
#include "vm_rpn.h"
#include "vm_bind.h"

static VM_INLINE void vm_record_key(vm_context_t* context, vm_variable_t* v, vm_atom_t n){
	v = vm_dereferance(context, v);
	if(v->type == VM_VARIBLE_RECORD){
		for(uint32_t i=0; i<v->value.v_record.size; i++){
			if(v->value.v_record.data.pair[i].key == n){
				vm_stack_push(context, v->value.v_record.data.pair[i].value);
				return;
			}
		}
		vm_throw(context,VM_EXCEPTION_KEY_ERROR);
	}else if(v->type == VM_VARIBLE_TUPLE){
		if(v->value.v_record.size >= n && n > 0){
			vm_stack_push(context, v->value.v_record.data.item[n-1]);
			return;
		}
		vm_throw(context,VM_EXCEPTION_KEY_ERROR);
	}
	vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
}

static VM_INLINE vm_bytecode_t* vm_instance_function(vm_context_t* context, vm_function_t* prototype){
	vm_bytecode_t* b = vm_bytecode_next(context->env.pc,1);
	vm_variable_t* fun = vm_allocate_function(context, prototype->closure_size);
	vm_stack_push(context, fun);
	fun->value.v_function.prototype = prototype;
	for(uint32_t i=0; i<prototype->closure_size; i++){
		fun->value.v_function.closure[i] = vm_dereferance_bytecode(context, b, 0);
		b = vm_bytecode_next(b,1);
	}
	return b;
}

VM_FLAT void vm_exec(vm_context_t* context){
	while(1){
		vm_bytecode_t* b = context->env.pc;
		//vm_print_bytecode(&context->smem,b,-1);
		//putchar('\n');
		switch(b->op){
			case VM_OP_INT:
				vm_return(context, context->env.function->value.v_function.prototype->addr.v_native(context));
				context->env.pc = vm_bytecode_next(context->env.pc,1);
				break;
			case VM_OP_YIELD:
				context->env.pc = vm_bytecode_next(b,0);
				context->thread->value.v_thread.env = context->env;
				vm_thread_resume(context,context->thread);
				vm_throw(context,VM_EXCEPTION_THREAD_STOP);
			case VM_OP_PUSH:
				if(b->type[0] == VM_BYTECODE_ACC){
					vm_stack_push(context, vm_rpn_convert(context));
					context->env.pc = vm_bytecode_next(b,0);
				}else{
					vm_stack_push(context, vm_dereferance_bytecode(context,b,0));
					context->env.pc = vm_bytecode_next(b,1);
				}
				break;
			case VM_OP_POP:
				context->env.sp -= b->value[0];
				context->env.pc = vm_bytecode_next(b,1);
				break;
			case VM_OP_CREATE:
				vm_stack_transaction(context);
				for(uint32_t i=0; i<b->value[0]; i++)
					 vm_stack_tpush(context,vm_allocate_undef(context));
				vm_stack_commit(context);
				context->env.pc = vm_bytecode_next(b,1);
				break;
			case VM_OP_CALL:
				if(context->root->halt)
					vm_thread_halt(context);
				vm_call(context, vm_dereferance_bytecode(context,b,0));
				break;
			case VM_OP_RCALL:
				if(context->root->halt)
					vm_thread_halt(context);
				vm_rcall(context, vm_dereferance_bytecode(context,b,0));
				break;
			case VM_OP_TCALL:
				if(context->root->halt)
					vm_thread_halt(context);
				vm_tcall(context, vm_dereferance_bytecode(context,b,0));
				context->env.pc = vm_bytecode_next(b,1);
				break;
			case VM_OP_RET:
				vm_return(context, b->value[0]);
				context->env.pc = vm_bytecode_next(context->env.pc,1);
				break;
			case VM_OP_JMP:
				context->env.pc = (vm_bytecode_t*)(context->smem.program + b->value[0]);
				break;
			case VM_OP_BRANCH:
				if(b->type[0] == VM_BYTECODE_ACC){
					if(context->env.acc.type != VM_RPN_BOOL)
						vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
					if(context->env.acc.value.v_int == 1){
						context->env.pc = (vm_bytecode_t*)(context->smem.program + b->value[1]);
					}else{
						context->env.pc = vm_bytecode_next(b,2);
					}
				}else{
					vm_variable_t* v = vm_dereferance_bytecode(context,b,0);
					v = vm_dereferance(context, v);
					if(v->type != VM_VARIBLE_ATOM)
						vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
					if(v->value.v_atom == VM_ATOM_TRUE){
						context->env.pc = (vm_bytecode_t*)(context->smem.program + b->value[1]);
					}else if(v->value.v_atom == VM_ATOM_FALSE){
						context->env.pc = vm_bytecode_next(b,2);
					}else{
						vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
					}
				}
				break;
			case VM_OP_INST:
				context->env.pc = vm_instance_function(context, context->smem.prototypes + b->value[0]);
				break;
			case VM_OP_CLOSURE:
				vm_throw(context,VM_EXCEPTION_BAD_OPCODE);
			case VM_OP_KEY:
				vm_record_key(context, vm_dereferance_bytecode(context,b,0),b->value[1]);
				context->env.pc = vm_bytecode_next(b,2);
				break;
			case VM_OP_BIND:
				vm_stack_transaction(context);
				vm_bind(context, vm_dereferance_bytecode(context,b,0),vm_dereferance_bytecode(context,b,1));
				vm_stack_commit(context);
				context->env.pc = vm_bytecode_next(b,2);
				break;
			case VM_OP_MATCH:
				vm_stack_transaction(context);
				context->env.acc.type = VM_RPN_BOOL;
				context->env.acc.value.v_int = vm_match(context, vm_dereferance_bytecode(context,b,0),vm_dereferance_bytecode(context,b,1));
				if(context->env.acc.value.v_int)
					vm_stack_commit(context);
				context->env.pc = vm_bytecode_next(b,2);
				break;
			case VM_OP_MOVE:
				if(b->type[0] == VM_BYTECODE_LOCAL){
					context->stack[context->env.bp+b->value[0]] = vm_dereferance_bytecode(context,b,1);
				}else{
					context->closure[context->env.bp+b->value[0]] = vm_dereferance_bytecode(context,b,1);
				}
				context->env.pc = vm_bytecode_next(b,2);
				break;
			case VM_OP_RPN:
				context->env.pc = vm_rpn(context);
				break;
			default:
				vm_throw(context,VM_EXCEPTION_BAD_OPCODE);
		}
	}
}