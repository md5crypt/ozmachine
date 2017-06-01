#include "vm_util.h"
#include "vm_base.h"
#include "vm_exec.h"
#include "vm_gc.h"
#include "vm_print.h"

VM_FLAT void vm_root_create(vm_root_t* root){
	root->idle_cnt = root->process_cnt;
	void* block = malloc(sizeof(vm_context_t)*root->process_cnt + sizeof(pthread_mutex_t)*root->mutex_cnt);
	pthread_mutex_t* mutex_array = block+sizeof(vm_context_t)*root->process_cnt;
	root->process = (vm_context_t*)block;
	root->queue.head = NULL;
	root->queue.tail = NULL;
	root->supervisor_cmd = VM_SUPERVISOR_NOP;
	root->halt = 0;
	for(uint32_t i=0; i<root->process_cnt; i++){
		root->process[i].rseed = rand();
		root->process[i].mutex_mask = root->mutex_cnt-1;
		root->process[i].mutex = mutex_array;
		root->process[i].root = root;
	}
	for(uint32_t i=0; i<root->mutex_cnt; i++)
		pthread_mutex_init(mutex_array+i,NULL);
	pthread_mutex_init(&root->synch.queue_mutex,NULL);
	pthread_mutex_init(&root->synch.alloc_mutex,NULL);
	pthread_cond_init(&root->synch.queue_cond,NULL);
	pthread_cond_init(&root->synch.supervisor_cond,NULL);
	root->dmem.block = NULL;
	root->dmem.offset = 0;
	vm_dynamicmem_create(root,root->dmem.min_page);
}

VM_FLAT void vm_root_load(vm_root_t* root, vm_staticmem_t* smem){
	for(uint32_t i=0; i<root->process_cnt; i++)
		root->process[i].smem = smem[0];
}

VM_FLAT void vm_staticmem_destroy(vm_staticmem_t* smem){
	free(smem->prototypes);
}

VM_FLAT void vm_root_destroy(vm_root_t* root){
	free(root->dmem.block);
	free(root->process);
}

static VM_FLAT void* vm_fork(void* arg){
	vm_context_t* context = (vm_context_t*)arg;
	int e = setjmp(context->interrupt);
	if(e != 0){
		switch(e){
			case VM_EXCEPTION_THREAD_STOP:
				break;
			case VM_EXCEPTION_THREAD_EXIT:;
				break;
			case VM_EXCEPTION_THREAD_OOM:
				vm_thread_grow(context);
				vm_exec(context);
			case VM_EXCEPTION_OOM:
				context->thread->value.v_thread.env = context->env;
				vm_queue_lock(context);
				vm_thread_queue(context,context->thread);
				context->root->supervisor_cmd = VM_SUPERVISOR_GC;
				context->root->halt = 1;
			case VM_EXCEPTION_HALT:
				//!!queue_mutex is locked!!//
				context->root->idle_cnt--;
				if(context->root->idle_cnt == 0)
					vm_supervisor_signal(context);
				while(context->root->halt){
					switch(context->root->supervisor_cmd){
						case VM_SUPERVISOR_TERMINATE:
							vm_queue_unlock(context);
							return NULL;
						default:
							vm_queue_wait(context);
					}
				}
				context->root->idle_cnt++;
				vm_queue_unlock(context);
				break;
			default:
				vm_queue_lock(context);
				vm_print_exception(context,e);
				vm_terminate(context);
		}
	}
	vm_variable_t* thread = vm_thread_get(context);
	context->thread = thread;
	context->env = thread->value.v_thread.env;
	context->stack = thread->value.v_thread.stack;
	context->closure = context->env.function->value.v_function.closure;
	vm_exec(context);
	return NULL;
}

VM_FLAT void vm_stop(vm_root_t* root){
	pthread_mutex_lock(&root->synch.queue_mutex);
	root->supervisor_cmd = VM_SUPERVISOR_TERMINATE;
	root->halt = 1;
	pthread_cond_broadcast(&root->synch.queue_cond);
	pthread_mutex_unlock(&root->synch.queue_mutex);
}

VM_FLAT void vm_run(vm_root_t* root){
	vm_tcall(root->process,root->process[0].smem.constants[VM_VECTOR_MAIN]);
	for(uint32_t i=0; i<root->process_cnt; i++)
		pthread_create(&root->process[i].self,NULL,vm_fork,root->process+i);
	pthread_mutex_lock(&root->synch.queue_mutex);
	while(1){
		while(root->idle_cnt > 0)
			pthread_cond_wait(&root->synch.supervisor_cond,&root->synch.queue_mutex);
		switch(root->supervisor_cmd){
			case VM_SUPERVISOR_NOP:
				VM_FATAL("bad cmd");				
			case VM_SUPERVISOR_TERMINATE:
				//puts("shutdown");
				pthread_mutex_unlock(&root->synch.queue_mutex);
				for(uint32_t i=0; i<root->process_cnt; i++){
					void* ret;
					pthread_join(root->process[i].self,&ret);
				}
				return;
			case VM_SUPERVISOR_GC:{
				size_t pre_gc = root->dmem.offset;
				size_t old_size = root->dmem.size;
				vm_gc(root);
				if(root->dmem.offset == pre_gc && root->dmem.data == root->dmem.block && root->dmem.size == old_size){
					VM_FATAL("out of memory");		
					root->supervisor_cmd = VM_SUPERVISOR_TERMINATE;
					pthread_cond_broadcast(&root->synch.queue_cond);
					continue;
				}
				pthread_cond_broadcast(&root->synch.queue_cond);
				break;
			}
		}
		root->halt = 0;
		root->supervisor_cmd = VM_SUPERVISOR_NOP;
		pthread_cond_wait(&root->synch.supervisor_cond,&root->synch.queue_mutex);
	}
}