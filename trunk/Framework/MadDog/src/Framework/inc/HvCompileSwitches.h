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

/**************************************************************
 * Original:
 * This file is added to gather the available compile switches.
 * Currently it includes all the strategy switches.there are 
 * 3 types of strategies in total: Memory Strategy, Debug Strategy
 * and Trap Dispatcher Stragegy. They are located in 
 * the sub folder of the project root.
 * So a developer can configure his/her hypervisor by switch on/off
 * these definitions.
 * The definitions appeared in other places should not be changed.
 * Superymk Wed May 20 3:19 PM 2009
 **************************************************************/
#pragma once

//+++++++++Memory Strategies++++++++++++++
#define USE_MEMORY_DEFAULT_STRATEGY 
//#define USE_MEMORY_MEMORYHIDING_STRATEGY