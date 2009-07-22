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
char InputBuffer[MAX_PATH];

SDL_TimerID Cursor = NULL;
size_t CursorPos = NULL;
bool Set = false;

typedef struct HistoryBuffer {
	HistoryBuffer * NextEntry;
	HistoryBuffer * PreviousEntry;
	unsigned int CommandNr;
	char Command[80];
} HistoryBuffer, *PHistoryBuffer;
unsigned int HistoryEntry = NULL;
unsigned int HistoryMax = NULL;

HistoryBuffer * HistoryList;

// data window related
SDL_TimerID CursorData = NULL;
size_t CursorPosData = NULL;
size_t CursorPosYData = NULL;