#include <string.h>
#include <stdio.h>
#include "asm_convert.h"

static inline uint32_t asm_varible_real_size(asm_varible_t* v){
	uint32_t acc = 4;
	//printf("%d ",v->type);
	switch(v->type){
		case VM_VARIBLE_INT:
			acc += sizeof(vm_int_t);
			break;
		case VM_VARIBLE_ATOM:
			acc += sizeof(vm_atom_t);
			break;
		case VM_VARIBLE_FLOAT:
			acc += sizeof(vm_float_t);
			break;
		case VM_VARIBLE_PAIR:
			acc += sizeof(vm_pair_t);
			break;
		case VM_VARIBLE_TUPLE:	{
			acc += sizeof(vm_record_t);
			asm_record_pair_t* r = v->value.v_record->data;
			while(r != NULL){
				acc += sizeof(vm_variable_t*);
				r = r->next;
			}
			break;
		}
		case VM_VARIBLE_RECORD:	{
			acc += sizeof(vm_record_t);
			asm_record_pair_t* r = v->value.v_record->data;
			while(r != NULL){
				acc += sizeof(vm_record_pair_t);
				r = r->next;
			}
			break;
		}
		case VM_VARIBLE_FUNCTION:
			acc += sizeof(vm_function_instance_t);
			break;
		case VM_VARIBLE_PATTERN:
			acc += sizeof(vm_bytecode_t)+sizeof(uint32_t);
			break;
		default:
			break;
	}
	return acc;
}

static uint32_t asm_varible_convert(vm_staticmem_t* smem, uint8_t* dst_raw, asm_varible_t* src){
	vm_variable_t* dst = (vm_variable_t*)dst_raw;
	dst->type = src->type;
	dst->clone = src->clone;
	dst->mutex_id = VM_NOCOPY_MUTEX;
	switch(src->type){
		case VM_VARIBLE_INT:
			dst->value.v_int = src->value.v_int; 
			return 4+sizeof(vm_int_t);
		case VM_VARIBLE_ATOM:
			dst->value.v_atom = src->value.v_int; 
			return 4+sizeof(vm_atom_t);
		case VM_VARIBLE_FLOAT:
			dst->value.v_float = src->value.v_float; 
			return 4+sizeof(vm_float_t);
		case VM_VARIBLE_PAIR:
			dst->value.v_pair.head = (vm_variable_t*)src->value.v_pair->head;
			dst->value.v_pair.tail = (vm_variable_t*)src->value.v_pair->tail;
			return 4+sizeof(vm_pair_t);
		case VM_VARIBLE_TUPLE:	{
			dst->value.v_record.id = src->value.v_record->id;
			asm_record_pair_t* r = src->value.v_record->data;
			uint32_t index = 0;
			while(r != NULL){
				dst->value.v_record.data.item[index] = (vm_variable_t*)r->value;
				index++;
				r = r->next;
			}
			dst->value.v_record.size = index;
			return 4+sizeof(vm_record_t)+index*sizeof(vm_variable_t*);
		}
		case VM_VARIBLE_RECORD:	{
			dst->value.v_record.id = src->value.v_record->id;
			asm_record_pair_t* r = src->value.v_record->data;
			uint32_t index = 0;
			while(r != NULL){
				dst->value.v_record.data.pair[index].key = r->key;
				dst->value.v_record.data.pair[index].value = (vm_variable_t*)r->value;
				index++;
				r = r->next;
			}
			dst->value.v_record.size = index;
			return 4+sizeof(vm_record_t)+index*sizeof(vm_record_pair_t);
		}
		case VM_VARIBLE_FUNCTION:
			dst->value.v_function.prototype = &smem->prototypes[src->value.v_function->index];
			return 4+sizeof(vm_function_instance_t);
		case VM_VARIBLE_PATTERN:
			dst->value.v_bytecode.op = 0;
			dst->value.v_bytecode.type[0] = src->value.v_ref.type;
			dst->value.v_bytecode.type[1] = 0;
			dst->value.v_bytecode.type[2] = 0;
			dst->value.v_bytecode.value[0] = src->value.v_ref.value;
			return 4+sizeof(vm_bytecode_t)+sizeof(uint32_t);
		default:
			return 4;
	}
	return 4;
}

static inline uint32_t asm_varibles_size(asm_context_t* context){
	uint32_t acc = 0;
	var_avl_t node = var_avl_first(&context->vtree);
	while(node != NULL){
		acc += asm_varible_real_size(node->key);
		node = var_avl_next(node);
	}
	return acc;
}

static inline uint32_t asm_progmem_size(asm_context_t* context){
	uint32_t acc = 0;
	asm_bytecode_t* head = context->head;
	while(head != NULL){
		head->offset = acc;
		head->line = head->op>>16;
		head->op &= 0xFFFF;
		acc += sizeof(vm_bytecode_t);
		switch(head->op){	
			case VM_OP_INT:
			case VM_OP_YIELD:
				break;
			case VM_OP_BRANCH:
			case VM_OP_KEY:
			case VM_OP_BIND:
			case VM_OP_MATCH:
			case VM_OP_MOVE:
				acc += 2*sizeof(uint32_t);
				break;
			case VM_OP_RPN:
				head->line = head->value[0].v_int>>16;
				head->value[0].v_int &= 0xFFFF;
				acc += sizeof(uint32_t);
				if(head->value[0].v_int == VM_RPN_PUSH)
					acc += sizeof(uint32_t);
				break;
			case VM_OP_PUSH:
				if(head->type[0] != VM_BYTECODE_ACC)
					acc += sizeof(uint32_t);
				break;
			default:
				acc += sizeof(uint32_t);
				break;
		}
		head = head->next;
	}
	return acc;
}

static inline uint32_t asm_symbols_size(asm_context_t* context){
	uint32_t acc = 0;
	char_avl_t node = char_avl_first(&context->stree);
	while(node != NULL){
		func_avl_t fnode = func_avl_get(&context->ftree,node->value);
		if(fnode == NULL){
			int_avl_t anode = int_avl_get(&context->atree,node->value);
			if(anode != NULL)
				acc += strlen(node->key)+1;
		}else{
			acc += strlen(node->key)+1;
		}
		node = char_avl_next(node);
	}
	return acc;
}

static inline char* str_copy(char* dst, char* src){
	while(*src != 0)
		*dst++ = *src++;
	*dst++ = 0;
	return dst;
}

static inline vm_lib_t* lib_find(vm_lib_t* lib, char* name){
	while(lib->name != NULL){
		if(strcmp(lib->name,name) == 0)
			return lib;
		lib++;
	}
	return NULL;
}

static inline char* find_label(asm_context_t* context, asm_bytecode_t** b){
	label_avl_t lnode = label_avl_first(&context->ltree);
	while(lnode != NULL){
		if(lnode->value == b)
			break;
		lnode = label_avl_next(lnode);
	}
	if(lnode == NULL)
		return NULL;
	char_avl_t cnode = char_avl_first(&context->stree);
	while(cnode != NULL){
		if(cnode->value == lnode->key)
			return cnode->key;
		cnode = char_avl_next(cnode);
	}
	return NULL;
}

static inline uint32_t derefernace_label(asm_context_t* context, asm_bytecode_t** b){
	if(b[0] == NULL){
		char* str = find_label(context,b);
		if(str == NULL)
			printf("syntax error: undefined label\n");
		else
			printf("syntax error: undefined label '%s'\n",str);
		return 0;
	}
	return b[0]->next->offset;
}

void* asm_convert(asm_context_t* context, vm_staticmem_t* smem, vm_lib_t* lib){
	//allocate memory
	uint32_t smem_size = 0;
	uint32_t proto_pos = 0;
	smem_size += sizeof(vm_function_t)*context->ftree.size;
	uint32_t vptr_pos = smem_size;
	smem_size += sizeof(vm_variable_t*)*context->vtree_index;
	uint32_t progmem_pos = smem_size;
	smem_size += asm_progmem_size(context);
	uint32_t str_pos = smem_size;
	smem_size += asm_symbols_size(context);
	uint32_t satom_pos = smem_size;
	smem_size += sizeof(char**)*context->atree.size;
	uint32_t sfunc_pos = smem_size;
	smem_size += sizeof(char**)*context->ftree.size;
	uint32_t line_pos = smem_size;
	smem_size += context->tail->line*sizeof(uint32_t);
	uint32_t var_pos = smem_size;
	smem_size += asm_varibles_size(context);
	uint8_t* block = (uint8_t*)malloc(smem_size);
	memset(block,0,smem_size);
	smem->prototypes = (vm_function_t*)(block + proto_pos);
	smem->constants = (vm_variable_t**)(block + vptr_pos);
	smem->program = block + progmem_pos;
	smem->symbols.atom = (char**)(block + satom_pos);
	smem->symbols.function = (char**)(block + sfunc_pos);
	smem->symbols.line = (uint32_t*)(block + line_pos);
	
	//convert progmem
	asm_bytecode_t* head = context->head;
	while(head != NULL){
		vm_bytecode_t* b = (vm_bytecode_t*)(block+progmem_pos+head->offset);
		b->op = head->op;
		b->type[0] = head->type[0];
		b->type[1] = head->type[1];
		b->type[2] = 0;
		switch(head->op){	
			case VM_OP_INT:
			case VM_OP_YIELD:
				break;
			case VM_OP_PUSH:
				if(head->type[0] != VM_BYTECODE_ACC)
					b->value[0] = head->value[0].v_int;
				break;
			case VM_OP_JMP:
				b->value[0] = derefernace_label(context,head->value[0].v_addr);
				if(b->value[0] == 0){
					free(block);
					return NULL;
				}
				break;
			case VM_OP_BRANCH:
				b->value[0] = head->value[0].v_int;
				b->value[1] = derefernace_label(context,head->value[1].v_addr);
				if(b->value[1] == 0){
					free(block);
					return NULL;
				}
				break;
			case VM_OP_KEY:
			case VM_OP_BIND:
			case VM_OP_MATCH:
			case VM_OP_MOVE:
				b->value[0] = head->value[0].v_int;
				b->value[1] = head->value[1].v_int;
				break;
			case VM_OP_RPN:
				if(head->value[0].v_int == VM_RPN_PUSH){
					b->value[0] = head->value[0].v_int;
					b->value[1] = head->value[1].v_int;
					break;
				}
			default:
				b->value[0] = head->value[0].v_int;
		}
		head = head->next;
	}
	
	uint32_t* linetable = smem->symbols.line;
	uint32_t current = 1;
	asm_bytecode_t* b = context->head->next;
	while(1){
		while(b != NULL && b->line <= current)
			b = b->next;
		if(b == NULL)
			break;
		while(b->line > current){
			(linetable++)[0] = b->offset;
			current++;
		}
	}
	while(context->tail->line >= current){
		(linetable++)[0] = 0xFFFFFFFF;
		current++;
	}
		
	//convert symbols
	char* str = (char*)(block+str_pos);
	char_avl_t cnode = char_avl_first(&context->stree);
	while(cnode != NULL){
		func_avl_t fnode = func_avl_get(&context->ftree,cnode->value);
		if(fnode == NULL){
			int_avl_t anode = int_avl_get(&context->atree,cnode->value);
			if(anode != NULL){
				smem->symbols.atom[anode->value] = str;
				str = str_copy(str,cnode->key);
			}
		}else{
			smem->symbols.function[fnode->value.index] = str;
			str = str_copy(str,cnode->key);
		}
		cnode = char_avl_next(cnode);
	}
	
	//convert prototypes
	func_avl_t fnode = func_avl_first(&context->ftree);
	while(fnode != NULL){
		asm_function_t* src = &fnode->value;
		vm_function_t* dst = &smem->prototypes[src->index];
		if(src->initilized != 2){
			printf("link error: undeclared function '%s'\n",smem->symbols.function[src->index]);
			free(block);
			return NULL;
		}
		dst->arity = src->arity;
		dst->closure_size = src->closure;
		if(src->addr == NULL){
			dst->type = VM_FUNCTION_NATIVE;
			vm_lib_t* l = lib_find(lib,smem->symbols.function[src->index]);
			if(l == NULL){
				printf("link error: unknown external function '%s'\n",smem->symbols.function[src->index]);
				free(block);
				return NULL;
			}
			dst->addr.v_native = l->addr;
			dst->arity = l->arity;
		}else{
			dst->type = VM_FUNCTION_DYNAMIC;
			dst->addr.v_dynamic = (vm_bytecode_t*)(smem->program+src->addr->next->offset);
		}
		fnode = func_avl_next(fnode);
	}

	//convert varibles
	//fill pointer array
	uint8_t* ptr = block+var_pos;
	for(uint32_t i=0; i<context->vtree.size; i++){
		asm_varible_t* v = context->vtree.data[i].key;
		uint32_t off = asm_varible_convert(smem,ptr,v);
		v->value.v_conv = (vm_variable_t*)ptr;
		if(v->index >= 0)
			smem->constants[v->index] = (vm_variable_t*)ptr;
		ptr += off;	
	}
	//update pointers
	for(uint32_t i=0; i<context->vtree.size; i++){
		vm_variable_t* v = context->vtree.data[i].key->value.v_conv;
		switch(v->type){
			case VM_VARIBLE_PAIR:
				v->value.v_pair.head = ((asm_varible_t*)v->value.v_pair.head)->value.v_conv;
				v->value.v_pair.tail = ((asm_varible_t*)v->value.v_pair.tail)->value.v_conv;
				break;
			case VM_VARIBLE_TUPLE:
				for(uint32_t j=0; j<v->value.v_record.size; j++)
					v->value.v_record.data.item[j] = ((asm_varible_t*)(v->value.v_record.data.item[j]))->value.v_conv;
				break;
			case VM_VARIBLE_RECORD:
				for(uint32_t j=0; j<v->value.v_record.size; j++)
					v->value.v_record.data.pair[j].value =  ((asm_varible_t*)(v->value.v_record.data.pair[j].value))->value.v_conv;
				break;
			case VM_VARIBLE_FUNCTION:
				if(v->value.v_function.prototype->closure_size != 0){
					printf("link error: static call of non-static function '%s'\n",smem->symbols.function[v->value.v_function.prototype-smem->prototypes]);
					free(block);
					return NULL;
				}
				break;
			default:
				break;
		}
	}
	
	//fwrite(block,1,smem_size,fopen("aa","wb"));
	return block;
}