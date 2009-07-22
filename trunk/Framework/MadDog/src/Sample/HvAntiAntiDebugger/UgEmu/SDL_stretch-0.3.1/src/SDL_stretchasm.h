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

#ifndef _SDL_STRETCHASM_IMPLEMENTATION_
#define _SDL_STRETCHASM_IMPLEMENTATION_

/* development header - do not use */

#if (defined(WIN32) && !defined(_M_ALPHA) && !defined(_WIN32_WCE))
#define SDL_STRETCH_I386
#define SDL_STRETCH_MASM
#define SDL_STRETCH_USE_ASM
#endif

#if  defined(__i386__) && defined(__GNUC__)
#define SDL_STRETCH_I386
#define SDL_STRETCH_GAS
#define SDL_STRETCH_USE_ASM
#endif

#if  defined(__x86_64__) && defined(__GNUC__)
#define SDL_STRETCH_X86_64
#define SDL_STRETCH_GAS
#define SDL_STRETCH_USE_ASM
#endif

#if defined SDL_STRETCH_I386 && defined SDL_STRETCH_MASM
#define SDL_STRETCH_CALL(code, srcp, dstp) \
__asm {\
    push edi ;\
    push esi ;\
    mov edi, dstp ;\
    mov esi, srcp ;\
    call dword ptr code ;\
    pop esi ;\
    pop edi ;\
}
#endif

#if defined SDL_STRETCH_X86_64 && defined SDL_STRETCH_MASM
#define SDL_STRETCH_CALL(code, srcp, dstp) \
__asm {\
    push rdi ;\
    push rsi ;\
    mov rdi, dstp ;\
    mov rsi, srcp ;\
    call qword ptr code ;\
    pop rsi ;\
    pop rdi ;\
}
#endif

#if defined SDL_STRETCH_I386 && defined SDL_STRETCH_GAS
#define SDL_STRETCH_CALL(code, srcp, dstp) \
__asm__ __volatile__ ("call *%%eax" \
                      :: "S" (dstp), "D" (srcp), "a" (code));
#endif

#if defined SDL_STRETCH_X86_64 && defined SDL_STRETCH_GAS
#define SDL_STRETCH_CALL(code, srcp, dstp) \
__asm__ __volatile__ ("call *%%rax" \
                      :: "S" (dstp), "D" (srcp), "a" (code));
#endif

#if defined USE_ASM_STRETCH && ! defined SDL_STRETCH_USE_ASM
#define SDL_STRETCH_USE_ASM 1
#endif

#if defined SDL_STRETCH_DISABLE_ASM
#undef SDL_STRETCH_USE_ASM
#undef SDL_STRETCH_CALL
#endif

#if defined SDL_STRETCH_USE_INTERPRETED_CODE
#undef SDL_STRETCH_CALL
#endif

#if defined SDL_STRETCH_I386 || defined WIN32 || !defined SDL_STRETCH_CALL
#define SDL_STRETCH_DATA_EXECUTABLE
#else
#define SDL_STRETCH_DATA_NOEXEC
#endif

#endif
