/* 
 * File:   apu.h
 * Author: cassiano
 *
 * Created on January 31, 2016, 7:10 PM
 */

#include <stdio.h>
#include <stdint.h>

#ifndef APU_H
#define	APU_H

#ifdef	__cplusplus
extern "C" {
#endif

void apu_writereg(uint16_t addr, uint8_t value);
uint8_t apu_readreg(uint16_t addr);


#ifdef	__cplusplus
}
#endif

#endif	/* APU_H */

