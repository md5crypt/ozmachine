%option bison-bridge
%option bison-locations
%option noyywrap
%option reentrant
%option warn
%option yylineno

%{
	#include "parser.h"
	#define YY_USER_ACTION \
	    yylloc->first_line = yylloc->last_line; \
	    yylloc->first_column = yylloc->last_column; \
	    for(int i = 0; yytext[i] != '\0'; i++) { \
		if(yytext[i] == '\n') { \
		    yylloc->last_line++; \
		    yylloc->last_column = 0; \
		} \
		else { \
		    yylloc->last_column++; \
		} \
	    }
%}

%%

[ \f\r\t\v\n]+	/* eat whitespace */
[;][^\n]*		/* comments */
[,:!@|()\[\]_]	{ return yytext[0]; }
(?i:def)		{ return TOKEN_KEYWORD_DEF; }
(?i:acc)		{ return TOKEN_KEYWORD_ACC; }
(?i:extern)		{ return TOKEN_KEYWORD_EXTERN; }
(?i:push)		{ yylval->v_int = VM_OP_PUSH|(yylloc->last_line<<16); return TOKEN_OP_ANY; }
(?i:pop)		{ yylval->v_int = VM_OP_POP|(yylloc->last_line<<16); return TOKEN_OP_INT; }
(?i:create)		{ yylval->v_int = VM_OP_CREATE|(yylloc->last_line<<16); return TOKEN_OP_CINT; }
(?i:call)		{ yylval->v_int = VM_OP_CALL|(yylloc->last_line<<16); return TOKEN_OP_FUN; }
(?i:rcall)		{ yylval->v_int = VM_OP_RCALL|(yylloc->last_line<<16); return TOKEN_OP_FUN; }
(?i:tcall)		{ yylval->v_int = VM_OP_TCALL|(yylloc->last_line<<16); return TOKEN_OP_FUN; }
(?i:ret)		{ yylval->v_int = VM_OP_RET|(yylloc->last_line<<16); return TOKEN_OP_CINT; }
(?i:jmp)		{ yylval->v_int = VM_OP_JMP|(yylloc->last_line<<16); return TOKEN_OP_LABEL; }
(?i:branch)		{ yylval->v_int = VM_OP_BRANCH|(yylloc->last_line<<16); return TOKEN_OP_BRANCH; }
(?i:inst)		{ yylval->v_int = VM_OP_INST|(yylloc->last_line<<16); return TOKEN_OP_INST; }
(?i:closure)		{ yylval->v_int = VM_OP_CLOSURE|(yylloc->last_line<<16); return TOKEN_OP_ANY; }
(?i:key)		{ yylval->v_int = VM_OP_KEY|(yylloc->last_line<<16); return TOKEN_OP_KEY; }
(?i:bind)		{ yylval->v_int = VM_OP_BIND|(yylloc->last_line<<16); return TOKEN_OP_PATTERN; }
(?i:match)		{ yylval->v_int = VM_OP_MATCH|(yylloc->last_line<<16); return TOKEN_OP_PATTERN; }
(?i:mov)		{ yylval->v_int = VM_OP_MOVE|(yylloc->last_line<<16); return TOKEN_OP_MOVE; }
(?i:yield)		{ yylval->v_int = VM_OP_YIELD|(yylloc->last_line<<16); return TOKEN_OP_NOARG; }
(?i:rpn-push)		{ yylval->v_int = VM_RPN_PUSH|(yylloc->last_line<<16); return TOKEN_OP_RPNPUSH; }
(?i:rpn-add)		{ yylval->v_int = VM_RPN_ADD|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-sub)		{ yylval->v_int = VM_RPN_SUB|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-mul)		{ yylval->v_int = VM_RPN_MUL|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-pow)		{ yylval->v_int = VM_RPN_POW|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-div)		{ yylval->v_int = VM_RPN_DIV|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-mod)		{ yylval->v_int = VM_RPN_MOD|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-eq)		{ yylval->v_int = VM_RPN_EQ|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-neq)		{ yylval->v_int = VM_RPN_NEQ|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-ge)		{ yylval->v_int = VM_RPN_GE|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-le)		{ yylval->v_int = VM_RPN_LE|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-geq)		{ yylval->v_int = VM_RPN_GEQ|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-leq)		{ yylval->v_int = VM_RPN_LEQ|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-or)		{ yylval->v_int = VM_RPN_OR|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-and)		{ yylval->v_int = VM_RPN_AND|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-xor)		{ yylval->v_int = VM_RPN_XOR|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
(?i:rpn-not)		{ yylval->v_int = VM_RPN_NOT|(yylloc->last_line<<16); return TOKEN_OP_RPN; }
s[.][0-9]+			{ yylval->v_int = atoi(yytext+2); return TOKEN_REF_STACK; }
c[.][0-9]+			{ yylval->v_int = atoi(yytext+2); return TOKEN_REF_CLOSURE; }
[-]?[0-9]+			{ yylval->v_int = atoi(yytext); return TOKEN_CONST_INTEGER; }
[-]?[0-9]+[.][0-9]+		{ yylval->v_float = atof(yytext); return TOKEN_CONST_FLOAT; }
['"][a-z][_0-9A-Za-z]*["']	{ yylval->v_int = asm_add_string(context,yytext+1,1); return TOKEN_CONST_ATOM; }
['"][A-Z][_0-9A-Za-z]*["'] 	{ yylval->v_int = asm_add_string(context,yytext+1,1); return TOKEN_CONST_VARNAME; }
[a-z][_0-9A-Za-z]*		{ yylval->v_int = asm_add_string(context,yytext,0); return TOKEN_CONST_ATOM; }
[A-Z][_0-9A-Za-z]* 		{ yylval->v_int = asm_add_string(context,yytext,0); return TOKEN_CONST_VARNAME; }
. 				{ return TOKEN_INVALID; }
%%

