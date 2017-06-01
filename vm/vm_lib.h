#pragma once
#include "vm_base.h"
uint32_t vm_lib_Write(vm_context_t* context);
uint32_t vm_lib_Readint(vm_context_t* context);
uint32_t vm_lib_Readfloat(vm_context_t* context);
uint32_t vm_lib_Write2(vm_context_t* context);
uint32_t vm_lib_Exit(vm_context_t* context);