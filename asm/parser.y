%define api.pure full
%define parse.error verbose
%define parse.lac full
%locations
%token-table
%lex-param {yyscan_t scanner}
%parse-param {void* scanner}
%param {asm_context_t* context}
%expect 2

%code requires{
	#include <inttypes.h>
	#include "asm_struct.h"
	#define YY_DECL int yylex(YYSTYPE * yylval_param, YYLTYPE * yylloc_param , yyscan_t yyscanner, asm_context_t* context)
}

%union { 
	asm_record_pair_t* v_recpair;
	asm_varible_t* v_varible;
	asm_refpair_t v_refpair;
	uint32_t v_int;
	double v_float;
}

%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "lexer.h"
	extern YY_DECL;
	int yyerror(YYLTYPE *locp, yyscan_t scanner, asm_context_t* context, char const *msg);
%}

%right '|'

%token <v_int> TOKEN_INVALID
%token <v_int> TOKEN_KEYWORD_EXTERN TOKEN_KEYWORD_DEF TOKEN_KEYWORD_ACC
%token <v_int> TOKEN_CONST_ATOM TOKEN_CONST_VARNAME
%token <v_int> TOKEN_CONST_INTEGER TOKEN_REF_STACK TOKEN_REF_CLOSURE
%token <v_int> TOKEN_OP_ANY TOKEN_OP_FUN TOKEN_OP_REF TOKEN_OP_INT TOKEN_OP_CINT TOKEN_OP_INST TOKEN_OP_LABEL TOKEN_OP_KEY TOKEN_OP_MOVE TOKEN_OP_NOARG
%token <v_int> TOKEN_OP_BRANCH TOKEN_OP_PATTERN TOKEN_OP_RPN TOKEN_OP_RPNPUSH
%token <v_float> TOKEN_CONST_FLOAT

%type <v_recpair> record_pair record_pairs
%type <v_refpair> anyref scref stackref closureref accref
%type <v_varible> o_integer o_float o_atom o_varname o_any o_pattern o_struct o_num o_ref o_spec pattern struct pair list list_items record

%%

program:
	  program function
	| program extern
	| %empty
;

extern:	TOKEN_KEYWORD_EXTERN extern_items ;

extern_items:
	  extern_items extern_item
	| extern_item
;

extern_item: TOKEN_CONST_VARNAME {
	asm_function_t* f = asm_add_function(context,$1);
	f->addr = NULL;
	f->initilized = 2;
};

function: function_head statements statement_notlabel ;

function_head: TOKEN_KEYWORD_DEF TOKEN_CONST_VARNAME '(' TOKEN_CONST_INTEGER ',' TOKEN_CONST_INTEGER ')' {
	asm_function_t* f = asm_add_function(context,$2);
	if(f->initilized == 2){
		yyerror(&yylloc, scanner, context, "syntax error: function redefined");
		YYERROR;
	}
	f->arity = $4;
	f->closure = $6;
	f->addr = context->tail;
	f->initilized = 2;
	label_avl_clear(&context->ltree);
};
	
statements:	statements statement | %empty;

statement: 
	 statement_notlabel
	 | TOKEN_CONST_ATOM ':'{ 
		if(asm_allocate_label(context,$1,context->tail)==NULL){
	  		yyerror(&yylloc, scanner, context, "syntax error: label redefined");
			YYERROR;
		}
};

statement_notlabel:	
	  TOKEN_OP_ANY o_any				{ asm_register_var(context,$2); asm_bytecode_next(context,$1,VM_BYTECODE_CONSTANT,$2->index); }
	| TOKEN_OP_ANY anyref				{ asm_bytecode_next(context,$1,$2.type,$2.value); }
	| TOKEN_OP_FUN o_varname			{ asm_register_var(context,$2); asm_bytecode_next(context,$1,VM_BYTECODE_CONSTANT,$2->index); }
	| TOKEN_OP_FUN scref				{ asm_bytecode_next(context,$1,$2.type,$2.value); }
	| TOKEN_OP_REF scref				{ asm_bytecode_next(context,$1,$2.type,$2.value); }
	| TOKEN_OP_NOARG					{ asm_bytecode_next(context,$1,VM_BYTECODE_OTHER,0); }
	| TOKEN_OP_INST TOKEN_CONST_VARNAME { asm_bytecode_next(context,$1,VM_BYTECODE_OTHER,asm_add_function(context,$2)->index); }
	| TOKEN_OP_RPN						{ asm_bytecode_next(context,VM_OP_RPN,VM_BYTECODE_OTHER,$1); }
	| TOKEN_OP_INT TOKEN_CONST_INTEGER	{ asm_bytecode_next(context,$1,VM_BYTECODE_OTHER,$2); }
	| TOKEN_OP_CINT TOKEN_CONST_INTEGER	{ asm_bytecode_next(context,$1,VM_BYTECODE_OTHER,$2); }
	| TOKEN_OP_CINT						{ asm_bytecode_next(context,$1,VM_BYTECODE_OTHER,(($1&0xFFFF)==VM_OP_RET?0:1)); }
	| TOKEN_OP_MOVE scref ',' o_any {
		asm_register_var(context,$4);
		asm_bytecode_next(context,$1,$2.type,$2.value);
		context->tail->type[1] = VM_BYTECODE_CONSTANT; 
		context->tail->value[1].v_int = $4->index; }
	| TOKEN_OP_MOVE scref ',' scref {
		asm_bytecode_next(context,$1,$2.type,$2.value);
		context->tail->type[1] = $4.type; 
		context->tail->value[1].v_int = $4.value; }
	| TOKEN_OP_KEY scref ',' TOKEN_CONST_ATOM {
		asm_bytecode_next(context,$1,$2.type,$2.value);
		context->tail->type[1] = VM_BYTECODE_OTHER; 
		context->tail->value[1].v_int = asm_add_atom(context,$4); }
	| TOKEN_OP_KEY scref ',' TOKEN_CONST_INTEGER {
		asm_bytecode_next(context,$1,$2.type,$2.value);
		context->tail->type[1] = VM_BYTECODE_OTHER; 
		context->tail->value[1].v_int = $4; }
	| TOKEN_OP_LABEL TOKEN_CONST_ATOM {
		asm_bytecode_next(context,$1,VM_BYTECODE_OTHER,0);
		context->tail->value[0].v_addr = asm_allocate_label(context,$2,NULL);}
	| TOKEN_OP_RPNPUSH o_num {
		asm_register_var(context,$2); 
		asm_bytecode_next(context,VM_OP_RPN,VM_BYTECODE_OTHER,$1);
		context->tail->type[1] = VM_BYTECODE_CONSTANT; 
		context->tail->value[1].v_int = $2->index; }
	| TOKEN_OP_RPNPUSH anyref {
		asm_bytecode_next(context,VM_OP_RPN,VM_BYTECODE_OTHER,$1); 
		context->tail->type[1] = $2.type;
		context->tail->value[1].v_int = $2.value; }						
	| TOKEN_OP_BRANCH anyref ',' TOKEN_CONST_ATOM {
		asm_bytecode_next(context,$1,$2.type,$2.value);
		context->tail->value[1].v_addr = asm_allocate_label(context,$4,NULL); }
	| TOKEN_OP_PATTERN o_pattern ',' o_pattern {
		asm_register_var(context,$2);
		asm_register_var(context,$4);
		asm_bytecode_next(context,$1,VM_BYTECODE_CONSTANT,$2->index);
		context->tail->type[1] = VM_BYTECODE_CONSTANT; 
		context->tail->value[1].v_int = $4->index; }
	| TOKEN_OP_PATTERN scref ',' o_pattern {
		asm_register_var(context,$4);
		asm_bytecode_next(context,$1,$2.type,$2.value);
		context->tail->type[1] = VM_BYTECODE_CONSTANT; 
		context->tail->value[1].v_int = $4->index; }
	| TOKEN_OP_PATTERN o_pattern ',' scref {
		asm_register_var(context,$2);
		asm_bytecode_next(context,$1,VM_BYTECODE_CONSTANT,$2->index);
		context->tail->type[1] = $4.type; 
		context->tail->value[1].v_int = $4.value; }
	| TOKEN_OP_PATTERN scref ',' scref {
		asm_bytecode_next(context,$1,$2.type,$2.value);
		context->tail->type[1] = $4.type; 
		context->tail->value[1].v_int = $4.value; }
;

accref: TOKEN_KEYWORD_ACC		{ $$.type = VM_BYTECODE_ACC; $$.value = 0; } ;
stackref: TOKEN_REF_STACK		{ $$.type = VM_BYTECODE_LOCAL; $$.value = $1; } ;
closureref: TOKEN_REF_CLOSURE	{ $$.type = VM_BYTECODE_CLOSURE; $$.value = $1; } ;
scref: stackref | closureref ;
anyref: scref | accref ;

o_integer: TOKEN_CONST_INTEGER 	{ $$ = asm_allocate_integer(context,$1); } ;
o_float: TOKEN_CONST_FLOAT 		{ $$ = asm_allocate_float(context,$1); } ;
o_atom: TOKEN_CONST_ATOM 		{ $$ = asm_allocate_atom(context,$1); } ;
o_varname: TOKEN_CONST_VARNAME	{ $$ = asm_allocate_function(context,$1); } ;

o_any: o_struct | o_integer | o_float | o_atom | o_varname ;
o_num: o_integer | o_float | o_atom ;
o_ref: scref	{ $$ = asm_allocate_pattern(context,$1); } ;
o_spec:
	  '_'		{ $$ = asm_allocate_other(context,VM_VARIBLE_WILDCHAR); }
	| '@'		{ $$ = asm_allocate_other(context,VM_VARIBLE_PLACEHOLDER); }
;
o_struct: struct { $$=$1; asm_push_pattern(context,&$$,0); } ;
o_pattern: o_any |  o_spec ;


pattern: struct | o_spec | o_num | o_varname | o_ref ;
struct: record | pair | list;

pair:
	  '(' pair ')'			{ $$ = $2; }
	| '!' '(' pair ')'		{ $$ = $3; $$->clone = 1; }
	| pattern '|' pattern	{ $$ = asm_allocate_pair(context,$1,$3); }
;

record:
	  TOKEN_CONST_ATOM '(' record_pairs ')'			{ $$ = asm_allocate_record(context,$1,$3); }
	|  '!' TOKEN_CONST_ATOM '(' record_pairs ')'	{ $$ = asm_allocate_record(context,$2,$4); $$->clone = 1; }
;

record_pairs:
	  record_pair
	| record_pair record_pairs 			{ $1->next = $2; $$ = $1; }
;
	
record_pair:
	  TOKEN_CONST_ATOM ':' pattern	{ $$ = asm_allocate_record_pair(context,$1,$3); } 
	| pattern { $$ = asm_allocate_record_pair(context,0xFFFFFFFF,$1); } 
;
	
list:
	  '[' list_items ']'		{ $$ = $2; }
	| '!' '[' list_items ']'	{ $$ = $3; $$->clone = 1; }
;

list_items:
	  pattern					{ $$ = asm_allocate_pair(context,$1,context->nil); }
	| pattern list_items		{ $$ = asm_allocate_pair(context,$1,$2); }
;

%%

int yyerror (YYLTYPE *locp, yyscan_t scanner, asm_context_t* context, char const *msg){
	if(locp)
		fprintf(stderr, "%s (:%d.%d -> :%d.%d)\n", msg, locp->first_line, locp->first_column, locp->last_line, locp->last_column);
 	else
    	fprintf(stderr, "%s\n", msg);
	return 0;
}
