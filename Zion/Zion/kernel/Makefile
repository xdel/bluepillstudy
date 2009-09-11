#
# Makefile for Zion kernel.
# This is NOT a complete makefile;
# you must run GNU make in the top-level directory
# where the first Makefile is located.
#

OBJDIRS += kern

KERN_LDFLAGS := $(LDFLAGS) -T kernel/kernel.ld -nostdlib

# entry.S must be first, so that it's the first code in the text segment!!!
#
# We also snatch the use of a couple handy source files
# from the lib directory, to avoid gratuitous code duplication.
KERN_SRCFILES :=	kernel/head.S \
			kernel/init.c \
			kernel/console.c \
			kernel/monitor.c \
			kernel/kclock.c \
			kernel/picirq.c \
			kernel/printf.c \
			kernel/trap.c \
			kernel/trapentry.S \
			kernel/sched.c \
			kernel/kdebug.c \
			lib/printfmt.c \
			lib/readline.c \
			lib/string.c \
			mm/pmap.c \
			lib/malloc.c
			

# Only build files if they exist.
KERN_SRCFILES := $(wildcard $(KERN_SRCFILES))

KERN_BINFILES := 

KERN_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(KERN_SRCFILES))
KERN_OBJFILES := $(patsubst %.S, $(OBJDIR)/%.o, $(KERN_OBJFILES))
KERN_OBJFILES := $(patsubst %.cc, $(OBJDIR)/%.o, $(KERN_OBJFILES))
KERN_OBJFILES := $(patsubst $(OBJDIR)/lib/%, $(OBJDIR)/kernel/%, $(KERN_OBJFILES))
KERN_OBJFILES := $(patsubst $(OBJDIR)/vmx/%, $(OBJDIR)/kernel/%, $(KERN_OBJFILES))

KERN_BINFILES := $(patsubst %, $(OBJDIR)/%, $(KERN_BINFILES))

# How to build kernel object files
$(OBJDIR)/kernel/%.o: kernel/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<

$(OBJDIR)/mm/%.o: mm/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<

$(OBJDIR)/kernel/%.o: kernel/%.cc
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<

$(OBJDIR)/kernel/%.o: kernel/%.S
	@echo + as $<
	@mkdir -p $(@D)
	$(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

$(OBJDIR)/kernel/%.o: lib/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(KERN_CXXFLAGS) -c -o $@ $<


