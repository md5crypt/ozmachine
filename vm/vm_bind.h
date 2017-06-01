#pragma once

#include "vm_base.h"
#include "vm_util.h"
#include "vm_dereferance.h"
#include "vm_allocate.h"
#include "vm_thread.h"

vm_variable_t* vm_clone_recursion(vm_context_t* context, vm_variable_t* pattern);
void vm_bind(vm_context_t* context, vm_variable_t* dst, vm_variable_t* src);
int vm_match(vm_context_t* context, vm_variable_t* dst, vm_variable_t* src);

static VM_INLINE vm_variable_t* vm_clone(vm_context_t* context, vm_variable_t* pattern){
	if(pattern->type == VM_VARIBLE_PATTERN)
		return vm_dereferance_pattern(context, pattern);
	if(pattern->clone == 0)
		return pattern;
	return vm_clone_recursion(context, pattern);
}

static VM_INLINE vm_variable_t* vm_clone_resolve(vm_context_t* context, vm_variable_t* pattern){
	if(pattern->type == VM_VARIBLE_PATTERN)
		return vm_dereferance_pattern(context, pattern);
	return vm_clone(context, pattern);
}
