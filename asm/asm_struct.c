#include <string.h>
#include "asm_struct.h"

void asm_context_create(asm_context_t* context){
	context->vtree_index = 0;
	context->ftree_index = 0;
	context->atree_index = 0;
	context->stree_index = 0;
	heap_create(&context->heap);
	var_avl_create(&context->vtree,128);
	func_avl_create(&context->ftree,128);
	char_avl_create(&context->stree,128);
	label_avl_create(&context->ltree,128);
	int_avl_create(&context->atree,128);
	context->head = asm_allocate_bytecode(context,VM_OP_INT);
	context->tail = context->head;
	asm_register_var(context,asm_allocate_atom(context,asm_add_string(context,"false",0)));
	asm_register_var(context,asm_allocate_atom(context,asm_add_string(context,"true",0)));
	context->nil = asm_allocate_atom(context,asm_add_string(context,"nil",0));
	asm_register_var(context,context->nil);
	asm_register_var(context,asm_allocate_function(context,asm_add_string(context,"Main",0)));
}

void asm_context_destroy(asm_context_t* context){
	heap_destroy(&context->heap);
	var_avl_destroy(&context->vtree);
	func_avl_destroy(&context->ftree);
	int_avl_destroy(&context->atree);
	char_avl_destroy(&context->stree);
	label_avl_destroy(&context->ltree);
}

static inline asm_varible_t* asm_allocate_var(asm_context_t* context, asm_varible_t* v){
	var_avl_t node = var_avl_get(&context->vtree,v);
	if(node != NULL)
		return node->key;
	asm_varible_t* ptr = (asm_varible_t*)heap_alloc(&context->heap,sizeof(asm_varible_t));
	ptr[0] = v[0];
	var_avl_insert(&context->vtree,ptr,-1);
	return ptr;
}

void asm_register_var(asm_context_t* context, asm_varible_t* v){
	var_avl_t node = var_avl_get(&context->vtree,v);
	if(node->value == -1){
		node->value = context->vtree_index;
		node->key->index = context->vtree_index;
		context->vtree_index++;
	}
}

asm_varible_t* asm_allocate_integer(asm_context_t* context, int n){
	asm_varible_t v = {VM_VARIBLE_INT,0,-1,{.v_int=n}};
	return asm_allocate_var(context,&v);
}

uint32_t asm_add_atom(asm_context_t* context, int n){
	int_avl_t node = int_avl_get(&context->atree,n);
	if(node == NULL)
		node = int_avl_set(&context->atree,n,context->atree_index++);
	return node->value;
}

asm_varible_t* asm_allocate_atom(asm_context_t* context, int n){
	asm_varible_t v = {VM_VARIBLE_ATOM,0,-1,{.v_int=asm_add_atom(context,n)}};
	return asm_allocate_var(context,&v);
}

asm_varible_t* asm_allocate_float(asm_context_t* context, double n){
	asm_varible_t v = {VM_VARIBLE_FLOAT,0,-1,{.v_float=n}};
	return asm_allocate_var(context,&v);
}

asm_varible_t* asm_allocate_other(asm_context_t* context, vm_varible_enum_t type){
	asm_varible_t v = {type,0,-1,{.v_int=0}};
	return asm_allocate_var(context,&v);
}

asm_varible_t* asm_allocate_pattern(asm_context_t* context, asm_refpair_t ref){
	asm_varible_t v = {VM_VARIBLE_PATTERN,0,-1,{.v_ref=ref}};
	return asm_allocate_var(context,&v);
}

uint32_t asm_add_string(asm_context_t* context, char* atom, int n){
	int len = strlen(atom)+1;
	if(n > 0)
		atom[len-n-1] = 0;
	char_avl_t node = char_avl_get(&context->stree,atom);
	if(node != NULL)
		return node->value;
	char* str = (char*)heap_alloc(&context->heap,len-n);
	memcpy(str,atom,len-n);
	char_avl_insert(&context->stree,str,context->stree_index++);
	return context->stree_index-1;
}

asm_record_pair_t* asm_allocate_record_pair(asm_context_t* context, uint32_t key, asm_varible_t* value){
	asm_record_pair_t* ptr = (asm_record_pair_t*)heap_alloc(&context->heap,sizeof(asm_record_pair_t));
	ptr->key = key==0xFFFFFFFF?key:asm_add_atom(context,key);
	ptr->value = value;
	ptr->next = NULL;
	return ptr;
}

asm_varible_t* asm_allocate_record(asm_context_t* context, uint32_t key, asm_record_pair_t* data){
	asm_varible_t* ptr = (asm_varible_t*)heap_alloc(&context->heap,sizeof(asm_varible_t));
	ptr->type = data->key==0xFFFFFFFF?VM_VARIBLE_TUPLE:VM_VARIBLE_RECORD;
	ptr->index = -1;
	ptr->clone = 0;
	asm_record_t* rec = (asm_record_t*)heap_alloc(&context->heap,sizeof(asm_record_t));
	rec->id = asm_add_atom(context,key);
	rec->data = data;
	ptr->value.v_record = rec;
	return ptr;
}

asm_varible_t* asm_allocate_pair(asm_context_t* context, asm_varible_t* head, asm_varible_t* tail){	
	asm_varible_t* ptr = (asm_varible_t*)heap_alloc(&context->heap,sizeof(asm_varible_t));
	ptr->type = VM_VARIBLE_PAIR;
	ptr->index = -1;
	ptr->clone = 0;
	asm_pair_t* pair = (asm_pair_t*)heap_alloc(&context->heap,sizeof(asm_pair_t));
	pair->head = head;
	pair->tail = tail;
	ptr->value.v_pair = pair;
	return ptr;
}

asm_function_t* asm_add_function(asm_context_t* context, uint32_t str){
	func_avl_t node = func_avl_get(&context->ftree, str);
	if(node != NULL)
		return &node->value;
	asm_function_t f = {context->ftree_index++,0,0,0,0};
	return &(func_avl_insert(&context->ftree,str,f)->value);
}

asm_varible_t* asm_allocate_function(asm_context_t* context, uint32_t str){
	asm_function_t* func = asm_add_function(context,str);
	if(func->initilized == 0)
		func->initilized = 1;
	asm_varible_t v = {VM_VARIBLE_FUNCTION,0,context->vtree_index,{.v_function=func}};
	return asm_allocate_var(context,&v);	
}

asm_bytecode_t* asm_allocate_bytecode(asm_context_t* context, uint32_t op){
	asm_bytecode_t* b = (asm_bytecode_t*)heap_alloc(&context->heap,sizeof(asm_bytecode_t));
	b->op = op;
	b->type[0] = VM_BYTECODE_OTHER;
	b->type[1] = VM_BYTECODE_OTHER;
	b->value[0].v_int = 0;
	b->value[1].v_int = 0;
	b->next = NULL;
	b->line = -1;
	return b;
}

asm_bytecode_t** asm_allocate_label(asm_context_t* context, uint32_t id, asm_bytecode_t* b){
	label_avl_t node = label_avl_get(&context->ltree,id);
	if(node != NULL){
		if(b != NULL && node->value[0] == NULL){
			node->value[0] = b;
		}else if(b != NULL){
			return NULL;
		}
		return node->value;
	}
	asm_bytecode_t** ptr = (asm_bytecode_t**)heap_alloc(&context->heap,sizeof(asm_bytecode_t**));
	ptr[0] = b;
	label_avl_insert(&context->ltree,id,ptr);
	return ptr;
}

void asm_bytecode_next(asm_context_t* context, uint32_t op, uint32_t type, uint32_t value){
	asm_bytecode_t* b = asm_allocate_bytecode(context,op);
	b->type[0] = type;
	b->value[0].v_int = value;
	context->tail->next = b;
	context->tail = b;
}

static inline void struct_add(asm_context_t* context, asm_varible_t** v){
	asm_varible_t* r = var_avl_addget(&context->vtree,v[0],-1)->key;
	if(v[0] != r)
		v[0] = r;
}

uint32_t asm_push_pattern(asm_context_t* context, asm_varible_t** v, int clone){
	if(v[0]->clone)
		clone = 1;
	switch(v[0]->type){
		case VM_VARIBLE_PAIR:
			v[0]->clone = asm_push_pattern(context,&v[0]->value.v_pair->head,clone) | asm_push_pattern(context,&v[0]->value.v_pair->tail,clone);
			struct_add(context,v);
			return v[0]->clone;
		case VM_VARIBLE_TUPLE:
		case VM_VARIBLE_RECORD:{
			int a = 0;
			asm_record_pair_t* r = v[0]->value.v_record->data;
			while(r != NULL){
				a |= asm_push_pattern(context,&r->value,clone);
				r = r->next;
			}
			v[0]->clone = a;
			struct_add(context,v);
			return a;
		}			
		case VM_VARIBLE_PATTERN:
			return clone;
		default:
			return 0;
	}
}

