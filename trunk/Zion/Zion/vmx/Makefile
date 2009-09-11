#
# Makefile for Zion VMM.
# This is NOT a complete makefile;
# you must run GNU make in the top-level directory
# where the first Makefile is located.
#

OBJDIRS += vmx

VMX_SRCFILES :=		vmx/common.c \
				    vmx/vmx.c \
					vmx/hvm.c  \
					vmx/vmxtraps.c \
					vmx/trap.c  \
					vmx/memory.c \
					vmx/vmxapi.c \
					

VMX_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(VMX_SRCFILES))

$(OBJDIR)/vmx/%.o: vmx/%.c
	@echo + c++[VMX] $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<
