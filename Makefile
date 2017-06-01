APPNAME=ozvm
CFLAGS=-Wall -Wextra -std=gnu99 -Wfatal-errors -Wno-unused-function -Wno-unused-parameter -Wno-attributes
OPT=-O2
COBJS=main.o
CC=gcc

export CFLAGS
export CC
export OPT
	
all: asm/all vm/all $(APPNAME)
$(APPNAME): main.o asm/asm.a vm/vm.a
	$(CC) $(OPT) $(CFLAGS) -pthread -lpthread $^ -lm -o $@
debug: OPT= -pg -DDEBUG -ggdb
	export OPT
debug: all
debug-run: debug
	$(GDB) ./$(APPNAME)
x32:
	OPT+= -m32
x64:
	OPT+= -m64
run: all
	./$(APPNAME)
clean:
	$(RM) -f *.o *.d $(APPNAME)
	$(MAKE) -C asm clean
	$(MAKE) -C vm clean
asm/%:
	$(MAKE) -C asm $(@F)
vm/%:
	$(MAKE) -C vm $(@F)
%.o: %.c Makefile
	$(CC) $(OPT) -c $(CFLAGS) -MMD -MF $(patsubst %.o,%.d,$@) -o $@ $<
	
DEPS=$(COBJS:.o=.d)
-include $(DEPS)
