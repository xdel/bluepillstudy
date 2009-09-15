-include env.mk

OBJDIR := obj
TOP = .

CC			:= gcc -pipe
CXX 		:= c++ -pipe
OBJCOPY	:= objcopy
OBJDUMP	:= objdump

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS	:= $(CFLAGS) $(DEFS) $(LABDEFS) -O1 -fno-builtin -I$(TOP) -MD
CFLAGS	+= -Wall -Wno-format -Wno-unused -Werror -gstabs -m32
# Add -fno-stack-protector if the option exists.
CFLAGS	+= $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti

# libgcc defines some necessary functions
GCC_LIB := $(shell $(CC) -print-libgcc-file-name)

LDFLAGS := -T kernel.ld -nostdlib


# Include Makefiles for subdirectories
include kern/Makefile
include arch/Makefile
include lib/Makefile
include vmx/Makefile

OBJS := $(KERN_OBJFILES) $(ARCH_OBJFILES) $(LIB_OBJFILES)
OBJS += $(VMX_OBJFILES)

### Make entries ###
all: $(OBJDIR)/Zion-VMOS

$(OBJDIR)/kernel: $(OBJS) kernel.ld
	@echo + ld $@
	$(V)$(LD) -o $@ $(LDFLAGS) $(OBJS) $(GCC_LIB) -b binary 
	$(V)$(OBJDUMP) -S $@ > $@.asm

$(OBJDIR)/Zion-VMOS: $(OBJDIR)/kernel
	@echo + oc $@
	$(V)$(OBJCOPY) --adjust-vma=0x10000000 $^ $@

install: $(OBJDIR)/Zion-VMOS
	sudo cp $< /boot
	sudo cp $< ~/VM_project/Seraph
	sudo dd if=$< of=/dev/sda bs=512 count=512 seek=71682500 

clean:
	rm -rf $(OBJDIR) 

