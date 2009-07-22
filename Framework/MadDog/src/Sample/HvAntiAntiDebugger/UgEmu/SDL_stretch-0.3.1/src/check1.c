#include "SDL_stretch.h"
#include "SDL/SDL.h"
#include "SDL/SDL_main.h"

/** really, I don't know why SDL's BlitSurface does not work immediatly.
 *
 */

int main(void) {
    printf("%s\n", SDL_StretchInfo());
    SDL_Init(SDL_INIT_VIDEO);
    void *s_pixels = "0123456789" "ABCDEFGHIJ" "QRSTUVWXYZ" "01234567890" "ABCDEFGHIJ";
    int e = 0;
    SDL_Surface* s = SDL_CreateRGBSurfaceFrom(strdup(s_pixels),
                10, 5, 16 /*depth*/, 0/*pitch*/, 070000, 07700, 0077, 0100000);
    SDL_Surface* t = SDL_CreateRGBSurface
        (SDL_SWSURFACE,
                10, 5, 16 /*depth*/, 070000, 07700, 0077, 0100000);
    SDL_FillRect(t, 0, 0x3A3F);
    SDL_Surface* u = SDL_CreateRGBSurface
        (SDL_SWSURFACE,
                10, 5, 16 /*depth*/, 070000, 07700, 0077, 0100000);
    SDL_Surface* v = SDL_CreateRGBSurface
        (SDL_SWSURFACE,
                15, 10, 16 /*depth*/, 070000, 07700, 0077, 0100000);
    SDL_Surface* w = SDL_CreateRGBSurface
        (SDL_SWSURFACE,
                20, 3, 16 /*depth*/, 070000, 07700, 0077, 0100000);
    // SDL_Surface* screen =  screen = SDL_SetVideoMode(640, 480, 16, SDL_DOUBLEBUF);
    printf("SDL_Stretch CHECK START %i %i\n", s->format->BytesPerPixel, t->format->BytesPerPixel);
    printf("src %i*%i '%.*s'\n", s->w, s->h, s->w * s->h, (char*) s->pixels);
    printf("pre %i*%i '%.*s' \n", t->w, t->h, t->w * t->h, (char*) t->pixels);
    printf("SDL_BlitSurface 1\n");
    e = SDL_BlitSurface(s, 0, t, 0);
    if (e == -1) printf("ERROR: %s", SDL_GetError());
    printf("dst %i*%i '%.*s'\n", t->w, t->h, t->w * t->h, (char*) t->pixels);
    printf("SDL_BlitSurface 1\n");
    e = SDL_BlitSurface(s, 0, t, 0);
    if (e == -1) printf("ERROR: %s", SDL_GetError());
    printf("dst %i*%i '%.*s' \n", t->w, t->h, t->w * t->h, (char*) t->pixels);
    printf("SDL_StretchSurfaceBlit 1\n");
    e = SDL_StretchSurfaceBlit(s, 0, u, 0);
    printf("dst %i*%i '%.*s' \n", u->w, u->h, u->w * u->h, (char*) u->pixels);
    printf("SDL_StretchSurfaceRect 1\n");
    e = SDL_StretchSurfaceRect(s, 0, u, 0);
    printf("dst %i*%i '%.*s' \n", u->w, u->h, u->w * u->h, (char*) u->pixels);
    printf("SDL_StretchSurfaceBlit 1.5\n");
    e = SDL_StretchSurfaceBlit(s, 0, v, 0);
    printf("dst %i*%i '%.*s' \n", v->w, v->h, v->w * v->h, (char*) v->pixels);
    printf("SDL_StretchSurfaceRect 1.5\n");
    e = SDL_StretchSurfaceRect(s, 0, v, 0);
    printf("dst %i*%i '%.*s' \n", v->w, v->h, v->w * v->h, (char*) v->pixels);
    printf("SDL_StretchSurfaceBlit 2\n");
    e = SDL_StretchSurfaceBlit(s, 0, w, 0);
    printf("dst %i*%i '%.*s' \n", w->w, w->h, w->w * w->h, (char*) w->pixels);
    printf("SDL_StretchSurfaceRect 2\n");
    e = SDL_StretchSurfaceRect(s, 0, w, 0);
    printf("dst %i*%i '%.*s' \n", w->w, w->h, w->w * w->h, (char*) w->pixels);
    printf("SDL_Stretch CHECK DONE - OK\n");
    SDL_VideoQuit();
    return 0;
}
