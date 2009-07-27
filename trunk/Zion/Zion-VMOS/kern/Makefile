OBJDIRS += kern

KERN_LDFLAGS := $(LDFLAGS) -T kern/kernel.ld -nostdlib

# entry.S must be first, so that it's the first code in the text segment!!!
#
# We also snatch the use of a couple handy source files
# from the lib directory, to avoid gratuitous code duplication.
KERN_SRCFILES :=\
			kern/entry.S 		\
			kern/init.c 		\
			kern/console.c 		\
			kern/monitor.c 		\
			kern/pmap.c 		\
			kern/printf.c 		\
			kern/kclock.c 		\
			kern/trap.c 		\
			kern/trapentry.S 	\
			kern/kdebug.c 		\
			lib/printfmt.c 		\
			lib/readline.c 		\
			lib/string.c 		\
			lib/stdlib.c 		\
			lib/cpu.c 			



# Only build files if they exist.
KERN_SRCFILES := $(wildcard $(KERN_SRCFILES))

KERN_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(KERN_SRCFILES))
KERN_OBJFILES := $(patsubst %.S, $(OBJDIR)/%.o, $(KERN_OBJFILES))
KERN_OBJFILES := $(patsubst %.cc, $(OBJDIR)/%.o, $(KERN_OBJFILES))
KERN_OBJFILES := $(patsubst $(OBJDIR)/lib/%, $(OBJDIR)/kern/%, $(KERN_OBJFILES))

KERN_BINFILES :=
KERN_BINFILES := $(patsubst %, $(OBJDIR)/%, $(KERN_BINFILES))



# Build kernel object files
$(OBJDIR)/kern/%.o: kern/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<

$(OBJDIR)/kern/%.o: kern/%.cc
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<

$(OBJDIR)/kern/%.o: kern/%.S
	@echo + as $<
	@mkdir -p $(@D)
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

$(OBJDIR)/kern/%.o: lib/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<

#$(OBJDIR)/kern/cpu.o: arch/cpu.c
#	@echo + c++ $<
#	@mkdir -p $(@D)
#	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<


# Build the kernel itself
$(OBJDIR)/kernel: $(KERN_OBJFILES) $(KERN_BINFILES) kern/kernel.ld
	@echo + ld $@
	$(V)$(LD) -o $@ $(KERN_LDFLAGS) $(KERN_OBJFILES) $(GCC_LIB) -b binary $(KERN_BINFILES)
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ | awk '{ print $$1, $$3 }' > $@.sym



