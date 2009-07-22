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

void HighlightCode (Uint8 * R, Uint8 * G, Uint8 * B, char * Mnemnomnic)
{
	if (_stricmp (Mnemnomnic,"call") == NULL) {
		*R = 255, *G = 255, *B = 0;
		return;
	}
	if (_stricmp (Mnemnomnic,"push") == NULL) {
		*R = 255, *G = 140, *B = 0;
		return;
	}
	return;
}