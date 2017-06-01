#include "vm_print.h"
#include "vm_util.h"

static inline const char* get_atom(vm_staticmem_t* smem, vm_atom_t atom){
	if(smem == NULL || smem->symbols.atom == NULL)
		return "_null";
	return smem->symbols.atom[atom];
}

static inline const char* get_function(vm_staticmem_t* smem, vm_atom_t atom){
	if(smem == NULL || smem->symbols.function == NULL)
		return "_null";
	return smem->symbols.function[atom];
}

static void vm_print_pair(vm_staticmem_t* smem, vm_variable_t* v, int depth){
	putchar('(');
	vm_print_varible(smem,v->value.v_pair.head,depth);
	printf(" | ");
	if(depth == 0){
		printf("...)");
		return;
	}
	vm_variable_t* tail = v->value.v_pair.tail;
	while(tail->type == VM_VARIBLE_REF)
		tail = tail->value.v_varible;
	if(tail->type == VM_VARIBLE_PAIR)
		vm_print_pair(smem,tail,depth-1);
	else
		vm_print_varible(smem,tail,depth);
	putchar(')');
}

static void vm_print_list(vm_staticmem_t* smem, vm_variable_t* v, int depth){
	vm_variable_t* head = v;
	int d = 100;
	if(d < depth)
		d = depth;
	while((d > 0 || depth<0) && head->type == VM_VARIBLE_PAIR){
		d--;
		head = head->value.v_pair.tail;
		while(head->type == VM_VARIBLE_REF)
			head = head->value.v_varible;
	}
	if(d == 0 || head->type != VM_VARIBLE_ATOM || head->value.v_atom != VM_ATOM_NIL){
		vm_print_pair(smem, v, depth);
		return;
	}
	putchar('[');
	vm_print_varible(smem,v->value.v_pair.head,depth);
	v = v->value.v_pair.tail;
	while(v->type == VM_VARIBLE_REF)
		v = v->value.v_varible;
	while(v != head){
		if(depth == 0)
			break;
		putchar(' ');
		vm_print_varible(smem,v->value.v_pair.head,depth);
		v = v->value.v_pair.tail;
		while(v->type == VM_VARIBLE_REF)
			v = v->value.v_varible;
	}
	putchar(']');
}

static void vm_print_record(vm_staticmem_t* smem, vm_variable_t* v, int depth){
	printf("%s(",get_atom(smem,v->value.v_record.id));
	if(v->value.v_record.size > 0){
		printf("%s:",get_atom(smem,v->value.v_record.data.pair[0].key));
		vm_print_varible(smem,v->value.v_record.data.pair[0].value,depth);
		for(uint32_t i = 1; i<v->value.v_record.size; i++){
			if(depth == 0)
				break;
			printf(" %s:",get_atom(smem,v->value.v_record.data.pair[i].key));
			vm_print_varible(smem,v->value.v_record.data.pair[i].value,depth);
		}
	}
	putchar(')');
}

static void vm_print_tuple(vm_staticmem_t* smem, vm_variable_t* v, int depth){
	printf("%s(",get_atom(smem,v->value.v_record.id));
	if(v->value.v_record.size > 0){
		vm_print_varible(smem,v->value.v_record.data.item[0],depth);
		for(uint32_t i = 1; i<v->value.v_record.size; i++){
			if(depth == 0)
				break;
			putchar(' ');
			vm_print_varible(smem,v->value.v_record.data.item[i],depth);
		}
	}
	putchar(')');
}

static void vm_print_function(vm_staticmem_t* smem, vm_variable_t* v, int depth){
	vm_function_t* p = v->value.v_function.prototype;
	if(smem == NULL)
		printf("_function");
	printf("_function<%s>",get_function(smem,p-smem->prototypes));
	if(p->closure_size > 0){
		putchar('{');
		vm_print_varible(smem,v->value.v_function.closure[0],depth);
		for(uint32_t i = 1; i<p->closure_size; i++){
			if(depth == 0)
				break;
			putchar(' ');
			vm_print_varible(smem,v->value.v_function.closure[i],depth);
		}
		putchar('}');
	}
}

static void vm_print_argument(vm_staticmem_t* smem, vm_bytecode_t* b, int n, int depth){
	switch(b->type[n]){
		case VM_BYTECODE_OTHER:
			printf("_other");
			break;
		case VM_BYTECODE_LOCAL:
			printf("s.%d",b->value[n]);
			break;
		case VM_BYTECODE_CLOSURE:
			printf("c.%d",b->value[n]);
			break;
		case VM_BYTECODE_CONSTANT:
			vm_print_varible(smem,smem->constants[b->value[n]],depth);
			break;
		case VM_BYTECODE_ACC:
			printf("acc");
			break;
		default:
			printf("_?");
	}
}

void vm_print_varible(vm_staticmem_t* smem, vm_variable_t* v, int depth){
	if(depth == 0){
		printf("...");
		return;
	}
	depth--;
	i_love_goto:
	switch(v->type){
		case VM_VARIBLE_UNDEF:
			putchar('_');
			break;
		case VM_VARIBLE_INT:
			printf("%d",v->value.v_int);
			break;
		case VM_VARIBLE_ATOM:
			printf("%s",get_atom(smem,v->value.v_atom));
			break;
		case VM_VARIBLE_FLOAT:
			printf("%#g",v->value.v_float);
			break;
		case VM_VARIBLE_PAIR:
			vm_print_list(smem, v, depth);
			break;
		case VM_VARIBLE_TUPLE:
			vm_print_tuple(smem, v, depth);
			break;
		case VM_VARIBLE_RECORD:
			vm_print_record(smem, v, depth);
			break;
		case VM_VARIBLE_REF:
			while(v->type == VM_VARIBLE_REF)
				v = v->value.v_varible;
			goto i_love_goto;
		case VM_VARIBLE_GCREF:
			printf("_gcref");
			break;
		case VM_VARIBLE_FUNCTION:
			vm_print_function(smem, v, depth);
			break;
		case VM_VARIBLE_THREAD:
			printf("_thread");
			break;
		case VM_VARIBLE_WILDCHAR:
			printf("_wildchar");
			break;
		case VM_VARIBLE_PATTERN:
			vm_print_argument(smem,&v->value.v_bytecode,0,depth);
			break;
		case VM_VARIBLE_PLACEHOLDER:
			putchar('@');
			break;
		default:
			printf("_unknown");
			break;
	}
}

static const char* op_symbols[] = {
	"int","yield","push","pop","create","call","rcall","tcall","ret","jmp",
	"branch","inst","closure","key","bind","match","move","rpn"
};

static inline int find_line(vm_staticmem_t* smem, uint32_t offset){
	if(smem->symbols.line == NULL)
		return 0;
	uint32_t line = 0;
	while(smem->symbols.line[line++] <= offset);
	return line;	
}

void vm_print_bytecode(vm_staticmem_t* smem, vm_bytecode_t* b, int depth){
	uint32_t offset = ((uint8_t*)b)-smem->program;
	printf("[%03d] %s ",find_line(smem,offset),op_symbols[b->op]);
	switch(b->op){
		case VM_OP_JMP:
			printf("l.%d",find_line(smem,b->value[0]));
			break;
		case VM_OP_BRANCH:
			vm_print_argument(smem,b,0,depth);
			printf(", l.%d",find_line(smem,b->value[1]));
			break;
		case VM_OP_POP:
		case VM_OP_CREATE:
		case VM_OP_RET:
			printf("%d",b->value[0]);
			break;
		case VM_OP_INST:
			printf("%s",get_function(smem,b->value[0]));
			break;
		case VM_OP_PUSH:
		case VM_OP_CALL:
		case VM_OP_RCALL:
		case VM_OP_TCALL:
		case VM_OP_KEY:
			vm_print_argument(smem,b,0,depth);
			break;
		case VM_OP_BIND:
		case VM_OP_MATCH:
		case VM_OP_MOVE:
			vm_print_argument(smem,b,0,depth);
			printf(", ");
			vm_print_argument(smem,b,1,depth);
			break;
		default:
			break;
	}
}

const char* exception_symbols[] = {
	"EXCEPTION_NONE",
	"EXCEPTION_THREAD_STOP",
	"EXCEPTION_THREAD_EXIT",
	"EXCEPTION_THREAD_OOM",
	"EXCEPTION_OOM",
	"EXCEPTION_HALT",
	"EXCEPTION_TYPE_ERROR",
	"EXCEPTION_KEY_ERROR",
	"EXCEPTION_BAD_OPCODE",
	"EXCEPTION_ZERO_DIVIDE",
	"EXCEPTION_OOB",
	"EXCEPTION_WTF"
};

void vm_print_exception(vm_context_t* context, vm_exception_enum_t e){
	printf("unhandled '%s' in ",exception_symbols[e]);
	vm_print_bytecode(&context->smem,context->env.pc,-1);
	if(context->env.size == context->env.rsp){
		putchar('\n');
		return;
	}
	printf("\nstack trace:\n");
	uint32_t rsp = context->env.rsp+1;
	int cnt = 0;
	while(cnt < 8 && rsp < context->env.size){
		cnt++;
		vm_print_bytecode(&context->smem,(vm_bytecode_t*)context->stack[rsp],-1);
		putchar('\n');
		rsp += 3;
	}
	if(cnt == 8 && rsp+3 < context->env.size)
		printf("...and %d more\n",(context->env.size-1-rsp)/3);
}