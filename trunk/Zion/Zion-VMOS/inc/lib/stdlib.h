#ifndef __STDLIB_H
#define __STDLIB_H

#include <inc/types.h>

int64_t atoi( char *str);
void * memcpy (void *__dest, const void *__src, uint32_t size8_t );
int memicmp(void const *buf1, void const *buf2, unsigned int count);
void *	memset(void *dst, int c, size_t len);
void *	memmove(void *dst, const void *src, size_t len);
int	memcmp(const void *s1, const void *s2, size_t len);
void *	memfind(const void *s, int c, size_t len);

#endif 
