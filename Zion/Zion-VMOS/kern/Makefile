OBJDIRS += kern

KERN_CFLAGS := $(CFLAGS) -DZION_KERNEL -gstabs
KERN_CXXFLAGS := $(KERN_CFLAGS) -fno-exceptions -fno-rtti

# entry.S must be first, so that it's the first code in the text segment!!!
KERN_SRCFILES :=\
			kern/entry.S 				\
			kern/init.c 					\
			kern/console.c 			\
			kern/disasm.c		\
			kern/common.c		\
			kern/dbg.c		\
			kern/asmserv.c		\
			kern/monitor.c 			\
			kern/pmap.c 				\
			kern/printf.c 				\
			kern/kclock.c 				\
			kern/trap.c 					\
			kern/trapentry.S 			\
			kern/kdebug.c 				\
			kern/hdd.c 					\
			kern/Loader.c

# Only build files if they exist.
KERN_SRCFILES := $(wildcard $(KERN_SRCFILES))
KERN_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(KERN_SRCFILES))
KERN_OBJFILES := $(patsubst %.S, $(OBJDIR)/%.o, $(KERN_OBJFILES))
KERN_OBJFILES := $(patsubst %.asm, $(OBJDIR)/%.o, $(KERN_OBJFILES))


# Build kernel object files
$(OBJDIR)/kern/%.o: kern/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<

$(OBJDIR)/kern/%.o: kern/%.S
	@echo + as $<
	@mkdir -p $(@D)
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

$(OBJDIR)/kern/%.o: kern/%.asm
	@echo + as $<
	@mkdir -p $(@D)
	$(V)$(NASM) $(NASMFLAGS) -o $@ $<
