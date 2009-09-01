OBJDIR 	= obj
DASMDIR = DASM

AS		= nasm
DAS 	= ndisasm
CC 		= gcc
LD 		= ld

ASFLAGS	= -O0 
CFLAGS 	= -c
LDFLAGS = -s

# All Phony Targets
.PHONY : all clean image $(OBJDIR)/boot $(OBJDIR)/boot2 $(OBJDIR)/boot_Zion

all: image 

# Write binary into bochs disk image
image: $(OBJDIR)/boot $(OBJDIR)/boot2 $(OBJDIR)/boot_Zion 
	dd if=$(OBJDIR)/boot of=Seraph_boot.img bs=512 count=1 conv=notrunc
	dd if=$(OBJDIR)/boot2 of=Seraph_boot.img bs=512 seek=41 count=1 conv=notrunc
	dd if=$(OBJDIR)/boot_Zion of=Seraph_boot.img bs=512 seek=42 count=6 conv=notrunc
	dd if=kernel of=Seraph_boot.img bs=512 seek=189 count=512 conv=notrunc
	dd if=$(OBJDIR)/boot of=Seraph_boot.img bs=512 seek=40 count=1 conv=notrunc

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
	sudo dd if=MBR.origin of=/dev/sda bs=512 count=1 seek=40

# Install Seraph on disk.
install:
	sudo dd if=$(OBJDIR)/boot of=/dev/sda bs=446 count=1
	sudo dd if=$(OBJDIR)/boot2 of=/dev/sda bs=512 count=1 seek=41
	sudo dd if=$(OBJDIR)/boot_Zion of=/dev/sda bs=512 count=6 seek=42

# Uninstall Seraph on disk.
uninstall:
	sudo dd if=/dev/sda of=MBR.temp bs=512 count=1 skip=40
	sudo dd if=MBR.temp of=/dev/sda bs=512 count=1
	rm -f MBR.temp

