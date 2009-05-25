#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <dos.h>

#define SNPROTECTOR_VERIFY	1000 //Used to tell the hypervisor to start run the target program

BOOLEAN bRegState; //Store the software registration state in the software side.

void RegSuccessful()
{
	printf("Thank you for your registration!\n");
}

void RegFailure()
{
	printf("Wrong SN\n");
}

typedef struct _Parameter
{
	PCHAR sUserName;
	PCHAR sSerialNumber;
} Parameter,*PParameter;

/**
 * Pass <pParameter> Argument to Context Counter
 * Return: The virtual address of result struct.
 * The content of the structure <pParameter> points to will be copied to the kernel memory.
 * <actionType>:either to be START_RECORD or END_RECORD or PING
 */
BOOLEAN __declspec(naked) VerifySN (PParameter pParameter) {
	__asm { 
		mov eax, SNPROTECTOR_VERIFY
		mov edx, pParameter
		cpuid
		ret
	}
}
int __cdecl main(int argc, char **argv) {
	Parameter passin;
	if (argc != 3) {
		printf ("Crackme <UserName> <S/N Key>\n");
		return 0;
	}

	//Construct Parameter struct
	passin.sUserName = argv[1];
	passin.sSerialNumber =argv[2];
	
	__try {
	bRegState = VerifySN(&passin); 
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		printf ("CPUDID caused exception");
		return 0;
	}
	
	//I am Cracker!!!
	bRegState = TRUE;

	//Output proper information in the client side.
	if(bRegState)
	{
		RegSuccessful();
	}
	else
	{
		RegFailure();
	}
	
	//To simulate a real commercial software
	while(bRegState)
	{
		printf("Work Work!\n");
		Sleep(2000);
	}
	//we won't permit it exit.
	while(1){}
	return 0;
}
