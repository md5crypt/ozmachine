#pragma once

#include <inttypes.h>
#include <stdint.h>
#include <setjmp.h>
#include <pthread.h>
#include "vm_define.h"
#include "vm_enum.h"

#define VM_PACKED __attribute__ ((__packed__))


#if INTPTR_MAX == INT64_MAX
typedef uint64_t vm_atom_t;
typedef int64_t vm_int_t;
#else
typedef uint32_t vm_atom_t;
typedef int32_t vm_int_t;
#endif
typedef double vm_float_t;

typedef struct vm_variable_t vm_variable_t;
typedef struct vm_pair_t vm_pair_t;
typedef struct vm_record_t vm_record_t;
typedef struct vm_record_pair_t vm_record_pair_t;
typedef struct vm_bytecode_t vm_bytecode_t;
typedef struct vm_thread_t vm_thread_t;
typedef struct vm_function_t vm_function_t;
typedef struct vm_function_instance_t vm_function_instance_t;
typedef struct vm_rpn_t vm_rpn_t;
typedef struct vm_staticmem_t vm_staticmem_t;
typedef struct vm_dynamicmem_t vm_dynamicmem_t;
typedef struct vm_environment_t vm_environment_t;
typedef struct vm_synch_t vm_synch_t;
typedef struct vm_heap_t vm_heap_t;
typedef struct vm_root_t vm_root_t;
typedef struct vm_context_t vm_context_t;
typedef struct vm_lib_t vm_lib_t;

typedef uint32_t (*vm_native_func_t)(vm_context_t* context);

struct vm_lib_t{
	char* name;
	uint32_t arity;
	vm_native_func_t addr;
};

struct vm_record_pair_t{
	vm_atom_t key;
	vm_variable_t* value;
};

struct VM_PACKED vm_record_t{
	uint32_t size;
	vm_atom_t id;
	union {
		vm_record_pair_t pair[0];
		vm_variable_t* item[0];
	} data;
};

struct vm_pair_t{
	vm_variable_t* head;
	vm_variable_t* tail;
};

struct VM_PACKED vm_bytecode_t{
	vm_op_enum_t op;
	vm_bytecode_enum_t type[3];
	uint32_t value[0];
};

struct vm_function_t{
	vm_function_enum_t type;
	union {
		vm_bytecode_t* v_dynamic;
		vm_native_func_t v_native;
	} addr;
	uint32_t arity;
	uint32_t closure_size;
};

struct VM_PACKED vm_function_instance_t{
	vm_function_t* prototype;
	vm_variable_t* closure[0];
};

struct vm_rpn_t{
	vm_rpn_type_enum_t type;
	union{
		vm_int_t v_int;
		vm_float_t v_float;
	} value;
};

struct vm_environment_t{
	uint32_t bp;
	uint32_t sp;
	uint32_t rsp;
	uint32_t size;
	vm_rpn_t acc;
	vm_bytecode_t* pc;
	vm_variable_t* function;
};

struct VM_PACKED vm_thread_t{
	vm_environment_t env;
	vm_variable_t* next;
	vm_variable_t* stack[0];
};

struct VM_PACKED vm_variable_t{
	volatile vm_varible_enum_t type;
	uint8_t clone;
	uint16_t mutex_id;
	union VM_PACKED {
		vm_int_t v_int;
		vm_atom_t v_atom;
		vm_float_t v_float;
		vm_record_t v_record;
		vm_pair_t v_pair;
		vm_function_instance_t v_function;
		vm_thread_t v_thread;
		vm_bytecode_t v_bytecode;
		vm_variable_t* v_varible;
	} value;
};

struct vm_staticmem_t{
	vm_function_t* prototypes;
	vm_variable_t** constants;
	uint8_t* program;
	struct{
		char** atom;
		char** function;
		uint32_t* line;
	} symbols;
};

struct vm_heap_t{
	size_t size;
	size_t offset;
	uint8_t* data;
	uint8_t** page;
};

struct vm_dynamicmem_t{
	size_t size;
	size_t offset;
	size_t min_page;
	size_t max_page;
	uint8_t* block;
	uint8_t* data;
};

struct vm_synch_t{
	pthread_cond_t queue_cond;
	pthread_cond_t supervisor_cond;
	pthread_mutex_t queue_mutex;
	pthread_mutex_t alloc_mutex;
};

struct vm_root_t {
	vm_context_t* process;
	vm_dynamicmem_t dmem;
	vm_synch_t synch;
	uint32_t process_cnt;
	uint32_t mutex_cnt;
	volatile uint32_t halt;
	volatile uint32_t idle_cnt;
	volatile vm_pair_t queue;
	volatile vm_supervisor_enum_t supervisor_cmd;
};

struct vm_context_t{
	vm_staticmem_t smem;
	vm_environment_t env;
	vm_heap_t heap;
	jmp_buf interrupt;
	uint32_t spbuff;
	uint32_t rseed;
	uint32_t mutex_mask;
	pthread_t self;
	pthread_mutex_t* mutex;
	vm_root_t* root;
	vm_variable_t* thread;
	vm_variable_t** closure;
	vm_variable_t** stack;
	vm_rpn_t rpn_stack[128];
};

#undef VM_PACKED