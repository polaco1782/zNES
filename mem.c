#include <stdio.h>
#include <stdint.h>

#include "mapper.h"
#include "ppu.h"
#include "apu.h"
#include "joystick.h"

uint8_t ram[0x2000];

// 8 bit mem reading
uint8_t mem_read(uint16_t addr)
{
    if(addr < 0x2000)  return ram[addr%0x800];
    if(addr < 0x4000)  return ppu_readreg(0x2000+addr%8);
    if(addr == 0x4014) return apu_readreg(addr);
    if(addr == 0x4015) return apu_readreg(addr);
    if(addr == 0x4016) return joy_readreg(1);
    if(addr == 0x4017) return joy_readreg(2);
    //if(addr < 0x6000) return I/O registers;
    if(addr >= 0x6000) return mapper_read(addr);

    printf("unhandled cpu memory read at address: 0x%04X\n",  addr);

    return 0xff;
}

// 8 bit mem writing
void mem_write(uint16_t addr, uint8_t value)
{
    if(addr < 0x2000)  { ram[addr%0x800] = value; return; }
    if(addr < 0x4000)  { ppu_writereg(0x2000+addr%8, value); return; }
    if(addr < 0x4014)  { apu_writereg(addr, value); return; }
    if(addr == 0x4014) { ppu_writereg(addr, value); return; }
    if(addr == 0x4015) { apu_writereg(addr, value); return; }
    if(addr == 0x4016) { joy_writereg(1, value); joy_writereg(2, value); return; }
    if(addr == 0x4017) { apu_writereg(addr, value); return; }
    //if(addr < 0x6000) { I/O registers }
    if(addr >= 0x6000) { mapper_write(addr, value); return; }

    printf("unhandled cpu memory write at address: 0x%04X\n", addr);
}

// 16 bit mem reading
uint16_t mem_read16(uint16_t addr)
{
    return mem_read(addr)|mem_read(addr+1)<<8;
}

