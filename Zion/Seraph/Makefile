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

# All Phony Targets
.PHONY : all clean image $(OBJDIR)/boot $(OBJDIR)/boot2 $(OBJDIR)/boot_Zion

all: image 

# Write binary into bochs disk image
image: $(OBJDIR)/boot $(OBJDIR)/boot2 $(OBJDIR)/boot_Zion 
	dd if=$(OBJDIR)/boot of=Seraph_boot.img bs=512 count=1 conv=notrunc
	dd if=$(OBJDIR)/boot2 of=Seraph_boot.img bs=512 seek=33 count=1 conv=notrunc
	dd if=$(OBJDIR)/boot_Zion of=Seraph_boot.img bs=512 seek=34 count=6 conv=notrunc
	dd if=Zion-VMOS of=Seraph_boot.img bs=512 seek=189 count=512 conv=notrunc
	dd if=$(OBJDIR)/boot of=Seraph_boot.img bs=512 seek=32 count=1 conv=notrunc

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
	sudo dd if=/dev/sda of=MBR.origin bs=512 count=1
	sudo dd if=MBR.origin of=/dev/sda bs=512 count=1 seek=32

# Install Seraph on disk.
install: $(OBJDIR)/boot $(OBJDIR)/boot2 $(OBJDIR)/boot_Zion
	sudo dd if=$(OBJDIR)/boot of=/dev/sda bs=446 count=1
	sudo dd if=$(OBJDIR)/boot2 of=/dev/sda bs=512 count=1 seek=33
	sudo dd if=$(OBJDIR)/boot_Zion of=/dev/sda bs=512 count=20 seek=34
	sudo dd if=Zion-VMOS of=/dev/sda bs=512 count=512 seek=61431000 

# Uninstall Seraph on disk.
uninstall:
	sudo dd if=/dev/sda of=MBR.temp bs=512 count=1 skip=32
	sudo dd if=MBR.temp of=/dev/sda bs=512 count=1
	rm -f MBR.temp
