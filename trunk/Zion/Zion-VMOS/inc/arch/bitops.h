#define 	BITS_PER_LONG 	32

static inline int 
test_bit(unsigned int nr, const volatile uint32_t *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &	(((unsigned long *)addr)[nr / BITS_PER_LONG])) != 0;
}//test_bit()
			

