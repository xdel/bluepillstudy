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

/**
 * effects: Return the value of Host CR3
 */
ULONG NTAPI HvMmGetHostCR3 (
);

/**
 * effects: Return the origin value of Guest CR3 before install the hypervisor
 */
ULONG NTAPI HvMmGetOriginGuestCR3 (
);