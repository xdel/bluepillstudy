/*
    SDL_stretch - Stretch Functions For The Simple DirectMedia Layer
    Copyright (C) 2003 Guido Draheim

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Guido Draheim, guidod@gmx.de
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: sdlcode.c $";
#endif

#include "config.h"
#include "SDL/SDL_error.h"
#include "SDL/SDL_video.h"
#include "SDL_stretchcode.h"
#include "SDL_stretchasm.h"

#ifdef SDL_STRETCH_DATA_NOEXEC
#ifdef __unix__
#include <unistd.h>
#include <sys/mman.h>
#endif
#include <malloc.h>
#endif

#ifdef SDL_STRETCH_CALL
#if defined SDL_STRETCH_I386 || defined SDL_STRETCH_X86_64
/*
 * in AMD64 "long mode" all traditional opcodes stay 32bit
 * and 64bit cousins have to be encoded via REX prefixes.
 * The only exception are PUSH/POP/RET opcodes that will
 * implicitly be 64bit. This makes it so the following
 * opcode settings work in both 32bit and 64bit mode but
 * for one case: the "INCSI" bit series is dropped and used
 * for the "REX" prefix opcodes in AMD64 long mode.
 */
#define iLOADSB   0xAC
#define iLOADSW   0xAD
#define iSTOSB    0xAA
#define iSTOSW    0xAB
#define iPUSHCX   0x51
#define iPOPCX    0x59
#define iINCSI    0x46
#define iXCHGCXAX 0x91
#define iPREFIX16 0x66
#define iRET      0xC3
/*
 * does XCHG yield a fast register renaming on P >= P3 ?
 */
# define PREP_1BYTE(dst)
# define PREP_2BYTE(dst)
# define PREP_3BYTE(dst)    { *dst++ = iPUSHCX; }
# define PREP_4BYTE(dst)
# define LOAD_1BYTE(dst)    {                     *dst++ = iLOADSB; }
# define LOAD_2BYTE(dst)    { *dst++ = iPREFIX16; *dst++ = iLOADSW; }
# define LOAD_3BYTE(dst)    {                     *dst++ = iLOADSB;   *dst++ = iXCHGCXAX; \
                              *dst++ = iPREFIX16; *dst++ = iLOADSW; }
# define LOAD_4BYTE(dst)    {                     *dst++ = iLOADSW; }
# define STORE_1BYTE(dst)   {                     *dst++ = iSTOSB; }
# define STORE_2BYTE(dst)   { *dst++ = iPREFIX16; *dst++ = iSTOSW; }
# define STORE_3BYTE(dst)   { *dst++ = iXCHGCXAX; *dst++ = iSTOSB;    *dst++ = iXCHGCXAX; \
                              *dst++ = iPREFIX16; *dst++ = iSTOSW; }
# define STORE_4BYTE(dst)   {                     *dst++ = iSTOSW; }
# define RETURN_1BYTE(dst)  {                     *dst++ = iRET; }
# define RETURN_2BYTE(dst)  {                     *dst++ = iRET; }
# define RETURN_3BYTE(dst)  { *dst++ = iPOPCX;    *dst++ = iRET; }
# define RETURN_4BYTE(dst)  {                     *dst++ = iRET; }
# ifdef SDL_STRETCH_I386
# define SKIP_1BYTE(dst)    {                     *dst++ = iINCSI; }
# define SKIP_2BYTE(dst)    { *dst++ = iINCSI;    *dst++ = iINCSI; }
# define SKIP_3BYTE(dst)    { *dst++ = iINCSI;    *dst++ = iINCSI;    *dst++ = iINCSI; }
#endif
#define HAVE_NATIVE_CODE
#endif
#endif

/* ASM i386 */
#ifndef HAVE_NATIVE_CODE
#define PREP_1BYTE(dst)
#define PREP_2BYTE(dst)
#define PREP_3BYTE(dst)
#define PREP_4BYTE(dst)
#define LOAD_1BYTE(dst)    { *dst++ = 001; }
#define LOAD_2BYTE(dst)    { *dst++ = 002; }
#define LOAD_3BYTE(dst)    { *dst++ = 003; }
#define LOAD_4BYTE(dst)    { *dst++ = 004; }
#define STORE_1BYTE(dst)   { *dst++ = 011; }
#define STORE_2BYTE(dst)   { *dst++ = 012; }
#define STORE_3BYTE(dst)   { *dst++ = 013; }
#define STORE_4BYTE(dst)   { *dst++ = 014; }
#define RETURN_1BYTE(dst)  { *dst++ = 0; }
#define RETURN_2BYTE(dst)  { *dst++ = 0; }
#define RETURN_3BYTE(dst)  { *dst++ = 0; }
#define RETURN_4BYTE(dst)  { *dst++ = 0; }
#define HAVE_INTERPRETED_CODE
#endif

#ifndef SKIP_1BYTE
#define SKIP_1BYTE(dst) LOAD_1BYTE(dst)
#endif
#ifndef SKIP_2BYTE
#define SKIP_2BYTE(dst) LOAD_2BYTE(dst)
#endif
#ifndef SKIP_3BYTE
#define SKIP_3BYTE(dst) LOAD_3BYTE(dst)
#endif
#ifndef SKIP_4BYTE
#define SKIP_4BYTE(dst) LOAD_4BYTE(dst)
#endif

#ifdef SDL_STRETCH_DATA_NOEXEC
unsigned char* SDL_TheRowStretchCode = NULL;
#else
unsigned char SDL_TheRowStretchCode[SDL_STRETCHCODE_BUFFERSIZE];
#endif

unsigned char* SDL_NewRowStretchCode (unsigned size) {
    if (size == 0) size = SDL_STRETCHCODE_BUFFERSIZE;
#   if defined SDL_STRETCH_DATA_NOEXEC && defined __unix__
    unsigned char* buffer = memalign(getpagesize(), SDL_STRETCHCODE_BUFFERSIZE);
    if (-1 == mprotect(buffer, SDL_STRETCHCODE_BUFFERSIZE, PROT_READ|PROT_WRITE|PROT_EXEC)) {
        free (buffer);
        SDL_SetError("SDL_TheRowStretchCode - can not allocate and make executable");
        return 0;
    }
#   else
    unsigned char* buffer = malloc(SDL_STRETCHCODE_BUFFERSIZE);
#   endif
    return buffer;
}

unsigned char* SDL_GetRowStretchCode (void) {
#ifdef SDL_STRETCH_DATA_NOEXEC
    if (SDL_TheRowStretchCode == NULL) {
        SDL_TheRowStretchCode = SDL_NewRowStretchCode(0);
    }
#endif
    return SDL_TheRowStretchCode;
}

unsigned char* SDL_SetRowStretchCode (int src_w, int dst_w, int bpp)
{
#ifdef SDL_STRETCH_DATA_NOEXEC
    if (SDL_TheRowStretchCode == NULL) {
        SDL_TheRowStretchCode = SDL_GetRowStretchCode();
    }
#endif
    return SDL_PutRowStretchCode(SDL_TheRowStretchCode,
				 sizeof(SDL_TheRowStretchCode),
				 src_w, dst_w, bpp);
}

/*
 * The computed stretch code represents a kind of Bresenham x/y line
 * algorithm where our stretching factor matches with the steepness.
 */
unsigned char* SDL_PutRowStretchCode(unsigned char* buf_ptr, int buf_len,
				  int src_w, int dst_w, int bpp)
{
    static struct {
	int bpp;
	int src_w;
	int dst_w;
    } last;

    int i = 0; int w = dst_w; int src = 0;
    unsigned char *buf = buf_ptr;
    unsigned char *buf_end = buf + buf_len;

    if (last.src_w == src_w && last.dst_w == dst_w && last.bpp == bpp)
    {   /* so, we do not need to regenerate the code buffer */
	return 0;
    }else{
	last.src_w = src_w; last.dst_w = dst_w; last.bpp = bpp;
    }

    switch (bpp)
    {
    case 4:
	PREP_4BYTE(buf);
	while(src<src_w) {
	    LOAD_4BYTE(buf);
	draw4:
	    STORE_4BYTE(buf);
	    --w; i += src_w;
	    if (! w) break;
	    if (buf >= buf_end) goto overflow;
	    if (i < dst_w) goto draw4;
	    i -= dst_w; src++;
	    while (i >= dst_w) { i -= dst_w; src++; SKIP_4BYTE(buf); }
	};
	RETURN_4BYTE(buf);
	break;
    case 3:
	PREP_3BYTE(buf);
	while(src<src_w) {
	    LOAD_3BYTE(buf);
	draw3:
	    STORE_3BYTE(buf);
	    --w; i += src_w;
	    if (! w) break;
	    if (buf >= buf_end) goto overflow;
	    if (i < dst_w) goto draw3;
	    i -= dst_w; src++;
	    while (i >= dst_w) { i -= dst_w; src++; SKIP_3BYTE(buf); }
	};
	RETURN_3BYTE(buf);
	break;
    case 2:
	PREP_2BYTE(buf);
	while(src<src_w) {
	    LOAD_2BYTE(buf);
	draw2:
	    STORE_2BYTE(buf);
	    --w; i += src_w;
	    if (! w) break;
	    if (buf >= buf_end) goto overflow;
	    if (i < dst_w) goto draw2;
	    i -= dst_w; src++;
	    while (i >= dst_w) { i -= dst_w; src++; SKIP_2BYTE(buf); }
	};
	RETURN_2BYTE(buf);
	break;
    case 1:
	PREP_1BYTE(buf);
	while(src<src_w) {
	    LOAD_1BYTE(buf);
	draw1:
	    STORE_1BYTE(buf);
	    --w; i += src_w;
	    if (! w) break;
	    if (buf >= buf_end) goto overflow;
	    if (i < dst_w) goto draw1;
	    i -= dst_w; src++;
	    while (i >= dst_w) { i -= dst_w; src++; SKIP_1BYTE(buf); }
	};
	RETURN_1BYTE(buf);
	break;
    default:
	SDL_SetError("ASM stretch of %d bytes isn't supported\n", bpp);
	return 0;
    }
#   if 0 /* debugging - gives a nice visual representation of the code */
    *buf = 0; buf = SDL_TheRowStretchCode; fprintf (stderr,"\n");
    while (*buf) {
	fprintf(stderr,"%c", (*buf++)&0x7F);
	if (! (((long)buf)&0x3F)) fprintf (stderr, "\n");
    }   fprintf(stderr,"(%iB=(%i->%i))\n", buf-SDL_TheRowStretchCode,
		src_w, dst_w);
#   endif
    return buf_ptr; /* SUCCESS */

 overflow:
    /* so, we did overflow - too late anyway */
    SDL_SetError("Copy buffer overflow");
    return 0;
}


void           SDL_RunRowStretchCode(unsigned char* code,
				     unsigned char* srcp,
				     unsigned char* dstp)
{
#  if defined HAVE_NATIVE_CODE && defined SDL_STRETCH_CALL
    SDL_STRETCH_CALL(code, srcp, dstp);
#  elif defined HAVE_INTERPRETED_CODE
    unsigned char a = 0, b = 0, c = 0, d= 0;
    while (1) {
        switch (*code++) {
        case 014: *dstp++ = a;
        case 013: *dstp++ = b;
        case 012: *dstp++ = c;
        case 011: *dstp++ = d;
        break;
        case 004: a = *srcp++;
        case 003: b = *srcp++;
        case 002: c = *srcp++;
        case 001: d = *srcp++;
        break;
        default:
            return;
        }
    }
#  else
#  error Need inline assembly for this compiler
#  endif
}

#define DEFINE_COPY_ROW(name, type)			 \
void name(type *src, int src_w, type *dst, int dst_w)	 \
{							 \
	int i = 0;  			                 \
 	int w = dst_w;					 \
	type pixel = 0;					 \
        type* src_max = src + src_w;                     \
  						         \
	while(src < src_max) {                           \
	    pixel = *src;                                \
	draw:                                            \
	    *dst++ = pixel;                              \
	    --w; i += src_w;                             \
	    if (! w) break;                              \
	    if (i < dst_w) goto draw;                    \
	    do { i -= dst_w; src++; } while (i >= dst_w);\
	}; return;                                       \
}
DEFINE_COPY_ROW(SDL_StretchRow1, Uint8)
DEFINE_COPY_ROW(SDL_StretchRow2, Uint16)
DEFINE_COPY_ROW(SDL_StretchRow4, Uint32)

/* The ASM code has a hard time to handle 24-bpp stretch blits */
void SDL_StretchRow3(Uint8 *src, int src_w, Uint8 *dst, int dst_w)
{
        int i = 0;
	int w = dst_w;
	Uint8 pixel [3];
        Uint8* src_max = src + src_w;

	while(src < src_max) {
	    pixel[0] = *src++;
	    pixel[1] = *src++;
	    pixel[2] = *src++;
	draw:
	    *dst++ = pixel[0];
	    *dst++ = pixel[1];
	    *dst++ = pixel[2];
	    --w ; i += src_w;
	    if (! w) break;
	    if (i < dst_w) goto draw; /* draw same pixel again */
	    do { i -= dst_w; src+=3; } while (i >= dst_w);
	}; return;
}

char* SDL_StretchRowInfo(void) {
    return "sdlstretch"
#ifdef SDL_STRETCH_I386
            "-i386"
#endif
#ifdef SDL_STRETCH_X86_64
            "-x86_64"
#endif
#ifdef HAVE_NATIVE_CODE
            "-native"
#endif
#ifdef HAVE_INTERPRETED_CODE
            "-interpreted"
#endif
#ifdef SDL_STRETCH_CALL
            "-call"
#endif
#ifdef SDL_STRETCH_DATA_NOEXEC
            "-noexec"
#endif
#ifdef SDL_STRETCH_DISABLE_ASM
            "-disabledasm"
#endif
#ifdef SDL_STRETCH_USE_ASM
           "-autodetectedasm"
#endif
#ifdef VERSION
            "-" VERSION
#else
            "-X"
#endif
    ;
}
