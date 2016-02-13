/* 
 * File:   ppu.h
 * Author: cassiano
 *
 * Created on January 24, 2016, 1:48 PM
 */

#include <stdio.h>
#include <stdint.h>

#ifndef PPU_H
#define	PPU_H

#ifdef	__cplusplus
extern "C" {
#endif

void ppu_writereg(uint16_t addr, uint8_t value);
uint8_t ppu_readreg(uint16_t addr);
void ppu_reset();
void ppu_exec();


#ifdef	__cplusplus
}
#endif

#endif	/* PPU_H */

