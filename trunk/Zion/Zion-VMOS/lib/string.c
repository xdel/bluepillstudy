// Basic string routines.

#include <inc/lib/string.h>
#include <inc/lib/stdlib.h>

int
strlen ( const char *s )
{
	int n;

	for ( n = 0; *s != '\0'; s++ )
		n++;
	return n;
}//strlen()



int
strnlen ( const char *s, size_t size )
{
	int n;

	for ( n = 0; size > 0 && *s != '\0'; s++, size-- )
		n++;
	return n;
}//strnlen()



char *
strcpy ( char *dst, const char *src )
{
	char *ret = dst;

	while ( (*dst++ = *src++) != '\0' )
		/* do nothing */;
	return ret;
}//strcpy()



char *
strncpy ( char *dst, const char *src, size_t size )
{
	char *ret = dst;

	for (size_t i = 0; i < size; i++) {
		*dst++ = *src;
		// If strlen(src) < size, null-pad 'dst' out to 'size' chars
		if (*src != '\0')
			src++;
	}
	return ret;
}//strncpy()



size_t
strlcpy ( char *dst, const char *src, size_t size )
{
	char *dst_in = dst;

	if (size > 0) {
		while (--size > 0 && *src != '\0')
			*dst++ = *src++;
		*dst = '\0';
	}
	return dst - dst_in;
}//strlcpy()



int
strcmp ( const char *p, const char *q )
{
	while (*p && *p == *q)
		p++, q++;
	return (unsigned char) *p - (unsigned char) *q;
}//strcmp()



int
strncmp ( const char *p, const char *q, size_t n )
{
	while (n > 0 && *p && *p == *q)
		n--, p++, q++;
	if (n == 0)
		return 0;
	else
		return (unsigned char) *p - (unsigned char) *q;
}//strncmp()



// Return a pointer to the first occurrence of 'c' in 's',
// or a null pointer if the string has no 'c'.
char *
strchr ( const char *s, char c )
{
	for (; *s; s++)
		if (*s == c)
			return (char *) s;
	return 0;
}//strchr()



// Return a pointer to the first occurrence of 'c' in 's',
// or a pointer to the string-ending null character if the string has no 'c'.
char *
strfind ( const char *s, char c )
{
	for (; *s; s++)
		if (*s == c)
			break;
	return (char *) s;
}//strfind()



char * 
strcat ( char *dst, char const *src )
{
	while ( *dst );
		++dst;
	return strcpy(dst, src);
}//strcat()



long 
strtol ( const char *s, char **endptr, int base )
{
	int neg = 0;
	long val = 0;

	// gobble initial whitespace
	while (*s == ' ' || *s == '\t')
		s++;

	// plus/minus sign
	if (*s == '+')
		s++;
	else if (*s == '-')
		s++, neg = 1;

	// hex or octal base prefix
	if ((base == 0 || base == 16) && (s[0] == '0' && s[1] == 'x'))
		s += 2, base = 16;
	else if (base == 0 && s[0] == '0')
		s++, base = 8;
	else if (base == 0)
		base = 10;

	// digits
	while (1) {
		int dig;

		if (*s >= '0' && *s <= '9')
			dig = *s - '0';
		else if (*s >= 'a' && *s <= 'z')
			dig = *s - 'a' + 10;
		else if (*s >= 'A' && *s <= 'Z')
			dig = *s - 'A' + 10;
		else
			break;
		if (dig >= base)
			break;
		s++, val = (val * base) + dig;
		// we don't properly detect overflow!
	}

	if (endptr)
		*endptr = (char *) s;
	return (neg ? -val : val);
}//strtol()



int64_t 
str2num ( char *str )
{ 
	return atoi(str); 
}//str2num()



uint 
str2addr (char *str)
{
	uint 		i=2;
	uint 	addr=0;
	
	if ( str[0] != '0' || str[1] != 'x') {
		return -1;
	}//if

	while ( str[i] != '\0' ) {
		switch ( str[i] ) {
			case 'f': case 'e': case 'd': case 'c': case 'b': case 'a':
				addr = addr * 16 + ((int)str[i]-'a' + 10);
				i++;
				break;
			default:
				addr = addr * 16 + ((int)str[i]-'0');
				i++;
				break;
		}//switch
	}//while
	
	return addr;
}//str2addr()
