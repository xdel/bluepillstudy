#
# Makefile for Zion VMM.
# This is NOT a complete makefile;
# you must run GNU make in the top-level directory
# where the first Makefile is located.
#

OBJDIRS += vmx

VMX_SRCFILES :=		vmx/vmx.c \
			vmx/vmxapi.c \
			vmx/hvm.c \
			vmx/common.c

VMX_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(VMX_SRCFILES))

$(OBJDIR)/vmx/%.o: vmx/%.c
	@echo + c++[VMX] $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<
