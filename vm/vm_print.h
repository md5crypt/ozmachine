#pragma once
#include "vm_struct.h"

void vm_print_exception(vm_context_t* context, vm_exception_enum_t e);
void vm_print_bytecode(vm_staticmem_t* smem, vm_bytecode_t* b, int depth);
void vm_print_varible(vm_staticmem_t* smem, vm_variable_t* v, int depth);
