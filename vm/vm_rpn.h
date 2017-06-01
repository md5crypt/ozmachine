#pragma once

#include <math.h>
#include "vm_base.h"
#include "vm_dereferance.h"
#include "vm_allocate.h"

static VM_INLINE void vm_rpn_floatcast(vm_context_t* context, vm_rpn_t* stack){
	if(stack[-1].type != stack[-2].type){
		if(stack[-1].type == VM_RPN_BOOL || stack[-2].type == VM_RPN_BOOL)
			vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
		if(stack[-1].type == VM_RPN_INT){
			stack[-1].type = VM_RPN_FLOAT;
			stack[-1].value.v_float = stack[-1].value.v_int;
		}else{
			stack[-2].type = VM_RPN_FLOAT;
			stack[-2].value.v_float = stack[-2].value.v_int;
		}
	}
}

static VM_INLINE vm_rpn_t* vm_rpn_execute(vm_context_t* context, vm_rpn_t* stack, vm_rpn_op_enum_t op){
	switch(op){
		case VM_RPN_ADD:
			vm_rpn_floatcast(context, stack);
			if(stack[-1].type == VM_RPN_INT)
				stack[-2].value.v_int = stack[-2].value.v_int + stack[-1].value.v_int;
			else
				stack[-2].value.v_float = stack[-2].value.v_float + stack[-1].value.v_float;
			return stack-1;
		case VM_RPN_SUB:
			vm_rpn_floatcast(context, stack);
			if(stack[-1].type == VM_RPN_INT)
				stack[-2].value.v_int = stack[-2].value.v_int - stack[-1].value.v_int;
			else
				stack[-2].value.v_float = stack[-2].value.v_float - stack[-1].value.v_float;
			return stack-1;
		case VM_RPN_MUL:
			vm_rpn_floatcast(context, stack);
			if(stack[-1].type == VM_RPN_INT)
				stack[-2].value.v_int = stack[-2].value.v_int * stack[-1].value.v_int;
			else
				stack[-2].value.v_float = stack[-2].value.v_float * stack[-1].value.v_float;
			return stack-1;
		case VM_RPN_DIV:
			vm_rpn_floatcast(context, stack);
			if(stack[-1].type == VM_RPN_INT){
				if(stack[-1].value.v_int == 0)
					vm_throw(context,VM_EXCEPTION_ZERO_DIVIDE);
				stack[-2].value.v_int = stack[-2].value.v_int / stack[-1].value.v_int;
			}else{
				if(stack[-1].value.v_float == 0)
					vm_throw(context,VM_EXCEPTION_ZERO_DIVIDE);
				stack[-2].value.v_float = stack[-2].value.v_float / stack[-1].value.v_float;
			}
			return stack-1;
		case VM_RPN_MOD:
			if(stack[-1].type != VM_RPN_INT || stack[-2].type != VM_RPN_INT)
				vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
			if(stack[-1].value.v_int == 0)
				vm_throw(context,VM_EXCEPTION_ZERO_DIVIDE);
			stack[-2].value.v_int = stack[-2].value.v_int % stack[-1].value.v_int;
			return stack-1;
		case VM_RPN_POW:
			vm_rpn_floatcast(context, stack);
			if(stack[-1].type == VM_RPN_INT){
				vm_int_t pow = stack[-2].value.v_int;
				vm_int_t exp = stack[-1].value.v_int;
				if(exp < 0){
					if(pow == 0)
						vm_throw(context,VM_EXCEPTION_ZERO_DIVIDE);
					stack[-2].value.v_int = (pow*pow != 1)? 0: (exp&1)? pow : 1;
				}else{
					vm_int_t v = 1;
					while(exp > 0){
						if (exp & 1)
							v *= pow;
						pow *= pow;
						exp >>= 1;
					}
					stack[-2].value.v_int = v;
				}
			}else{
				if(stack[-2].value.v_float == 0)
					vm_throw(context,VM_EXCEPTION_ZERO_DIVIDE);
				stack[-2].value.v_float = pow(stack[-2].value.v_float,stack[-1].value.v_float);
			}
			return stack-1;
		case VM_RPN_NEG:
			if(stack[-1].type == VM_RPN_INT){
				stack[-1].value.v_int = -stack[-1].value.v_int;
				return stack;
			}
			if(stack[-1].type == VM_RPN_FLOAT){
				stack[-1].value.v_float = -stack[-1].value.v_float;
				return stack;					
			}
			vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
		case VM_RPN_EQ:
			if(stack[-1].type != stack[-2].type)
				stack[-2].value.v_int = 0;
			else
				stack[-2].value.v_int = stack[-2].value.v_int ==  stack[-1].value.v_int;
			stack[-2].type = VM_RPN_BOOL;
			return stack-1;
		case VM_RPN_NEQ:
			if(stack[-1].type != stack[-2].type)
				stack[-2].value.v_int = 1;
			else
				stack[-2].value.v_int = stack[-2].value.v_int !=  stack[-1].value.v_int;
			stack[-2].type = VM_RPN_BOOL;
			return stack-1;
		case VM_RPN_LEQ:
			vm_rpn_floatcast(context, stack);
			if(stack[-1].type == VM_RPN_INT)
				stack[-2].value.v_int = stack[-2].value.v_int <= stack[-1].value.v_int;
			else
				stack[-2].value.v_int = stack[-2].value.v_float <= stack[-1].value.v_float;
			stack[-2].type = VM_RPN_BOOL;
			return stack-1;
		case VM_RPN_GEQ:
			vm_rpn_floatcast(context, stack);
			if(stack[-1].type == VM_RPN_INT)
				stack[-2].value.v_int = stack[-2].value.v_int >= stack[-1].value.v_int;
			else
				stack[-2].value.v_int = stack[-2].value.v_float >= stack[-1].value.v_float;
			stack[-2].type = VM_RPN_BOOL;
			return stack-1;
		case VM_RPN_LE:
			vm_rpn_floatcast(context, stack);
			if(stack[-1].type == VM_RPN_INT)
				stack[-2].value.v_int = stack[-2].value.v_int < stack[-1].value.v_int;
			else
				stack[-2].value.v_int = stack[-2].value.v_float < stack[-1].value.v_float;
			stack[-2].type = VM_RPN_BOOL;
			return stack-1;
		case VM_RPN_GE:
			vm_rpn_floatcast(context, stack);
			if(stack[-1].type == VM_RPN_INT)
				stack[-2].value.v_int = stack[-2].value.v_int > stack[-1].value.v_int;
			else
				stack[-2].value.v_int = stack[-2].value.v_float > stack[-1].value.v_float;
			stack[-2].type = VM_RPN_BOOL;
			return stack-1;
		case VM_RPN_OR:
			if(stack[-1].type != stack[-2].type || stack[-1].type == VM_RPN_FLOAT)
				vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
			stack[-2].value.v_int = stack[-2].value.v_int | stack[-1].value.v_int;
			return stack-1;
		case VM_RPN_AND:
			if(stack[-1].type != stack[-2].type || stack[-1].type == VM_RPN_FLOAT)
				vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
			stack[-2].value.v_int = stack[-2].value.v_int & stack[-1].value.v_int;
			return stack-1;
		case VM_RPN_XOR:
			if(stack[-1].type != stack[-2].type || stack[-1].type == VM_RPN_FLOAT)
				vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
			stack[-2].value.v_int = stack[-2].value.v_int ^ stack[-1].value.v_int;
			return stack-1;
		case VM_RPN_NOT:
			if(stack[-1].type == VM_RPN_FLOAT)
				vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
			stack[-1].value.v_int = ~stack[-1].value.v_int;
			if(stack[-1].type == VM_RPN_BOOL)
				stack[-1].value.v_int &= 0x01;
			return stack;	
		default:
			vm_throw(context,VM_EXCEPTION_WTF);
	}
	return NULL;
}

static VM_INLINE vm_bytecode_t* vm_rpn(vm_context_t* context){
	vm_rpn_t* stack = context->rpn_stack;
	vm_bytecode_t* b = context->env.pc;
	while(b->op == VM_OP_RPN){
		if(b->value[0] == VM_RPN_PUSH){
			if(b->type[1] == VM_BYTECODE_ACC){
				stack[0] = context->env.acc;
			}else{
				vm_variable_t* v = vm_dereferance_bytecode(context, b, 1);
				v = vm_dereferance(context, v);
				switch(v->type){
					case VM_VARIBLE_INT:
						stack[0].type = VM_RPN_INT;
						stack[0].value.v_int = v->value.v_int;
						break;
					case VM_VARIBLE_ATOM:
						stack[0].type = VM_RPN_BOOL;
						if(v->value.v_atom == VM_ATOM_FALSE){
							stack[0].value.v_int = 0;
						}else if(v->value.v_atom == VM_ATOM_TRUE){
							stack[0].value.v_int = 1;
						}else{
							vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
						}
						break;
					case VM_VARIBLE_FLOAT:
						stack[0].type = VM_RPN_FLOAT;
						stack[0].value.v_float = v->value.v_float;
						break;
					default:
						vm_throw(context,VM_EXCEPTION_TYPE_ERROR);
				}
			}
			stack++;
			b = vm_bytecode_next(b,2);
		}else{
			stack = vm_rpn_execute(context, stack, b->value[0]);
			b = vm_bytecode_next(b,1);
		}
	}
	context->env.acc = stack[-1];
	return b;
}

static VM_INLINE vm_variable_t* vm_rpn_convert(vm_context_t* context){
	vm_variable_t* v;
	switch(context->env.acc.type){
		case VM_RPN_INT:
			v = vm_allocate_int(context);
			v->value.v_int = context->env.acc.value.v_int;
			break;
		case VM_RPN_BOOL:
			v = context->smem.constants[context->env.acc.value.v_int?VM_ATOM_TRUE:VM_ATOM_FALSE];
			break;
		case VM_RPN_FLOAT:
			v = vm_allocate_float(context);
			v->value.v_float = context->env.acc.value.v_float;
			break;
		default:
			vm_throw(context,VM_EXCEPTION_WTF);
	}
	return v;
}
