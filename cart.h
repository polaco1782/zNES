/* 
 * File:   cart.h
 * Author: cassiano
 *
 * Created on January 23, 2016, 10:04 PM
 */

#include <stdio.h>
#include <stdint.h>

#ifndef CART_H
#define	CART_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct 
{
    uint32_t nes;
    uint8_t prg_banks;
    uint8_t chr_banks;
    uint8_t control1;
    uint8_t control2;
    uint8_t prg_ram;
    
    uint8_t pad[7];
} __attribute__((packed)) ines_t;

typedef struct 
{
    uint8_t *prg;
    uint8_t *chr;
    uint8_t *sram;
    uint8_t mapper;
    uint8_t mirror;
    uint8_t battery;

    uint8_t prgbanks;    // total PRG banks
} Cart;

extern ines_t header;
extern Cart cart;

int load();

#ifdef	__cplusplus
}
#endif

#endif	/* CART_H */

