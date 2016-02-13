/* 
 * File:   mapper.h
 * Author: cassiano
 *
 * Created on January 23, 2016, 10:06 PM
 */

#include <stdio.h>
#include <stdint.h>

#ifndef MAPPER_H
#define	MAPPER_H

#ifdef	__cplusplus
extern "C" {
#endif

uint8_t mapper0_read(uint16_t addr);
uint8_t mapper_read(uint16_t addr);
void mapper_write(uint16_t addr, uint8_t value);
void mapper_init();


#ifdef	__cplusplus
}
#endif

#endif	/* MAPPER_H */

