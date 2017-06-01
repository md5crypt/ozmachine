#include "asm_base.h"
#include "asm_struct.h"
#include "asm_convert.h"
#include "parser.h"
#include "lexer.h"

int asm_compile_file(vm_staticmem_t* smem, const vm_lib_t* lib, const char* file){
	FILE* fp = fopen(file,"rb");
	if(fp == NULL){
		printf("open file '%s' failed\n",file);
		return 1;
	}
	asm_context_t asm_context;
	asm_context_create(&asm_context);
	yyscan_t scanner;
	yylex_init(&scanner);
	YY_BUFFER_STATE buf = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
	yy_switch_to_buffer(buf,scanner);
	int result = yyparse(scanner,&asm_context);
	yy_delete_buffer(buf, scanner);
	yylex_destroy(scanner);
	fclose(fp);
	if(result!=0){
		asm_context_destroy(&asm_context);
		return 1;
	}
	uint8_t* block = asm_convert(&asm_context,smem,(vm_lib_t*)lib);
	asm_context_destroy(&asm_context);
	return (block == NULL);
}