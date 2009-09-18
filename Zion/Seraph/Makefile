TOP 					= .
OBJDIR 			:= obj
DASMDIR 		:= DASM

AS					:= nasm
DAS 				:= ndisasm

ASFLAGS			:= -O0 

# Source files list
SERAPH_SRCFILES :=\
		boot/boot.asm 				\
		boot/boot2.asm 				\
		boot/kernel_loader.asm 
SERAPH_SRCFILES 	:= $(wildcard $(SERAPH_SRCFILES))
SERAPH_OBJS 			:= $(patsubst %.asm, $(OBJDIR)/%, $(notdir $(SERAPH_SRCFILES)))


# include Makefile parameter head file
include 	parameter.def

# All Phony Targets
.PHONY : all clean image 

all: image 

# Create debug flags
bochs_debug: 
	echo "%define __BOCHS_DEBUG__" > include/bochs.inc

no_debug: 
	echo "%define __NO_BOCHS_DEBUG__" > include/bochs.inc

# Generate targets
$(OBJDIR)/% : boot/%.asm
	@mkdir -p $(@D)
	@mkdir -p $(DASMDIR)
	$(AS) $(ASFLAGS) -o $@ $<
	$(DAS) $@ > $(DASMDIR)/$(@F).asm

# Write binary into bochs disk image
image: bochs_debug $(SERAPH_OBJS)
	dd if=$(OBJDIR)/boot of=$(BochsImgName) bs=512 count=1 conv=notrunc
	dd if=$(OBJDIR)/boot2 of=$(BochsImgName) bs=512 seek=$(StartSecOfBoot2) count=1 conv=notrunc
	dd if=$(OBJDIR)/kernel_loader of=$(BochsImgName) bs=512 seek=$(StartSecOfKernelLoader) count=$(SizeOfKernelLoader) conv=notrunc
	dd if=$(KernelName) of=$(BochsImgName) bs=512 seek=$(StartSecOfKernalFile_bochs) count=$(SizeOfKernelFile) conv=notrunc
	dd if=$(OBJDIR)/boot of=$(BochsImgName) bs=512 seek=$(StartSecOfMBR_bak) count=1 conv=notrunc

clean:
	-rm -rf $(OBJDIR) $(DASMDIR) *~

# Backup original MBR
backupMBR:
	rm -f MBR.origin
	sudo dd if=$(HardDisk_Dev) of=MBR.origin bs=512 count=1
	sudo dd if=MBR.origin of=$(HardDisk_Dev) bs=512 count=1 seek=$(StartSecOfMBR_bak)

# Install Seraph on disk.
install: no_debug $(SERAPH_OBJS)
	sudo dd if=$(OBJDIR)/boot of=$(HardDisk_Dev) bs=436 count=1
	sudo dd if=$(OBJDIR)/boot2 of=$(HardDisk_Dev) bs=512 count=1 seek=$(StartSecOfBoot2)
	sudo dd if=$(OBJDIR)/kernel_loader of=$(HardDisk_Dev) bs=512 count=$(SizeOfKernelLoader) seek=$(StartSecOfKernelLoader)
	sudo dd if=$(KernelName) of=$(HardDisk_Dev) bs=512 count=$(SizeOfKernelFile) seek=$(StartSecOfKernalFile)

# Uninstall Seraph on disk.
uninstall:
	sudo dd if=$(HardDisk_Dev) of=MBR.temp bs=512 count=1 skip=$(StartSecOfMBR_bak)
	sudo dd if=MBR.temp of=$(HardDisk_Dev) bs=512 count=1
	rm -f MBR.temp
