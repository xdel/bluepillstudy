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