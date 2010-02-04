 /*
 * Copyright (c) 2010, Trusted Computing Lab in Shanghai Jiaotong University.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Copyright (C) Miao Yu <superymkfounder@hotmail.com>
 */
 
#pragma once

#include <ntddk.h>
#include "snprintf.h"

#define NUM_DEBUG_PAGES 200

extern PVOID g_debugWindowAddrVA;
typedef ULONG PRINTSPIN_LOCK, *PPRINTSPIN_LOCK;
#define DEBUG_WINDOW_TAG 'DBG'
/*************************Public Functions*****************/

/**
 * Effects: Write info with format.
 **/
NTSTATUS NTAPI WriteDbgInfo (PUCHAR fmt,...);

/**
 * Effects: Initialize SpinLock, must be called before invoke WriteDbgInfo function
**/
void NTAPI WriteInfoInit();

void NTAPI WriteInfoDispose();