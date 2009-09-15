/* Writen by zhumin in 2009-07-25. */

#include <inc/lib/stdio.h>
#include <inc/lib/malloc.h>
#include <inc/types.h>
#include <inc/kern/pmap.h>

/*
 * Simple malloc/free.
 *
 * Uses the address space to do most of the hard work.
 * The address space from mbegin to mend is scanned
 * in order. Pages are allocated, used to fill successive
 * malloc requests, and then left alone. Free decrements
 * a ref count maintained in the page; the page is freed
 * when the ref count hits zero.
 *
 * If we need to allocate a large amount (more than a page)
 * we can't put a ref count at the end of each page,
 * so we mark the pte entry with the bit PTE_CONTINUED.
 */

/*
 * Some more features we need:
 * must have a memory pool for storing memory fragment.
 * so this malloc/free function is not finished!!!
 */
enum
{
    MAXMALLOC = 1024*1024    /* max size of one allocated chunk */
};

extern size_t npages;			// Amount of physical memory (in pages)

//struct mem_pool_desc{
//	void					*page;
//	struct mem_pool_desc	*next;
//	void					*freeptr;
//	uint8_t				ref;
//	uint32_t				size;
//}

//struct mem_pool_desc *free_mem_pool = (struct mem_pool_desc *) 0;

#define PTE_CONTINUED 0x400

static uint8_t *mbegin = (uint8_t*) 0xc0000000;
static uint8_t *mend = (uint8_t*) 0xf0000000;
static uint8_t *mptr;

static int
isfree(void *v, size_t n)
{
    uintptr_t va = (uintptr_t) v;
    int32_t size = n;
    pte_t *pt = (pte_t*) KADDR(PTE_ADDR(kern_pgdir[PDX(va)]));

    for (; size > 0; va += PGSIZE, size -= PGSIZE)
        if (va >= (uintptr_t) mend
         || ((kern_pgdir[PDX(va)] & PTE_P) && (pt[PTX(va)] & PTE_P)))
            return 0;
    return 1;
}

void*
malloc(size_t n)
{
    uint32_t i, cont;
    uint32_t nwrap;
    uint32_t *ref;
    void *v;

    if (mptr == 0)
        mptr = mbegin;

    //n = round_up(n, 4);

    //if (n >= MAXMALLOC) /* larger than the size of malloc could allocate. */
    //    return 0;		/* return error. */

    if ((uintptr_t) mptr % PGSIZE){
     	/*
         * we're in the middle of a partially
         * allocated page - can we add this chunk?
         * ref is for allocating count.
         */
        ref =(uintptr_t *)&va2page(mptr)->alloc_ref;
        if ((uintptr_t) mptr / PGSIZE == (uintptr_t) (mptr + n - 1) / PGSIZE) {
            (*ref)++;
            v = mptr;
            mptr += n;
            return v;
        }
        /*
         * stop working on this page and move on.
         */
        free(mptr);    /* drop reference to this page */
        mptr = round_down(mptr + PGSIZE, PGSIZE);
    }

    /*
     * now we need to find some address space for this chunk.
     * if it's less than a page we leave it open for allocation.
     * runs of more than a page can't have ref counts so we
     * flag the PTE entries instead.
     */
    nwrap = 0;
    while (1) {
        if (isfree(mptr, n))
            break;
        mptr += PGSIZE;
        if (mptr == mend) {
            mptr = mbegin;
            if (++nwrap == 2)
            {
            	cprintf("malloc:out of address space!\n");
				return 0;    /* out of address space */
            }
        }
    }

    /*
     * allocate at mptr - makes sure change alloc_ref count.
     */
    for (i = 0; i < n; i += PGSIZE){
        cont = (i + PGSIZE < n) ? PTE_CONTINUED : 0;
        if (page_map((uintptr_t)mptr + i, PTE_P|PTE_W|cont) < 0){
            for (; i >= 0; i -= PGSIZE)
                page_unmap((uintptr_t)mptr + i);
            cprintf("malloc:out of physical memory!\n");
            return 0;    /* out of physical memory */
        }
    }

    ref =(uintptr_t *)&va2page(mptr + i - PGSIZE)->alloc_ref;
    *ref = 1;    /* reference for mptr, reference for returned block */
    v = mptr;
    mptr += n;
    return v;
}

void
free(void *v)
{
    uint8_t *c;
    uint32_t *ref;
    pte_t *pt = (pte_t*) KADDR(PTE_ADDR(kern_pgdir[PDX(v)]));

    if (v == 0)
        return;
    assert(mbegin <= (uint8_t*) v && (uint8_t*) v < mend);

    c = round_down((uint8_t *)v, PGSIZE);

    while (pt[PTX(c)] & PTE_CONTINUED) {
        page_unmap((uintptr_t)c);
        c += PGSIZE;
        assert(mbegin <= c && c < mend);
    }

    /*
     * c is just a piece of this page, so dec the ref count
     * and maybe free the page.
     */
    ref = (uint32_t*)&va2page(c)->alloc_ref;
    if (--(*ref) == 0)
        page_unmap((uintptr_t)c);
}

void*
MmAllocPages(size_t page_num, uint32_t *physicaladdr)
{
	void * va;
	va = malloc(page_num * PGSIZE);
	if(physicaladdr != NULL)
		*physicaladdr = GetPhysicalAddress(kern_pgdir,(uint32_t)va);
	return va;
}

