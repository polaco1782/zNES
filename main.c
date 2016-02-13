/* 
 * File:   main.c
 * Author: cassiano
 *
 * Created on January 16,  2016,  8:03 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include "cart.h"
#include "cpu.h"
#include "ppu.h"
#include "video_sdl.h"

int main(int argc,  char** argv)
{
    int cpucycles;

    load();
    cpu_reset();
    ppu_reset();

    init_video();

    for(;;)
    {
        cpucycles = cpu_execop();

        // run PPU code every cpu cycle*3
        for(int i=0; i<cpucycles*3; i++)
        {
            ppu_exec();
        }
    }


    return (EXIT_SUCCESS);
}

