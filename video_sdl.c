#include <SDL/SDL.h>
#include <SDL2/SDL_video.h>
#include "video_sdl.h"

SDL_Surface *screen;
uint32_t palette[64];

uint32_t colors[] =
{
    0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600, 0x561D00,
    0x333500, 0x0B4800, 0x005200, 0x004F08, 0x00404D, 0x000000, 0x000000, 0x000000,
    0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC, 0xB71E7B, 0xB53120, 0x994E00,
    0x6B6D00, 0x388700, 0x0C9300, 0x008F32, 0x007C8D, 0x000000, 0x000000, 0x000000,
    0xFFFEFF, 0x64B0FF, 0x9290FF, 0xC676FF, 0xF36AFF, 0xFE6ECC, 0xFE8170, 0xEA9E22,
    0xBCBE00, 0x88D800, 0x5CE430, 0x45E082, 0x48CDDE, 0x4F4F4F, 0x000000, 0x000000,
    0xFFFEFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF, 0xFBC2FF, 0xFEC4EA, 0xFECCC5, 0xF7D8A5,
    0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC, 0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000
};

void init_video()
{
    screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE|SDL_ANYFORMAT);
    SDL_WM_SetCaption("zNES",NULL);
    if(screen == NULL)
    {
        printf("Couldn't set 640x480x8 video mode: %s\n", SDL_GetError());
        exit(1);
    }

    printf("Set 640x480 at %d bits-per-pixel mode\n", screen->format->BitsPerPixel);

    for(int i=0; i<64; i++)
    {
        uint8_t r = (colors[i]>>16)&0xff;
        uint8_t g = (colors[i]>>8)&0xff;
        uint8_t b = colors[i]&0xff;

        palette[i] = SDL_MapRGB(screen->format, r, g, b);
    }
}

void bgpixel(int x, int y, uint8_t color)
{
    putpixel(x, y, palette[color]);
}

inline void putpixel(int x, int y, uint32_t pixel)
{
    /* Here p is the address to the pixel we want to set */
    uint8_t *p = (uint8_t *)screen->pixels + y * screen->pitch + x * screen->format->BytesPerPixel;

    switch(screen->format->BytesPerPixel)
    {
        case 1:
            *p = pixel;
            break;
        case 2:
            *(uint16_t *)p = pixel;
            break;
        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
        case 4:
            *(uint32_t *)p = pixel;
            break;
    }
}

void swap_buffers()
{
    SDL_Flip(screen);
}