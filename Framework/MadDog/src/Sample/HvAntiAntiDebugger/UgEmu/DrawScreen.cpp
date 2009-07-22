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
///
///
///
/// This File is in charge of drawing to the surface
///
///
///
///

#include "helper.h"

SDL_Surface * CharSurfaces[255];
RECT CurRect;

RECT * GetScreenPos (HWND hwnd)
{
	memset (&CurRect,0,sizeof(RECT));
	int bRes = 0;
	bRes = GetWindowRect (hwnd,&CurRect);
	if (bRes == 0) {
		return false;
	}
	return &CurRect;
}

void MoveDebuggerWindow (HWND hwnd, int X, int Y, int WindowSizeX, int WindowSizeY)
{
	RECT CurRect;
	memset (&CurRect,0,sizeof(RECT));
	int bRes = 0;
	bRes = GetWindowRect (hwnd,&CurRect);
	if (bRes == 0) {
		// push message to queue
		return;
	}
	bRes = MoveWindow (hwnd,CurRect.left + X,CurRect.top + Y,WindowSizeX,WindowSizeY,true);
	if (bRes == 0) {
		// push message to queue
		return;
	}
}

void CenterWindow (HWND hwnd, int WindowSizeX, int WindowSizeY)
{
	int DesktopSizeX = 0;
	int DesktopSizeY = 0;
	int bRes = 0;
	bRes = GetSystemMetrics (SM_CXSCREEN);
	if (bRes == 0) {
		// push message to queue
		return;
	}
	DesktopSizeX = bRes;
	bRes = GetSystemMetrics (SM_CYSCREEN);
	if (bRes == 0) {
		// push message to queue
		return;
	}
	DesktopSizeY = bRes;
	bRes = MoveWindow (hwnd,(DesktopSizeX/2)-(WindowSizeX/2),(DesktopSizeY/2)-(WindowSizeY/2),WindowSizeX,WindowSizeY,true);
	if (bRes == 0) {
		// push message to queue
		return;
	}
		
	return;
}

void Convert (BYTE Number,char * Output)
{
	unsigned char Template[9] = "00000000";
	_itoa_s (Number,(char *)Output,_countof (Template),2);
	strcpy_s ((char *)&Template[8-strlen ((const char *)Output)],_countof(Template),(const char *)Output);
	memcpy (Output,Template,8);
	return;
}

void DrawPixel(SDL_Surface *screen, int x, int y,Uint8 R, Uint8 G,Uint8 B)
{
    Uint32 color = SDL_MapRGB(screen->format, R, G, B);

    if ( SDL_MUSTLOCK(screen) ) 
	{
        if ( SDL_LockSurface(screen) < 0 ) 
		{
            return;
        }
    }
    switch (screen->format->BytesPerPixel) 
	{
        case 1: { /* vermutlich 8 Bit */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;

        case 2: { /* vermutlich 15 Bit oder 16 Bit */
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3: { /* langsamer 24-Bit-Modus, selten verwendet */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
            if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
                bufp[0] = color;
                bufp[1] = color >> 8;
                bufp[2] = color >> 16;
            } else {
                bufp[2] = color;
                bufp[1] = color >> 8;
                bufp[0] = color >> 16;
            }
        }
        break;

        case 4: { /* vermutlich 32 Bit */
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }
    if ( SDL_MUSTLOCK(screen) ) 
	{
        SDL_UnlockSurface(screen);
    }
    SDL_UpdateRect(screen, x, y, 1, 1);
}

void DrawTextPixels (SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G,Uint8 B,char * Output)
{	
	if (Output[0] == '1') DrawPixel (screen,0+x,y,R,G,B);
	if (Output[1] == '1') DrawPixel (screen,1+x,y,R,G,B);
	if (Output[2] == '1') DrawPixel (screen,2+x,y,R,G,B);
	if (Output[3] == '1') DrawPixel (screen,3+x,y,R,G,B);
	if (Output[4] == '1') DrawPixel (screen,4+x,y,R,G,B);
	if (Output[5] == '1') DrawPixel (screen,5+x,y,R,G,B);
	if (Output[6] == '1') DrawPixel (screen,6+x,y,R,G,B);
	if (Output[7] == '1') DrawPixel (screen,7+x,y,R,G,B);
}

char CharTable[256*8*8] = "";
int CharCounter[256];

SDL_Rect Source;
SDL_Rect Dest;

void InitCharTable (RETBUFFER * InFile,SDL_Surface *screen) {
	for (int i = 0; i < _countof(CharTable); i++) {
		CharTable[i] = '0';
	}
	PFont MyFont = (PFont) (BYTE *)InFile->FileOffset;

	for (int i = 0; i < _countof(CharCounter); i++) {
		CharCounter[i] = i;

		char Output [] = "00000000";

		Convert (MyFont[CharCounter[i]].Font1,Output);
		memcpy (&CharTable[i*64],Output,8);
		Convert (MyFont[CharCounter[i]].Font2,Output);
		memcpy (&CharTable[i*64+8],Output,8);
		Convert (MyFont[CharCounter[i]].Font3,Output);
		memcpy (&CharTable[i*64+8+8],Output,8);
		Convert (MyFont[CharCounter[i]].Font4,Output);
		memcpy (&CharTable[i*64+8+8+8],Output,8);
		Convert (MyFont[CharCounter[i]].Font5,Output);
		memcpy (&CharTable[i*64+8+8+8+8],Output,8);
		Convert (MyFont[CharCounter[i]].Font6,Output);
		memcpy (&CharTable[i*64+8+8+8+8+8],Output,8);
		Convert (MyFont[CharCounter[i]].Font7,Output);
		memcpy (&CharTable[i*64+8+8+8+8+8+8],Output,8);
		Convert (MyFont[CharCounter[i]].Font8,Output);
		memcpy (&CharTable[i*64+8+8+8+8+8+8+8],Output,8);

	}

	/*
	for (int i = 0; i < _countof(CharCounter); i++) {
		for (int j = 0; j < 8; j++) {
			for (int g = 0; g < 8; g++) {
				printf("%c",CharTable[i*64+j*8+g]);
			}
			printf ("\n");
		}
		printf ("\n");
	}
	*/

	
	Uint8 R,G,B;
	R = 192;G = 192;B = 192;

	for (int i = 0; i < 255; i++) {
		CharSurfaces[i] = SDL_CreateRGBSurface (screen->flags,8,8,32,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
		Uint32 c_Black = SDL_MapRGB(CharSurfaces[i]->format, 0, 0, 0); 
		int x = 0;
		int y = 0;
		DrawTextPixels (CharSurfaces[i],x,y,R,G,B,&CharTable[i*64]);
		DrawTextPixels (CharSurfaces[i],x,y+1,R,G,B,&CharTable[i*64+8]);
		DrawTextPixels (CharSurfaces[i],x,y+2,R,G,B,&CharTable[i*64+16]);
		DrawTextPixels (CharSurfaces[i],x,y+3,R,G,B,&CharTable[i*64+24]);
		DrawTextPixels (CharSurfaces[i],x,y+4,R,G,B,&CharTable[i*64+32]);
		DrawTextPixels (CharSurfaces[i],x,y+5,R,G,B,&CharTable[i*64+40]);
		DrawTextPixels (CharSurfaces[i],x,y+6,R,G,B,&CharTable[i*64+48]);
		DrawTextPixels (CharSurfaces[i],x,y+7,R,G,B,&CharTable[i*64+56]);

		SDL_SetColorKey(CharSurfaces[i], SDL_SRCCOLORKEY, c_Black);
	}
	Source.h = 8;
	Source.w = 8;
	Source.x = 0;
	Source.y = 0;
	Dest.h = 8;
	Dest.w = 8;

}

void BiosTextOut (RETBUFFER * InFile,SDL_Surface *screen, int x, int y, Uint8 R, Uint8 G,Uint8 B,char * Text,...)
{
	char Buf[512];
	va_list ap;
	va_start(ap, Text);
	vsprintf_s (Buf,512,Text,ap);
	va_end(ap);

	Dest.x = x;
	Dest.y = y;

	for (unsigned int i = 0; i < strlen (Buf);i ++)
	{
		// needs more investigation but blitting surfaces doesn't seem to give the expected speed increase
/*
		SDL_BlitSurface (CharSurfaces[Buf[i]],&Source,screen,&Dest);
		Dest.x += 8;
		*/

		DrawTextPixels (screen,x,y,R,G,B,&CharTable[Buf[i]*64]);
		DrawTextPixels (screen,x,y+1,R,G,B,&CharTable[Buf[i]*64+8]);
		DrawTextPixels (screen,x,y+2,R,G,B,&CharTable[Buf[i]*64+16]);
		DrawTextPixels (screen,x,y+3,R,G,B,&CharTable[Buf[i]*64+24]);
		DrawTextPixels (screen,x,y+4,R,G,B,&CharTable[Buf[i]*64+32]);
		DrawTextPixels (screen,x,y+5,R,G,B,&CharTable[Buf[i]*64+40]);
		DrawTextPixels (screen,x,y+6,R,G,B,&CharTable[Buf[i]*64+48]);
		DrawTextPixels (screen,x,y+7,R,G,B,&CharTable[Buf[i]*64+56]);

		x += 8;
	}
	return;
}





