/* 
 * File:   mem.h
 * Author: cassiano
 *
 * Created on January 23, 2016, 10:10 PM
 */

#include <stdio.h>
#include <stdint.h>

#ifndef MEM_H
#define	MEM_H

#ifdef	__cplusplus
extern "C" {
#endif

uint8_t mem_read(uint16_t addr);
void mem_write(uint16_t addr, uint8_t value);
uint16_t mem_read16(uint16_t addr);


#ifdef	__cplusplus
}
#endif

#endif	/* MEM_H */

