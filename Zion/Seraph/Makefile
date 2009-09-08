OBJDIR 	= obj
DASMDIR = DASM

AS		= nasm
DAS 	= ndisasm
CC 		= gcc
LD 		= ld

ASFLAGS		= -O0 
CFLAGS 		= -c
LDFLAGS 		= -s
DASFLAG 	= 

# include Makefile parameter head file
include 	parameter.def

# All Phony Targets
.PHONY : all clean image $(OBJDIR)/boot $(OBJDIR)/boot2 $(OBJDIR)/boot_Zion

all: image 

bochs_debug: 
	echo "%define __BOCHS_DEBUG__" > include/bochs.inc
	
no_debug: 
	echo "%define __NO_BOCHS_DEBUG__" > include/bochs.inc

# Write binary into bochs disk image
image: bochs_debug $(OBJDIR)/boot $(OBJDIR)/boot2 $(OBJDIR)/boot_Zion 
	dd if=$(OBJDIR)/boot of=$(BochsImgName) bs=512 count=1 conv=notrunc
	dd if=$(OBJDIR)/boot2 of=$(BochsImgName) bs=512 seek=$(StartSecOfBoot2) count=1 conv=notrunc
	dd if=$(OBJDIR)/boot_Zion of=$(BochsImgName) bs=512 seek=$(StartSecOfBoot_Zion) count=$(SizeOfBoot_Zion) conv=notrunc
	dd if=$(KernelName) of=$(BochsImgName) bs=512 seek=$(StartSecOfKernalFile_bochs) count=$(SizeOfKernelFile) conv=notrunc
	dd if=$(OBJDIR)/boot of=$(BochsImgName) bs=512 seek=$(StartSecOfMBR_bak) count=1 conv=notrunc

$(OBJDIR)/boot: boot/boot.asm 
	@mkdir -p $(@D)
	@mkdir -p $(DASMDIR)
	$(AS) $(ASFLAGS) -o $@ $<
	$(DAS) $@ > $(DASMDIR)/boot.asm

$(OBJDIR)/boot2: boot/boot2.asm 
	@mkdir -p $(@D)
	@mkdir -p $(DASMDIR)
	$(AS) $(ASFLAGS) -o $@ $<
	$(DAS) $@ > $(DASMDIR)/boot2.asm

$(OBJDIR)/boot_Zion:boot/boot_Zion.asm
	@mkdir -p $(@D)
	@mkdir -p $(DASMDIR)
	$(AS) $(ASFLAGS) -o $@ $<
	$(DAS) $@ > $(DASMDIR)/boot_Zion.asm

clean:
	rm -rf $(OBJDIR) $(DASMDIR) *~

# Backup original MBR
backupMBR:
	rm -f MBR.origin
	sudo dd if=$(HardDisk_Dev) of=MBR.origin bs=512 count=1
	sudo dd if=MBR.origin of=$(HardDisk_Dev) bs=512 count=1 seek=$(StartSecOfMBR_bak)

# Install Seraph on disk.
install: no_debug $(OBJDIR)/boot $(OBJDIR)/boot2 $(OBJDIR)/boot_Zion
	sudo dd if=$(OBJDIR)/boot of=$(HardDisk_Dev) bs=446 count=1
	sudo dd if=$(OBJDIR)/boot2 of=$(HardDisk_Dev) bs=512 count=1 seek=$(StartSecOfBoot2)
	sudo dd if=$(OBJDIR)/boot_Zion of=$(HardDisk_Dev) bs=512 count=$(SizeOfBoot_Zion) seek=$(StartSecOfBoot_Zion)
	sudo dd if=$(KernelName) of=$(HardDisk_Dev) bs=512 count=$(SizeOfKernelFile) seek=$(StartSecOfKernalFile)

# Uninstall Seraph on disk.
uninstall:
	sudo dd if=$(HardDisk_Dev) of=MBR.temp bs=512 count=1 skip=$(StartSecOfMBR_bak)
	sudo dd if=MBR.temp of=$(HardDisk_Dev) bs=512 count=1
	rm -f MBR.temp
