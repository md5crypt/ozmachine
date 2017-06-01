COBJS=asm_convert.o asm_struct.o avl.o heap.o parser.o lexer.o asm_base.o

all: lexer.c parser.c $(COBJS) asm.a
asm.a: $(COBJS)
	$(AR) rs $@ $^
clean:
	$(RM) -f *.o *.d *.a *.output lexer.c parser.c
lexer.c: lexer.l Makefile
	flex --header-file=lexer.h --outfile=lexer.c lexer.l
parser.c: parser.y Makefile
	bison --output-file=parser.c --defines=parser.h --warnings=all --feature=all parser.y --report=itemset	
%.o: %.c Makefile
	$(CC) -c $(OPT) $(CFLAGS) -MMD -MF $(patsubst %.o,%.d,$@) -o $@ $<
	
DEPS=$(COBJS:.o=.d)
-include $(DEPS)
