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

#include <ntddk.h>
#include <windef.h>

PBYTE Memsearch( const PBYTE target, const PBYTE search, ULONG32 tlen, ULONG32 slen);
VOID Memcpy( const PBYTE dest, const PBYTE src, ULONG32 slen);
VOID InitSpinLock();
VOID AcquireSpinLock();
VOID ReleaseSpinLock();

VOID WPON();
VOID WPOFF();