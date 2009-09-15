OBJDIRS += lib

LIB_SRCFILES := \
		lib/readline.c 		\
		lib/printfmt.c 		\
		lib/stdlib.c 			\
		lib/string.c 			\
		lib/malloc.c

# Only build files if they exist.
LIB_SRCFILES := $(wildcard $(LIB_SRCFILES))
LIB_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(LIB_SRCFILES))


# Build object files
$(OBJDIR)/lib/%.o: lib/%.c
	@echo + c++ $<
	@mkdir -p $(@D)
	$(V)$(CXX) -nostdinc $(CXXFLAGS) -c -o $@ $<
