#include <inc/arch/x86.h>
#include <inc/kern/mmu.h>
#include <inc/error.h>
#include <inc/lib/string.h>
#include <inc/assert.h>
#include <inc/kern/pmap.h>
#include <inc/kern/kclock.h>

// These variables are set by i386_mem_detect()
size_t npages;			// Amount of physical memory (in pages)
static size_t n_base_pages;	// Amount of base memory (in pages)

// These variables are set in mem_init()
pde_t *kern_pgdir;		// Kernel's initial page directory
struct Page *pages;		// Physical page state array

static Page *free_pages;	// Free list of physical pages

extern char bootstack[];	// Lowest addr in boot-time kernel stack

// Global descriptor table.
//
// The kernel and user segments are identical (except for the DPL).
// To load the SS register, the CPL must equal the DPL.  Thus,
// we must duplicate the segments for the user and the kernel.
//
struct Segdesc gdt[] = {
	SEG_NULL,				// 0x0 - unused (always faults)
	SEG(STA_X | STA_R, 0x0, 0xffffffff, 0),	// 0x8 - kernel code segment
	SEG(STA_W, 0x0, 0xffffffff, 0),		// 0x10 - kernel data segment
	SEG(STA_X | STA_R, 0x0, 0xffffffff, 3),	// 0x18 - user code segment
	SEG(STA_W, 0x0, 0xffffffff, 3),		// 0x20 - user data segment
	SEG_NULL				// 0x28 - tss, initialized in
						// idt_init()
};

struct Pseudodesc gdt_pd = {
	sizeof(gdt) - 1, (unsigned long) gdt
};

static int nvram_read(int r)
{
	return mc146818_read(r) | (mc146818_read(r + 1) << 8);
}

static void i386_mem_detect(void)
{
	uint32_t n_extended_pages;
	
	// Use CMOS calls to measure available base & extended memory.
	// (CMOS calls return results in kilobytes.)
	n_base_pages = nvram_read(NVRAM_BASELO) * 1024 / PGSIZE;
	n_extended_pages = nvram_read(NVRAM_EXTLO) * 1024 / PGSIZE;

	// Calculate the maximum physical address based on whether
	// or not there is any extended memory.  See comment in <inc/mmu.h>.
	if (n_extended_pages)
		npages = (EXTPHYSMEM / PGSIZE) + n_extended_pages;
	else
		npages = n_base_pages;

	cprintf("Physical memory: %uK available, ", npages * PGSIZE / 1024);
	cprintf("base = %uK, extended = %uK\n", n_base_pages * PGSIZE / 1024,
		n_extended_pages * PGSIZE / 1024);
}


// --------------------------------------------------------------
// Set up initial memory mappings and turn on MMU.
// --------------------------------------------------------------

static void *boot_alloc(uint32_t n);
static void page_init(void);
static void page_map_segment(pte_t *pgdir, uintptr_t va, size_t size, physaddr_t pa, int perm);

static void page_alloc_check(void);
static void boot_mem_check(void);
static void page_check(void);

static char *nextfree = 0;	// pointer to next byte of free mem


// Initializes virtual memory.
//
// Sets up the kernel's page directory 'kern_pgdir' (which contains those
// virtual memory mappings common to all user environments), installs that
// page directory, and turns on paging.  Then effectively turns off segments.
// 
// This function only sets up the kernel part of the address space
// (ie. addresses >= UTOP).  The user part of the address space
// will be set up later.
//
// From UTOP to ULIM, the user is allowed to read but not write.
// Above ULIM the user cannot read (or write). 
void mem_init(void)
{
	uint32_t cr0;
	size_t n;
	int r;
	struct Page *pp;
	
	nextfree = (char *)0; // TODO!!!
	
	// Find out how much memory the machine has ('npages' & 'n_base_pages')
	i386_mem_detect();

	// Allocate 'pages', an array of 'struct Page' structures, one for
	// each physical memory page.  So there are 'npages' elements in the
	// array total (see i386_mem_detect()).
	// We advise you set the memory to 0 after allocating it, since that
	// will help you catch bugs later.
	//
	// LAB 2: Your code here.
	pages = (Page *)boot_alloc(npages * sizeof(struct Page));
	memset(pages, 0, npages * sizeof(struct Page));
//	cprintf("TEST: boot_alloc() \n");
	
	// Now that we've allocated the 'pages' array, initialize it
	// by putting all free physical pages onto a list.  After this point,
	// all further memory management will go through the page_* functions.
	page_init();
//	cprintf("TEST: page_init() \n");

	// Allocate the kernel's initial page directory, 'kern_pgdir'.
	// This starts out empty (all zeros).  Any virtual
	// address lookup using this empty 'kern_pgdir' would fault.
	// Then we add mappings to 'kern_pgdir' as we go long.
	pp = page_alloc();
	pp->pp_ref++;		// make sure we mark the page as used!

	kern_pgdir = (pte_t *) pp->data();
	memset(kern_pgdir, 0, PGSIZE);

	
	// Map the kernel stack at virtual address 'KSTACKTOP-KSTKSIZE'.
	// A large range of virtual memory, [KSTACKTOP-PTSIZE, KSTACKTOP),
	// is marked out for the kernel stack.  However, only part of this
	// range is allocated.  The rest of it is left unmapped, so that
	// kernel stack overflow will cause a page fault.
	// - [KSTACKTOP-PTSIZE, KSTACKTOP-KSTKSIZE) -- not present
	// - [KSTACKTOP-KSTKSIZE, KSTACKTOP) -- kernel RW, user NONE
	//
	// The kernel already has a stack, so it is not necessary to allocate
	// a new one.  That stack's bottom address is 'bootstack'.  (Q: Where
	// is 'bootstack' allocated?)
	//
	page_map_segment(kern_pgdir, KSTACKTOP-KSTKSIZE, KSTKSIZE, PADDR(bootstack), PTE_W | PTE_P);

	// Map all of physical memory at KERNBASE.
	// I.e., the VA range [KERNBASE, 2^32) should map to
	//       the PA range [0, 2^32 - KERNBASE).
	// We might not have 2^32 - KERNBASE bytes of physical memory, but
	// we just set up the mapping anyway.
	// Permissions: kernel RW, user NONE
	//
	page_map_segment(kern_pgdir, KERNBASE, (0xffffffff)-KERNBASE+1, 0, PTE_W | PTE_P);
//	cprintf("TEST: page_map_segment() \n");

	
	// On x86, segmentation maps a VA to a LA (linear addr) and
	// paging maps the LA to a PA; we write VA => LA => PA.  If paging is
	// turned off the LA is used as the PA.  There is no way to
	// turn off segmentation; the closest thing is to set the base
	// address to 0, so the VA => LA mapping is the identity.

	// The current mapping: VA KERNBASE+x => PA x.
	//     (segmentation base = -KERNBASE, and paging is off.)

	// From here on down we must maintain this VA KERNBASE + x => PA x
	// mapping, even though we are turning on paging and reconfiguring
	// segmentation.

	// Map VA 0:4MB same as VA KERNBASE, i.e. to PA 0:4MB.
	// (Limits our kernel to <4MB)
	kern_pgdir[0] = kern_pgdir[PDX(KERNBASE)];

	// Install page table.
//	cprintf("TEST: begin load cr3 \n");
	lcr3(PADDR(kern_pgdir));
//	cprintf("TEST: after load cr3 \n");

	// Turn on paging.
	cr0 = rcr0();
	cr0 |= CR0_PE|CR0_PG|CR0_AM|CR0_WP|CR0_NE|CR0_TS|CR0_EM|CR0_MP;
	cr0 &= ~(CR0_TS|CR0_EM);
	lcr0(cr0);
//	cprintf("TEST: load cr0 \n");

	// Current mapping: VA KERNBASE+x => LA x => PA x.
	// (x < 4MB so uses paging kern_pgdir[0])

	// Reload all segment registers.
	asm volatile("lgdt gdt_pd");
	asm volatile("movw %%ax,%%gs" :: "a" (GD_UD|3));
	asm volatile("movw %%ax,%%fs" :: "a" (GD_UD|3));
	asm volatile("movw %%ax,%%es" :: "a" (GD_KD));
	asm volatile("movw %%ax,%%ds" :: "a" (GD_KD));
	asm volatile("movw %%ax,%%ss" :: "a" (GD_KD));
	asm volatile("ljmp %0,$1f\n 1:\n" :: "i" (GD_KT));  // reload cs
	asm volatile("lldt %%ax" :: "a" (0));

	// Final mapping: VA KERNBASE+x => LA KERNBASE+x => PA x.

	// This mapping was only used after paging was turned on but
	// before the segment registers were reloaded.
	kern_pgdir[0] = 0;
	// Flush the TLB for good measure, to kill the kern_pgdir[0] mapping.
	lcr3(PADDR(kern_pgdir));
}


// This simple physical memory allocator is used only while JOS is setting
// up its virtual memory system.  page_alloc() is the real allocator.
//
// Allocate enough pages of contiguous physical memory to hold 'n' bytes.
// Doesn't initialize the memory.  Returns a kernel virtual address.
//
// If 'n' is 0, boot_alloc() should return the KVA of the next free page
// (without allocating anything).
//
// If we're out of memory, boot_alloc should panic.
// This function may ONLY be used during initialization,
// before the free_pages list has been set up.

static void * boot_alloc(uint32_t n)
{
	extern char end[];
	void *v;

//	cprintf("TEST: linker generated: %x \n", end);
//	cprintf("TEST: boot_alloc() nextfree0  %x \n", nextfree);
	// Initialize nextfree if this is the first time.
	// 'end' is a magic symbol automatically generated by the linker,
	// which points to the end of the kernel's bss segment:
	// the first virtual address that the linker did *not* assign
	// to any kernel code or global variables.
	if (nextfree == 0)
	{
//		cprintf("TEST: ROUNDUP call : %x %x\n", end, PGSIZE);
		nextfree = round_up((char *) end, PGSIZE);
//		cprintf("TEST: boot_alloc() first nextfree: %x \n", nextfree);
//		cprintf("TEST: test roundup(12,4096):\n", round_up((char *)12, 4096));
	}
//	cprintf("TEST: boot_alloc() nextfree1: %x \n", nextfree);
	// Allocate a chunk large enough to hold 'n' bytes, then update
	// nextfree.  Make sure nextfree is kept aligned
	// to a multiple of PGSIZE.
	//
	nextfree = round_up(nextfree, PGSIZE);	// Round up to be aligned properly.
//	cprintf("TEST: boot_alloc() nextfree2: %x \n", nextfree);
	v = nextfree;	// Save current value of "nextfree" as allocated chunk.
	nextfree += n;	// Increase to record allocation.
//	cprintf("TEST: boot_alloc() return: %x \n", v);
	return v;		// Return allocated chunk.
}//boot_alloc()


// This function returns the physical address of the page containing 'va',
// defined by the page directory 'pgdir'.  The hardware normally performs
// this functionality for us!  We define our own version to help
// the boot_mem_check() function; it shouldn't be used elsewhere.
static physaddr_t check_va2pa(pde_t *pgdir, uintptr_t va)
{
	pte_t *p;

	pgdir = &pgdir[PDX(va)];
	if (!(*pgdir & PTE_P))
		return ~0;
	p = (pte_t*) KADDR(PTE_ADDR(*pgdir));
	if (!(p[PTX(va)] & PTE_P))
		return ~0;
	return PTE_ADDR(p[PTX(va)]);
}
		

// --------------------------------------------------------------
// Tracking of physical pages.
// The 'pages' array has one 'struct Page' entry per physical page.
// Pages are reference counted, and free pages are kept on a linked list.
// --------------------------------------------------------------

// Initialize page structure and memory free list.
// After this point, ONLY use the page_ functions
// to allocate and deallocate physical memory via the free_pages list,
// and NEVER use boot_alloc() or the related boot-time functions above.
void page_init(void)
{
	// The example code here marks all pages as free.
	// However this is not truly the case.  What memory is free?
	//  1) Mark page 0 as in use.
	//     This way we preserve the real-mode IDT and BIOS structures
	//     in case we ever need them.  (Currently we don't, but...)
	//  2) Mark the rest of base memory as free.
	//  3) Then comes the IO hole [IOPHYSMEM, EXTPHYSMEM).
	//     Mark it as in use so that it can never be allocated.      
	//  4) Then extended memory [EXTPHYSMEM, ...).
	//     Some of it is in use, some is free.  Where is the kernel?
	//     Which pages are used for page tables and other data structures
	//     allocated by boot_alloc()?  (Hint: boot_alloc() will tell you
	//     if you give it the right argument!)
	//
	physaddr_t 	pageStart, pageEnd;
	physaddr_t 	kernelStart = PADDR(KERNBASE);
	physaddr_t 	kernelEnd = (physaddr_t)PADDR(boot_alloc(0));
	size_t 		i;

	free_pages = NULL;		// Initialize free_pages list as NULL.
	pages[0].pp_ref = 1;	// Initialize page 0, marked as in use for IDT and BIOS.

	// Initialize the rest pages.
	for( i=1, pageStart=PGSIZE, pageEnd=pageStart+PGSIZE-1; i<npages; i++ ) {
		// Initialize the page structure
		pages[i].pp_ref = 0;
		
		// Mark I/O holes as in use.
		if( (pageStart>=IOPHYSMEM && pageStart<EXTPHYSMEM) ||\
			(pageEnd>=IOPHYSMEM && pageEnd<EXTPHYSMEM) ) {
			pages[i].pp_ref = 1;
		} else 
		// Mark kernel space as in use.
		if( (pageStart>=kernelStart && pageStart<kernelEnd) ||\
			(pageEnd>=kernelStart && pageEnd<kernelEnd) ) {
			pages[i].pp_ref = 1;
		} else {
			// Add this page to the free_pages list.
			pages[i].pp_next = free_pages;
			free_pages = &pages[i];
		}//if...else...if...else

		pageStart += PGSIZE;
		pageEnd += PGSIZE;
	}//for
}//page_init()



// Allocate a physical page, without necessarily initializing it.
// Returns a pointer to the Page struct of the newly allocated page.
// If there were no free pages, returns NULL.
//
// Hint: set the Page's fields appropriately
// Hint: the returned page's pp_ref should be zero
// Software Engineering Hint: It can be extremely useful for later debugging
//   if you erase allocated memory.  For instance, you might write the value
//   0xCC (the int3 instruction) over the page before you return it.  This will
//   cause your kernel to crash QUICKLY if you ever make a bookkeeping mistake,
//   such as freeing a page while someone is still using it.  A quick crash is
//   much preferable to a SLOW crash, where *maybe* a long time after your
//   kernel boots, a data structure gets corrupted because its containing page
//   was used twice!  Note that erasing the page with a non-zero value is
//   usually better than erasing it with 0.  (Why might this be?)
struct Page * page_alloc()
{
	// If there were no free pages, returns NULL.
	if( free_pages==NULL ) {
		return NULL;
	}//if

	struct Page	*pp = free_pages;	// Get the pointer of a free page.
	free_pages = free_pages->pp_next; 	// Remove the allocated page from free_pages list.
	memset(KADDR(pp->physaddr()), 0xcc, PGSIZE); 	// Write 0xcc over the allocated page.
	memset(pp, 0, sizeof(*pp)); 	// Clear the allocated page's fields.

	return pp;		// Return the pointer of the allocated page.
}//page_alloc()

// Return a page to the free list.
// (This function should only be called when pp->pp_ref reaches 0:
// you may want to check this with an assert.)
//
// Software Engineering Hint: It can be extremely useful for later debugging
//   if you erase each page's memory as soon as it is freed.  See the Software
//   Engineering Hint above for reasons why.
void page_free(struct Page *pp)
{
	assert(pp->pp_ref==0); 	// Make sure that pp_ref reaches 0.
	
	// Return the page to the free_pages list.
	memset(KADDR(pp->physaddr()), 0, PGSIZE); 	// Clear the page.
	pp->pp_next = free_pages;
	free_pages = pp;
}//page_free()

// Decrement the reference count on a page.
// Free it if there are no more refs afterwards.
void page_decref(struct Page *pp)
{
	if (--pp->pp_ref == 0)
		page_free(pp);
}

// Given 'pgdir', a pointer to a page directory, pgdir_walk returns
// a pointer to the page table entry (PTE) for linear address 'va'.
// This requires walking the two-level page table structure.
//
// If the relevant page table doesn't exist in the page directory, then:
//    - If create == 0, pgdir_walk returns NULL.
//    - Otherwise, pgdir_walk tries to allocate a new page table
//	with page_alloc.  If this fails, pgdir_walk returns NULL.
//    - Otherwise, pgdir_walk returns a pointer into the new page table.
//
// Hint: you can turn a Page * into the physical address of the
// page it refers to with Page::physaddr() from inc/memlayout.h.
pte_t * pgdir_walk(pde_t *pgdir, uintptr_t va, int create)
{
	pde_t 		*p_pde;		// pointer to the page directory entry
	pte_t 		*pt;		// pointer to the page table
	struct Page *pp;		// pointer to the page

	p_pde = &pgdir[PDX(va)];	// Get PDE's VA from PD base and its offset
	if( !(*p_pde & PTE_P) ){	// If the page table doesn't exist...
		if( !create ) 
			return NULL;
		if( free_pages==NULL) {	// Try to allocate a new page.
			return NULL;
		} else {
			pp = page_alloc(); 		// Allocate a page.
		}//if..else
		pp->pp_ref = 1;
		memset(KADDR(pp->physaddr()), 0, PGSIZE);	// Clear the allocated page.
		*p_pde = pp->physaddr() | PTE_U | PTE_W | PTE_P;	// Get the new page's PA.
	}//if
	pt = (pte_t *)(KADDR(PTE_ADDR(*p_pde)));	// Convert page's PA to PT base.
	return &pt[PTX(va)];	// Return PTE's VA.
}//pgdir_walk


// Maps the physical page 'pp' at virtual address 'va'.
// The permissions (the low 12 bits) of the page table entry
// are set to 'perm|PTE_P'.
//
// Details
//   - If there is already a page mapped at 'va', it is page_remove()d.
//   - If niecessary, on demand, allocates a page table and inserts it into
//     'pgdir'.
//   - pp->pp_ref should be incremented if the insertion succeeds.
//   - The TLB must be invalidated if a page was formerly present at 'va'.
//   - It is safe to page_insert() a page that is already mapped at 'va'.
//     This is useful to change permissions for a page.
//
// RETURNS: 
//   0 on success
//   -E_NO_MEM, if page table couldn't be allocated
//
// Hint: The TA solution is implemented using pgdir_walk, page_remove,
// and Page::physaddr.
int page_insert(pde_t *pgdir, struct Page *pp, uintptr_t va, int perm) 
{
	pte_t *p_pte;	// pointer to PTE

	// Get PTE's VA from PD base and page's VA;
	// if page table doesn't exist, allocate one.
	p_pte = pgdir_walk(pgdir, va, 1);

	if( p_pte==NULL )		// Error: No memory!
		return -E_NO_MEM;
	
	pp->pp_ref++;	// Increase page's pp_ref.

	// If the PTE has been mapped something, remove it.
	if( (*p_pte & PTE_P)!=0 ) {	
		page_remove(pgdir, va);
	}//if

	*p_pte = pp->physaddr() | perm | PTE_P;	// Map page's PA to the VA of PTE.
	return 0;
}//page_insert

// Returns the page mapped at virtual address 'va'.
// If pte_store is not null, then '*pte_store' is set to the address
// of the pte for this page.  This is used by page_remove.
//
// Returns 0 if there is no page mapped at va.
//
// Hint: the TA solution uses pgdir_walk.
struct Page * page_lookup(pde_t *pgdir, uintptr_t va, pte_t **pte_store)
{
	pte_t 	*p_pte; 	// pointer to PTE
	
	p_pte = pgdir_walk(pgdir, va, 0);	// Get PTE's VA from PD base and page's VA
	if( p_pte==NULL ) {		// Check whether such PTE exist; if no, return NULL.
		return NULL;
	}//if
	if( pte_store!=NULL ) {	// If pte_store is not NULL, save PTE's VA to it.
		*pte_store = p_pte;
	}//if

 	return &pages[PGNUM(*p_pte)];	// Return page's pointer.
}//page_lookup()


// Unmaps the physical page at virtual address 'va'.
// If there is no physical page at that address, silently does nothing.
//
// Details:
//   - The ref count on the physical page should decrement.
//   - The physical page should be freed if the refcount reaches 0.
//   - The page table entry corresponding to 'va' should be set to 0
//     (if such a PTE exists).
//   - The TLB must be invalidated if you remove an entry from
//     the pg dir/pg table.
//
// Hint: The TA solution is implemented using page_lookup,
// 	tlb_invalidate, and page_decref.
void page_remove(pde_t *pgdir, uintptr_t va)
{
	pte_t 		*p_pte;		// PTE's VA
	struct Page *pp;		// pointer to page

	pp = page_lookup(pgdir, va, &p_pte);	// Get page's pointer and its PTE's VA.

	if( pp==NULL )	// If there is no physical page at this VA, silently does nothing.
		return;

	page_decref(pp);	// Decrease the ref count for the page.

	if( p_pte!=NULL ) {	// Set the respective PTE to 0.
		*p_pte = 0;
	}//if

	tlb_invalidate(pgdir, va);	// Invalidate TLB.
}//page_remove()


// Invalidate a TLB entry, but only if the page tables being
// edited are the ones currently in use by the processor.
void tlb_invalidate(pde_t *pgdir, uintptr_t va)
{
	// Flush the entry only if we're modifying the current address space.
	// For now, there is only one address space, so always invalidate.
	invlpg(va);
}

physaddr_t
GetPhysicalAddress(pde_t *pgdir, uintptr_t va)
{
	return check_va2pa(pgdir, va);
}



// return the page structure of virtual address.
Page*
va2page(void* va){
	return &pages[check_va2pa(kern_pgdir, (uintptr_t)va)/PGSIZE];
}



// allocate a free page from kernel space([KERNBASE, 2^32]).
void *
alloc_free_page()
{
	static char *freeptr;
	struct Page *pp0, *pp1 = free_pages;
	void * va;

	if(freeptr == 0)
		freeptr = (char *)boot_alloc(0);

	while ((uintptr_t)freeptr <= 0xfffff000)
	{
		pp0 = &pages[check_va2pa(kern_pgdir, (uintptr_t)freeptr)/PGSIZE];
		if(pp0->pp_next == NULL)
			freeptr += PGSIZE;
		else
		{
			if(pp1 == pp0){
				pp0->pp_ref++;
				pp0->pp_next = NULL;
				va = freeptr;
				freeptr += PGSIZE;
				return va;
			}
			else{
				while(pp1->pp_next != pp0){
					if(pp1->pp_next == NULL)
						return NULL;
					pp1 = pp1->pp_next;
				}
				memset(pp0->data(), 0, PGSIZE);
				pp0->pp_ref++;
				pp1->pp_next = pp0->pp_next;
				pp0->pp_next = NULL;
				va = freeptr;
				freeptr += PGSIZE;
				return va;
			}
		}

	}
	panic("alloc_free_page: out of kernel space");
	return NULL;
}


// Map a free page at va, and initialized.
// This is a kernel function.
// Modified by zhumin in 2009-7-28.
int
page_map(uintptr_t va, int perm)
{
	struct Page * allocatepage = page_alloc();
	if(!allocatepage)
	{
		return -E_NO_MEM;
	}
	memset(allocatepage->data(), 0, PGSIZE);
	page_insert(kern_pgdir, allocatepage, va, perm);
	return 0;
}

// Unmap a free page at va.
// This is a kernel function.
// Modified by zhumin in 2009-7-28.
void
page_unmap(uintptr_t va)
{
	page_remove(kern_pgdir, va);
}


// Map [la, la+size) of linear address space to physical [pa, pa+size)
// in the kernel's page table 'kern_pgdir'.  Size is a multiple of PGSIZE.
// Use permission bits perm|PTE_P for the entries.
//
// This function resembles page_insert(), but is meant for use at boot time on
// reserved portions of physical memory.
// There's no need here to manage page reference counts or invalidate the TLB.
static void page_map_segment(pte_t *pgdir, uintptr_t la, size_t size, physaddr_t pa, int perm)
{
	pte_t 		*p_pte;	// pointer to PTE
	uintptr_t 	i;

	size = round_up(size, PGSIZE);	// Set size to a multiple of PGSIZE.
	// Map [la, la+size) of LA space to physical [pa, pa+size)
	for( i=0; i<size; i+=PGSIZE ) {
		p_pte = pgdir_walk(pgdir, la+i, 1);	// Get PTE's VA.
		*p_pte = (pa+i) | perm | PTE_P;		// Map PA to PTE.
	}//for
}//page_map_segment


