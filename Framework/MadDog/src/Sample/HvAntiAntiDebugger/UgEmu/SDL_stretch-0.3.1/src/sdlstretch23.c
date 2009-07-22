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

/* routines that stretch by constant factor (3 / 2) = 150% */

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: sdstretch.c $";
#endif

#include "SDL/SDL_error.h"
#include "SDL/SDL_video.h"
#include "SDL_stretch.h"
#include "SDL_stretchasm.h"

#if 0
/* the first part is for documentation only */

/* Perform a stretch blit between two surfaces of the same format.
   NOTE:  This function is not safe to call from multiple threads!
*/
int SDL_StretchSurface_23(SDL_Surface *src, SDL_Rect *srcrect,
			  SDL_Surface *dst, SDL_Rect *dstrect)
{
    union { Uint8* b; Uint16* w; Uint32* x; } srcp, dstp, endp;
    unsigned int srci, dsti;
    SDL_Rect rect;
    SDL_Rect clip;
    const unsigned bpp = src->format->BytesPerPixel;

    if ( src->format->BitsPerPixel != dst->format->BitsPerPixel ) {
	SDL_SetError("Only works with same format surfaces");
	return(-1);
    }

    /* Verify the source blit rectangle - copy good values to "rect",
     * note that the input srcrect is clipped with the src->clip_rect
     * and not just the src-surface borders.
     */
    if ( srcrect ) {
	rect = *srcrect;
#if 0   /* for debugging, you might want to assume good srcrect values */
	clip.x = clip.y = 0; clip.h = src->h; clip.w = src->w;
#else   /* otherwise we test with the clprect as well */
	clip = src->clip_rect;

	if (rect.x > clip.x + clip.w ||
	    rect.x + rect.w < clip.x ||
	    rect.y > clip.y + clip.h ||
	    rect.y + rect.h < clip.y) return 1;
	if (rect.x < clip.x) { rect.w -= clip.x-rect.x; rect.x = clip.x; }
	if (rect.w > clip.x + clip.w - rect.x)
	    rect.w = clip.x + clip.w - rect.x;
	if (rect.y < clip.y) { rect.h -= clip.y-rect.y; rect.y = clip.y; }
	if (rect.h > clip.y + clip.h - rect.y)
	    rect.h = clip.y + clip.h - rect.y;
#endif
	if (! rect.h || !rect.w) return 1;
    } else
    { /* in the default however, we use srcrect == cliprect == src-screen */
	rect.x = 0;
	rect.y = 0;
	rect.w = src->w;
	rect.h = src->h;
	clip = rect;
    }

    { /* and now compute the dest rect area - starting off with fixscaled */
	int dest_x = (rect.x - clip.x) * 3 / 2;
	int dest_y = (rect.y - clip.y) * 3 / 2;
	int dest_w = rect.w * 3 / 2;
	int dest_h = rect.h * 3 / 2;

	/* great, so we have the dest measure from src according to the scale
	 * but we need to check if that is still inside the dst->clip_rect
	 * and possibly adapt the source rect for it to work. Items in
	 * comments represent what a dest-only oriented routine would do.
	 */

	if (dest_x >= dst->clip_rect.x + dst->clip_rect.w ||
	    dest_y >= dst->clip_rect.y + dst->clip_rect.h ||
	    dest_x + dest_w <= dst->clip_rect.x ||
	    dest_y + dest_h <= dst->clip_rect.y) return 1;
	if (dest_x < dst->clip_rect.x)
	{ /*dest_w -= dst->clip_rect.x - dest_x; dest_x = dst->clip_rect.x; */
	    rect.w = (dest_w - dst->clip_rect.x - dest_x) / 3;
	    rect.x += (dst->clip_rect.x - dest_x) / 3 * 2;
	    dest_w = rect.w * 3; rect.w *= 2;    dest_x = dst->clip_rect.x; }
	if (dest_w > dst->clip_rect.x + dst->clip_rect.w - dest_x)
	{ /*dest_w = dst->clip_rect.x + dst->clip_rect.w - dest_x; */
	    rect.w = (dst->clip_rect.x + dst->clip_rect.w - dest_x) / 3;
	    dest_w = rect.w * 3; rect.w *= 2; }
	if (dest_y < dst->clip_rect.y)
	{ /*dest_h -= dst->clip_rect.y - dest_y; dest_y = dst->clip_rect.y; */
	    rect.h = (dest_h - dst->clip_rect.y - dest_y) / 3;
	    rect.y += (dst->clip_rect.y - dest_y) / 3 * 2;
	    dest_h = rect.h * 3; rect.h *= 2;    dest_y = dst->clip_rect.y; }
	if (dest_h > dst->clip_rect.y + dst->clip_rect.h - dest_y)
	{ /*dest_h = dst->clip_rect.y + dst->clip_rect.h - dest_y; */
	    rect.h = (dst->clip_rect.y + dst->clip_rect.h - dest_y) / 3;
	    dest_h = rect.h * 3; rect.h *= 2; }
	if (!dest_h || !dest_w || !rect.h || !rect.w) return 1;

	if (dstrect) {                  /* inform the caller of the     */
	    dstrect->y = dest_y;        /* computed results - if the    */
	    dstrect->x = dest_x;        /* "dst" surface is actually    */
	    dstrect->w = dest_w;        /* the user screen then one can */
	    dstrect->h = dest_h;        /* just use Update(dst,dstret)  */
	}

	/* these computations happen to be the same for all bitdepths */
	dstp.b = (Uint8*)dst->pixels +
	    (dst->pitch*dest_y) + (bpp*dest_x);
	srcp.b = (Uint8*)src->pixels +
	    (src->pitch*rect.y) + (bpp*rect.x);
	endp.b = srcp.b +
	    src->pitch*rect.h;
	srci = src->pitch - (bpp*rect.w);
	dsti = dst->pitch - (bpp*dest_w);
    }

    switch (bpp)  /* the copy operations shall be as tight as possible */
    {             /* to let all values be in cpu registers and the     */
    case 4:       /* complete series in the pipeline instruction cache */
	do {
	    int count = rect.w / 2;
	    do {
		*dstp.x++ = *srcp.x;
		*dstp.x++ = *srcp.x++;
		*dstp.x++ = *srcp.x++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	} while (srcp.b != endp.b);
	break;
    case 3:
	do {
	    int count = rect.w / 2;
	    do {
		*dstp.b++ = *srcp.b++;
		*dstp.w++ = *srcp.w; srcp.b--;
		*dstp.w++ = *srcp.w++;
		*dstp.x++ = *srcp.x++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	} while (srcp.b != endp.b);
	break;
    case 2:
	do {
	    int count = rect.w / 2;
	    do {
		*dstp.w++ = *srcp.w;
		*dstp.w++ = *srcp.w++;
		*dstp.w++ = *srcp.w++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	} while (srcp.b != endp.b);
	break;
    case 1:
	do {
	    int count = rect.w / 2;
	    do {
		*dstp.b++ = *srcp.b;
		*dstp.b++ = *srcp.b++;
		*dstp.b++ = *srcp.b++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	} while (srcp.b != endp.b);
	break;
    }
    return(0);
}

#elif 0
/* the second part shows a nice optimization:
 *
 *  we keep the rect.w and rect.h at halfsize - consequently,
 *  dest.w == 3 * rect.w and dest.h == 3 * src.w always - the
 *  copy-loops go in two src-lines -> three dst-lines anyway.
 *
 * and it does the fix the wrong behavior of the previous one,
 * if you try it then you will see that it does scale correctly
 * in the horizontal direction but the vertical does sometimes
 * seem to get messed up. That is because we did not yet unroll
 * the vertical part into being XY->XXY and instead it was kept
 * as XYz->XYz i.e. it would copy a bit too much out of the
 * source area instead of stretching it.
 */


/* Perform a stretch blit between two surfaces of the same format.
   NOTE:  This function is not safe to call from multiple threads!
*/
int SDL_StretchSurface_23(SDL_Surface *src, SDL_Rect *srcrect,
			  SDL_Surface *dst, SDL_Rect *dstrect)
{
    union { Uint8* b; Uint16* w; Uint32* x; } srcp, dstp, endp;
    unsigned int srci, dsti;
    SDL_Rect rect;
    SDL_Rect clip;
    const unsigned bpp = src->format->BytesPerPixel;

    if ( src->format->BitsPerPixel != dst->format->BitsPerPixel ) {
	SDL_SetError("Only works with same format surfaces");
	return(-1);
    }

    /* Verify the blit rectangles */
    if ( srcrect ) {
	rect = *srcrect;
#if 0
	clip.x = clip.y = 0; clip.h = src->h; clip.w = src->w;
#else
	clip = src->clip_rect;

	if (rect.x > clip.x + clip.w ||
	    rect.x + rect.w < clip.x ||
	    rect.y > clip.y + clip.h ||
	    rect.y + rect.h < clip.y) return 1;
	if (rect.x < clip.x) { rect.w -= clip.x-rect.x; rect.x = clip.x; }
	if (rect.w > clip.x + clip.w - rect.x)
	    rect.w = clip.x + clip.w - rect.x;
	if (rect.y < clip.y) { rect.h -= clip.y-rect.y; rect.y = clip.y; }
	if (rect.h > clip.y + clip.h - rect.y)
	    rect.h = clip.y + clip.h - rect.y;
#endif
	if (! rect.h || !rect.w) return 1;
	rect.x /= 2;     rect.w /= 2;
	rect.y /= 2;     rect.h /= 2;
	clip.x /= 2;     clip.w /= 2;
	clip.y /= 2;     clip.h /= 2;

    } else {
	rect.x = 0;
	rect.y = 0;
	rect.w = src->w / 2;
	rect.h = src->h / 2;
	clip = rect;
    }

    {
	int dest_x = (rect.x - clip.x) * 3;
	int dest_y = (rect.y - clip.y) * 3;
	int dest_w = rect.w * 3;
	int dest_h = rect.h * 3;

	if (dest_x >= dst->clip_rect.x + dst->clip_rect.w ||
	    dest_y >= dst->clip_rect.y + dst->clip_rect.h ||
	    dest_x + dest_w <= dst->clip_rect.x ||
	    dest_y + dest_h <= dst->clip_rect.y) return 1;
	if (dest_x < dst->clip_rect.x)
	{
	    rect.w = (dest_w - dst->clip_rect.x - dest_x) / 3;
	    rect.x += (dst->clip_rect.x - dest_x) / 3;
	    dest_w = rect.w * 3;         dest_x = dst->clip_rect.x; }
	if (dest_w > dst->clip_rect.x + dst->clip_rect.w - dest_x)
	{
	    rect.w = (dst->clip_rect.x + dst->clip_rect.w - dest_x) / 3;
	    dest_w = rect.w * 3;  }
	if (dest_y < dst->clip_rect.y)
	{
	    rect.h = (dest_h - dst->clip_rect.y - dest_y) / 3;
	    rect.y += (dst->clip_rect.y - dest_y) / 3;
	    dest_h = rect.h * 3;         dest_y = dst->clip_rect.y; }
	if (dest_h > dst->clip_rect.y + dst->clip_rect.h - dest_y)
	{
	    rect.h = (dst->clip_rect.y + dst->clip_rect.h - dest_y) / 3;
	    dest_h = rect.h * 3; }
	if (!dest_h || !dest_w || !rect.h || !rect.w) return 1;

	if (dstrect) { /*returnvalue*/
	    dstrect->y = dest_y;
	    dstrect->x = dest_x;
	    dstrect->w = dest_w;
	    dstrect->h = dest_h;
	}

	dstp.b = (Uint8*)dst->pixels +
	    (dst->pitch*dest_y) + (bpp*dest_x);
	srcp.b = (Uint8*)src->pixels +
	    (src->pitch*rect.y*2) + (bpp*rect.x*2);
	endp.b = srcp.b + src->pitch*rect.h*2;
	srci = src->pitch - (bpp*rect.w*2);
	dsti = dst->pitch - (bpp*dest_w);
    }

    switch (bpp)  /* the copy operations shall be as tight as possible */
    {             /* to let all values be in cpu registers and the     */
    case 4:       /* complete series in the pipeline instruction cache */
	do {
	    int count = rect.w;
	    do {
		*dstp.x++ = *srcp.x;
		*dstp.x++ = *srcp.x++;
		*dstp.x++ = *srcp.x++;
	    } while (--count != 0);
	    srcp.b -= rect.w*2*4;
	    dstp.b += dsti;
	    count = rect.w;
	    do {
		*dstp.x++ = *srcp.x;
		*dstp.x++ = *srcp.x++;
		*dstp.x++ = *srcp.x++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	    count = rect.w;
	    do {
		*dstp.x++ = *srcp.x;
		*dstp.x++ = *srcp.x++;
		*dstp.x++ = *srcp.x++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	} while (srcp.b != endp.b);
	break;
    case 3:
	do {
	    int count = rect.w;
	    do {
		*dstp.b++ = *srcp.b++;
		*dstp.w++ = *srcp.w; srcp.b--;
		*dstp.w++ = *srcp.w++;
		*dstp.x++ = *srcp.x++;
	    } while (--count != 0);
	    srcp.b -= rect.w*2*3;
	    dstp.b += dsti;
	     count = rect.w;
	    do {
		*dstp.b++ = *srcp.b++;
		*dstp.w++ = *srcp.w; srcp.b--;
		*dstp.w++ = *srcp.w++;
		*dstp.x++ = *srcp.x++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	    count = rect.w;
	    do {
		*dstp.b++ = *srcp.b++;
		*dstp.w++ = *srcp.w; srcp.b--;
		*dstp.w++ = *srcp.w++;
		*dstp.x++ = *srcp.x++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	} while (srcp.b != endp.b);
	break;
    case 2:
	do {
	    int count = rect.w;
	    do {
		*dstp.w++ = *srcp.w;
		*dstp.w++ = *srcp.w++;
		*dstp.w++ = *srcp.w++;
	    } while (--count != 0);
	    srcp.b -= rect.w*2*2;
	    dstp.b += dsti;
	    count = rect.w;
	    do {
		*dstp.w++ = *srcp.w;
		*dstp.w++ = *srcp.w++;
		*dstp.w++ = *srcp.w++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	    count = rect.w;
	    do {
		*dstp.w++ = *srcp.w;
		*dstp.w++ = *srcp.w++;
		*dstp.w++ = *srcp.w++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	} while (srcp.b != endp.b);
	break;
    case 1:
	do {
	    int count = rect.w;
	    do {
		*dstp.b++ = *srcp.b;
		*dstp.b++ = *srcp.b++;
		*dstp.b++ = *srcp.b++;
	    } while (--count != 0);
	    srcp.b -= rect.w*2*1;
	    dstp.b += dsti;
	    count = rect.w;
	    do {
		*dstp.b++ = *srcp.b;
		*dstp.b++ = *srcp.b++;
		*dstp.b++ = *srcp.b++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	    count = rect.w;
	    do {
		*dstp.b++ = *srcp.b;
		*dstp.b++ = *srcp.b++;
		*dstp.b++ = *srcp.b++;
	    } while (--count != 0);
	    srcp.b += srci;
	    dstp.b += dsti;
	} while (srcp.b != endp.b);
	break;
    }
    return(0);
}


#elif 0
/* the third part shows two extra optimizations:
 *
 * (1) the i386 is a desperatly register-starved architecture
 *     and for the case of GNUC we say now where to store the
 *     copyloop variables and we even cut in some assembler snippets.
 * (2) for that we better use copies of the srcp/dstp/endp pointers and
 *     therefore the initial computations can be done on Byte-pointers
 *     instead of the union-of-pointers as shown above for clarification.
 *
 *                                                              enough for UAE
 */


/* Perform a stretch blit between two surfaces of the same format.
   NOTE:  This function is not safe to call from multiple threads!
*/
int SDL_StretchSurface_23(SDL_Surface *src, SDL_Rect *srcrect,
			  SDL_Surface *dst, SDL_Rect *dstrect)
{
    Uint8 *srcp, *dstp, *endp;
    unsigned int srci, dsti;
    SDL_Rect rect;
    SDL_Rect clip;
    const unsigned bpp = src->format->BytesPerPixel;

    if ( src->format->BitsPerPixel != dst->format->BitsPerPixel ) {
	SDL_SetError("Only works with same format surfaces");
	return(-1);
    }

    /* Verify the blit rectangles */
    if ( srcrect ) {
	rect = *srcrect;
#if 0
	clip.x = clip.y = 0; clip.h = src->h; clip.w = src->w;
#else
	clip = src->clip_rect;

	if (rect.x > clip.x + clip.w ||
	    rect.x + rect.w < clip.x ||
	    rect.y > clip.y + clip.h ||
	    rect.y + rect.h < clip.y) return 1;
	if (rect.x < clip.x) { rect.w -= clip.x-rect.x; rect.x = clip.x; }
	if (rect.w > clip.x + clip.w - rect.x)
	    rect.w = clip.x + clip.w - rect.x;
	if (rect.y < clip.y) { rect.h -= clip.y-rect.y; rect.y = clip.y; }
	if (rect.h > clip.y + clip.h - rect.y)
	    rect.h = clip.y + clip.h - rect.y;
#endif
	if (! rect.h || !rect.w) return 1;
	rect.x /= 2;     rect.w /= 2;
	rect.y /= 2;     rect.h /= 2;
	clip.x /= 2;     clip.w /= 2;
	clip.y /= 2;     clip.h /= 2;

    } else {
	rect.x = 0;
	rect.y = 0;
	rect.w = src->w / 2;
	rect.h = src->h / 2;
	clip = rect;
    }

    {
	int dest_x = (rect.x - clip.x) * 3;
	int dest_y = (rect.y - clip.y) * 3;
	int dest_w = rect.w * 3;
	int dest_h = rect.h * 3;

	if (dest_x >= dst->clip_rect.x + dst->clip_rect.w ||
	    dest_y >= dst->clip_rect.y + dst->clip_rect.h ||
	    dest_x + dest_w <= dst->clip_rect.x ||
	    dest_y + dest_h <= dst->clip_rect.y) return 1;
	if (dest_x < dst->clip_rect.x)
	{
	    rect.w = (dest_w - dst->clip_rect.x - dest_x) / 3;
	    rect.x += (dst->clip_rect.x - dest_x) / 3;
	    dest_w = rect.w * 3;         dest_x = dst->clip_rect.x; }
	if (dest_w > dst->clip_rect.x + dst->clip_rect.w - dest_x)
	{
	    rect.w = (dst->clip_rect.x + dst->clip_rect.w - dest_x) / 3;
	    dest_w = rect.w * 3;  }
	if (dest_y < dst->clip_rect.y)
	{
	    rect.h = (dest_h - dst->clip_rect.y - dest_y) / 3;
	    rect.y += (dst->clip_rect.y - dest_y) / 3;
	    dest_h = rect.h * 3;         dest_y = dst->clip_rect.y; }
	if (dest_h > dst->clip_rect.y + dst->clip_rect.h - dest_y)
	{
	    rect.h = (dst->clip_rect.y + dst->clip_rect.h - dest_y) / 3;
	    dest_h = rect.h * 3; }
	if (!dest_h || !dest_w || !rect.h || !rect.w) return 1;

	if (dstrect) { /*returnvalue*/
	    dstrect->y = dest_y;
	    dstrect->x = dest_x;
	    dstrect->w = dest_w;
	    dstrect->h = dest_h;
	}

	dstp = (Uint8*)dst->pixels +
	    (dst->pitch*dest_y) + (bpp*dest_x);
	srcp = (Uint8*)src->pixels +
	    (src->pitch*rect.y*2) + (bpp*rect.x*2);
	endp = srcp + src->pitch*rect.h*2;
	srci = src->pitch - (bpp*rect.w*2);
	dsti = dst->pitch - (bpp*dest_w);
    }

#if defined __i386_ || defined __x86_64_
#define ASM_i386
#endif

# if defined __GNUC__ && defined ASM_i386
# define ECX __asm__("%ecx")
# define ESI __asm__("%esi")
# define EDI __asm__("%edi")
# else
# define ECX
# define ESI
# define EDI
# endif

    switch (bpp)
    {
    case 4:
    {
	register Uint32* dstP EDI = (Uint32*) dstp;
	register Uint32* srcP ESI = (Uint32*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/4;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 3:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2*3;
	    dstP += dsti;
	    count = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	    count = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    case 2:
    {
	register Uint16* dstP EDI = (Uint16*) dstp;
	register Uint16* srcP ESI = (Uint16*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/2;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 1:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp -= rect.w*2;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    }
    return 0;
}


#elif 1
/* a pentium-specific one: using PREFETCH - use with Athlon or Piii only.
 *
 * PREFETCHn prefetch into cache (Intel 24547112.pdf, page 649)
 * MOVNTI store dword using nontemporal hint (page 516)
 * generally, noticable effect is only seen with PREFETCHnta and MOVNTI
 * for memcpy implementations. However, in SDL we do often blit to a
 * surface that will also be used right away, so it is fine to keep it
 * in a (distant cache).
 */


/* Perform a stretch blit between two surfaces of the same format.
   NOTE:  This function is not safe to call from multiple threads!
*/
int SDL_StretchSurface_23(SDL_Surface *src, SDL_Rect *srcrect,
			  SDL_Surface *dst, SDL_Rect *dstrect)
{
    Uint8 *srcp, *dstp, *endp;
    unsigned int srci, dsti;
    SDL_Rect rect;
    SDL_Rect clip;
    const unsigned bpp = src->format->BytesPerPixel;

    if ( src->format->BitsPerPixel != dst->format->BitsPerPixel ) {
	SDL_SetError("Only works with same format surfaces");
	return(-1);
    }

    /* Verify the blit rectangles */
    if ( srcrect ) {
	rect = *srcrect;
#if 0
	clip.x = clip.y = 0; clip.h = src->h; clip.w = src->w;
#else
	clip = src->clip_rect;

	if (rect.x > clip.x + clip.w ||
	    rect.x + rect.w < clip.x ||
	    rect.y > clip.y + clip.h ||
	    rect.y + rect.h < clip.y) return 1;
	if (rect.x < clip.x) { rect.w -= clip.x-rect.x; rect.x = clip.x; }
	if (rect.w > clip.x + clip.w - rect.x)
	    rect.w = clip.x + clip.w - rect.x;
	if (rect.y < clip.y) { rect.h -= clip.y-rect.y; rect.y = clip.y; }
	if (rect.h > clip.y + clip.h - rect.y)
	    rect.h = clip.y + clip.h - rect.y;
#endif
	if (! rect.h || !rect.w) return 1;
	rect.x /= 2;     rect.w /= 2;
	rect.y /= 2;     rect.h /= 2;
	clip.x /= 2;     clip.w /= 2;
	clip.y /= 2;     clip.h /= 2;

    } else {
	rect.x = 0;
	rect.y = 0;
	rect.w = src->w / 2;
	rect.h = src->h / 2;
	clip = rect;
    }

    {
	int dest_x = (rect.x - clip.x) * 3;
	int dest_y = (rect.y - clip.y) * 3;
	int dest_w = rect.w * 3;
	int dest_h = rect.h * 3;

	if (dest_x >= dst->clip_rect.x + dst->clip_rect.w ||
	    dest_y >= dst->clip_rect.y + dst->clip_rect.h ||
	    dest_x + dest_w <= dst->clip_rect.x ||
	    dest_y + dest_h <= dst->clip_rect.y) return 1;
	if (dest_x < dst->clip_rect.x)
	{
	    rect.w = (dest_w - dst->clip_rect.x - dest_x) / 3;
	    rect.x += (dst->clip_rect.x - dest_x) / 3;
	    dest_w = rect.w * 3;         dest_x = dst->clip_rect.x; }
	if (dest_w > dst->clip_rect.x + dst->clip_rect.w - dest_x)
	{
	    rect.w = (dst->clip_rect.x + dst->clip_rect.w - dest_x) / 3;
	    dest_w = rect.w * 3;  }
	if (dest_y < dst->clip_rect.y)
	{
	    rect.h = (dest_h - dst->clip_rect.y - dest_y) / 3;
	    rect.y += (dst->clip_rect.y - dest_y) / 3;
	    dest_h = rect.h * 3;         dest_y = dst->clip_rect.y; }
	if (dest_h > dst->clip_rect.y + dst->clip_rect.h - dest_y)
	{
	    rect.h = (dst->clip_rect.y + dst->clip_rect.h - dest_y) / 3;
	    dest_h = rect.h * 3; }
	if (!dest_h || !dest_w || !rect.h || !rect.w) return 1;

	if (dstrect) { /*returnvalue*/
	    dstrect->y = dest_y;
	    dstrect->x = dest_x;
	    dstrect->w = dest_w;
	    dstrect->h = dest_h;
	}

	dstp = (Uint8*)dst->pixels +
	    (dst->pitch*dest_y) + (bpp*dest_x);
	srcp = (Uint8*)src->pixels +
	    (src->pitch*rect.y*2) + (bpp*rect.x*2);
	endp = srcp + src->pitch*rect.h*2;
	srci = src->pitch - (bpp*rect.w*2);
	dsti = dst->pitch - (bpp*dest_w);
    }

# if defined __GNUC__ && defined ASM_i386
# define ECX __asm__("%ecx")
# define ESI __asm__("%esi")
# define EDI __asm__("%edi")
# else
# define ECX
# define ESI
# define EDI
# endif

    switch (bpp)
    {
    case 4:
    {
	register Uint32* dstP EDI = (Uint32*) dstp;
	register Uint32* srcP ESI = (Uint32*) srcp;
	do {
	    register int count ECX = rect.w;
#              if defined __GNUC__ && defined ASM_i386
	    if (rect.w^(rect.w&255))
		__asm__ __volatile__ ("prefetchnta 320(%0)\n"
				      :: "r" (srcP));
#              endif
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/4;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 3:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2*3;
	    dstP += dsti;
	    count = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	    count = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    case 2:
    {
	register Uint16* dstP EDI = (Uint16*) dstp;
	register Uint16* srcP ESI = (Uint16*) srcp;
#              if defined __GNUC__ && defined ASM_i386
	    if (rect.w^(rect.w&255))
		__asm__ __volatile__ ("prefetchnta 512(%0)\n"
				      :: "r" (srcP));
#              endif
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/2;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 1:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp -= rect.w*2;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    }
    return 0;
}

#elif 1

/* that routine is not a perfect stretch either - hopefully you have
 * already noticed it: we have implicitly cut everything to div-2,
 * the src-start and src-width are all on even columns and rows and
 * the stretch algorithm will always do XY -> XXY and never the way
 * of the odd rows/columns with a XY -> XYY copy operation. If we
 * want to fix it but still want to keep the copy-loop as short as
 * ever possible then we need to make an outer switch of 4x4 = 16 cases
 * given that src-x/src-y can be each even/odd (four combinations)
 * and that src-w/src-h be each even/odd as well (four combinations).
 * Along with four bytedepths, we would come out at 64 copy-loop
 * definitions but in reality there are 128 copy-loops. Guess why.
 *                                                                       oddX
 *                                                                       only
 */

/* Perform a stretch blit between two surfaces of the same format.
   NOTE:  This function is not safe to call from multiple threads!
*/
int SDL_StretchSurface_23(SDL_Surface *src, SDL_Rect *srcrect,
			  SDL_Surface *dst, SDL_Rect *dstrect)
{
    Uint8 *srcp, *dstp, *endp;
    unsigned int srci, dsti;
    SDL_Rect rect;
    SDL_Rect clip;
    const unsigned bpp = src->format->BytesPerPixel;
    Uint8 oddX = 0;

    if ( src->format->BitsPerPixel != dst->format->BitsPerPixel ) {
	SDL_SetError("Only works with same format surfaces");
	return(-1);
    }

    /* Verify the blit rectangles */
    if ( srcrect ) {
	rect = *srcrect;
#if 0
	clip.x = clip.y = 0; clip.h = src->h; clip.w = src->w;
#else
	clip = src->clip_rect;

	if (rect.x > clip.x + clip.w ||
	    rect.x + rect.w < clip.x ||
	    rect.y > clip.y + clip.h ||
	    rect.y + rect.h < clip.y) return 1;
	if (rect.x < clip.x) { rect.w -= clip.x-rect.x; rect.x = clip.x; }
	if (rect.w > clip.x + clip.w - rect.x)
	    rect.w = clip.x + clip.w - rect.x;
	if (rect.y < clip.y) { rect.h -= clip.y-rect.y; rect.y = clip.y; }
	if (rect.h > clip.y + clip.h - rect.y)
	    rect.h = clip.y + clip.h - rect.y;
#endif
	if (! rect.h || !rect.w) return 1;
	oddX = rect.x&1;
	rect.x /= 2;
	rect.y /= 2;
	rect.w /= 2;
	rect.h /= 2;
    } else {
	rect.x = 0;
	rect.y = 0;
	rect.w = src->w / 2;
	rect.h = src->h / 2;
	clip = rect;
    }

    {
	int dest_x = (rect.x - clip.x/2) * 3 + oddX;
	int dest_y = (rect.y - clip.y/2) * 3;
	int dest_w = rect.w * 3;
	int dest_h = rect.h * 3;

	if (dest_x >= dst->clip_rect.x + dst->clip_rect.w ||
	    dest_y >= dst->clip_rect.y + dst->clip_rect.h ||
	    dest_x + dest_w <= dst->clip_rect.x ||
	    dest_y + dest_h <= dst->clip_rect.y) return 1;
	if (dest_x < dst->clip_rect.x)
	{
	    rect.w = (dest_w - dst->clip_rect.x - dest_x) / 3;
	    rect.x += (dst->clip_rect.x - dest_x) / 3; oddX = 0;
	    dest_w = rect.w * 3;               dest_x = dst->clip_rect.x; }
	if (dest_w > dst->clip_rect.x + dst->clip_rect.w - dest_x)
	{
	    rect.w = (dst->clip_rect.x + dst->clip_rect.w - dest_x) / 3;
	    dest_w = rect.w * 3; }
	if (dest_y < dst->clip_rect.y)
	{
	    rect.h = (dest_h - dst->clip_rect.y - dest_y) / 3;
	    rect.y += (dst->clip_rect.y - dest_y) / 3;
	    dest_h = rect.h * 3;               dest_y = dst->clip_rect.y; }
	if (dest_h > dst->clip_rect.y + dst->clip_rect.h - dest_y)
	{
	    rect.h = (dst->clip_rect.y + dst->clip_rect.h - dest_y) / 3;
	    dest_h = rect.h * 3; }
	if (!dest_h || !dest_w || !rect.h || !rect.w) return 1;

	if (dstrect) { /*returnvalue*/
	    dstrect->y = dest_y;
	    dstrect->x = dest_x;
	    dstrect->w = dest_w;
	    dstrect->h = dest_h;
	}

	dstp = (Uint8*)dst->pixels +
	    (dst->pitch*dest_y) + (bpp*dest_x);
	srcp = (Uint8*)src->pixels +
	    (src->pitch*rect.y*2) + (bpp*(rect.x*2+oddX));
	endp = srcp + src->pitch*rect.h*2;
	srci = src->pitch - (bpp*rect.w*2);
	dsti = dst->pitch - (bpp*dest_w);
    }

# if defined __GNUC__ && defined ASM_i386
# define ECX __asm__("%ecx")
# define ESI __asm__("%esi")
# define EDI __asm__("%edi")
# else
# define ECX
# define ESI
# define EDI
# endif

    if (oddX) goto odd_X_even_Y;

    switch (bpp)
    {
    case 4:
    {
	register Uint32* dstP EDI = (Uint32*) dstp;
	register Uint32* srcP ESI = (Uint32*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/4;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 3:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2*3;
	    dstP += dsti;
	    count = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	    count = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    case 2:
    {
	register Uint16* dstP EDI = (Uint16*) dstp;
	register Uint16* srcP ESI = (Uint16*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/2;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 1:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp -= rect.w*2;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    }
    return 0;

odd_X_even_Y:
    switch (bpp)
    {
    case 4:
    {
	register Uint32* dstP EDI = (Uint32*) dstp;
	register Uint32* srcP ESI = (Uint32*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsl\n" "lodsl\n" "stosl\n" "stosl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/4;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsl\n" "lodsl\n" "stosl\n" "stosl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsl\n" "lodsl\n" "stosl\n" "stosl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 3:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
	    } while (--count != 0);
	    srcP -= rect.w*2*3;
	    dstP += dsti;
	    count = rect.w;
	    do {
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	    count = rect.w;
	    do {
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    case 2:
    {
	register Uint16* dstP EDI = (Uint16*) dstp;
	register Uint16* srcP ESI = (Uint16*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "lodsw\n" "stosw\n" "stosw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/2;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "lodsw\n" "stosw\n" "stosw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "lodsw\n" "stosw\n" "stosw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 1:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsb\n" "lodsb\n" "stosb\n" "stosb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp -= rect.w*2;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsb\n" "lodsb\n" "stosb\n" "stosb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsb\n" "lodsb\n" "stosb\n" "stosb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    }
    return 0;
}

#elif 1

/* that routine is not a perfect stretch either - hopefully you have
 * already noticed it: we have implicitly cut everything to div-2,
 * the src-start and src-width are all on even columns and rows and
 * the stretch algorithm will always do XY -> XXY and never the way
 * of the odd rows/columns with a XY -> XYY copy operation. If we
 * want to fix it but still want to keep the copy-loop as short as
 * ever possible then we need to make an outer switch of 4x4 = 16 cases
 * given that src-x/src-y can be each even/odd (four combinations)
 * and that src-w/src-h be each even/odd as well (four combinations).
 * Along with four bytedepths, we would come out at 64 copy-loop
 * definitions but in reality there are 128 copy-loops. Guess why.
 *                                                                    [[ ALL ]]
 *                                                   unfinished
 */

/* Perform a stretch blit between two surfaces of the same format.
   NOTE:  This function is not safe to call from multiple threads!
*/
int SDL_StretchSurface_23(SDL_Surface *src, SDL_Rect *srcrect,
			  SDL_Surface *dst, SDL_Rect *dstrect)
{
    Uint8 *srcp, *dstp, *endp;
    unsigned int srci, dsti;
    SDL_Rect rect;
    SDL_Rect clip;
    const unsigned bpp = src->format->BytesPerPixel;
    Uint8 kind = 0;

    if ( src->format->BitsPerPixel != dst->format->BitsPerPixel ) {
	SDL_SetError("Only works with same format surfaces");
	return(-1);
    }

    if (0) fprintf(stderr,
		   "[%4i  +%4i  /%4i  +%4i  ] ->"
		   "[%4i+%4i/%4i+%4i]\n",
		   srcrect->x, srcrect->w,
		   srcrect->y, srcrect->h,
		   dst->clip_rect.x, dst->clip_rect.w,
		   dst->clip_rect.y, dst->clip_rect.h);

    /* Verify the blit rectangles */
    if ( srcrect ) {
	rect = *srcrect;
#if 0
	clip.x = clip.y = 0; clip.h = src->h; clip.w = src->w;
#else
	clip = src->clip_rect;

	if (rect.x > clip.x + clip.w ||
	    rect.x + rect.w < clip.x ||
	    rect.y > clip.y + clip.h ||
	    rect.y + rect.h < clip.y) return 1;
	if (rect.x < clip.x) { rect.w -= clip.x-rect.x; rect.x = clip.x; }
	if (rect.w > clip.x + clip.w - rect.x)
	    rect.w = clip.x + clip.w - rect.x;
	if (rect.y < clip.y) { rect.h -= clip.y-rect.y; rect.y = clip.y; }
	if (rect.h > clip.y + clip.h - rect.y)
	    rect.h = clip.y + clip.h - rect.y;
#endif
	if (! rect.h || !rect.w) return 1;
	kind |= rect.x&1; rect.x /= 2; kind<<=1;
	kind |= rect.y&1; rect.y /= 2; kind<<=1;
	kind |= rect.w&1; rect.w /= 2; kind<<=1;
	kind |= rect.h&1; rect.h /= 2;
    } else {
	rect.x = 0;
	rect.y = 0;               kind |= src->w&1; kind<<=1;
	rect.w = src->w / 2;      kind |= src->h&1;
	rect.h = src->h / 2;
	clip = rect;
    }

/* bitmasks */
#   define _H 1
#   define _W 2
#   define _Y 4
#   define _X 8
/* shiftvals */
#   define _H_ 0
#   define _W_ 1
#   define _Y_ 2
#   define _X_ 3

    {
	int dest_x = (rect.x - clip.x/2) * 3;
	int dest_y = (rect.y - clip.y/2) * 3;
	int dest_w = rect.w * 3;
	int dest_h = rect.h * 3;
	if (0)    fprintf(stderr,
			  "[%3i*2%c+%3i*2%c/%3i*2%c+%3i*2%c] ->"
			  "[%4i+%4i/%4i+%4i]\n",
			  rect.x,(kind&_X)?'>':'.',rect.w,(kind&_W)?'>':'.',
			  rect.y,(kind&_Y)?'>':'.',rect.h,(kind&_H)?'>':'.',
			  dest_x,dest_w,	     dest_y,dest_h);
	switch(kind)      /* a switch statement is not an if() - it turns */
	{                 /* out to be a table lookup operation actually */
	case _X|_Y|_W|_H: dest_h ++; /*>>*/
	case _X|_Y|_W:    dest_w ++; /*>>*/
	case _X|_Y:       dest_y ++; /*>>*/
	case _X:          dest_x ++; break;
	case _Y|_W|_H:    dest_h ++; /*>>*/
	case _Y|_W:       dest_w ++; /*>>*/
	case _Y:          dest_x ++; break;
	case _W|_X:       dest_x ++; goto case__W;
	case _W|_H|_X:    dest_x ++; /*>>*/
	case _W|_H:       dest_h ++; /*>>*/
	case__W:
	case _W:          dest_w ++; break;
	case _H|_Y:       dest_y ++; goto case__H;
	case _H|_X|_Y:    dest_y ++; /*>>*/
	case _H|_X:       dest_x ++; /*>>*/
	case__H:
	case _H:          dest_h ++; break;
	case 0:           break;
	}
	if (0)    fprintf(stderr,
			  "[%3i*2%c+%3i*2%c/%3i*2%c+%3i*2%c] ->"
			  "[%4i+%4i/%4i+%4i] !\n",
			  rect.x,(kind&_X)?'>':'.',rect.w,(kind&_W)?'>':'.',
			  rect.y,(kind&_Y)?'>':'.',rect.h,(kind&_H)?'>':'.',
			  dest_x,dest_w,	     dest_y,dest_h);

	if (dest_x >= dst->clip_rect.x + dst->clip_rect.w ||
	    dest_y >= dst->clip_rect.y + dst->clip_rect.h ||
	    dest_x + dest_w <= dst->clip_rect.x ||
	    dest_y + dest_h <= dst->clip_rect.y) return 1;
	if (dest_x < dst->clip_rect.x)
	{
	    rect.w = (dest_w - dst->clip_rect.x - dest_x) / 3;
	    rect.x += (dst->clip_rect.x - dest_x) / 3;
	    dest_w = rect.w * 3;               dest_x = dst->clip_rect.x;
	    kind &= _X|_Y|_H; }
	if (dest_w > dst->clip_rect.x + dst->clip_rect.w - dest_x)
	{
	    rect.w = (dst->clip_rect.x + dst->clip_rect.w - dest_x) / 3;
	    dest_w = rect.w * 3;
	    kind &= _X|_Y|_H; }
	if (dest_y < dst->clip_rect.y)
	{
	    rect.h = (dest_h - dst->clip_rect.y - dest_y) / 3;
	    rect.y += (dst->clip_rect.y - dest_y) / 3;
	    dest_h = rect.h * 3;               dest_y = dst->clip_rect.y;
	    kind &= _X|_Y|_W; }
	if (dest_h > dst->clip_rect.y + dst->clip_rect.h - dest_y)
	{
	    rect.h = (dst->clip_rect.y + dst->clip_rect.h - dest_y) / 3;
	    dest_h = rect.h * 3;
	    kind &= _X|_Y|_W; }
	if (!dest_h || !dest_w || !rect.h || !rect.w) return 1;

	if (dstrect) { /*returnvalue*/
	    dstrect->y = dest_y;
	    dstrect->x = dest_x;
	    dstrect->w = dest_w;
	    dstrect->h = dest_h;
	}

	if (0)    fprintf(stderr,
			  "[%3i*2%c+%3i*2%c/%3i*2%c+%3i*2%c] ->"
			  "[%4i+%4i/%4i+%4i] !!\n",
			  rect.x,(kind&_X)?'>':'.',rect.w,(kind&_W)?'>':'.',
			  rect.y,(kind&_Y)?'>':'.',rect.h,(kind&_H)?'>':'.',
			  dest_x,dest_w,	     dest_y,dest_h);

	dstp = (Uint8*)dst->pixels +
	    (dst->pitch*dest_y) + (bpp*dest_x);
	srcp = (Uint8*)src->pixels +
	    (src->pitch*rect.y*2) + (bpp*rect.x*2);
	endp = srcp + src->pitch*rect.h*2;
	srci = src->pitch - (bpp*rect.w*2);
	dsti = dst->pitch - (bpp*dest_w);
    }

# if defined __GNUC__ && defined ASM_i386
# define ECX __asm__("%ecx")
# define ESI __asm__("%esi")
# define EDI __asm__("%edi")
# else
# define ECX
# define ESI
# define EDI
# endif

    if (kind&_X) goto odd_X_even_Y;

    switch (bpp)
    {
    case 4:
    {
	register Uint32* dstP EDI = (Uint32*) dstp;
	register Uint32* srcP ESI = (Uint32*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/4;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsl\n" "stosl\n" "stosl\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 3:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2*3;
	    dstP += dsti;
	    count = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	    count = rect.w;
	    do {
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "movsl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    case 2:
    {
	register Uint16* dstP EDI = (Uint16*) dstp;
	register Uint16* srcP ESI = (Uint16*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/2;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsw\n" "stosw\n" "stosw\n" "movsw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 1:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp -= rect.w*2;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("lodsb\n" "stosb\n" "stosb\n" "movsb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    }
    return 0;

odd_X_even_Y:
    switch (bpp)
    {
    case 4:
    {
	register Uint32* dstP EDI = (Uint32*) dstp;
	register Uint32* srcP ESI = (Uint32*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsl\n" "lodsl\n" "stosl\n" "stosl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/4;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsl\n" "lodsl\n" "stosl\n" "stosl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsl\n" "lodsl\n" "stosl\n" "stosl\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/4;                  /* gcc is clever enough to make */
	    dstP += dsti/4;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 3:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
	    } while (--count != 0);
	    srcP -= rect.w*2*3;
	    dstP += dsti;
	    count = rect.w;
	    do {
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	    count = rect.w;
	    do {
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		dstP[0] = srcP[0];
		dstP[1] = srcP[1];
		dstP[2] = srcP[2];
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
		*dstP++ = *srcP++;
	    } while (--count != 0);
	    srcP += srci;
	    dstP += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    case 2:
    {
	register Uint16* dstP EDI = (Uint16*) dstp;
	register Uint16* srcP ESI = (Uint16*) srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "lodsw\n" "stosw\n" "stosw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP -= rect.w*2;
	    dstP += dsti/2;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "lodsw\n" "stosw\n" "stosw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsw\n" "lodsw\n" "stosw\n" "stosw\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcP += srci/2;                  /* gcc is clever enough to make */
	    dstP += dsti/2;                  /* it really a "ADD dsti, dstP" */
	} while ((Uint8*)srcP != endp);
    } break;
    case 1:
    {
	register Uint8* dstP EDI = dstp;
	register Uint8* srcP ESI = srcp;
	do {
	    register int count ECX = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsb\n" "lodsb\n" "stosb\n" "stosb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp -= rect.w*2;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsb\n" "lodsb\n" "stosb\n" "stosb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	    count = rect.w;
	    do {
#              if defined __GNUC__ && defined ASM_i386
		__asm__ __volatile__ ("movsb\n" "lodsb\n" "stosb\n" "stosb\n"
				      :: "S" (srcP), "D" (dstP) : "%eax");
#              else
		*dstP++ = *srcP++;
		*dstP++ = *srcP;
		*dstP++ = *srcP++;
#              endif
	    } while (--count != 0);
	    srcp += srci;
	    dstp += dsti;
	} while ((Uint8*)srcP != endp);
    } break;
    }
    return 0;
}

#endif

/* --------------------------------------------------------------------- */
/*                                  RowCode                              */
/* --------------------------------------------------------------------- */
