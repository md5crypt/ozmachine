#pragma once

typedef enum {
	VM_SUPERVISOR_NOP,
	VM_SUPERVISOR_TERMINATE,
	VM_SUPERVISOR_GC
} vm_supervisor_enum_t;

typedef enum {
	VM_EXCEPTION_NONE,
	VM_EXCEPTION_THREAD_STOP,
	VM_EXCEPTION_THREAD_EXIT,
	VM_EXCEPTION_THREAD_OOM,
	VM_EXCEPTION_OOM,
	VM_EXCEPTION_HALT,
	VM_EXCEPTION_TYPE_ERROR,
	VM_EXCEPTION_KEY_ERROR,
	VM_EXCEPTION_BAD_OPCODE,
	VM_EXCEPTION_ZERO_DIVIDE,
	VM_EXCEPTION_OOB,
	VM_EXCEPTION_WTF
} vm_exception_enum_t;

typedef enum __attribute__ ((__packed__)) {
	VM_VARIBLE_UNDEF,
	VM_VARIBLE_INT,
	VM_VARIBLE_ATOM,
	VM_VARIBLE_FLOAT,
	VM_VARIBLE_PAIR,
	VM_VARIBLE_TUPLE,
	VM_VARIBLE_RECORD,
	VM_VARIBLE_REF,
	VM_VARIBLE_GCREF,
	VM_VARIBLE_FUNCTION,
	VM_VARIBLE_THREAD,
	VM_VARIBLE_WILDCHAR, //new undef on copy, match any on match
	VM_VARIBLE_PATTERN, //read var from local stack
	VM_VARIBLE_PLACEHOLDER  //push var onto local stack
} vm_varible_enum_t;

typedef enum __attribute__ ((__packed__)) {
	VM_BYTECODE_OTHER,
	VM_BYTECODE_LOCAL,
	VM_BYTECODE_CLOSURE,
	VM_BYTECODE_CONSTANT,
	VM_BYTECODE_ACC
} vm_bytecode_enum_t;

typedef enum{
	VM_RPN_PUSH,
	VM_RPN_ADD,
	VM_RPN_SUB,
	VM_RPN_MUL,
	VM_RPN_DIV,
	VM_RPN_MOD,
	VM_RPN_POW,
	VM_RPN_NEG,
	VM_RPN_EQ,
	VM_RPN_NEQ,
	VM_RPN_LEQ,
	VM_RPN_GEQ,
	VM_RPN_LE,
	VM_RPN_GE,
	VM_RPN_OR,
	VM_RPN_AND,
	VM_RPN_XOR,
	VM_RPN_NOT
} vm_rpn_op_enum_t;

typedef enum{
	VM_RPN_INT,
	VM_RPN_FLOAT,
	VM_RPN_BOOL
} vm_rpn_type_enum_t;

typedef enum __attribute__ ((__packed__)) {
	VM_OP_INT,
	VM_OP_YIELD,
	//stack
	VM_OP_PUSH,		/* var */
	VM_OP_POP,		/* n */
	VM_OP_CREATE,	/* n */
	//controll
	VM_OP_CALL,		/* var */		//--
	VM_OP_RCALL,	/* var */		//--
	VM_OP_TCALL,	/* var */		//--
	VM_OP_RET,		/*  n  */		//--
	VM_OP_JMP,		/* addr */
	VM_OP_BRANCH,	/* addr */
	VM_OP_INST,		/* func */		//--
	VM_OP_CLOSURE,	/* var */		//--
	//arth
	VM_OP_KEY,		/* id */
	VM_OP_BIND,		/* var */		//--
	VM_OP_MATCH,	/* pattern */	//--
	VM_OP_MOVE,
	VM_OP_RPN,		/* op */ 		//--
} vm_op_enum_t;

typedef enum {
	VM_FUNCTION_DYNAMIC,
	VM_FUNCTION_NATIVE
} vm_function_enum_t;
