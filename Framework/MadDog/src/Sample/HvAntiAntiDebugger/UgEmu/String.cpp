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

bool TextFile_getNumberofLines (char * FileName, ULONG * LineCounter)
{
	FILE *file;
	ULONG bRes = 0;
	fopen_s (&file,FileName, "r");
	if (file != NULL) {
		static char line[8192];
		while (fgets (line, sizeof line, file ) != NULL ) {
			bRes++;
		}
		*LineCounter = bRes;
		fclose (file);
		return true;
	}
	return false;
}

bool TextFile_ReadFile (char * FileName, StringList * StringListPtr)
{
	FILE *file;
	fopen_s (&file,FileName, "r");
	if (file != NULL) {
		static char line[8192] = "";
		while (fgets (line, sizeof line, file ) != NULL ) {
			memcpy (StringListPtr->Line,line,_countof (StringListPtr->Line));
			StringListPtr++;
		}
		fclose (file);
		return true;
	}
	return false;
}
