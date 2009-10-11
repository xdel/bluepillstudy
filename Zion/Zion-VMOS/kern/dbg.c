/* kernel debugger designed for Zion
|| simply take charge of int1 and int3 only
|| by Kenmark
|| Version : 1.0.0002 (mod @ 2009-10-9 13:16)
*/

#include <inc/kern/dbg.h>
#include <inc/kern/common.h>
#include <inc/kern/mmu.h>
#include <inc/lib/stdio.h>
#include <inc/types.h>
#include <inc/trap.h>
#include <inc/lib/string.h>
#include <inc/lib/stdlib.h>
#include <inc/kern/disasm.h>


#ifdef DEBUGGER_ENABLED
extern struct Segdesc gdt[]; // TODO

static struct dbg_block dbg;

/*~ file scope static functions(private) */
static uint32_t _get_seg_base(uint32_t segid)
{// TODO
	uint32_t t;
	t = gdt[segid / 0x8].sd_base_15_0;
	t |= gdt[segid / 0x8].sd_base_23_16 <<16;
	t |= gdt[segid / 0x8].sd_base_31_24 <<24;
	return t;
}

static void _dump_addr(void *raw, char *fmt)
{
	void *t = raw;
	char *p = fmt;
	int n = 0;
	while (*p && *p != '\n')
	{
		switch (*p++)
		{
		case 'd': /* uint32_t */
			cprintf("$+%2x:%08x\n", n, *(uint32_t *)t);
			n += 4;
			t = (void *)((uint32_t *)t + 1);
			break;
		case 'w': /* uint16_t */
			cprintf("$+%2x:----%04x\n", n, *(uint16_t *)t);
			n += 2;
			t = (void *)((uint16_t *)t + 1);
			break;
		case 'b': /* uint8_t */
			cprintf("$+%2x:------%02x\n", n, *(uint8_t *)t);
			n += 1;
			t = (void *)((uint8_t *)t + 1);
			break;
		case 'a': /* ADDRESS */
			// 2TODO
			cprintf("$+%2x:%08x\n", n, *(uint32_t *)t);
			n += 4;
			t = (void *)((uint32_t *)t + 1);
			break;
		default:
			debug_warning("invalid format while debugger dumping the stack!");
			return ;
		}
	}
}

static void _dbg_print_bp(int n, int print_empty)
{
	if (n >= MAX_DEBUGGER_BPS)
	{
		debug_warning("try printing breakpoint out of bound!\n");
	}
	else if (print_empty || dbg.bps[n].valid)
	{
		cprintf("breakpoint #%d(%s):0x%08x\n", n, dbg.bps[n].valid ? "active" : "inactive",dbg.bps[n].addr);
	}
}

static int _get_bpidx_by_addr(uint32_t addr)
{
	int i;
	for (i = 0; i < MAX_DEBUGGER_BPS; ++i)
	{
		if (dbg.bps[i].valid && dbg.bps[i].addr == addr)
			break;
	}
	return i;
}

static void _dbg_print_ctx(struct Trapframe *ctx)
{
	t_disasm da;
	if (ctx->tf_trapno == 3)
	{
		cprintf("breakpoint #%d @ %08x\n",_get_bpidx_by_addr(dbg.cur_bp->addr), dbg.cur_bp->addr);
	}
	else if (ctx->tf_trapno == 1)
		;
	else
		debug_warning("why send it to debugger??\n");
	ideal=0; lowercase=1; putdefseg=0;
	Disasm((char *)ctx->tf_eip, ctx->tf_eip, 0,&da,DISASM_CODE);
	cprintf("%08x  %-24s  %-24s\n", ctx->tf_eip, da.dump, da.result);
}

static char *_eat_white_char(char *p)
{
	while (*p == ' ' || *p == '\t' || *p == '\n')
		++p;
	return p;
}

static int _dbg_add_bp(uint32_t addr)
{
	int i;
	for (i = 0; i < MAX_DEBUGGER_BPS; ++i)
	{
		if (!dbg.bps[i].valid)
		{
			dbg.bps[i].valid = 1;
			dbg.bps[i].addr = addr;
			dbg.bps[i].origin = *(uint8_t *)addr;
			*(uint8_t *)addr = 0xCC; /* set int3 */
			break;
		}
	}
	return i;
}

static void _print_sregs(struct Trapframe *ctx)
{
	// TODO
	cprintf("  es   0x----%04x\n", ctx->tf_es);
	cprintf("  ds   0x----%04x\n", ctx->tf_ds);
	cprintf("  trap 0x%08x %s\n", ctx->tf_trapno, "noname");
	cprintf("  err  0x%08x\n", ctx->tf_err);
	cprintf("  eip  0x%08x\n", ctx->tf_eip);
	cprintf("  cs   0x----%04x\n", ctx->tf_cs);
	cprintf("  flag 0x%08x\n", ctx->tf_eflags);
	cprintf("  esp  0x%08x\n", ctx->tf_esp);
	cprintf("  ss   0x----%04x\n", ctx->tf_ss);
}

static void _print_cregs(struct Trapframe *ctx)
{
	// TODO
	cprintf("  edi  0x%08x\n", ctx->tf_regs.reg_edi);
	cprintf("  esi  0x%08x\n", ctx->tf_regs.reg_esi);
	cprintf("  ebp  0x%08x\n", ctx->tf_regs.reg_ebp);
	cprintf("  oesp  0x%08x\n", ctx->tf_regs.reg_oesp);
	cprintf("  ebx  0x%08x\n", ctx->tf_regs.reg_ebx);
	cprintf("  edx  0x%08x\n", ctx->tf_regs.reg_edx);
	cprintf("  ecx  0x%08x\n", ctx->tf_regs.reg_ecx);
	cprintf("  eax  0x%08x\n", ctx->tf_regs.reg_eax);
}

static int _parse_data(uint8_t *dest, char *src_fmt)
{
	// TODO
	return 0;
}

/*~ extern functions */
int dbg_init()
{
	int i;
	for (i = 0; i < MAX_DEBUGGER_BPS; ++i)
		dbg.bps[i].valid = 0;
	dbg.ctx = NULL;
	dbg.step_trap = 0;
	return 0;
}

int dbg_console()
{
	char *p;
loop:
	p = readline("k> ");
	switch (*p++)
	{
	case 'b': /* break point */
		if (*p == '-') /* delete */
		{
			int n;
			p = _eat_white_char(++p);
			if (*p == '#')
			{
				n = strtol(++p, NULL, 10);
			}
			else
			{
				if ((n = _get_bpidx_by_addr(strtol(p, NULL, 16))) == MAX_DEBUGGER_BPS)
					cprintf("no breakpoint @ %s\n", p);
			}
			*(uint8_t *)dbg.bps[n].addr = dbg.bps[n].origin;
			dbg.bps[n].valid = 0;
		}
		else if (*p == '?') /* query */
		{
			p = _eat_white_char(++p);
			if (*p == '#') /* specific breakpoint */
			{
				_dbg_print_bp(strtol(++p, NULL, 10), 1);
			}
			else /* find breakpoint on address */
			{
				int i;
				for (i = 0; i < MAX_DEBUGGER_BPS; ++i)
					_dbg_print_bp(i, 0);
			}
		}
		else /* add */
		{
			int n;
			p = _eat_white_char(p);
			n = _dbg_add_bp(strtol(p, NULL, 16));
			if (n >= MAX_DEBUGGER_BPS)
			{
				cprintf("breakpoint slot run out!\n");
			}
			else
			{
				*(uint8_t *)dbg.bps[n].addr = 0xCC;
				cprintf("breakpoint #%d set to %s\n", n, p);
			}
		}
		break;
	case 'g': /* go */
		if (dbg.ctx == NULL)
			goto out_ret;
		if (dbg.ctx->tf_trapno != 3)
			dbg.ctx->tf_eflags &= ~FL_TF; /* clear eeflag.tp */
		//asm volatile {rdmsr } // TODO
		//asm volatile {wrmsr }; /* clear msr.btf */
		goto out_ret;
	case 's': /* step */
		if (dbg.ctx == NULL)
		{
			cprintf("no debugging context!\n");
			break;
		}
		dbg.step_trap = 1;
		dbg.ctx->tf_eflags |= FL_TF; /* set eeflag.tp */
		if (*p == 'o') /* over */
		{
			// TODO set msr
		}
		goto out_ret;
	case '$': /* stack */
		if (dbg.ctx == NULL)
		{
			cprintf("no debugging context!\n");
			break;
		}
		p = _eat_white_char(p);
		_dump_addr((void *)(_get_seg_base(dbg.ctx->tf_ss) + dbg.ctx->tf_esp), p); // TODO ctx.ss+esp
		break;
	case 'r': /* register */
		if (dbg.ctx == NULL)
		{
			cprintf("no debugging context!\n");
			break;
		}
		if (*p == 's')
		{
			_print_sregs(dbg.ctx);
		}
		else
		{
			_print_cregs(dbg.ctx);
		}
		break;
	/* context free operations */
	case 'd': /* disasm(memory) */
		do
		{
			int i;
			char *addr = (char *)strtol(_eat_white_char(p), NULL, 16), *t = addr;
			t_disasm da;
			ideal=0; lowercase=1; putdefseg=0;
			for (i = 0; i < 16; ++i)
			{
				t += Disasm(addr, (unsigned long)addr, 0, &da, DISASM_CODE);
				cprintf("%08x  %-24s  %-24s\n", addr, da.dump, da.result);
				addr = t;
			}
		} while (0);
		break;
	case 'l': /* looking */
		do
		{
			uint8_t buffer[128] = {0};
			char *addr_begin = (char *)strtol(_eat_white_char(p), &p, 16);
			char *addr_end = (char *)strtol(_eat_white_char(p), &p, 16);
			_parse_data(buffer, p);
			// TODO
		} while(0);
		break;
	case 'x': /* check memory */
		do
		{
			void *addr = (void *)strtol(_eat_white_char(p), &p, 16);
			_dump_addr(addr, _eat_white_char(p));
		} while(0);
		break;
	case 'w': /* write memory */
		do
		{
			uint8_t buffer[128];
			char *addr = (char *)strtol(_eat_white_char(p), &p, 16);
			int len = _parse_data(buffer, p);
			memcpy(addr, buffer, len);
		} while(0);
		break;
	default:
		cprintf("invalid debugge command!\n");
		break;
	}
	goto loop;
out_ret:
	dbg.ctx = NULL;
	return 0;
}

int dbg_break(struct Trapframe *ctx)
{
	if (dbg.ctx != NULL)
	{
		debug_warning("nested debugging context not handled!\n");
		return 1;
	}
	if (ctx->tf_trapno == 1)
	{
		if (dbg.cur_bp != NULL)
		{ /* breakpoint fix! */
			*(uint8_t *)dbg.cur_bp->addr = 0xCC;
			dbg.cur_bp = NULL;
			ctx->tf_eflags &= ~FL_TF;
			// TODO unset msrs
		}
		if (dbg.step_trap)
		{
			_dbg_print_ctx(dbg.ctx = ctx);
			dbg.step_trap = 0;
			return dbg_console();
		}
		return 0;
	}
	if (ctx->tf_trapno == 3)
	{
		int n = _get_bpidx_by_addr(ctx->tf_eip - 1);
		if (n != MAX_DEBUGGER_BPS)
		{
			dbg.cur_bp = &dbg.bps[n];
		}
		else
		{
			debug_warning("unknown breakpoint encountered!\n");
			return 1;
		}
		ctx->tf_eflags |= FL_TF; /* set eeflag.tp for fix*/
		--ctx->tf_eip;
		*(uint8_t *)dbg.cur_bp->addr = dbg.cur_bp->origin;
		
		_dbg_print_ctx(dbg.ctx = ctx);
		return dbg_console();
	}
	debug_warning("debugger error!\n");
	return 1;
}

#endif /* DEBUGGER_ENABLED */
