/*
 * "Parallax Scrolling Example II"
 *
 *   Nghia             <nho@optushome.com.au>
 *   Randi J. Relander <rjrelander@users.sourceforge.net>
 *   David Olofson     <david@gardena.net>
 *
 * This software is released under the terms of the GPL.
 *
 * Contact authors for permission if you want to use this
 * software, or work derived from it, under other terms.
 *
 *
 *  Command line options:
 *	-f	Fullscreen
 *	-d	Double buffer
 *	-<n>	Depth = <n> bits
 */

#define SOFTSTRETCH 1
/* Modfication for SDL_stretch ###########################
 * - the final blit is using a softstretch into a window
 * that may be changed in size dynamically (unlike the
 * original demo). Changes to the orignal code are wrapped
 * in "ifdef SOFTSTRETCH" parts - do the same to your code
 * to get same softstretch capability to your program.
 */

#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#ifdef SOFTSTRETCH
#include <assert.h>
#include "SDL_stretch.h"
void draw_tile(SDL_Surface *screen, SDL_Surface *tiles, int x, int y, char tile); /* optional */
#endif

/*----------------------------------------------------------
	Definitions...
----------------------------------------------------------*/

/* foreground and background velocities in pixels/sec */
#define FOREGROUND_VEL_X	100.0
#define FOREGROUND_VEL_Y	50.0

/*
 * Size of the screen in pixels
 */
#define	SCREEN_W	320
#define	SCREEN_H	240

/*
 * Size of one background tile in pixels
 */
#define	TILE_W		48
#define	TILE_H		48

/*
 * The maps are 16 by 16 squares, and hold one
 * character per square. The characters determine
 * which tiles are to be drawn in the corresponding
 * squares on the screen. Space (" ") means that
 * no tile will be drawn.
 */
#define	MAP_W		16
#define	MAP_H		16

typedef char map_data_t[MAP_H][MAP_W];

typedef struct
{
	float		pos_x, pos_y;
	float		vel_x, vel_y;
	map_data_t	*map;
	SDL_Surface	*tiles;
} tiled_layer_t;

void tl_init(tiled_layer_t *tl, map_data_t *map, SDL_Surface *tiles);
void tl_pos(tiled_layer_t *tl, float x, float y);
void tl_vel(tiled_layer_t *tl, float x, float y);
void tl_animate(tiled_layer_t *tl, float dt);
void tl_limit_bounce(tiled_layer_t *tl);
void tl_link(tiled_layer_t *tl, tiled_layer_t *to_tl, float ratio);
void tl_render(tiled_layer_t *tl, SDL_Surface *screen);


/*----------------------------------------------------------
	...some globals...
----------------------------------------------------------*/

/*
 * Foreground map.
 */
map_data_t foreground_map = {
/*	 0123456789ABCDEF */
	"3333333333333333",
	"3   2   3      3",
	"3   222 3  222 3",
	"3333 22     22 3",
	"3       222    3",
	"3   222 2 2  333",
	"3   2 2 222    3",
	"3   222      223",
	"3        333   3",
	"3  22 23 323  23",
	"3  22 32 333  23",
	"3            333",
	"3 3  22 33     3",
	"3    222  2  3 3",
	"3  3     3   3 3",
	"3333333333333333"
};

/*
 * Middle level map; where the planets are.
 */
map_data_t middle_map = {
/*	 0123456789ABCDEF */
	"   1    1       ",
	"           1   1",
	"  1             ",
	"     1  1    1  ",
	"   1            ",
	"         1      ",
	" 1            1 ",
	"    1   1       ",
	"          1     ",
	"   1            ",
	"        1    1  ",
	" 1          1   ",
	"     1          ",
	"        1       ",
	"  1        1    ",
	"                "
};

/*
 * Background map.
 */
map_data_t background_map = {
/*	 0123456789ABCDEF */
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000",
	"0000000000000000"
};


/*----------------------------------------------------------
	...and some code. :-)
----------------------------------------------------------*/

void draw_tile(SDL_Surface *screen, SDL_Surface *tiles, int x, int y, char tile)
{
	SDL_Rect source_rect, dest_rect;

	/* Study the following expression. Typo trap! :-) */
	if(' ' == tile)
		return;

	source_rect.x = 0;	/* Only one column, so we never change this. */
	source_rect.y = (tile - '0') * TILE_H;	/* Select tile from image! */
	source_rect.w = TILE_W;
	source_rect.h = TILE_H;

	dest_rect.x = x;
	dest_rect.y = y;

	SDL_BlitSurface(tiles, &source_rect, screen, &dest_rect);
}


int main(int argc, char* argv[])
{
	SDL_Surface	*screen;
	SDL_Surface	*tiles_bmp;
	SDL_Surface	*tiles;
	tiled_layer_t	foreground_layer;
	tiled_layer_t	middle_layer;
	tiled_layer_t	background_layer;
	long		tick1, tick2;
	float		dt;
	SDL_Event	event;
	int		bpp = 0, flags = 0;
	int		i;

	SDL_Init(SDL_INIT_VIDEO);

	/* *NOT* SDL_QUIT here! SDL_Quit works much better... ;-) */
	atexit(SDL_Quit);

	for(i = 1; i < argc; ++i)
	{
		if(strncmp(argv[i], "-d", 2) == 0)
			flags |= SDL_DOUBLEBUF;
		else if(strncmp(argv[i], "-f", 2) == 0)
			flags |= SDL_FULLSCREEN;
		else
			bpp = -atoi(argv[i]);
	}

#     ifdef SOFTSTRETCH
	SDL_Surface* realscreen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags | SDL_RESIZABLE);
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE,
	        realscreen->w, realscreen->h, realscreen->format->BitsPerPixel,
	        realscreen->format->Rmask, realscreen->format->Gmask,
	        realscreen->format->Bmask, realscreen->format->Amask);
#     else
        screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags);
#     endif
	SDL_WM_SetCaption("Parallax Scrolling Example 2", "Parallax 2");

#     ifdef SOFTSTRETCH
	tiles_bmp = SDL_LoadBMP("parallax2.bmp");
	if (tiles_bmp == NULL) tiles_bmp = SDL_LoadBMP("../../src/parallax2.bmp");
        assert(tiles_bmp != NULL);
#     else
        tiles_bmp = SDL_LoadBMP("tiles.bmp");
#     endif
	tiles = SDL_DisplayFormat(tiles_bmp);
	SDL_FreeSurface(tiles_bmp);

	/* set colorkey to bright magenta */
	SDL_SetColorKey(tiles,
		SDL_SRCCOLORKEY|SDL_RLEACCEL,
		SDL_MapRGB(tiles->format,255,0,255));

	tl_init(&foreground_layer, &foreground_map, tiles);
	tl_init(&middle_layer, &middle_map, tiles);
	tl_init(&background_layer, &background_map, tiles);

	tl_vel(&foreground_layer, FOREGROUND_VEL_X, FOREGROUND_VEL_Y);

	/* get initial tick for time calculation */
	tick1 = SDL_GetTicks();

	while (SDL_PollEvent(&event) >= 0)
	{
#             ifdef SOFTSTRETCH
		/* Click to exit */
                if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_QUIT)
                        break;
                else if (event.type == SDL_VIDEORESIZE)
                    realscreen = SDL_SetVideoMode(event.resize.w, event.resize.h,
                            realscreen->format->BitsPerPixel, flags | SDL_RESIZABLE);
#             else
                /* Click to exit */
                if (event.type == SDL_MOUSEBUTTONDOWN)
                        break;
#             endif
		/* calculate time since last update */
		tick2 = SDL_GetTicks();
		dt = (tick2 - tick1) * 0.001f;
		tick1 = tick2;

		/* Update foreground position */
		tl_animate(&foreground_layer, dt);
		tl_limit_bounce(&foreground_layer);

		/*
		 * Link the middle and background levels
		 * to the foreground level
		 */
		tl_link(&middle_layer, &foreground_layer, 0.5);
		tl_link(&background_layer, &foreground_layer, 0.25);

		/* Render layers */
		tl_render(&background_layer, screen);
		tl_render(&middle_layer, screen);
		tl_render(&foreground_layer, screen);

		/* draw "title" tile in upper left corner */
		draw_tile(screen, tiles, 0, 0, '4');

#             ifndef SOFTSTRETCH
		/* make changes visible */
		SDL_Flip(screen);
#             else
		SDL_StretchSurfaceBlit(screen, 0, realscreen, 0);
                SDL_Flip(realscreen);
#             endif

		/* let operating system breath */
		SDL_Delay(1);
	}

	SDL_FreeSurface(tiles);
	exit(0);
}


/*----------------------------------------------------------
	tiled_layer_t functions
----------------------------------------------------------*/

/* Initialivze layer; set up map and tile graphics data. */
void tl_init(tiled_layer_t *tl, map_data_t *map, SDL_Surface *tiles)
{
	tl->pos_x = tl->pos_y = 0.0;
	tl->vel_x = tl->vel_y = 0.0;
	tl->map = map;
	tl->tiles = tiles;
}

/* Set position */
void tl_pos(tiled_layer_t *tl, float x, float y)
{
	tl->pos_x = x;
	tl->pos_y = y;
}

/* Set velocity */
void tl_vel(tiled_layer_t *tl, float x, float y)
{
	tl->vel_x = x;
	tl->vel_y = y;
}

/* Update animation (apply the velocity, that is) */
void tl_animate(tiled_layer_t *tl, float dt)
{
	tl->pos_x += dt * tl->vel_x;
	tl->pos_y += dt * tl->vel_y;
}

/* Bounce at map limits */
void tl_limit_bounce(tiled_layer_t *tl)
{
	int	maxx = MAP_W * TILE_W - SCREEN_W;
	int	maxy = MAP_H * TILE_H - SCREEN_H;

	if(tl->pos_x >= maxx)
	{
		/* v.out = - v.in */
		tl->vel_x = -tl->vel_x;
		/*
		 * Mirror over right limit. We need to do this
		 * to be totally accurate, as we're in a time
		 * discreet system! Ain't that obvious...? ;-)
		 */
		tl->pos_x = maxx * 2 - tl->pos_x;
	}
	else if(tl->pos_x <= 0)
	{
		/* Basic physics again... */
		tl->vel_x = -tl->vel_x;
		/* Mirror over left limit */
		tl->pos_x = -tl->pos_x;
	}

	if(tl->pos_y >= maxy)
	{
		tl->vel_y = -tl->vel_y;
		tl->pos_y = maxy * 2 - tl->pos_y;
	}
	else if(tl->pos_y <= 0)
	{
		tl->vel_y = -tl->vel_y;
		tl->pos_y = -tl->pos_y;
	}
}

/*
 * Link the position of this layer to another layer, w/ scale ratio
 *
 * BTW, it would be kind of neat implementing the link in a more
 * automatic fashion - tl_link() one layer to another an init time,
 * and then forget about it! Oh well, that's another tutorial. :-)
 */
void tl_link(tiled_layer_t *tl, tiled_layer_t *to_tl, float ratio)
{
	tl->pos_x = to_tl->pos_x * ratio;
	tl->pos_y = to_tl->pos_y * ratio;
}

/* Render layer to the specified surface */
void tl_render(tiled_layer_t *tl, SDL_Surface *screen)
{
	int map_x, map_y, map_x_loop;
	int fine_x, fine_y;
	int x, y;

	/* Calculate which part of the map to draw */
	map_x = (int)tl->pos_x / TILE_W;
	map_y = (int)tl->pos_y / TILE_H;

	/*
	 * Calculate where the screen is, with pixel accuracy.
	 * (This gets "negated" later, as it's a screen
	 * coordinate rather than a map coordinate.)
	 */
	fine_x = (int)tl->pos_x % TILE_W;
	fine_y = (int)tl->pos_y % TILE_H;

	/*
	 * Draw all visible tiles.
	 *
	 * Note that this means that we need to draw the size
	 * of one tile outside the screen on each side! (The
	 * parts that are outside aren't actually rendered, of
	 * course - SDL clips them away.)
	 */
	for (y = -fine_y; y < SCREEN_H; y += TILE_H)
	{
		map_x_loop = map_x;
		for (x = -fine_x; x < SCREEN_W; x += TILE_W)
			draw_tile(screen, tl->tiles, x, y,
					(*tl->map)[map_y][map_x_loop++]);
		++map_y;
	}
}


/*----------------------------------------------------------
	EOF
----------------------------------------------------------*/
