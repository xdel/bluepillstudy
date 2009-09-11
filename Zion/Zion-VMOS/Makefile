OBJDIR := obj

-include conf/env.mk

TOP = .

CC	:= gcc -pipe
AS	:= as
AR	:= ar
LD	:= ld
OBJCOPY	:= objcopy
OBJDUMP	:= objdump
NM	:= nm

# Native commands
NCC		:= gcc -pipe
TAR		:= gtar
PERL	:= perl

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS	:= $(CFLAGS) $(DEFS) $(LABDEFS) -O1 -fno-builtin -I$(TOP) -MD
CFLAGS	+= -Wall -Wno-format -Wno-unused -Werror -gstabs -m32
# Add -fno-stack-protector if the option exists.
CFLAGS	+= $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# libgcc defines some necessary functions
GCC_LIB := $(shell $(CC) -print-libgcc-file-name)

# Eliminate default suffix rules
.SUFFIXES:

# Delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:

# make it so that no intermediate .o files are ever deleted
.PRECIOUS: %.o $(OBJDIR)/kern/%.o $(OBJDIR)/lib/%.o $(OBJDIR)/arch/%.o

KERN_CFLAGS := $(CFLAGS) -DZION_KERNEL -gstabs
USER_CFLAGS := $(CFLAGS) -DZION_USER -gstabs

CXX := c++ -pipe
KERN_CXXFLAGS := $(KERN_CFLAGS) -fno-exceptions -fno-rtti
USER_CXXFLAGS := $(USER_CFLAGS) -fno-exceptions -fno-rtti


# Include Makefiles for subdirectories
include kern/Makefile


### Make entries ###
all: $(OBJDIR)/Zion-VMOS

$(OBJDIR)/Zion-VMOS: $(OBJDIR)/kernel
	@echo + oc $@
	$(V)$(OBJCOPY) --adjust-vma=0x10000000 $^ $@

install: $(OBJDIR)/Zion-VMOS
	sudo cp $< /boot
	sudo cp $< ~/VM_project/Seraph
	sudo dd if=$< of=/dev/sda bs=512 count=512 seek=71682500 

clean:
	rm -rf $(OBJDIR) 

