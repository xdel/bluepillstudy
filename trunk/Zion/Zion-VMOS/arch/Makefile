OBJDIRS += arch

# entry.S must be first, so that it's the first code in the text segment!!!
ARCH_SRCFILES := arch/cpu.c

# Only build files if they exist.
ARCH_SRCFILES := $(wildcard $(ARCH_SRCFILES))
ARCH_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(ARCH_SRCFILES))


# Build kernel object files
$(OBJDIR)/arch/%.o: arch/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(CXXFLAGS) -c -o $@ $<
