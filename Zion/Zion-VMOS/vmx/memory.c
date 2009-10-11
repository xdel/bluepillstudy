#include <inc/vmx/memory.h>
#include <inc/lib/malloc.h>
#include <inc/lib/stdlib.h>

extern size_t npages;			// Amount of physical memory (in pages)


ZVMSTATUS MmInitManager(uint32_t *pgdir,uint32_t *hostcr3)
{
   void *va;
   uint32_t pa,tmp;
   memcpy(hostcr3,pgdir,PGSIZE);

   for(uint32_t i=0; i<1024; i++)
   {
	   if(hostcr3[i]!=0)
	   {
		   va = MmAllocPages(1,&pa);
		   tmp = hostcr3[i];
		   tmp = tmp & 0xfffff000;
		   
		   memcpy(va,KADDR(tmp),PGSIZE); // 从物理地址找虚拟地址
		   hostcr3[i] = hostcr3[i] & 0xfff;
		   hostcr3[i] = hostcr3[i] | pa	;   
	   }
   }	
   return ZVMSUCCESS;
}
