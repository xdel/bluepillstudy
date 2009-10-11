#ifndef __STRING_H
#define __STRING_H

#include <inc/types.h>

int	strlen(const char *s);
int	strnlen(const char *s, size_t size);
char *	strcpy(char *dst, const char *src);
char * strcat(char *dst, char const *src);
char *	strncpy(char *dst, const char *src, size_t size);
size_t	strlcpy(char *dst, const char *src, size_t size);
int	strcmp(const char *s1, const char *s2);
int	strncmp(const char *s1, const char *s2, size_t size);
char *	strchr(const char *s, char c);
char *	strfind(const char *s, char c);

long	strtol(const char *s, char **endptr, int base);
int64_t str2num(char *str);
uint str2addr(char *str);

#endif 
