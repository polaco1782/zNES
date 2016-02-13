#include <stdio.h>
#include <stdint.h>
#include "cart.h"
#include "mapper.h"

struct mapper
{
    uint8_t prgbank1;
    uint8_t prgbank2;
} mapper0;

void mapper0_init()
{
    mapper0.prgbank1 = 0;
    mapper0.prgbank2 = cart.prgbanks - 1;
}

uint8_t mapper0_read(uint16_t addr)
{
    if(addr < 0x2000)  return cart.chr[addr];
    if(addr >= 0xC000) return cart.prg[mapper0.prgbank2*0x4000+(addr-0xc000)];
    if(addr >= 0x8000) return cart.prg[mapper0.prgbank1*0x4000+(addr-0x8000)];
    if(addr >= 0x6000) return cart.sram[addr-0x6000];

    printf("unhandled mapper read at address: 0x%04X",  addr);
    return 0;
}

void mapper0_write(uint16_t addr, uint8_t value)
{
    if(addr < 0x2000) cart.chr[addr] = value; return;
    if(addr >= 0x8000) mapper0.prgbank1 = value%cart.prgbanks;
    if(addr >= 0x6000) cart.sram[addr-0x6000] = value; return;

    printf("unhandled mapper2 write at address: 0x%04x", addr);
}


//  global access funcions
uint8_t mapper_read(uint16_t addr)
{
    //printf("Mapper read: 0x%04x\n", addr);

    switch(cart.mapper)
    {
        case 0: return mapper0_read(addr); break;
    }
}

void mapper_write(uint16_t addr, uint8_t value)
{
    //printf("Mapper write: 0x%04x, 0x%02x\n", addr, value);

    switch(cart.mapper)
    {
        case 0: mapper0_write(addr, value); break;
    }    
}

void mapper_init()
{
    switch(cart.mapper)
    {
        case 0: mapper0_init(); break;
    }    
}