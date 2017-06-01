#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include "asm/asm_base.h"
#include "vm/vm_base.h"
#include "vm/vm_lib.h"
#include <getopt.h>

static const char* help_str = \
"usage: [options] file\n\
-n --min-page\n\
-x --max-page\n\
-p --process-count\n\
-m --mutex-count";

static const vm_lib_t lib[] = {
	{"Write",1,vm_lib_Write},
	{"Write2",2,vm_lib_Write2},
	{"Exit",0,vm_lib_Exit},
	{"Readfloat",0,vm_lib_Readfloat},
	{"Readint",0,vm_lib_Readint},
	{NULL,0,NULL}
};
static const struct option long_options[] = {
	{"min-page",required_argument,0,'n'},
	{"max-page",required_argument,0,'x'},
	{"process-count",required_argument,0,'p'},
	{"mutex-count",required_argument,0,'m'},
	{"help",no_argument,0,'h'},
	{0, 0, 0, 0}
};

int main (int argc, char **argv){
	vm_root_t root;
	root.process_cnt = 4;
	root.mutex_cnt = 128;
	root.dmem.min_page = 64;
	root.dmem.max_page = 8*1024;
	
	while (1){   
		int option_index = 0;
		int c = getopt_long (argc,argv,"n:x:p:m:h",long_options,&option_index);
		if (c == -1)
			break;
		switch(c){
			case 'n':
				root.dmem.min_page = atoi(optarg);
				break;
			case 'x':
				root.dmem.max_page = atoi(optarg);
				break;
			case 'p':
				root.process_cnt = atoi(optarg);
				break;
			case 'm':
				root.mutex_cnt = atoi(optarg);
				break;
			case 'h':
				puts(help_str);
				return 0;
			default:
				return 0;
		}
	}
	if(optind != argc-1){
		puts(help_str);
		return 0;
	}
	vm_staticmem_t smem;
	if(asm_compile_file(&smem,lib,argv[optind]))
		exit(1);
	vm_root_create(&root);
	vm_root_load(&root,&smem);
	vm_run(&root);
	vm_root_destroy(&root);
	vm_staticmem_destroy(&smem);
	return 0;
}
