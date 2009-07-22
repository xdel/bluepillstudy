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

#ifndef _SDL_stretch_h
#define _SDL_stretch_h

#include "SDL_video.h"
#ifndef  _SDL_video_h
#include <SDL/SDL_video.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DECLSPEC
#define DECLSPEC
#endif

/** Perform a stretch blit between two surfaces of the same format.
 *  NOTE:  This function is not safe to call from multiple threads!
 *
 * This function will stretch a srcrect smoothly to the full area of
 * the dst surface. If no srcrect is given then the full area of the
 * src surface is stretched smoothly to the full dst surface. The
 * dstrect is ignored always.
 *
 * This function will also watch for a clip rectangle on the src
 * surface. This may speed up handling in your programs by creating
 * a larger src surface with an associated viewframe, and the srcrect
 * argument needs not be recomputed.
 */
extern DECLSPEC int
SDL_StretchSurfaceRect(SDL_Surface *src, SDL_Rect *srcrect,
			  SDL_Surface *dst, SDL_Rect *dstrect);
/** Perform a stretch blit between two surfaces of the same format.
 *  NOTE:  This function is not safe to call from multiple threads!
 *
 * This function will stretch a srcrect smoothly to the full area of
 * the dst surface. If no srcrect is given then the full area of the
 * src surface is stretched smoothly to the full dst surface. The
 * dstrect is ignored always.
 *
 * Remember that this is the inverse meaning, the main SDL lib will
 * ignore the srcrect and only watch for the dstrect if any.
 */
extern DECLSPEC int
SDL_StretchSurfaceBlit(SDL_Surface *src, SDL_Rect *srcrect,
			  SDL_Surface *dst, SDL_Rect *dstrect);
/**
 *  This function will stretch to 150%. This is not only a fast function
 *  but it is also safe to call from multiple threads. If the srcrect
 *  is given then only that rect is copied. Otherwise the full src
 *  surface is copied to the full dst surface. The dstrect is ignored.
 */
extern DECLSPEC int
SDL_StretchSurface_23(SDL_Surface *src, SDL_Rect *srcrect,
			 SDL_Surface *dst, SDL_Rect *dstrect);

/**
 * return some informative information.
 */
extern DECLSPEC char*
SDL_StretchInfo(void);


#ifdef __cplusplus
}
#endif
#endif
