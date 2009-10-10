#ifndef ZION_DEBUGGER_H_
#define ZION_DEBUGGER_H_

#define DEBUGGER_ENABLED
#define MAX_DEBUGGER_BPS 64

#include <inc/kern/common.h>
#include <inc/trap.h>

struct dbg_bp
{
	uint32_t valid;
	uint32_t addr;
	uint8_t origin;
};

struct dbg_block
{
	int step_trap;
	struct dbg_bp bps[MAX_DEBUGGER_BPS];
	struct Trapframe *ctx;
	struct dbg_bp *cur_bp;
};

extern int dbg_init();
extern int dbg_console();
extern int dbg_break(struct Trapframe *ctx);
inline int dbg_dummy_console(int argc, char **argv, struct Trapframe *tf)
{
	return dbg_console();
}

#endif /* ZION_DEBUGGER_H_*/
