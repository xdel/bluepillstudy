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

#ifndef _SDL_STRETCHCODE_IMPLEMENTATION_
#define _SDL_STRETCHCODE_IMPLEMENTATION_

#ifdef __cplusplus
extern "C" {
#endif

/* development header - do not use */

#define SDL_STRETCHCODE_BUFFERSIZE 8192

/**
 * TheRowStretchCode is a shared buffer between Stretch-routines that
 * use no extra buffer-argument. You should call SDL_SetRowStretchCode
 * to fill this internal buffer and set a "call"-operation for your target
 * cpu to execute this static buffer. That is done for effiency
 * as the RowStretch is often called in a tight loop for each Row
 * in a rectengular stretch and it is best to not use a variable
 * argument with an indirect call or a function call that would
 * build up a callframe and release that callframe later.
 *
 * If you do not need that effiency, use PutRowStretchCode and RunRowStretchCode
 * which are also good in a multithreading environment. To allocate a new buffer
 * for usage with Put/Run you can use the NewRowStretchCode routine which is
 * also used on NX machines (e.g. AMD64) where the data segment is set to be
 * not-executable (in that case it will allocate from heap and use mprotect).
 * if the argument is 0 then a buffer of the default size is allocated. If the
 * buffer allocation (or mprotect) fails it will return NULL and SDL_SetError.
 */
extern unsigned char* SDL_GetRowStretchCode(void);

/** => SDL_GetRowStretchCode */
extern unsigned char* SDL_NewRowStretchCode(unsigned size);

/**
 * The SetRowStretchCode is a wrapper around PutRowStretchCode that
 * uses the Adress and Size of the shared SDL_TheRowStretchCode buffer.
 * The PutRowStretchCode will fill the given buffer with an assembler
 * stream that should be called with SDL_RunRowStretchCode. The
 * assembler stream is usually faster as all the scale decisions are
 * done in advance of the execution. This helps when a RunCode is
 * done multiple times with the same src_w/dst_w/bpp pair. All the
 * pixel-get and pixel-set calls are unrolled in that buffer. Therefore,
 * the buffer should be big enough - as a rule of thumb use a buffer
 * of size (src_w+dst_w)*5
 *
 * If PutCode or SetCode fails, a NULL is returned and SDL_SetError.
 * Otherwise, the start adress of the machine code buffer is returned,
 * which is also the input argument of PutCode and RunCode.
 */
unsigned char* SDL_SetRowStretchCode(int src_w, int dst_w, int bpp);
/** => SDL_SetRowStretchCode */
unsigned char* SDL_PutRowStretchCode(unsigned char* buffer, int buflen,
				     int src_w, int dst_w, int bpp);
/** => SDL_SetRowStretchCode */
void           SDL_RunRowStretchCode(unsigned char* buffer,
				     unsigned char* src,
				     unsigned char* dst);

/** => SDL_SetRowStretchCode
 * If SDL_SetRowStretchCode fails, this function must be used instead.
 * This function and its cousins are singular routines that work in
 * a tight loop to scale a single row. The number specifies the
 * byte-width of each pixel (it is not a bit-width!).
 */
void SDL_StretchRow1(Uint8  *src, int src_w, Uint8  *dst, int dst_w);
/** => SDL_SetRowStretchCode */
void SDL_StretchRow2(Uint16 *src, int src_w, Uint16 *dst, int dst_w);
/** => SDL_SetRowStretchCode */
void SDL_StretchRow3(Uint8  *src, int src_w, Uint8  *dst, int dst_w);
/** => SDL_SetRowStretchCode */
void SDL_StretchRow4(Uint32 *src, int src_w, Uint32 *dst, int dst_w);

/**
 * return some informative information.
 */
extern char* SDL_StretchRowInfo(void);

#ifdef __cplusplus
}
#endif
#endif
