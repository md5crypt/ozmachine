#pragma once
#include "vm_struct.h"

int vm_dynamicmem_create(vm_root_t* root, size_t page_cnt);
void vm_gc(vm_root_t* root);
