#pragma once
#include "vm_struct.h"

void vm_root_create(vm_root_t* root);
void vm_root_load(vm_root_t* root, vm_staticmem_t* smem);
void vm_staticmem_destroy(vm_staticmem_t* smem);
void vm_root_destroy(vm_root_t* root);
void vm_run(vm_root_t* root);
void vm_stop(vm_root_t* root);
