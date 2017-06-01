COBJS=vm_bind.o vm_exec.o vm_lib.o vm_gc.o vm_base.o vm_print.o

all: $(COBJS) vm.a
vm.a: $(COBJS)
	$(AR) rs $@ $^
clean:
	$(RM) -f *.o *.d *.a
%.o: %.c Makefile
	$(CC) -c $(OPT) $(CFLAGS) -MMD -MF $(patsubst %.o,%.d,$@) -o $@ $<
	
DEPS=$(COBJS:.o=.d)
-include $(DEPS)