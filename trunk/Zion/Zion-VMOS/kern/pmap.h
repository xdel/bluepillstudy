#ifndef __KERN_PMAP_H
#define __KERN_PMAP_H
#ifndef ZION_KERNEL
# error "This is a Zion kernel header; user programs should not #include it"
#endif
#include <inc/memlayout.h>
#include <inc/assert.h>

// Takes a kernel virtual address 'kva' -- an address that points above
// KERNBASE, where the machine's maximum 256MB of physical memory is mapped --
// and returns the corresponding physical address.  Panics if 'kva' is a
// non-kernel virtual address.
#define PADDR(kva) 												\
({ 																\
	physaddr_t __m_kva = (physaddr_t) (kva);					\
	if (__m_kva < KERNBASE)										\
		panic("PADDR called with invalid kva %08lx", __m_kva);	\
	__m_kva - KERNBASE;											\
})

// Takes a physical address 'pa' and returns the corresponding kernel virtual
// address.  Panics if 'pa' is an invalid physical address.
#define KADDR(pa)												\
({																\
	physaddr_t __m_pa = (pa);									\
	uint32_t __m_ppn = PGNUM(__m_pa);							\
	if (__m_ppn >= npages)										\
		panic("KADDR called with invalid pa %08lx", __m_pa);	\
	(void*) (__m_pa + KERNBASE);								\
})


// The kernel's main page directory, set up at boot time by mem_init().
extern pde_t *kern_pgdir;

extern struct Segdesc gdt[];
extern struct Pseudodesc gdt_pd;

void	mem_init(void);

struct 	Page *page_alloc(void);
void	page_free(struct Page *pp);
int		page_insert(pde_t *pgdir, struct Page *pp, uintptr_t va, int perm);
void	page_remove(pde_t *pgdir, uintptr_t va);
struct 	Page *page_lookup(pde_t *pgdir, uintptr_t va, pte_t **pte_store);
void	page_decref(struct Page *pp);
pte_t *	pgdir_walk(pde_t *pgdir, uintptr_t va, int create);

void	tlb_invalidate(pde_t *pgdir, uintptr_t va);

#endif /* !JOS_KERN_PMAP_H */
