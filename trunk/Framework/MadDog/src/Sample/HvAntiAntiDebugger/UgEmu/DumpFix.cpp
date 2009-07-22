/*
This file is part of UgDbg.
 
UgDbg is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
UgDbg is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with UgDbg.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "helper.h"

int DumpFix (unsigned char * Offset,DebugStruc * DebugDataExchange)
{
	*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: dumpfix in progress...");
	PIMAGE_DOS_HEADER DosHdr = (PIMAGE_DOS_HEADER) (BYTE*)Offset;
	PIMAGE_NT_HEADERS NtHdr = (PIMAGE_NT_HEADERS) (BYTE*)(Offset+DosHdr->e_lfanew);
	PIMAGE_SECTION_HEADER SecHdr = (PIMAGE_SECTION_HEADER) (BYTE*)(Offset+DosHdr->e_lfanew+sizeof(IMAGE_NT_HEADERS));
	if ((DosHdr != NULL) && (DosHdr->e_magic == IMAGE_DOS_SIGNATURE)) {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: DOS Signature present...");
		if (NtHdr->Signature == IMAGE_NT_SIGNATURE) {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: NT Signature present...");
			// SHOW + FIX SECTIONs
			for (int i = 0; i < NtHdr->FileHeader.NumberOfSections; i ++) {
				SecHdr->SizeOfRawData = SecHdr->Misc.VirtualSize;
				SecHdr->PointerToRawData = SecHdr->VirtualAddress;
				*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: %-8s ROfs: %8.8x VOfs: %8.8x RSize: %8.8x VSize: %8.8x",SecHdr->Name,SecHdr->PointerToRawData,SecHdr->VirtualAddress,SecHdr->SizeOfRawData,SecHdr->Misc.VirtualSize);
				SecHdr++;
			}
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: dumpfix done...");
		} else {
			*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: NT Signature not present...");
		}
	} else {
		*DebugDataExchange->ShowLatestMessages = DisplayMessage (DebugDataExchange->Queue,"UGDBG: DOS Signature not present...");
	}
	return true;
}