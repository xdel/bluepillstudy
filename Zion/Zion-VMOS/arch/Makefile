OBJDIRS += arch

ARCH_SRCFILES := 	\
		arch/cpu.c 		\
		arch/port_op.asm 

# Only build files if they exist.
ARCH_SRCFILES := $(wildcard $(ARCH_SRCFILES))
ARCH_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(ARCH_SRCFILES))
ARCH_OBJFILES := $(patsubst %.asm, $(OBJDIR)/%.o, $(ARCH_OBJFILES))


# Build object files
$(OBJDIR)/arch/%.o: arch/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/arch/%.o: arch/%.asm
	@echo + as $<
	@mkdir -p $(@D)
	$(V)$(NASM) $(NASMFLAGS) -o $@ $<
