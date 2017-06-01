#include <string.h>
#include "asm_struct.h"

#define AVL_NO_HEADER

static int cmp_var(asm_varible_t* a, asm_varible_t* b){
	if(a == b)
		return 0;
	if(b == NULL)
		return 1;
	if(a == NULL)
		return -1;
	if(a->type != b->type)
		return a->type - b->type;
	if(a->clone != b->clone)
		return a->clone - b->clone;
	switch(a->type){
		case VM_VARIBLE_INT:
		case VM_VARIBLE_ATOM:
			return a->value.v_int - b->value.v_int;
		case VM_VARIBLE_FLOAT:
			return a->value.v_float - b->value.v_float;
		case VM_VARIBLE_FUNCTION:
			return a->value.v_function->index - b->value.v_function->index;
		case VM_VARIBLE_PAIR: {
			int r = cmp_var(a->value.v_pair->head,b->value.v_pair->head);
			if(r != 0)
				return r;
			return cmp_var(a->value.v_pair->tail,b->value.v_pair->tail);
		}
		case VM_VARIBLE_TUPLE:
		case VM_VARIBLE_RECORD: {
			if(a->value.v_record->id != b->value.v_record->id)
				return a->value.v_record->id - b->value.v_record->id;
			asm_record_pair_t* p1 = a->value.v_record->data;
			asm_record_pair_t* p2 = b->value.v_record->data;
			while(p1 != NULL && p2 != NULL){
				if(p1 != p2){
					if(p1->key != p2->key)
						return p1->key - p2->key;
					int r = cmp_var(p1->value,p2->value);
					if(r != 0)
						return r;
				}
				p1 = p1->next;
				p2 = p2->next;
			}
			if(p1 == p2)
				return 0;
			if(p2 == NULL)
				return 1;
			if(p1 == NULL)
				return -1;
		}
		case VM_VARIBLE_PATTERN:
			if(a->value.v_ref.type != b->value.v_ref.type)
				return a->value.v_ref.type - b->value.v_ref.type;
			return a->value.v_ref.value - b->value.v_ref.value;
		default:
			return 0;
	}
	return 0;
}

//<asm_varible_t*, int> tree
#define AVL_TEMPLATE_PREFIX		var_avl
#define AVL_KEY_TYPE 			asm_varible_t*
#define AVL_KEY_NEQ(a,b)		(cmp_var(a,b)!=0)
#define AVL_KEY_LE(a,b)			(cmp_var(a,b)<0)
#define AVL_RESIZE_UP
#include "avl_templete.h"

//<int, asm_bytecode_t*> tree
#define AVL_TEMPLATE_PREFIX		label_avl
#define AVL_VALUE_TYPE			asm_bytecode_t**
#define AVL_RESIZE_UP
#include "avl_templete.h"

//<int, asm_function_t> tree
#define AVL_TEMPLATE_PREFIX		func_avl
#define AVL_VALUE_TYPE 			asm_function_t
#define AVL_RESIZE_UP
#include "avl_templete.h"

//<char, int> tree
#define AVL_TEMPLATE_PREFIX		char_avl
#define AVL_KEY_TYPE 			char*
#define AVL_KEY_NEQ(a,b)		(strcmp(a,b)!=0)
#define AVL_KEY_LE(a,b)			(strcmp(a,b)<0)
#define AVL_RESIZE_UP
#include "avl_templete.h"

//<int, int> tree
#define AVL_TEMPLATE_PREFIX		int_avl
#include "avl_templete.h"