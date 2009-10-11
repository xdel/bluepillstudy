#include <inc/lib/stdlib.h>

int64_t 
atoi ( char *str )
{
	int64_t 	num = 0;
	uint 		i=0;
	uint8_t 	minus_flag = 0;

	if ( str[0]=='-' ) {
		minus_flag = 1;
		i++;
	}//if

	while ( str[i] != '\0' ) {
		if ( str[i]>='0' && str[i]<='9') {
			num = num * 10 + str[i] -'0';
		} else {
			return -1;
		}//if
		i++;
	}//while

	if ( minus_flag ) {
		num *= -1;
	}//if

	return num;
}//atoi()



void * 
memcpy (void *__dest, const void *__src, size_t size8_t )
{
	uint8_t *dest = (uint8_t *)__dest;
	const uint8_t *src = (const uint8_t *)__src;

	while ( size8_t-- > 0 ) {
		*dest++ = *src++;
	}//while
	
	return (void *)dest;
}//memcpy()



int 
memicmp ( void const *buf1, void const *buf2, unsigned int count )
{
	char const *p1 = (char const *)buf1;
	char const *p2 = (char const *)buf2;
	char c1 = 1, c2 = 1;
	do
	{
		c1 = *p1++; c2 = *p2++;
		if (c1 >= 'A' && c1 <= 'Z')
			c1 += 'a' - 'A';
		if (c2 >= 'A' && c2 <= 'Z')
			c2 += 'a' - 'A';
	} while (--count && c1 == c2 && c1 && c2);
	return c1 - c2;
}//memicmp()



void * 
memset ( void *v, int c, size_t n )
{
	char *p = (char *) v;
	int m = n;

	while (--m >= 0)
		*p++ = c;
	return v;
}//memset()



void * 
memmove ( void *dst, const void *src, size_t n )
{
	const char *s = (const char *) src;
	char *d = (char *) dst;

	if (s < d && s + n > d) {
		s += n;
		d += n;
		while (n-- > 0)
			*--d = *--s;
	} else
		while (n-- > 0)
			*d++ = *s++;

	return dst;
}//memmove()



int 
memcmp ( const void *v1, const void *v2, size_t n )
{
	const unsigned char *s1 = (const unsigned char *) v1;
	const unsigned char *s2 = (const unsigned char *) v2;

	while (n-- > 0) {
		if (*s1 != *s2)
			return *s1 - *s2;
		s1++, s2++;
	}
	return 0;
}//memcmp()



void * 
memfind ( const void *v, int c, size_t n )
{
	const unsigned char *s = (const unsigned char *) v;
	
	const unsigned char *ends = s + n;
	for (; s < ends; s++)
		if (*s == (unsigned char) c)
			break;
	return (void *) s;
}//memfind()

