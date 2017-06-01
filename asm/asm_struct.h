#pragma once

#include <inttypes.h>
#include "../vm/vm_struct.h"
#include "heap.h"

typedef struct asm_varible_t asm_varible_t;
typedef struct asm_pair_t asm_pair_t;
typedef struct asm_record_t asm_record_t;
typedef struct asm_record_pair_t asm_record_pair_t;
typedef struct asm_bytecode_t asm_bytecode_t;
typedef struct asm_context_t asm_context_t;
typedef struct asm_function_t asm_function_t;
typedef struct asm_refpair_t asm_refpair_t;

struct asm_function_t{
	uint32_t index;
	uint32_t arity;
	uint32_t closure;
	asm_bytecode_t* addr;
	uint8_t initilized;
};

struct asm_record_pair_t{
	uint32_t key;
	asm_record_pair_t* next;
	asm_varible_t* value;
};

struct asm_record_t{
	uint32_t id;
	asm_record_pair_t* data;
};

struct asm_pair_t{
	asm_varible_t* head;
	asm_varible_t* tail;
};

struct asm_refpair_t{
	vm_bytecode_enum_t type;
	uint32_t value;
};

struct asm_varible_t{
	vm_varible_enum_t type;
	uint32_t clone;
	int index;
	union{
		vm_int_t v_int;
		vm_float_t v_float;
		asm_record_t* v_record;
		asm_pair_t* v_pair;
		asm_function_t* v_function;
		asm_refpair_t v_ref;
		vm_variable_t* v_conv;
	} value;
};

struct asm_bytecode_t{
	uint32_t op;
	vm_bytecode_enum_t type[2];
	union{
		uint32_t v_int;
		asm_bytecode_t** v_addr;
	} value[2];
	uint32_t offset;
	uint32_t line;
	asm_bytecode_t* next;
};

//<asm_varible_t*, int> tree
#define AVL_TEMPLATE_PREFIX		var_avl
#define AVL_KEY_TYPE 			asm_varible_t*
#include "avl_templete.h"

//<int, asm_bytecode_t*> tree
#define AVL_TEMPLATE_PREFIX		label_avl
#define AVL_VALUE_TYPE			asm_bytecode_t**
#include "avl_templete.h"

//<int, asm_function_t> tree
#define AVL_TEMPLATE_PREFIX		func_avl
#define AVL_VALUE_TYPE 			asm_function_t
#include "avl_templete.h"

//<char, int> tree
#define AVL_TEMPLATE_PREFIX		char_avl
#define AVL_KEY_TYPE 			char*
#include "avl_templete.h"

//<int, int> tree
#define AVL_TEMPLATE_PREFIX		int_avl
#include "avl_templete.h"
#include "../vm/vm_struct.h"

struct asm_context_t{
	heap_t heap;
	var_avl_tree_t vtree;
	func_avl_tree_t ftree;
	int_avl_tree_t atree;
	label_avl_tree_t ltree;
	char_avl_tree_t stree;
	uint32_t vtree_index;
	uint32_t ftree_index;
	uint32_t atree_index;
	uint32_t stree_index;
	asm_bytecode_t* head;
	asm_bytecode_t* tail;
	asm_varible_t* nil;
};

void asm_context_create(asm_context_t* context);
void asm_context_destroy(asm_context_t* context);
asm_varible_t* asm_allocate_integer(asm_context_t* context, int n);
uint32_t asm_add_atom(asm_context_t* context, int n);
asm_varible_t* asm_allocate_atom(asm_context_t* context, int n);
asm_varible_t* asm_allocate_float(asm_context_t* context, double n);
asm_varible_t* asm_allocate_other(asm_context_t* context, vm_varible_enum_t type);
asm_varible_t* asm_allocate_pattern(asm_context_t* context, asm_refpair_t ref);
uint32_t asm_add_string(asm_context_t* context, char* atom, int n);
asm_record_pair_t* asm_allocate_record_pair(asm_context_t* context, uint32_t key, asm_varible_t* value);
asm_varible_t* asm_allocate_record(asm_context_t* context, uint32_t key, asm_record_pair_t* data);
asm_varible_t* asm_allocate_pair(asm_context_t* context, asm_varible_t* head, asm_varible_t* tail);
asm_function_t* asm_add_function(asm_context_t* context, uint32_t str);
asm_varible_t* asm_allocate_function(asm_context_t* context, uint32_t str);
asm_bytecode_t* asm_allocate_bytecode(asm_context_t* context, uint32_t op);
asm_bytecode_t** asm_allocate_label(asm_context_t* context, uint32_t id, asm_bytecode_t* b);
void asm_bytecode_next(asm_context_t* context, uint32_t op, uint32_t type, uint32_t value);
uint32_t asm_push_pattern(asm_context_t* context, asm_varible_t** v, int clone);
void asm_register_var(asm_context_t* context, asm_varible_t* v);