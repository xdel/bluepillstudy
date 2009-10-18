-include env.mk

OBJDIR := obj
TOP = .

CC			:= gcc -pipe
CXX 		:= c++ -pipe
NASM 		:= nasm
OBJCOPY	:= objcopy
OBJDUMP	:= objdump

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS	:= $(CFLAGS) $(DEFS) $(LABDEFS) -O1 -fno-builtin -I$(TOP) -MD
CFLAGS	+= -Wall -Wno-format -Wno-unused -gstabs -m32 
#-Werror 
# Add -fno-stack-protector if the option exists.
CFLAGS	+= $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti

NASMFLAGS := -O0 -f elf

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

# include Makefile parameter head file
include 	parameter.def

### Make entries ###
all: bochs_image

# Create debug flags
bochs_debug: 
	echo "#define __BOCHS_DEBUG__" > inc/bochs.h

no_debug: 
	echo "#define __NO_BOCHS_DEBUG__" > inc/bochs.h

$(OBJDIR)/kernel: $(OBJS) kernel.ld
	@echo + ld $@
	$(V)$(LD) -o $@ $(LDFLAGS) $(OBJS) $(GCC_LIB) -b binary 
	$(V)$(OBJDUMP) -S $@ > $@.asm

$(OBJDIR)/Zion-VMOS: $(OBJDIR)/kernel
	@echo + oc $@
	$(V)$(OBJCOPY) --adjust-vma=$(KernelELF_VA_adjust) $^ $@

bochs_image: bochs_debug $(OBJDIR)/Zion-VMOS
	dd if=$(OBJDIR)/Zion-VMOS of=$(BochsImgName) bs=512 seek=$(StartSecOfKernalFile_bochs) count=$(SizeOfKernelFile) conv=notrunc

install: no_debug $(OBJDIR)/Zion-VMOS
	sudo cp $(OBJDIR)/Zion-VMOS /boot
	sudo dd if=$(OBJDIR)/Zion-VMOS of=$(HardDisk_Dev) bs=512 count=$(SizeOfKernelFile) seek=$(StartSecOfKernalFile)

clean:
	rm -rf $(OBJDIR) 

