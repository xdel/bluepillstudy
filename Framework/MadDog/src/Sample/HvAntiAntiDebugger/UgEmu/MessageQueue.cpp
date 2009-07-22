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


// *****************************************************************
//  Function: InitQueue
// 
//  In: -
//  
//  Out: Messages * Ptr
//
//	Use: Initializes the Message Queue
//       
// *****************************************************************
Messages * InitQueue () {
	return (Messages *)calloc (1,sizeof (Messages));
}

// *****************************************************************
//  Function: DestroyQueue
// 
//  In: Messages * DirectoryContent
//  
//  Out: bool
//
//	Use: Frees the memory
//       
// *****************************************************************
bool DestroyQueue (Messages * Queue)
{
	while (Queue != 0)
	{
		Messages * QueueDestroy = Queue;
		Queue = Queue->NextEntry;
		free (QueueDestroy);
	}
	return true;
}


// *****************************************************************
//  Function: DisplayMessage
// 
//  In: Messages * Queue
//		Message 
//  
//  Out: bool
//
//	Use: Adds a message to the queue which gets displayed the next
//       event arises
//       
// *****************************************************************
bool DisplayMessage (Messages * Queue, char * Message,...) {
	int CurrentMessageNr = 0;
	while (Queue->NextEntry != 0) {
		CurrentMessageNr = Queue->MessageNr;
		Queue = Queue->NextEntry;
	}
	Queue->NextEntry = (Messages *)calloc (1,sizeof (Messages));
	if (Queue->NextEntry == 0) return false;

	char Buf[512];
	va_list ap;
	va_start(ap, Message);
	vsprintf_s (Buf,512,Message,ap);
	va_end(ap);

	strcpy_s ((char *)Queue->Message,_countof(Queue->Message),(const char *)Buf);
	Queue->MessageNr = CurrentMessageNr +1;
	// PUSH EVENT
	SDL_Event event;
	SDL_UserEvent userevent;
	userevent.type = SDL_USEREVENT;
	userevent.code = 0x100;
	event.type = SDL_USEREVENT;
	event.user = userevent;
	SDL_PushEvent (&event);

	return true;
}

// *****************************************************************
//  Function: CopyQueueToClipboard
// 
//  In: Messages * Queue
//		Message 
//  
//  Out: bool
//
//	Use: Adds a message to the queue which gets displayed the next
//       event arises
//       
// *****************************************************************
bool CopyQueueToClipboard (Messages * Queue) {
	LPTSTR  lptstrCopy; 

	if (!OpenClipboard(NULL)) return FALSE; 
    EmptyClipboard(); 
 
	ULONG Size = 0;
	Messages * QueueTmp = Queue;
	while (QueueTmp != 0) {
		Size += (ULONG)strlen ((const char *)QueueTmp->Message)+1;
		QueueTmp = QueueTmp->NextEntry;
	}
    HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE,Size); 
	if (hglbCopy == NULL) 
    { 
        CloseClipboard(); 
        return FALSE; 
    } 

	ULONG POS = 0;
    // Lock the handle and copy the text to the buffer. 
    lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
	QueueTmp = Queue;
	while (QueueTmp != NULL) {
		memcpy((void *)&lptstrCopy[POS], QueueTmp->Message, strlen ((const char *)QueueTmp->Message)); 
		POS += (ULONG)strlen ((const char *)QueueTmp->Message);
		sprintf_s (&lptstrCopy[POS], Size,"\n");
		POS ++;
		QueueTmp = QueueTmp->NextEntry;
	}
	GlobalUnlock(hglbCopy); 
    SetClipboardData(CF_TEXT, hglbCopy); 
    // Close the clipboard. 
    CloseClipboard(); 
	return true;
}



