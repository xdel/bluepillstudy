OBJDIRS += arch

VMX_SRCFILES := \
		vmx/common.c 		\
		vmx/vmx.c 				\
		vmx/hvm.c  				\
		vmx/vmxtraps.c 		\
		vmx/trap.c  				\
		vmx/memory.c 		\
		vmx/vmxapi.c

# Only build files if they exist.
VMX_SRCFILES := $(wildcard $(VMX_SRCFILES))
VMX_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(VMX_SRCFILES))


# Build object files
$(OBJDIR)/vmx/%.o: vmx/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(CXXFLAGS) -c -o $@ $<
