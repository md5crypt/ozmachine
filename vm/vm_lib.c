#include "vm_lib.h"
#include "vm_util.h"
#include "vm_dereferance.h"
#include "vm_allocate.h"
#include "vm_print.h"

uint32_t vm_lib_Write(vm_context_t* context){
	vm_print_varible(&context->smem,context->stack[context->env.bp],25);
	putchar('\n');
	return 0;
}

uint32_t vm_lib_Readint(vm_context_t* context){
	vm_variable_t* v = vm_allocate_int(context);
	vm_stack_push(context,v);
	int a;
	scanf("%d",&a);
	v->value.v_int = a;
	return 1;
}

uint32_t vm_lib_Readfloat(vm_context_t* context){
	vm_variable_t* v = vm_allocate_float(context);
	vm_stack_push(context,v);
	double a;
	scanf("%lf",&a);
	v->value.v_float = a;
	return 1;
}

uint32_t vm_lib_Write2(vm_context_t* context){
	vm_print_varible(&context->smem,context->stack[context->env.bp],context->stack[context->env.bp+1]->value.v_int);
	putchar('\n');
	return 0;
}

uint32_t vm_lib_Exit(vm_context_t* context){
	vm_queue_lock(context);
	vm_terminate(context);
	return 0;
}