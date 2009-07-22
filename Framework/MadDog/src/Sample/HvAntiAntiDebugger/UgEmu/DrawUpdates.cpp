/*
This file is part of UgDbg.
 
UgDbg is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
UgDbg is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with UgDbg.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "helper.h"
#include "split.h"

#ifdef b32BitBuild
DWORD lowerrange = 0;
DWORD upperrange = 0;
DWORD lowerrangedata = 0;
DWORD upperrangedata = 0;
#else
ULONGLONG lowerrange = 0;
ULONGLONG upperrange = 0;
ULONGLONG lowerrangedata = 0;
ULONGLONG upperrangedata = 0;
#endif

char szModNameSave[MAX_PATH];
char szSection[9];
char szModNameSaveData[MAX_PATH];
char szSectionData[9];
char OldFile[MAX_PATH];

// *****************************************************************
//  UPDATE LOG WINDOW
// 
// *****************************************************************
void UpdateLogWindow (RETBUFFER InFile,SDL_Surface *screen,Messages * Queue,int StartMessage, int LOGTEXT_Y_POS, int INPUTWINDOW_Y_POS, DebugStruc * DebugDataExchange, int WindowSizeX)
{
	int LineCounter = ((INPUTWINDOW_Y_POS - LOGTEXT_Y_POS)/LINE_H);

	if (LineCounter >= 2) {
		char HexBuff[1];
		int i = LOGTEXT_Y_POS;
		HexBuff[0] = 0x1E;
		BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i,192,192,192,"%c",HexBuff[0]);
		if (LineCounter >= 4) {
			HexBuff[0] = 0x18;
			BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i+10,192,192,192,"%c",HexBuff[0]);
			HexBuff[0] = 0x19;
			BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i+((LineCounter-2)*10),192,192,192,"%c",HexBuff[0]);
		}
		HexBuff[0] = 0x1F;
		BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i+((LineCounter-1)*10),192,192,192,"%c",HexBuff[0]);
	}

	while (Queue->NextEntry != 0) {
		if (Queue->MessageNr == StartMessage) {
			for (int i = 0;i < LineCounter;i++) {
				if ((Queue != 0) && (Queue->NextEntry != 0)) {
					//printf ("%s\n",Queue->Message);
					if (_strnicmp ((const char *)Queue->Message,"<COLOR>",7) != NULL) {
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN,LOGTEXT_Y_POS+i*LINE_H,192,192,192,"%s",Queue->Message);
					} else {
						Uint32 c_Highlighter = SDL_MapRGB(screen->format, 192, 192, 192); 
						Draw_FillRect (screen,RIGHT_MARGEIN,LOGTEXT_Y_POS+i*LINE_H-1,(*DebugDataExchange->WindowSizeX)-(RIGHT_MARGEIN*2),LINE_H,c_Highlighter);
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN,LOGTEXT_Y_POS+i*LINE_H,0,0,0,"%s",&Queue->Message[7]);
					}
				} else break;
				Queue = Queue->NextEntry;
			}
			break;
		}
		Queue = Queue->NextEntry;
	}
	
}


// *****************************************************************
//  DISPLAY MEMORY IN DATA WINDOW
// 
// *****************************************************************
#ifdef b32BitBuild
void UpdateMemory (SDL_Surface *screen, ULONG Offset, PROCESS_INFORMATION * PI, CONTEXT *CTX, RETBUFFER InFile, int DATA_Y_POS, int DISASSEMBLY_Y_POS, bool x64BitMode, int WindowSizeX)
#else
void UpdateMemory (SDL_Surface *screen, ULONGLONG Offset, PROCESS_INFORMATION * PI, CONTEXT *CTX, RETBUFFER InFile, int DATA_Y_POS, int DISASSEMBLY_Y_POS, bool x64BitMode, int WindowSizeX)
#endif
{
	unsigned char HexBuff[SIZE_OF_HEXBUFF+1];
	ULONG i = DATA_Y_POS;
	ULONG x = 0;
	ULONG u = 0;

	int LineCounter = (DISASSEMBLY_Y_POS - DATA_Y_POS) / 10;

	if (LineCounter >= 2) {
		HexBuff[0] = 0x1E;
		BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i,192,192,192,"%c",HexBuff[0]);
		if (LineCounter >= 4) {
			HexBuff[0] = 0x18;
			BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i+10,192,192,192,"%c",HexBuff[0]);
			HexBuff[0] = 0x19;
			BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i+((LineCounter-2)*10),192,192,192,"%c",HexBuff[0]);
		}
		HexBuff[0] = 0x1F;
		BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i+((LineCounter-1)*10),192,192,192,"%c",HexBuff[0]);
	}


	ZeroMemory (&HexBuff,SIZE_OF_HEXBUFF+1);
	// we need more dynamic
	// calc the lines to draw
	switch (x64BitMode)
	{	
		case false:
			for (int y = 0; y < LineCounter; y++) {
				bool QUESTIONMARKS = true;
				if (ReadMem (Offset+y*0x10,0x10,&HexBuff,PI) == true) {
					QUESTIONMARKS = false;
				}
				BiosTextOut (&InFile,screen,RIGHT_MARGEIN,i+y*10,192,192,192,"%4.4X:%8.8X",CTX->SegDs,Offset+x);
				u = 0;
				for (int z = 0; z < 16; z++) {
					if (QUESTIONMARKS == false)	{
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN+113+u,i+y*10,192,192,192,"%2.2X",HexBuff[z]);
					} else {
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN+113+u,i+y*10,192,192,192,"??");
					}
					u += 24;
				}
				u = 0;
				for (int z = 0; z < 16; z++) {
					if (isprint (HexBuff[z]) == NULL) {
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN+505+u,i+y*10,192,192,192,".");
					} else {
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN+505+u,i+y*10,192,192,192,"%c",HexBuff[z]);
					}
					u += 8;
				}
				x += 0x10;
			}
			break;
		case true:
			for (int y = 0; y < LineCounter; y++) {
				bool QUESTIONMARKS = true;
				if (ReadMem (Offset+y*0x10,0x10,&HexBuff,PI) == true) {
					QUESTIONMARKS = false;
				}
				BiosTextOut (&InFile,screen,RIGHT_MARGEIN,i+y*10,192,192,192,"%4.4X:%16.16llX",CTX->SegDs,Offset+x);
				u = 0;
				for (int z = 0; z < 16; z++) {
					if (QUESTIONMARKS == false)	{
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN+113+u+8*8,i+y*10,192,192,192,"%2.2X",HexBuff[z]);
					} else {
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN+113+u+8*8,i+y*10,192,192,192,"??");
					}
					u += 24;
				}
				u = 0;
				for (int z = 0; z < 16; z++) {
					if ((HexBuff[z] < 0x30) || (HexBuff[z] > 0x7F)) {
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN+505+u+8*8,i+y*10,192,192,192,".");
					} else {
						BiosTextOut (&InFile,screen,RIGHT_MARGEIN+505+u+8*8,i+y*10,192,192,192,"%c",HexBuff[z]);
					}
					u += 8;
				}
				x += 0x10;
			}
			break;
	}
	return;
}


// *****************************************************************
//  TEST IF BIT x IN DWORD y is set
// 
// *****************************************************************
BOOL BitTest (ULONG Value,ULONG Bit)
{

	BOOL bResult = false;
#ifdef b32BitBuild
	__asm
	{
		mov eax,Bit
		bt Value,eax
		jnc right
		mov bResult,1
right:
	}
#else
	if (bit_tst (&Value,Bit) != 0) return true;
#endif
	return bResult;
}

// *****************************************************************
//  Function: UpdateRegisters
// 
//  In: SDL_Surface
//  
//  Out: -
//
//	Use: Reads Context to get current register values
//       Draws Context values to screen
// *****************************************************************
void UpdateRegisters (SDL_Surface *screen,CONTEXT * CTX, CONTEXT * CTX_Compare, RETBUFFER InFile, int REGISTERS_Y_POS, bool x64BitMode)
{
	ULONG line_y_distance;
	ULONG line_x_distance;
	ULONG line_tmpy_distance;
	ULONG line_tmpx_eflag_distance;

	line_x_distance = RIGHT_MARGEIN;
	line_y_distance = REGISTERS_Y_POS;

	int R;
	int G;
	int B;

#ifdef b32BitBuild	
	line_tmpy_distance = line_y_distance;

	if (CTX->Eax != CTX_Compare->Eax) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance,line_tmpy_distance,R,G,B,"EAX=%8.8X",CTX->Eax);
	if (CTX->Ebx != CTX_Compare->Ebx) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8,line_tmpy_distance,R,G,B,"EBX=%8.8X",CTX->Ebx);
	if (CTX->Ecx != CTX_Compare->Ecx) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"ECX=%8.8X",CTX->Ecx);
	if (CTX->Edx != CTX_Compare->Edx) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"EDX=%8.8X",CTX->Edx);
	if (CTX->Esi != CTX_Compare->Esi) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"ESI=%8.8X",CTX->Esi);
	line_tmpy_distance += 9;
	if (CTX->Edi != CTX_Compare->Edi) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance,line_tmpy_distance,R,G,B,"EDI=%8.8X",CTX->Edi);
	if (CTX->Ebp != CTX_Compare->Ebp) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8,line_tmpy_distance,R,G,B,"EBP=%8.8X",CTX->Ebp);
	if (CTX->Esp != CTX_Compare->Esp) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"ESP=%8.8X",CTX->Esp);
	if (CTX->Eip != CTX_Compare->Eip) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"EIP=%8.8X",CTX->Eip);


	// DISPLAY EFlags right in softice style
	line_tmpx_eflag_distance = line_x_distance+480;

	if (BitTest (CTX->EFlags,OVERFLOW) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"o"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"O"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,DIRECTION) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"d"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"D"); 
	}
	line_tmpx_eflag_distance += 16;

	if (BitTest (CTX->EFlags,INTERRUPT) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"i"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"I"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,SIGN) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"s"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"S"); 
	}
	line_tmpx_eflag_distance += 16;

	if (BitTest (CTX->EFlags,ZERO) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"z"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"Z"); 
	}

	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,ADJUST) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"a"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"A"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,PARITY) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"p"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"P"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,CARRY) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"c"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"C"); 
	}
 	line_tmpy_distance += 9;
	BiosTextOut (&InFile,screen,line_x_distance,line_tmpy_distance,192,192,192,"CS=%4.4X   DS=%4.4X   SS=%4.4X   ES=%4.4X   FS=%4.4X   GS=%4.4X",CTX->SegCs,CTX->SegDs,CTX->SegSs,CTX->SegEs,CTX->SegFs,CTX->SegGs);
	line_tmpy_distance += 10;

#else
	line_tmpy_distance = line_y_distance;

	if (CTX->Rax != CTX_Compare->Rax) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance,line_tmpy_distance,R,G,B,"RAX=%16.16llX",CTX->Rax);
	line_x_distance += 64;
	if (CTX->Rbx != CTX_Compare->Rbx) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8,line_tmpy_distance,R,G,B,"RBX=%16.16llX",CTX->Rbx);
	line_x_distance += 64;
	if (CTX->Rcx != CTX_Compare->Rcx) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"RCX=%16.16llX",CTX->Rcx);
	line_x_distance += 64;
	if (CTX->Rdx != CTX_Compare->Rdx) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"RDX=%16.16llX",CTX->Rdx);
	line_tmpy_distance += 9;
	line_x_distance = RIGHT_MARGEIN;
	if (CTX->Rsi != CTX_Compare->Rsi) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance,line_tmpy_distance,R,G,B,"RSI=%16.16llX",CTX->Rsi);
	line_x_distance += 64;
	if (CTX->Rdi != CTX_Compare->Rdi) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8,line_tmpy_distance,R,G,B,"RDI=%16.16llX",CTX->Rdi);
	line_x_distance += 64;
	if (CTX->Rbp != CTX_Compare->Rbp) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"RBP=%16.16llX",CTX->Rbp);
	line_x_distance += 64;
	if (CTX->Rsp != CTX_Compare->Rsp) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"RSP=%16.16llX",CTX->Rsp);
	line_x_distance = RIGHT_MARGEIN;

	line_tmpy_distance += 9;
	if (CTX->R8 != CTX_Compare->R8) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance,line_tmpy_distance,R,G,B,"R08=%16.16llX",CTX->R8);
	line_x_distance += 64;
	if (CTX->R9 != CTX_Compare->R9) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8,line_tmpy_distance,R,G,B,"R09=%16.16llX",CTX->R9);
	line_x_distance += 64;
	if (CTX->R10 != CTX_Compare->R10) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"R10=%16.16llX",CTX->R10);
	line_x_distance += 64;
	if (CTX->R11 != CTX_Compare->R11) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"R11=%16.16llX",CTX->R11);

	line_tmpy_distance += 9;
	line_x_distance = RIGHT_MARGEIN;

	if (CTX->R12 != CTX_Compare->R12) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance,line_tmpy_distance,R,G,B,"R12=%16.16llX",CTX->R12);
	line_x_distance += 64;
	if (CTX->R13 != CTX_Compare->R13) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8,line_tmpy_distance,R,G,B,"R13=%16.16llX",CTX->R13);
	line_x_distance += 64;
	if (CTX->R14 != CTX_Compare->R14) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"R14=%16.16llX",CTX->R14);
	line_x_distance += 64;
	if (CTX->R15 != CTX_Compare->R15) { R=0;G=255;B=255; } else { R=192;G=192;B=192; }
	BiosTextOut (&InFile,screen,line_x_distance+8*3+12*8+8*3+12*8+8*3+12*8,line_tmpy_distance,R,G,B,"R15=%16.16llX",CTX->R15);

	line_x_distance = RIGHT_MARGEIN;
	if (CTX->Rip != CTX_Compare->Rip) {
		BiosTextOut (&InFile,screen,line_x_distance,line_tmpy_distance+9,0,255,255,"RIP=%16.16llX",CTX->Rip);
	} else {
		BiosTextOut (&InFile,screen,line_x_distance,line_tmpy_distance+9,192,192,192,"RIP=%16.16llX",CTX->Rip);
	}
	line_x_distance = RIGHT_MARGEIN;
	// DISPLAY EFlags
	line_tmpx_eflag_distance = line_x_distance+592;
	line_tmpy_distance += 9;
	if (BitTest (CTX->EFlags,OVERFLOW) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"o"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"O"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,DIRECTION) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"d"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"D"); 
	}
	line_tmpx_eflag_distance += 16;

	if (BitTest (CTX->EFlags,INTERRUPT) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"i"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"I"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,SIGN) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"s"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"S"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,ZERO) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"z"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"Z"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,ADJUST) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"a"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"A"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,PARITY) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"p"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"P"); 
	}
	line_tmpx_eflag_distance += 16;
	if (BitTest (CTX->EFlags,CARRY) == false) {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,192,192,192,"c"); 
	} else {
		BiosTextOut (&InFile,screen,line_tmpx_eflag_distance,line_tmpy_distance,0,255,255,"C"); 
	}
	BiosTextOut (&InFile,screen,line_x_distance+8*23,line_tmpy_distance,192,192,192,"CS=%4.4X DS=%4.4X SS=%4.4X ES=%4.4X FS=%4.4X GS=%4.4X",CTX->SegCs,CTX->SegDs,CTX->SegSs,CTX->SegEs,CTX->SegFs,CTX->SegGs);
	line_tmpy_distance += 10;
#endif
			
	return;
}

// *****************************************************************
//  Function: EnumerateSectionOffsetFromData
// 
//  In: SDL_Surface, DebugStruc * DebugDataExchange
//  
//  Out: -
//
//	Use: We want to display on the bottom line the current section
//		 name
//       
// *****************************************************************
#ifdef b32BitBuild
void EnumerateSectionOffsetFromData (SDL_Surface * screen,ULONG Offset,DebugStruc * DebugDataExchange,RETBUFFER InFile,int LINE_LOWER, int WindowSizeX)
#else
void EnumerateSectionOffsetFromData (SDL_Surface * screen,ULONGLONG Offset,DebugStruc * DebugDataExchange,RETBUFFER InFile,int LINE_LOWER, int WindowSizeX)
#endif
{
	HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
	bool Identified = false;
	unsigned char TempBuffer[0x1000];
	Uint32 c_SiceGreen = SDL_MapRGB(screen->format, 0, 120, 0);

	if ((Offset < lowerrangedata) || (Offset > upperrangedata) || (strlen (szSectionData) == 0)) {
		memset (szSectionData,0,_countof(szSectionData));
		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, DebugDataExchange->dwProcessId );
		if (NULL == hProcess) {
			Draw_Line (screen,(Sint16) (5*8+RIGHT_MARGEIN), LINE_LOWER+4,(WindowSizeX)-RIGHT_MARGEIN,LINE_LOWER+4, c_SiceGreen);
			return;
		}

		if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
			for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ ) {
				TCHAR szModName[MAX_PATH];
				if ( GetModuleFileNameEx( hProcess, hMods[i], szModName,sizeof(szModName) / sizeof(TCHAR))) {
#ifdef b32BitBuild
					if (ReadMem ((ULONG)hMods[i],0x1000,&TempBuffer,&DebugDataExchange->ProcessInfo)) {
#else
					if (ReadMem ((ULONGLONG)hMods[i],0x1000,&TempBuffer,&DebugDataExchange->ProcessInfo)) {
#endif
						PIMAGE_DOS_HEADER DosHdr = (PIMAGE_DOS_HEADER) (BYTE*)&TempBuffer;
						PIMAGE_NT_HEADERS NtHdr = (PIMAGE_NT_HEADERS) (BYTE*)(&TempBuffer[DosHdr->e_lfanew]);
						PIMAGE_SECTION_HEADER SecHdr = (PIMAGE_SECTION_HEADER) (BYTE*)(&TempBuffer[DosHdr->e_lfanew+sizeof(IMAGE_NT_HEADERS)]);
						for (unsigned int j = 0; j < NtHdr->FileHeader.NumberOfSections; j++) {
#ifdef b32BitBuild
							if ((Offset >= (SecHdr->VirtualAddress+(DWORD)hMods[i])) && (Offset <= (SecHdr->VirtualAddress+(DWORD)hMods[i]+SecHdr->Misc.VirtualSize))) {
								lowerrangedata = SecHdr->VirtualAddress+(DWORD)hMods[i];
								upperrangedata = SecHdr->VirtualAddress+(DWORD)hMods[i]+SecHdr->Misc.VirtualSize;
#else
							if ((Offset >= (SecHdr->VirtualAddress+(ULONGLONG)hMods[i])) && (Offset <= (SecHdr->VirtualAddress+(ULONGLONG)hMods[i]+SecHdr->Misc.VirtualSize))) {
								lowerrangedata = SecHdr->VirtualAddress+(ULONGLONG)hMods[i];
								upperrangedata = SecHdr->VirtualAddress+(ULONGLONG)hMods[i]+SecHdr->Misc.VirtualSize;
#endif
								Identified = true;
								strcpy_s (szModNameSaveData,_countof(szModNameSaveData),szModName);
								memset (szSectionData,0,_countof(szSectionData));
								memcpy (szSectionData,SecHdr->Name,_countof(SecHdr->Name));
							}
							SecHdr++;
						}
					}
				}
			}
		}
		char Final[_MAX_PATH];
		memset (&Final,0,_countof(Final));
		if (Offset != NULL) {
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];
			_splitpath_s (szModNameSaveData,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
			sprintf_s (Final,_countof(Final),"%s!%s+%x",fname,szSectionData,Offset-lowerrangedata);
			if (Identified == true) {
				BiosTextOut (&InFile,screen,(5*8+RIGHT_MARGEIN),LINE_LOWER,0,120,0,"%s",Final);
			} else memset (&Final,0,_countof(Final));
		}
		// DRAW A LINE BETWEEN THE DISASSEMBLY AND THE LOG WINDOW
		Uint32 c_SiceGreen = SDL_MapRGB(screen->format, 0, 120, 0);
		Draw_Line (screen,(Sint16) (strlen (Final)*8)+(5*8+RIGHT_MARGEIN), LINE_LOWER+4,(WindowSizeX)-RIGHT_MARGEIN,LINE_LOWER+4, c_SiceGreen);

		CloseHandle( hProcess );
	} else {
		char Final[_MAX_PATH];
		memset (&Final,0,_countof(Final));
		if (Offset != NULL) {
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];
			_splitpath_s (szModNameSaveData,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
			sprintf_s (Final,_countof(Final),"%s!%s+%x",fname,szSectionData,Offset-lowerrangedata);
			BiosTextOut (&InFile,screen,(5*8+RIGHT_MARGEIN),LINE_LOWER,0,120,0,"%s",Final);
		}
		// DRAW A LINE BETWEEN THE DISASSEMBLY AND THE LOG WINDOW
		Draw_Line (screen,(Sint16) (strlen (Final)*8)+(5*8+RIGHT_MARGEIN), LINE_LOWER+4,(WindowSizeX)-RIGHT_MARGEIN,LINE_LOWER+4, c_SiceGreen);
	}
}


// *****************************************************************
//  Function: EnumerateSectionOffset
// 
//  In: SDL_Surface, DebugStruc * DebugDataExchange
//  
//  Out: -
//
//	Use: We want to display on the bottom line the current section
//		 name
//       
// *****************************************************************
void EnumerateSectionOffset (SDL_Surface * screen,CONTEXT * CTX,DebugStruc * DebugDataExchange,RETBUFFER InFile,int LINE_LOWER, int WindowSizeX)
{
	HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
	unsigned char TempBuffer[0x1000];
	bool Identified = false;
	Uint32 c_SiceGreen = SDL_MapRGB(screen->format, 0, 120, 0); 

#ifdef b32BitBuild
	if ((CTX->Eip < lowerrange) || (CTX->Eip > upperrange) || (strcmp (DebugDataExchange->FileName,OldFile) != NULL)) {
#else
	if ((CTX->Rip < lowerrange) || (CTX->Rip > upperrange) || (strcmp (DebugDataExchange->FileName,OldFile) != NULL)) {
#endif
		strcpy_s (OldFile,_countof(OldFile),DebugDataExchange->FileName);
		strcpy_s (szModNameSave,_countof(szModNameSave),DebugDataExchange->FileName);

		memset (szSection,0,_countof(szSection));

		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, DebugDataExchange->dwProcessId );
		if (NULL == hProcess) {
			// DRAW A LINE BETWEEN THE DISASSEMBLY AND THE LOG WINDOW
			Draw_Line (screen,(Sint16) (RIGHT_MARGEIN), LINE_LOWER+3,(WindowSizeX)-RIGHT_MARGEIN,LINE_LOWER+3, c_SiceGreen);
			return;
		}

		if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
			for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ ) {
				TCHAR szModName[MAX_PATH];
				if ( GetModuleFileNameEx( hProcess, hMods[i], szModName,sizeof(szModName) / sizeof(TCHAR))) {
#ifdef b32BitBuild
					if (ReadMem ((ULONG)hMods[i],0x1000,&TempBuffer,&DebugDataExchange->ProcessInfo)) {
#else
					if (ReadMem ((ULONGLONG)hMods[i],0x1000,&TempBuffer,&DebugDataExchange->ProcessInfo)) {
#endif
						PIMAGE_DOS_HEADER DosHdr = (PIMAGE_DOS_HEADER) (BYTE*)&TempBuffer;
						PIMAGE_NT_HEADERS NtHdr = (PIMAGE_NT_HEADERS) (BYTE*)(&TempBuffer[DosHdr->e_lfanew]);
						PIMAGE_SECTION_HEADER SecHdr = (PIMAGE_SECTION_HEADER) (BYTE*)(&TempBuffer[DosHdr->e_lfanew+sizeof(IMAGE_NT_HEADERS)]);
						for (unsigned int j = 0; j < NtHdr->FileHeader.NumberOfSections; j++) {
#ifdef b32BitBuild
							if ((CTX->Eip >= (SecHdr->VirtualAddress+(DWORD)hMods[i])) && (CTX->Eip <= (SecHdr->VirtualAddress+(DWORD)hMods[i]+SecHdr->Misc.VirtualSize))) {
								lowerrange = SecHdr->VirtualAddress+(DWORD)hMods[i];
								upperrange = SecHdr->VirtualAddress+(DWORD)hMods[i]+SecHdr->Misc.VirtualSize;
#else
							if ((CTX->Rip >= (SecHdr->VirtualAddress+(ULONGLONG)hMods[i])) && (CTX->Rip <= (SecHdr->VirtualAddress+(ULONGLONG)hMods[i]+SecHdr->Misc.VirtualSize))) {
								lowerrange = SecHdr->VirtualAddress+(ULONGLONG)hMods[i];
								upperrange = SecHdr->VirtualAddress+(ULONGLONG)hMods[i]+SecHdr->Misc.VirtualSize;
#endif
								Identified = true;
								strcpy_s (szModNameSave,_countof(szModNameSave),szModName);
								memset (szSection,0,_countof(szSection));
								memcpy (szSection,SecHdr->Name,_countof(SecHdr->Name));
							}
							SecHdr++;
						}
					}
				}
			}
		}

		char Final[_MAX_PATH];
		memset (&Final,0,_countof(Final));
#ifdef b32BitBuild
		if (CTX->Eip != NULL) {
#else
		if (CTX->Rip != NULL) {
#endif
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];
			_splitpath_s (szModNameSave,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
#ifdef b32BitBuild
			sprintf_s (Final,_countof(Final),"%s!%s+%x",fname,szSection,CTX->Eip-lowerrange);
#else
			sprintf_s (Final,_countof(Final),"%s!%s+%x",fname,szSection,CTX->Rip-lowerrange);
#endif
			if (Identified == true) {
				BiosTextOut (&InFile,screen,(WindowSizeX/2)-11,LINE_LOWER,0,120,0,"%s",Final);
			} else memset (&Final,0,_countof(Final));
		}
		// DRAW A LINE BETWEEN THE DISASSEMBLY AND THE LOG WINDOW
		Draw_Line (screen,(Sint16) (strlen (Final)*8)+(WindowSizeX/2)-11, LINE_LOWER+3,(WindowSizeX)-RIGHT_MARGEIN,LINE_LOWER+3, c_SiceGreen);
		CloseHandle( hProcess );
	} else {
		char Final[_MAX_PATH];
		memset (&Final,0,_countof(Final));
#ifdef b32BitBuild
		if (CTX->Eip != NULL) {
#else
		if (CTX->Rip != NULL) {
#endif
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];
			_splitpath_s (szModNameSave,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
#ifdef b32BitBuild
			sprintf_s (Final,_countof(Final),"%s!%s+%x",fname,szSection,CTX->Eip-lowerrange);
#else
			sprintf_s (Final,_countof(Final),"%s!%s+%x",fname,szSection,CTX->Rip-lowerrange);
#endif
			BiosTextOut (&InFile,screen,(WindowSizeX/2)-11,LINE_LOWER,0,120,0,"%s",Final);
		}
		// DRAW A LINE BETWEEN THE DISASSEMBLY AND THE LOG WINDOW
		Draw_Line (screen,(Sint16) (strlen (Final)*8)+(WindowSizeX/2)-11, LINE_LOWER+3,(WindowSizeX)-RIGHT_MARGEIN,LINE_LOWER+3, c_SiceGreen);
	}
}


// *****************************************************************
//  Function: UpdateDisassembly
// 
//  In: SDL_Surface, CONTEXT
//  
//  Out: -
//
//	Use: Disassembles from CTX->Eip in defined context
//       
// *****************************************************************
#ifdef b32BitBuild
void UpdateDisassembly (SDL_Surface *screen, CONTEXT * CTX, ULONG Offset, _DecodeType dt,bool instructionHex,RETBUFFER InFile, int DISASSEMBLY_Y_POS, int LOGTEXT_Y_POS, bool x64BitMode, DebugStruc * DebugDataExchange, int WindowSizeX)
#else
void UpdateDisassembly (SDL_Surface *screen, CONTEXT * CTX, ULONGLONG Offset, _DecodeType dt,bool instructionHex,RETBUFFER InFile, int DISASSEMBLY_Y_POS, int LOGTEXT_Y_POS, bool x64BitMode, DebugStruc * DebugDataExchange, int WindowSizeX)
#endif
{
	// Holds the result of the decoding.
	_DecodeResult res;
	// next is used for instruction's offset synchronization.
	// Decoded instruction information.
	_DecodedInst decodedInstructions[MAX_INSTRUCTIONS];
	// decodedInstructionsCount holds the count of filled instructions' array by the decoder.
	unsigned int decodedInstructionsCount = 0;
	// Default offset for buffer is 0, could be set in command line.
	_OffsetType offset = 0;

	memset (&decodedInstructions,0,sizeof (_DecodedInst) * MAX_INSTRUCTIONS);
	unsigned char InstructionBuffer[MAX_INSTRUCTIONS*16] = "";
	
#ifdef b32BitBuild
	ULONG OffsetTmp = Offset;
#else
	ULONGLONG OffsetTmp = Offset;
#endif

	for (int i = 0;i < ((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H)+1; i ++) {
		int OpcLen = (int)FetchOpcodeSize (OffsetTmp,DebugDataExchange);
		ReadMem (OffsetTmp,OpcLen,&InstructionBuffer[OffsetTmp-Offset],&DebugDataExchange->ProcessInfo);
		// RESTORE INT3
		if (InstructionBuffer[OffsetTmp-Offset] == 0xCC) {
			PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
			while (BpxTable->Offset != 0) {
				if (BpxTable->Offset == OffsetTmp) {
					// READ MAX
					ReadMem (OffsetTmp,16,&InstructionBuffer[OffsetTmp-Offset],&DebugDataExchange->ProcessInfo);
					InstructionBuffer[OffsetTmp-Offset] = BpxTable->OrigByte;
					distorm_decode(Offset, (const unsigned char*)&InstructionBuffer[OffsetTmp-Offset], 16, dt, decodedInstructions, 15, &decodedInstructionsCount);
					OpcLen = decodedInstructions[0].size;
				}
				BpxTable++;
			}			
		}
		OffsetTmp += OpcLen;
	}

	// Decode the buffer at given offset (virtual address).
	res = distorm_decode(Offset, (const unsigned char*)InstructionBuffer, (((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H)+1)*16, dt, decodedInstructions, MAX_INSTRUCTIONS, &decodedInstructionsCount);
	if (res == DECRES_INPUTERR) {
		// Null buffer? Decode type not 16/32/64?
		//printf("UGDBG: Distorm Error: Input error, halting!");
		return;
	}



	int ScrollFixup = 0;

	// IF WE WANT TO DISPLAY SYMBOLS WE SHOULD MANIPULATE THE decodedInstructions structure
	int LineCounter = ((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H);
	int LineCounterTemp = ((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H);

	for (int i = 0; i < LineCounter+1; i++) {
		_strupr_s ((char *)&decodedInstructions[i].instructionHex.p,60);
		decodedInstructions[i].instructionHex.p[17]=0;
		while (stringReplace (" ","",(char *)&decodedInstructions[i].instructionHex.p) != NULL);

		if (strstr ((const char *)decodedInstructions[i].operands.p,"0x") != NULL) {
			if ((_stricmp ((const char *)decodedInstructions[i].mnemonic.p,"MOV") == 0) || (_stricmp ((const char *)decodedInstructions[i].mnemonic.p,"PUSH") == 0)) {
				int argc;
				char* argv[128];
				char* linecopy;

				linecopy = _strdup((const char *)&decodedInstructions[i].operands.p);
				argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
				if(argc != 0) {
					for (int j = 0; j < argc; j++) {
						if ((argv[j][0] == '0') && (argv[j][1] == 'x')) {
#ifdef b32BitBuild
							ULONG Destination = 0;
							sscanf_s (&argv[j][2],"%x",&Destination);
#else
							ULONGLONG Destination = 0;
							sscanf_s (&argv[j][2],"%llx",&Destination);
#endif

							char MyBuf[21] = "";
							if (ReadMem (Destination,20,&MyBuf,&DebugDataExchange->ProcessInfo) == true) {
								int g = 0;
								while (isprint (MyBuf[g]) != 0) g++;
								MyBuf[g] = 0;

								//printf ("%s\n",MyBuf);
								if (strlen (MyBuf) >= 3) {
									strcat_s ((char *)&decodedInstructions[i].operands.p,60," ; \"");
									strcat_s ((char *)&decodedInstructions[i].operands.p,60,MyBuf);
									strcat_s ((char *)&decodedInstructions[i].operands.p,60,"\"");
								}
							}
						}
					}
					free(linecopy);
				}
			}
			
			while (stringReplace ("0x","",(char *)&decodedInstructions[i].operands.p) != NULL);
		}
		_strupr_s ((char *)&decodedInstructions[i].operands.p,60);

	}

	for (int i = 0; i < LineCounter; i++) {
		// CHECK ALL OFFSETS AGAINST SYMBOLS, IF FOUND INSERT IT
		PDllTable MyDllTable = (PDllTable)DebugDataExchange->DllTablePtr;
		if (MyDllTable != NULL) {
			bool SymbolFound = false;
			while (MyDllTable->FunctionsAvailable != NULL) {
				PApiTable MyApiTable = MyDllTable->Apis;
				if (MyApiTable != NULL) {
					for (unsigned int j=0;j<MyDllTable->FunctionsAvailable;j++) {
						if (MyApiTable->Offset == decodedInstructions[i].offset) {
							char ReplaceChar[MAX_PATH] = "";
							strcat_s (ReplaceChar,_countof(ReplaceChar),(const char *)MyDllTable->DllAscii);
							stringReplace (".DLL","",ReplaceChar);
							_strupr_s (ReplaceChar,_countof(ReplaceChar));
							strcat_s (ReplaceChar,_countof(ReplaceChar),"!");
							strcat_s (ReplaceChar,_countof(ReplaceChar),(const char *)MyApiTable->ApiAscii);
							strcat_s (ReplaceChar,_countof(ReplaceChar),"");
							ReplaceChar[35] = 0;

							// perform the shift
							_DecodedInst decodedInstructionsTemp[MAX_INSTRUCTIONS];
							for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
								memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
							}
							
							strcpy_s ((char *)&decodedInstructions[i].mnemonic.p,_countof(decodedInstructions[i].mnemonic.p),ReplaceChar);
							decodedInstructions[i].size = 0;
							decodedInstructions[i].instructionHex.p[0] = 0;
							decodedInstructions[i].operands.p[0] = 0;
							LineCounterTemp--;

							for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
								memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
							}
							i++;

							SymbolFound = true;
							break;
						}
						MyApiTable++;
					}
					if (SymbolFound == true) break;
				}
				if (SymbolFound == true) break;
				MyDllTable++;
			}
		}
	}


	// PDB SYMBOL LOOKUP (INSTRUCTION LABELS + CALLS / JMPS)
	if (DebugDataExchange->PDBLoaded == true) {
		for (int i = 0; i < LineCounter+1; i++) {
			char ReturnString[_MAX_PATH];
			memset (&ReturnString,0,_countof(ReturnString));
			if ((_strnicmp ((const char *)&decodedInstructions[i].instructionHex.p,"e8",1) == 0) || (_strnicmp ((const char *)&decodedInstructions[i].instructionHex.p,"e9",1) == 0)) {
#ifdef b32BitBuild
				ULONG OffsetToRead = 0;
				if (ReadMem ((ULONG)decodedInstructions[i].offset+1,4,&OffsetToRead,&DebugDataExchange->ProcessInfo) == true) {
					OffsetToRead += (ULONG)decodedInstructions[i].offset;
#else
				ULONGLONG OffsetToRead = 0;
				if (ReadMem (decodedInstructions[i].offset+1,4,&OffsetToRead,&DebugDataExchange->ProcessInfo) == true) {
					OffsetToRead += decodedInstructions[i].offset;
#endif
					OffsetToRead += 5; // Size of call/jmp
					OffsetToRead -= DebugDataExchange->ImageBase;
					memset (&ReturnString,0,_countof(ReturnString));
					bool bRes = FindSymbolByRva ((ULONG)OffsetToRead,(char *)&ReturnString,_countof(ReturnString));
					if (bRes == true) {
						strcpy_s ((char *)&decodedInstructions[i].operands.p,60,ReturnString);
					}
				}
			}


#ifdef b32BitBuild
			ULONG Offset = (ULONG)decodedInstructions[i].offset;
#else
			ULONGLONG Offset = decodedInstructions[i].offset;
#endif
			Offset -= DebugDataExchange->ImageBase;
			memset (&ReturnString,0,_countof(ReturnString));

			bool bRes = FindSymbolByRva ((ULONG)Offset,(char *)&ReturnString,_countof(ReturnString));
			if (bRes == true) {	
				// perform the shift
				_DecodedInst decodedInstructionsTemp[MAX_INSTRUCTIONS];
				for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
					memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
				}
				
				memcpy ((char *)&decodedInstructions[i].mnemonic.p,ReturnString,_countof(decodedInstructions[i].mnemonic.p));
				decodedInstructions[i].size = 0;
				decodedInstructions[i].instructionHex.p[0] = 0;
				decodedInstructions[i].operands.p[0] = 0;
				LineCounterTemp--;

				for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
					memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
				}
				i++;
			}


			ULONG bResLine = FindLineByRva ((ULONG)Offset,(char *)&ReturnString,_countof(ReturnString));

			if (bResLine != NULL) {
				for (unsigned int j = 0; j < strlen (ReturnString); j++) {
					if (isprint (ReturnString[j]) == 0) ReturnString[j] = ' ';
				}

				char EmptyChar [_MAX_PATH] = "";
				_DecodedInst decodedInstructionsTemp[MAX_INSTRUCTIONS];
				// perform the shift
				for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
					memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
				}
				
				strcpy_s ((char *)&decodedInstructions[i].mnemonic.p,_countof(decodedInstructions[i].mnemonic.p),EmptyChar);
				decodedInstructions[i].size = 0;
				decodedInstructions[i].instructionHex.p[0] = 0;
				decodedInstructions[i].operands.p[0] = 0;
				LineCounterTemp--;

				for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
					memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
				}
				i++;


				// perform the shift
				int LoopCtr = (int)strlen (ReturnString);
				int Shifter = 0;
				do {
					for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
						memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
					}
					
					memcpy ((char *)&decodedInstructions[i].mnemonic.p,&ReturnString[Shifter],_countof(decodedInstructions[i].mnemonic.p)-1);
					decodedInstructions[i].size = 0;
					decodedInstructions[i].instructionHex.p[0] = 0;
					decodedInstructions[i].operands.p[0] = 0;
					LineCounterTemp--;

					for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
						memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
					}
					i++;
					Shifter += 59;
					LoopCtr -= 59;
				} while (LoopCtr > 0);


				// perform the shift
				for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
					memcpy (&decodedInstructionsTemp[j],&decodedInstructions[j],sizeof (_DecodedInst));
				}
				
				strcpy_s ((char *)&decodedInstructions[i].mnemonic.p,_countof(decodedInstructions[i].mnemonic.p),EmptyChar);
				decodedInstructions[i].size = 0;
				decodedInstructions[i].instructionHex.p[0] = 0;
				decodedInstructions[i].operands.p[0] = 0;
				LineCounterTemp--;

				for (int j = i; j < (MAX_INSTRUCTIONS-LineCounter); j++) {
					memcpy (&decodedInstructions[j+1],&decodedInstructionsTemp[j],sizeof (_DecodedInst));
				}
				i++;
			}
		}
	}



	if (DebugDataExchange->AllowScrolling == false) {
		// THIS IS NEEDED FOR PROPER SCROLLING
		_OffsetType FirstOpcodeOnScreen = decodedInstructions[0].offset;
		_OffsetType LastOpcodeOnScreen = decodedInstructions[((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H)-1].offset;
		_OffsetType FollowingOpcodeOnScreen = decodedInstructions[((LOGTEXT_Y_POS-DISASSEMBLY_Y_POS)/LINE_H)].offset;

#ifdef b32BitBuild
		if (CTX->Eip == FollowingOpcodeOnScreen) {
			if (decodedInstructions[1].offset == decodedInstructions[0].offset) {
				DebugDataExchange->DisassembleFromOffset = (DWORD)decodedInstructions[2].offset;
			} else DebugDataExchange->DisassembleFromOffset = (DWORD)decodedInstructions[1].offset;
#else
		if (CTX->Rip == FollowingOpcodeOnScreen) {
			if (decodedInstructions[1].offset == decodedInstructions[0].offset) {
				DebugDataExchange->DisassembleFromOffset = (ULONGLONG)decodedInstructions[2].offset;
			} else DebugDataExchange->DisassembleFromOffset = (ULONGLONG)decodedInstructions[1].offset;
#endif
			ScrollFixup += 1;
		}  else {
#ifdef b32BitBuild
			if ((CTX->Eip < FirstOpcodeOnScreen) || (CTX->Eip > FollowingOpcodeOnScreen)) {
				DebugDataExchange->DisassembleFromOffset = CTX->Eip;
#else
			if ((CTX->Rip < FirstOpcodeOnScreen) || (CTX->Rip > FollowingOpcodeOnScreen)) {
				DebugDataExchange->DisassembleFromOffset = CTX->Rip;
#endif
			}
		}
	}



	for (int i = 0; i < LineCounter; i++) {
		//***********************************************************************************************
		//
		// LOOKUP SYMBOLS FOR FF15 CALLS AND FF25 JMPS (i.e. CALL [GetVersion]
		//
		//***********************************************************************************************
		if ((_strnicmp ((const char *)&decodedInstructions[i+ScrollFixup].instructionHex.p,"ff15",4) == 0) || (_strnicmp ((const char *)&decodedInstructions[i+ScrollFixup].instructionHex.p,"ff25",4) == 0)) {
#ifdef b32BitBuild
			ULONG OffsetToRead = 0;
			if (ReadMem ((ULONG)decodedInstructions[i+ScrollFixup].offset+2,4,&OffsetToRead,&DebugDataExchange->ProcessInfo) == true) {
				if (ReadMem (OffsetToRead,4,&OffsetToRead,&DebugDataExchange->ProcessInfo) == true) {
#else
			ULONGLONG OffsetToRead = 0;
			if (ReadMem (decodedInstructions[i+ScrollFixup].offset+2,4,&OffsetToRead,&DebugDataExchange->ProcessInfo) == true) {
				OffsetToRead += decodedInstructions[i+ScrollFixup].offset;
				OffsetToRead += 6; // Size of ff15 call
				if (ReadMem (OffsetToRead,8,&OffsetToRead,&DebugDataExchange->ProcessInfo) == true) {
#endif
					// check if it's an api first
					PDllTable MyDllTable = (PDllTable)DebugDataExchange->DllTablePtr;
					if (MyDllTable != NULL) {
						bool SymbolFound = false;
						while (MyDllTable->FunctionsAvailable != NULL) {
							PApiTable MyApiTable = MyDllTable->Apis;
							if (MyApiTable != NULL) {
								for (unsigned int j=0;j<MyDllTable->FunctionsAvailable;j++) {
									if (MyApiTable->Offset == OffsetToRead) {
										char ReplaceChar[MAX_PATH] = "";
										strcat_s (ReplaceChar,_countof(ReplaceChar),"[");
										strcat_s (ReplaceChar,_countof(ReplaceChar),(const char *)MyDllTable->DllAscii);
										stringReplace (".DLL","",ReplaceChar);
										strcat_s (ReplaceChar,_countof(ReplaceChar),"!");
										strcat_s (ReplaceChar,_countof(ReplaceChar),(const char *)MyApiTable->ApiAscii);
										ReplaceChar[35] = 0;
										strcat_s (ReplaceChar,_countof(ReplaceChar),"]");
										strcpy_s ((char *)&decodedInstructions[i+ScrollFixup].operands.p,60,ReplaceChar);
										SymbolFound = true;
										break;
									}
									MyApiTable++;
								}
								if (SymbolFound == true) break;
							}
							if (SymbolFound == true) break;
							MyDllTable++;
						}
					}
				}
			}
		}
	}


	if (LineCounter >= 2) {
		char HexBuff[1];
		int i = DISASSEMBLY_Y_POS;
		HexBuff[0] = 0x1E;
		BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i,192,192,192,"%c",HexBuff[0]);
		if (LineCounter >= 4) {
			HexBuff[0] = 0x18;
			BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i+10,192,192,192,"%c",HexBuff[0]);
			HexBuff[0] = 0x19;
			BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i+((LineCounter-2)*10),192,192,192,"%c",HexBuff[0]);
		}
		HexBuff[0] = 0x1F;
		BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),i+((LineCounter-1)*10),192,192,192,"%c",HexBuff[0]);
	}


	for (int i = 0; i < LineCounter; i++) {
		Uint32 c_Highlighter = SDL_MapRGB(screen->format, 192, 192, 192); 
		Uint8 R = 192; Uint8 G = 192; Uint8 B = 192;

		if (GetOption ("highlight") == true) {
			HighlightCode (&R,&G,&B,(char *)decodedInstructions[i+ScrollFixup].mnemonic.p);
		}


#ifdef b32BitBuild
		if ((DebugDataExchange->PollEvent == true) && (decodedInstructions[i+ScrollFixup].offset == CTX->Eip)) {
#else
		if ((DebugDataExchange->PollEvent == true) && (decodedInstructions[i+ScrollFixup].offset == CTX->Rip)) {
#endif
			Draw_FillRect (screen,RIGHT_MARGEIN,DISASSEMBLY_Y_POS+(i*LINE_H)-1,(*DebugDataExchange->WindowSizeX)-(RIGHT_MARGEIN*2),LINE_H,c_Highlighter);
			R = 0; G = 0; B = 160;


			// CHECK IF CURRENT EIP IS A CONDITIONAL JUMP IF YES THEN CHECK THE EFLAGS AND INSERT JUMP/NO JUMP STATE
			if (decodedInstructions[i+ScrollFixup].mnemonic.p[0] == 'J') {
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JA") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNBE") == NULL)) {
					int Carry = BitTest (CTX->EFlags,CARRY);
					int Zero = BitTest (CTX->EFlags,ZERO);
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p, (Carry == NULL && Zero == NULL) ? "(JUMP)" : "(NO JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JAE") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNB") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNC") == NULL)) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,CARRY) != NULL ? "(NO JUMP)" : "(JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JB") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNAE") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JC") == NULL)) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,CARRY) == NULL ? "(NO JUMP)" : "(JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JBE") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNA") == NULL)) {
					int Carry = BitTest (CTX->EFlags,CARRY);
					int Zero = BitTest (CTX->EFlags,ZERO);
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p, (Carry != NULL || Zero != NULL) ? "(JUMP)" : "(NO JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JG") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNLE") == NULL)) {
					int Zero = BitTest (CTX->EFlags,ZERO);
					int Overflow = BitTest (CTX->EFlags,OVERFLOW);
					int Sign = BitTest (CTX->EFlags,SIGN);
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p, ((Zero == NULL) && (Overflow == Sign)) ? "(JUMP)" : "(NO JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JGE") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNL") == NULL)) {
					int Overflow = BitTest (CTX->EFlags,OVERFLOW);
					int Sign = BitTest (CTX->EFlags,SIGN);
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p, (Overflow == Sign) ? "(JUMP)" : "(NO JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JL") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNGE") == NULL)) {
					int Overflow = BitTest (CTX->EFlags,OVERFLOW);
					int Sign = BitTest (CTX->EFlags,SIGN);
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p, (Overflow != Sign) ? "(JUMP)" : "(NO JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JLE") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNG") == NULL)) {
					int Zero = BitTest (CTX->EFlags,ZERO);
					int Overflow = BitTest (CTX->EFlags,OVERFLOW);
					int Sign = BitTest (CTX->EFlags,SIGN);
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p, (Zero != NULL || (Overflow != Sign)) ? "(JUMP)" : "(NO JUMP)");
				}
				if (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNZ") == NULL) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,ZERO) != NULL ? "(NO JUMP)" : "(JUMP)");
				} 
				if (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JZ") == NULL) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,ZERO) == NULL ? "(NO JUMP)" : "(JUMP)");
				}
				if (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNO") == NULL) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,OVERFLOW) != NULL ? "(NO JUMP)" : "(JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNP") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JPO") == NULL)) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,PARITY) != NULL ? "(NO JUMP)" : "(JUMP)");
				}
				if (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JNS") == NULL) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,SIGN) != NULL ? "(NO JUMP)" : "(JUMP)");
				}
				if (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JO") == NULL) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,OVERFLOW) == NULL ? "(NO JUMP)" : "(JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JP") == NULL) || (_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JPE") == NULL)) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,PARITY) == NULL ? "(NO JUMP)" : "(JUMP)");
				}
				if ((_stricmp((const char *)decodedInstructions[i+ScrollFixup].mnemonic.p,"JS") == NULL)) {
					sprintf_s ((char *)decodedInstructions[i+ScrollFixup].operands.p,60,"%-20s %s",decodedInstructions[i+ScrollFixup].operands.p,BitTest (CTX->EFlags,SIGN) == NULL ? "(NO JUMP)" : "(JUMP)");
				}
			}

			if (LineCounter >= 2) {
				char HexBuff[1];
				int u = DISASSEMBLY_Y_POS;
				if (i == 0) {
					HexBuff[0] = 0x1E;
					BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),u,R,G,B,"%c",HexBuff[0]);
				}
				if (i == LineCounter-1) {
					HexBuff[0] = 0x1F;
					BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),u+((LineCounter-1)*10),R,G,B,"%c",HexBuff[0]);
				}
				if (LineCounter >= 4) {
					if (i == 1) {
						HexBuff[0] = 0x18;
						BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),u+10,R,G,B,"%c",HexBuff[0]);
					}
					if (i == LineCounter-2) {
						HexBuff[0] = 0x19;
						BiosTextOut (&InFile,screen,WindowSizeX-1-(RIGHT_MARGEIN*2),u+((LineCounter-2)*10),R,G,B,"%c",HexBuff[0]);
					}
				}
			}
		}


		PBreakpointTable BpxTable = (PBreakpointTable)&BpxBuffer;
		while (BpxTable->Offset != 0) {
			if ((BpxTable->Offset == decodedInstructions[i+ScrollFixup].offset) && (BpxTable->Enabled == TRUE)) {
				if (B != 160) { R = 0; G = 255; B = 255; }
				break;
			}
			BpxTable++;
		}			

		if (strlen ((const char *)decodedInstructions[i+ScrollFixup].instructionHex.p) != 0) {
			switch (instructionHex) {
				case true:
					BiosTextOut (&InFile,screen,RIGHT_MARGEIN,DISASSEMBLY_Y_POS+(i*10),R,G,B,"%4.4X:%0*I64X %-20s %-9s%s%s",CTX->SegCs, dt != Decode64Bits ? 8 : 16, decodedInstructions[i+ScrollFixup].offset, (char*)decodedInstructions[i+ScrollFixup].instructionHex.p, (char*)decodedInstructions[i+ScrollFixup].mnemonic.p, decodedInstructions[i+ScrollFixup].operands.length != 0 ? " " : "", (char*)decodedInstructions[i+ScrollFixup].operands.p);
					break;
				default:
					BiosTextOut (&InFile,screen,RIGHT_MARGEIN,DISASSEMBLY_Y_POS+(i*10),R,G,B,"%4.4X:%0*I64X %-9s%s%s",CTX->SegCs, dt != Decode64Bits ? 8 : 16, decodedInstructions[i+ScrollFixup].offset, (char*)decodedInstructions[i+ScrollFixup].mnemonic.p, decodedInstructions[i+ScrollFixup].operands.length != 0 ? " " : "", (char*)decodedInstructions[i+ScrollFixup].operands.p);
			}
		} else {
			BiosTextOut (&InFile,screen,RIGHT_MARGEIN,DISASSEMBLY_Y_POS+(i*10),R,G,B,"%s",(char*)decodedInstructions[i+ScrollFixup].mnemonic.p);
		}
	}
}