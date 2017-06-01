/*!
The MIT License (MIT)
Copyright (c) 2015 Marek Korzeniowski
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
--------------------------------------------------------------
           type generic AVL tree pure C templete
				    (avl_templete.h)
--------------------------------------------------------------

declares following datatypes:
	prefix_tree_t
		struct {size}
	prefix_node_t
		struct {value, key}
	prefix_t
		*prefix_node_t

declares following function:
	void prefix_create(prefix_tree_t* tree, int init_size)
		create tree with inital size. can be 0
	void prefix_clear(prefix_tree_t* tree)
		remove all data from tree
	void prefix_destroy(prefix_tree_t* tree)
		destroy tree
	void prefix_compact(prefix_tree_t* tree)
		move to possibly smallest memory block
	void prefix_copy(prefix_tree_t* src, prefix_tree_t* dst)
		copy tree. does NOT preform destenation cleanup before copy
	void prefix_removenode(prefix_tree_t* tree, prefix_t node)
		remove node from tree
	int prefix_remove(prefix_tree_t* tree, avl_key_t key)
		remove node from tree by key, return 1 on success
	prefix_t prefix_insert(prefix_tree_t* tree, avl_key_t key, avl_value_t value)
		insert node to tree. undefined behavior if key allready exists
	prefix_t prefix_get(prefix_tree_t* tree, avl_key_t key)
		find node by key, return NULL if node not in tree
	prefix_t prefix_first(prefix_tree_t* tree)
		return node with smallest key
	prefix_t prefix_next(prefix_t n);
		return node with key next in order
	prefix_t prefix_set(avltree_t* tree, avl_key_t key, avl_value_t value)
		bind value with key, insert if not exists, return node
	prefix_t prefix_addget(avltree_t* tree, avl_key_t key, avl_value_t value)
		find node with key, if not found insert with value, return node

usage:
	#define [option 1]
	#define [option 2]
	       ...
	#define [option N]
	#include "avl_templete.h"

where [option 1..N] can be 
	AVL_TEMPLATE_PREFIX
		prefix added to every function and datatype
		no defalut value, has to be defined before include
	AVL_KEY_TYPE
		key type, defaults to int
	AVL_VALUE_TYPE
		value type, defaults to int
	AVL_KEY_LE(a,b)
		lesser realation macro, defaults to (a<b)
	AVL_KEY_NEQ
		inequal relation macro, defaults to (a!=b)
	AVL_SIZE_TRESHOLD_HIGH
		tree is reallocated if the current tree size multiplied
		by this value is larger then the current memory space
		defaults to 1.00
	AVL_SIZE_TRESHOLD_LOW
		tree is reallocated if the current tree size multiplied
		by this value is smaller then the current memory space
		defaults to 1.75
	AVL_SIZE_SIZE_HIGH
		if tree is resized up, the new size is equal to
		current size multiplied by this value
		defaults to 1.50
	AVL_SIZE_SIZE_LOW
		if tree is resized down, the new size is equal to
		current size multiplied by this value
		defaults to 1.25
	AVL_COPY_STACK_SIZE
		internal stack size. Should be not less then
		log2(2*(n+2)) where n is the AVL_MAXimum tree size
		defaults to 64
	AVL_RESIZE_NONE
		disables tree resizeing
		by default not defined
	AVL_RESIZE_UP
		tree only grows, is never resized down
		by default not defined
	AVL_INCLUDE_BODY
		include c body after the header
		by default not defined
	AVL_ALL_STATIC
		declare all functions static
		implies AVL_INCLUDE_BODY
		by default not defined
		
all options are automatically undefined after the #include derective.
can be included multiply times in the same compilation block.

example:
//declare <char, int> tree
#define AVL_TEMPLATE_PREFIX		charavl
#define AVL_KEY_TYPE 			char*
#define AVL_KEY_NEQ(a,b)		(strcmp(a,b)!=0)
#define AVL_KEY_LE(a,b)			(strcmp(a,b)<0)
#define AVL_ALL_STATIC
#include "avl_templete.h"
//declare <int, int> tree
#define AVL_TEMPLATE_PREFIX		intavl
#define AVL_ALL_STATIC
#include "avl_templete.h"
int main(){
	intavl_tree_t avl1;
	charavl_tree_t avl2;
	intavl_create(&avl1,0);
	charavl_create(&avl2,0);
	intavl_set(&avl1,0,1);
	charavl_set(&avl2,"key",1);
}

*/

#include <inttypes.h>
#include <stdlib.h>

#ifndef AVL_SIZE_TRESHOLD_HIGH
#define AVL_SIZE_TRESHOLD_HIGH	   1
#endif

#ifndef AVL_SIZE_TRESHOLD_LOW
#define AVL_SIZE_TRESHOLD_LOW	1.75
#endif

#ifndef AVL_RESIZE_HIGH
#define AVL_RESIZE_HIGH		1.50
#endif

#ifndef AVL_RESIZE_LOW
#define AVL_RESIZE_LOW		1.25
#endif

#ifndef AVL_COPY_STACK_SIZE
#define AVL_COPY_STACK_SIZE		  64
#endif

#ifndef AVL_RESIZE_NONE
#ifndef AVL_RESIZE_UP
#ifndef AVL_RESIZE
#define AVL_RESIZE
#endif
#endif
#endif

#ifdef AVL_RESIZE
#ifndef AVL_RESIZE_UP
#define AVL_RESIZE_UP
#endif
#endif

#ifndef AVL_KEY_TYPE
#define AVL_KEY_TYPE 			int
#endif

#ifndef AVL_VALUE_TYPE
#define AVL_VALUE_TYPE 			int
#endif

#ifndef AVL_KEY_LE
#define AVL_KEY_LE(a,b)			(a<b)
#endif

#ifndef AVL_KEY_NEQ
#define AVL_KEY_NEQ(a,b)		(a!=b)
#endif

#define AVL_PRXN_2(x,suffix) x ## suffix
#define AVL_PRXN_1(x,suffix) AVL_PRXN_2(x,suffix)
#define AVL_PRXN(x) AVL_PRXN_1(AVL_TEMPLATE_PREFIX,x)

#ifdef AVL_ALL_STATIC
#define AVL_ATTR static __attribute__((flatten))
#ifndef AVL_INCLUDE_BODY
#define AVL_INCLUDE_BODY
#endif
#else
#define AVL_ATTR __attribute__((flatten))
#endif


#define avlnode_t AVL_PRXN(_node_t)
#define avl_t AVL_PRXN(_t)
#define avltree_t AVL_PRXN(_tree_t)
#define avl_key_t AVL_KEY_TYPE
#define avl_value_t AVL_VALUE_TYPE

#ifdef AVL_NO_HEADER
#ifndef AVL_INCLUDE_BODY
#define AVL_INCLUDE_BODY
#endif
#else

typedef struct avlnode_t avlnode_t;
typedef avlnode_t* avl_t;
typedef struct avltree_t avltree_t;

struct avlnode_t{
	avl_t p;
	avl_t l;
	avl_t r;
	int height;
	avl_key_t key;
	avl_value_t value;
};

struct avltree_t{
	avl_t root;
	avl_t data;
	size_t size;
	size_t allocated_memory;
	size_t init_size;
};

AVL_ATTR void AVL_PRXN(_create)(avltree_t* tree, size_t init_size);
AVL_ATTR void AVL_PRXN(_clear)(avltree_t* tree);
AVL_ATTR void AVL_PRXN(_destroy)(avltree_t* tree);
AVL_ATTR void AVL_PRXN(_compact)(avltree_t* tree);
AVL_ATTR void AVL_PRXN(_copy)(avltree_t* src, avltree_t* dst);
AVL_ATTR void AVL_PRXN(_removenode)(avltree_t* tree, avl_t node);
AVL_ATTR avl_t AVL_PRXN(_insert)(avltree_t* tree, avl_key_t key, avl_value_t value);
AVL_ATTR avl_t AVL_PRXN(_get)(avltree_t* tree, avl_key_t key);
AVL_ATTR avl_t AVL_PRXN(_next)(avl_t n);
AVL_ATTR avl_t AVL_PRXN(_first)(avltree_t* tree);

static inline int AVL_PRXN(_remove)(avltree_t* tree, avl_key_t key){
	avl_t node = AVL_PRXN(_get)(tree,key);
	if(node == NULL)
		return 0;
	AVL_PRXN(_removenode)(tree,node);
	return 1;
}

static inline avl_t AVL_PRXN(_set)(avltree_t* tree, avl_key_t key, avl_value_t value){
	avl_t node = AVL_PRXN(_get)(tree,key);
	if(node == NULL)
		node = AVL_PRXN(_insert)(tree,key,value);
	else
		node->value = value;
	return node;
}

static inline avl_t AVL_PRXN(_addget)(avltree_t* tree, avl_key_t key, avl_value_t value){
	avl_t node = AVL_PRXN(_get)(tree,key);
	if(node == NULL)
		node = AVL_PRXN(_insert)(tree,key,value);
	return node;
}
#endif

#ifdef AVL_INCLUDE_BODY

#define AVL_MAX(X,Y) ((X) < (Y) ? (Y) : (X))
const void* AVL_PRXN(_nullnode)[4] = {0,0,0,0};
#define AVL_NULLNODE ((avl_t)AVL_PRXN(_nullnode))

static inline avl_t AVL_PRXN(__rot_left)(avl_t n){
	avl_t t = n->r;
	if(n->p != AVL_NULLNODE){
		if(n->p->l==n)
			n->p->l = t;
		else	
			n->p->r = t;
	}
	t->p = n->p;
	n->p = t;
	n->r = t->l;
	if(n->r != AVL_NULLNODE)
		n->r->p = n;
	t->l = n;
	n->height = AVL_MAX(n->l->height,n->r->height)+1;
	t->height = AVL_MAX(t->l->height,t->r->height)+1;
	return t;	
}

static inline avl_t AVL_PRXN(__rot_right)(avl_t n){
	avl_t t = n->l;
	if(n->p != AVL_NULLNODE){
		if(n->p->l==n)
			n->p->l = t;
		else	
			n->p->r = t;
	}
	t->p = n->p;
	n->p = t;
	n->l = t->r;
	if(n->l != AVL_NULLNODE)
		n->l->p = n;
	t->r = n;
	n->height = AVL_MAX(n->l->height,n->r->height)+1;
	t->height = AVL_MAX(t->l->height,t->r->height)+1;
	return t;	
}

static inline avl_t AVL_PRXN(__balance)(avl_t n){
	while(1){
		int b = n->l->height - n->r->height;
		if(b >= 2){
			avl_t t = n->l;
			if(t->l->height < t->r->height)
				AVL_PRXN(__rot_left)(t);
			n = AVL_PRXN(__rot_right)(n);
		}
		else if(b <=- 2){
			avl_t t = n->r;
			if(t->r->height < t->l->height)
				AVL_PRXN(__rot_right)(t);
			n = AVL_PRXN(__rot_left)(n);
		}
		else
			n->height = AVL_MAX(n->l==NULL?0:n->l->height,n->r==NULL?0:n->r->height)+1;
		if(n->p == AVL_NULLNODE)
			return n;
		n = n->p;
	}
}

static inline avl_t AVL_PRXN(__treecopy)(avl_t root, size_t new_size){
	if(new_size == 0)
		return AVL_NULLNODE;
	avl_t mem = (avl_t)malloc(sizeof(avlnode_t)*new_size);
	if(mem == NULL)
		return NULL;
	if(root == AVL_NULLNODE)
		return mem;
	avl_t stack[AVL_COPY_STACK_SIZE];
	int sp = 1;
	size_t mem_p = 1;
	mem[0] = root[0];
	stack[0] = mem;
	while(sp > 0){
		avl_t node = stack[--sp];
		if(node->l != AVL_NULLNODE){
			mem[mem_p] = node->l[0];
			mem[mem_p].p = node;
			node->l = mem+mem_p;
			stack[sp++] = mem+mem_p;
			mem_p++;
		}
		if(node->r != AVL_NULLNODE){
			mem[mem_p] = node->r[0];
			mem[mem_p].p = node;
			node->r = mem+mem_p;
			stack[sp++] = mem+mem_p;
			mem_p++;
		}
	}
	return mem;
}

static inline void AVL_PRXN(__resize)(avltree_t* tree, size_t new_size){
	if(new_size < tree->init_size)
        new_size = tree->init_size;
	if(new_size < tree->size)
		new_size = tree->size;
	if(new_size == tree->allocated_memory)
		return;
	if(new_size == 0){
		if(tree->data){
			free(tree->data);
			tree->data = NULL;
		}
		tree->root = AVL_NULLNODE;
		tree->allocated_memory = 0;
		return;
	}
	avl_t copy = AVL_PRXN(__treecopy)(tree->root,new_size);
	if(tree->data)
		free(tree->data);
	tree->data = copy;
	if(tree->root != AVL_NULLNODE)
		tree->root = copy;
	tree->allocated_memory = new_size;
}

static inline void AVL_PRXN(__tryresize)(avltree_t* tree){
	size_t new_size = tree->allocated_memory;
#ifdef AVL_RESIZE_UP
	if(tree->allocated_memory <= tree->size*AVL_SIZE_TRESHOLD_HIGH){
		new_size = (tree->size+1) * AVL_RESIZE_HIGH;
	}
#ifdef AVL_RESIZE
	else if(tree->allocated_memory > tree->size*AVL_SIZE_TRESHOLD_LOW){
		new_size = tree->size * AVL_RESIZE_LOW;
	}
#endif
#endif
	AVL_PRXN(__resize)(tree,new_size);
}

static inline avl_t AVL_PRXN(__makenode)(avltree_t* tree, avl_key_t key, avl_value_t value, avl_t p){
	avl_t node = tree->data + tree->size++;
	node->key = key;
	node->value = value;
	node->height = 1;
	node->l = AVL_NULLNODE;
	node->r = AVL_NULLNODE;
	node->p = p;
	return node;
}

static inline void AVL_PRXN(__unconnect)(avltree_t* tree, avl_t node){
	avl_t old = tree->data + --tree->size;
	if(old != node){
		avl_t p = old->p;
		if(p == AVL_NULLNODE){
			tree->root = node;
		}else{
			if(p->l == old)
				p->l = node;
			else
				p->r = node;
		}
		if(old->l != AVL_NULLNODE)
			old->l->p = node;
		if(old->r != AVL_NULLNODE)
			old->r->p = node;
		node[0] = old[0];
	}
}

AVL_ATTR avl_t AVL_PRXN(_insert)(avltree_t* tree, avl_key_t key, avl_value_t value){
#ifdef AVL_RESIZE_UP
	AVL_PRXN(__tryresize)(tree);
#endif
	if(tree->root == AVL_NULLNODE){
		tree->root = AVL_PRXN(__makenode)(tree,key,value,AVL_NULLNODE);
		return tree->root;
	}
	avl_t n = tree->root;
	avl_t new_node;
	while(1){
		if(AVL_KEY_LE(n->key,key)){
			if(n->r == AVL_NULLNODE){
				new_node = AVL_PRXN(__makenode)(tree,key,value,n);
				n->r = new_node;
				break;
			}
			n = n->r;
		}else{
			if(n->l == AVL_NULLNODE){
				new_node = AVL_PRXN(__makenode)(tree,key,value,n);
				n->l = new_node;
				break;
			}
			n = n->l;
		}
	}
	tree->root = AVL_PRXN(__balance)(n);
	return new_node;
}

AVL_ATTR avl_t AVL_PRXN(_get)(avltree_t* tree, avl_key_t key){
	avl_t n = tree->root;
	while(n != AVL_NULLNODE && AVL_KEY_NEQ(n->key, key))
		n = (AVL_KEY_LE(n->key,key)?n->r:n->l);
	if(n == AVL_NULLNODE)
		return NULL;
	return n;
}

AVL_ATTR avl_t AVL_PRXN(_first)(avltree_t* tree){
	avl_t n = tree->root;
	if(n == AVL_NULLNODE)
		return NULL;
	while(n->l != AVL_NULLNODE)
		n = n->l;
	return n;
}

AVL_ATTR avl_t AVL_PRXN(_next)(avl_t n){
	if(n->r != AVL_NULLNODE){
		n = n->r;
		while(n->l != AVL_NULLNODE)
			n = n->l;
		return n;
	}
	avl_key_t key = n->key;
	while(!AVL_KEY_LE(key,n->key)){
		n = n->p;
		if(n==AVL_NULLNODE)
			return NULL;
	}
	return n;		
}

AVL_ATTR void AVL_PRXN(_removenode)(avltree_t* tree, avl_t n){
	if(n->l!=AVL_NULLNODE && n->r!=AVL_NULLNODE){
		avl_t t = n->r;
		while(t->l != AVL_NULLNODE)
			t = t->l;
		n->key = t->key;
		n = t;
	}
	avl_t p = n->p;
	avl_t c = n->r==AVL_NULLNODE?n->l:n->r;
	if(c != AVL_NULLNODE)
		c->p = p;
	if(p == AVL_NULLNODE){
		tree->root = c;
	}else{
		if(p->l==n)
			p->l = c;
		else
			p->r = c;
		tree->root = AVL_PRXN(__balance)(p);
	}
	AVL_PRXN(__unconnect)(tree,n);
#ifdef AVL_RESIZE
	AVL_PRXN(__tryresize)(tree);
#endif
	return;
}

AVL_ATTR void AVL_PRXN(_create)(avltree_t* tree, size_t init_size){
	tree->size = 0;
	tree->init_size = init_size;
	tree->allocated_memory = init_size;
	tree->data = (avl_t)malloc(init_size*sizeof(avlnode_t));
	tree->root = AVL_NULLNODE;
}

AVL_ATTR void AVL_PRXN(_destroy)(avltree_t* tree){
	free(tree->data);
	tree->data = NULL;
	tree->size = 0;
	tree->allocated_memory = 0;
	tree->root = AVL_NULLNODE;
}

AVL_ATTR void AVL_PRXN(_clear)(avltree_t* tree){
	tree->size = 0;
	tree->root = AVL_NULLNODE;
	AVL_PRXN(__resize)(tree,0);
}

AVL_ATTR void AVL_PRXN(_compact)(avltree_t* tree){
	AVL_PRXN(__resize)(tree,0);
}

AVL_ATTR void AVL_PRXN(_copy)(avltree_t* src, avltree_t* dst){
	dst[0] = src[0];
	dst->data = AVL_PRXN(__treecopy)(src->root,src->size*AVL_RESIZE_LOW);
	dst->root = dst->data;
}
#endif

#undef AVL_PRXN_2
#undef AVL_PRXN_1
#undef AVL_PRXN
#undef avlnode_t
#undef avltree_t
#undef avl_t
#undef avl_key_t
#undef avl_value_t
#undef AVL_KEY_NEQ
#undef AVL_KEY_LE
#undef AVL_VALUE_TYPE
#undef AVL_KEY_TYPE
#undef AVL_TEMPLATE_PREFIX
#undef AVL_COPY_STACK_SIZE
#undef AVL_RESIZE_LOW
#undef AVL_RESIZE_HIGH
#undef AVL_SIZE_TRESHOLD_LOW
#undef AVL_SIZE_TRESHOLD_HIGH
#undef AVL_ATTR

#ifdef AVL_ALL_STATIC
#undef AVL_ALL_STATIC
#undef AVL_INCLUDE_BODY
#undef AVL_MAX
#undef AVL_NULLNODE
#else
#ifdef AVL_INCLUDE_BODY
#undef AVL_MAX
#undef AVL_NULLNODE
#endif
#endif

#ifdef AVL_RESIZE
#undef AVL_RESIZE
#endif

#ifdef AVL_RESIZE_UP
#undef AVL_RESIZE_UP
#endif

#ifdef AVL_RESIZE_NONE
#undef AVL_RESIZE_NONE
#endif