#include "helper.h"

//IsDoubleClick got copied from the web
#define MAX_TICKS 350
BOOL IsDoubleClick (void)
{
    static ULONG LastClickTicks;
             ULONG CurrentClickTicks;

    if (! LastClickTicks) {
        LastClickTicks = SDL_GetTicks ();
        return (FALSE);
    } else {
        CurrentClickTicks = SDL_GetTicks ();
        if ((CurrentClickTicks - LastClickTicks) <= MAX_TICKS) {
            LastClickTicks = CurrentClickTicks;
            return (TRUE);
        }
        LastClickTicks = CurrentClickTicks;
        return (FALSE);
    }
}


