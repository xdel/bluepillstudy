#include <inc/lib/stdlib.h>
#include <inc/lib/string.h>

int 
atoi( char *str )
{
	int 	num = 0;

	for( int i=0; i<=strlen(str); i++ ) 
		if( str[i]>='0' && str[i]<='9')
			num = num * 10 + str[i] -'0';
		else if( str[0]=='-' && i==0 ) 
			num *= -1;
		else 
			return -1;

	return num;
}//atoi()


