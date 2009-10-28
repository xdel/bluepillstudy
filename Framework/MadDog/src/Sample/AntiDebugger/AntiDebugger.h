/* 
 * Copyright holder: Invisible Things Lab
 * 
 * This software is protected by domestic and International
 * copyright laws. Any use (including publishing and
 * distribution) of this software requires a valid license
 * from the copyright holder.
 *
 * This software is provided for the educational use only
 * during the Black Hat training. This software should not
 * be used on production systems.
 *
 */

#pragma once

#include <ntddk.h>
#include "HvCore.h"
#include "Handlers.h"
#include "Vmxtraps.h"
//#include "version.h"
#include <windef.h>

	
typedef struct{
	BYTE cpuid;
	BYTE mov_eax;
	LPVOID address;
	WORD jump_eax;
}ASMJUMP, *PASMJUMP;//0xB8//0xE0FF