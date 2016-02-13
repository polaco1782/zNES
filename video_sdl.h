/* 
 * File:   video_sdl.h
 * Author: cassiano
 *
 * Created on February 8, 2016, 12:45 AM
 */

#include <stdint.h>

#ifndef VIDEO_SDL_H
#define	VIDEO_SDL_H

#ifdef	__cplusplus
extern "C" {
#endif

void init_video();
void putpixel(int x, int y, uint32_t pixel);
void bgpixel(int x, int y, uint8_t color);
void swap_buffers();


#ifdef	__cplusplus
}
#endif

#endif	/* VIDEO_SDL_H */

