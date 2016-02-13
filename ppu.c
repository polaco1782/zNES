#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "mapper.h"
#include "cart.h"
#include "mem.h"
#include "cpu.h"
#include "cpu.h"
#include "ppu.h"
#include "video_sdl.h"

typedef struct
{
    union
    {
        struct
        {
            uint8_t r0:2;   // nametable address;
            uint8_t r2:1;   // vram address increment
            uint8_t r3:1;   // sprite table address
            uint8_t r4:1;   // background table address
            uint8_t r5:1;   // sprite size
            uint8_t r6:1;   // ppu master slave select
            uint8_t r7:1;   // nmi on blanking interval
        };
        uint8_t r;
    };

} PPU_ctrl;

typedef struct
{
    union
    {
        struct
        {
            uint8_t r0:1;   // Emphasize blue
            uint8_t r1:1;   // Emphasize green
            uint8_t r2:1;   // Emphasize red
            uint8_t r3:1;   // Show sprites
            uint8_t r4:1;   // Show background
            uint8_t r5:1;   // Show sprites in leftmost 8 pixels
            uint8_t r6:1;   // Show background in leftmost 8 pixels
            uint8_t r7:1;   // grayscale
        };
        uint8_t r;
    };

} PPU_mask;

typedef struct
{
    union
    {
        struct
        {
            uint8_t r0:5;   // bits previously written
            uint8_t r5:1;   // sprite overflow
            uint8_t r6:1;   // sprite 0 hit
            uint8_t r7:1;   // vblank started
        };
        uint8_t r;
    };

} PPU_stats;

typedef struct
{
    uint8_t oamdata[265];
    uint8_t palette[32];
    uint8_t nametabledata[2048];

    uint8_t nametablebyte;
    uint8_t attrbyte;
    uint8_t tile_lo;
    uint8_t tile_hi;
    uint64_t tiledata;

    uint8_t oamaddr;    // curretn oam address
    uint16_t vaddr;     // current vram address
    uint16_t tmp;
    uint8_t bufdata;    // for buffered reads

    uint8_t x;

    bool write;
    bool odd_frame;

    PPU_ctrl ctrl;
    PPU_mask mask;
    PPU_stats status;

    uint32_t cycle;
    uint32_t scanline;

    uint64_t frames;

} PPU_t;

uint16_t lookup[][4] =
{
    {0, 0, 1, 1},
    {0, 1, 0, 1},
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 1, 2, 3},
};

PPU_t ppu;

static inline uint16_t mirror_addr(uint16_t addr)
{
    addr = (addr-0x2000)%0x1000;
    uint8_t table = addr/0x400;
    uint16_t offset = addr%0x400;
    
    //printf("0x%04x\n",0x2000+lookup[cart.mirror][table]*0x400+offset);

    return 0x2000+lookup[cart.mirror][table]*0x400+offset;
}

uint8_t ppu_readpalette(uint16_t addr)
{
    if(addr>16 && addr%4==0)
        addr -= 16;

    return ppu.palette[addr];
}

uint8_t ppu_writepalette(uint16_t addr, uint8_t value)
{
    if(addr>16 && addr%4==0)
        addr -= 16;

    ppu.palette[addr] = value;
}

// used internally only
static void ppu_write(uint16_t addr, uint8_t value)
{
    addr %= 0x4000;
    if(addr < 0x2000) { mapper_write(addr, value); return; }
    if(addr < 0x3F00) { ppu.nametabledata[mirror_addr(addr)%2048] = value; return; }
    if(addr < 0x4000) { ppu_writepalette(addr%32, value); return; }

    printf("unhandled ppu memory write at address: 0x%04x\n", addr);
}

// used internally only
static uint8_t ppu_read(uint16_t addr)
{
    addr %= 0x4000;
    if(addr < 0x2000) return mapper_read(addr);
    if(addr < 0x3F00) return ppu.nametabledata[mirror_addr(addr)%2048];
    if(addr < 0x4000) return ppu_readpalette(addr%32);

    printf("unhandled ppu memory read at address: 0x%04x", addr);

    return 0;
}

void ppu_dma(uint8_t value)
{
    uint16_t addr = value<<8;

    for(int i=0; i<256; i++)
        ppu.oamdata[ppu.oamaddr++] = mem_read(addr+i);

    // hang cpu execution for 513 cycles
    cpu_stall_notify(513);
}

void ppu_writereg(uint16_t addr, uint8_t value)
{
    ppu.status.r = value;

    switch(addr)
    {
        case 0x2000:    // ctrl register
            ppu.ctrl.r = value;
            ppu.tmp &= 0xf3ff;
            ppu.tmp |= (value&0x3)<<10;
            break;
        case 0x2001:    // mask register
            ppu.mask.r = value;
            break;
        case 0x2003:    // oam address register
            ppu.oamaddr = value;
            break;
        case 0x2004:    // oam data register
            ppu.oamdata[ppu.oamaddr++] = value;
            break;
        case 0x2005:    // ppuscroll register
            if(ppu.write)
            {
                ppu.tmp &= 0x8fff;
                ppu.tmp |= (value&0x07)<<12;
                ppu.tmp &= 0xfc1f;
                ppu.tmp |= (value&0xf8)<<2;
            }
            else
            {
                ppu.tmp &= 0xffe0;
                ppu.tmp |= value>>3;
                ppu.x = value&0x7;
            }
            ppu.write^=1;
            break;
        case 0x2006:    // ppu address register
            if(ppu.write)
            {
                ppu.tmp &= 0xff00;
                ppu.tmp |= value;
                ppu.vaddr = ppu.tmp;
            }
            else
            {                
                ppu.tmp &= 0x80ff;
                ppu.tmp |= (value&0x3f)<<8;
            }
            ppu.write^=1;
            break;
        case 0x2007:    // ppu data register
            ppu_write(ppu.vaddr, value);
            if(ppu.ctrl.r2)
                ppu.vaddr+=32;
            else
                ppu.vaddr++;
            break;
        case 0x4014:    // ppu start dma transfer
            ppu_dma(value);
            break;

        default:
            printf("Unhandled PPU register write at 0x%04x\n", addr);
    }
}

uint8_t ppu_readreg(uint16_t addr)
{
    switch(addr)
    {
        case 0x2002: ppu.write = 0; return ppu.status.r; break;
        case 0x2004: return ppu.oamdata[ppu.oamaddr]; break;
        case 0x2007:
        {
            uint8_t value = ppu_read(ppu.vaddr);

            if((ppu.vaddr%0x4000) < 0x3f00)
            {
                uint8_t buf = ppu.bufdata;
                ppu.bufdata = value;
                value = buf;
            }
            else
                ppu.bufdata = ppu_read(ppu.vaddr-0x1000);

            if(ppu.ctrl.r2)
                ppu.vaddr+=32;
            else
                ppu.vaddr++;

            return value;
        }
        break;

        default:
            printf("Unhandled PPU register read at 0x%04x\n", addr);
    }
}

void ppu_reset()
{
    memset(&ppu, 0, sizeof(PPU_t));
    ppu.cycle = 340;
    ppu.scanline = 240;
    ppu.write = 0;
}

#define RENDER_ENABLED  ppu.mask.r3 || ppu.mask.r4
#define PRE_LINE        ppu.scanline==261
#define VISIBLE_LINE    ppu.scanline<240
#define RENDER_LINE     (ppu.scanline==261||ppu.scanline<240)
#define PRE_FETCHCYCLE  (ppu.cycle>=321 && ppu.cycle<=336)
#define VISIBLE_CYCLE   (ppu.cycle>=1 && ppu.cycle<=256)
#define FETCH_CYCLE     (PRE_FETCHCYCLE||VISIBLE_CYCLE)

#define VBLANK_START    ppu.scanline==241 && ppu.cycle==1
#define VBLANK_END      ppu.scanline==261 && ppu.cycle==1

void render_pixel()
{
    int x,y;
    uint8_t bg;

    x = ppu.cycle-1;
    y = ppu.scanline;

    // render background logic
    if(ppu.mask.r4)
    {
        uint8_t data;

        data = (ppu.tiledata>>32)>>((7-ppu.x)*4);

        bgpixel(x, y, ppu_readpalette(data%64));        
    }


    ppu.status.r6 = 1; // sprite 0 hit
}

void ppu_decode()
{
    ppu.tiledata <<= 4;
    switch(ppu.cycle%8)
    {
        case 1:
            // decode nametable index
            ppu.nametablebyte = ppu_read(0x2000|(ppu.vaddr&0xfff));
            break;
        case 3:
        {
            // decode attribute byte
            uint16_t addr = 0x23c0 | (ppu.vaddr&0x0c00)|((ppu.vaddr>>4)&0x38)|((ppu.vaddr>>2)&0x07);
            uint8_t shift = ((ppu.vaddr>>4)&4) | (ppu.vaddr&2);
            ppu.attrbyte = ((ppu_read(addr)>>shift)&3)<<2;
        }
        break;
        case 5:
        {
            // decode tile lo byte
            uint8_t fy = (ppu.vaddr>>12)&7;
            uint16_t addr = 0x1000*ppu.ctrl.r4+ppu.nametablebyte*16+fy;
            ppu.tile_lo = ppu_read(addr);
        }
        break;
        case 7:
        {
            // decode tile hi byte
            uint8_t fy = (ppu.vaddr>>12)&7;
            uint16_t addr = 0x1000*ppu.ctrl.r4+ppu.nametablebyte*16+fy;
            ppu.tile_hi = ppu_read(addr+8);
        }
        break;
        case 0:
        {
            uint32_t data;
            uint8_t p1,p2;

            for(int i=0; i<8; i++)
            {
                p1 = (ppu.tile_lo&0x80)>>7;
                p2 = (ppu.tile_hi&0x80)>>6;
                ppu.tile_lo<<=1;
                ppu.tile_hi<<=1;
                data<<=4;
                data|=(ppu.attrbyte|p1|p2);
            }

            ppu.tiledata |= data;
        }
        break;
    }
}

void ppu_exec()
{
    if(RENDER_ENABLED)
    {
        if(ppu.odd_frame==1 && PRE_LINE && ppu.cycle==339)
        {
            ppu.cycle = 0;
            ppu.scanline = 0;
            ppu.frames++;
            ppu.odd_frame ^= 1;
            return;
        }
    }

    // ppu raster simulation
    ppu.cycle++;
    if(ppu.cycle>340)
    {
        ppu.cycle = 0;
        ppu.scanline++;
        if(ppu.scanline>261)
        {
            ppu.scanline = 0;
            ppu.frames++;
            ppu.odd_frame ^=1;
        }
    }

    // background/sprite logic enabled
    if(RENDER_ENABLED)
    {
        if(VISIBLE_LINE && VISIBLE_CYCLE)
            render_pixel();

        if(RENDER_LINE && FETCH_CYCLE)
            ppu_decode();

        // copy Y
        if(PRE_LINE && ppu.cycle>=280 && ppu.cycle<=304)
        {
            ppu.vaddr &= 0x841f;
            ppu.vaddr |= (ppu.tmp&0x7be0);
        }

        // increment counters
        if(RENDER_LINE)
        {
            // increment X
            if(FETCH_CYCLE && ppu.cycle%8==0)
            {
                if((ppu.vaddr&0x1f) == 31)
                {
                    ppu.vaddr &= 0xffe0;
                    ppu.vaddr ^= 0x400;
                }
                else
                    ppu.vaddr++;
            }

            // increment Y
            if(ppu.cycle==256)
            {
                if((ppu.vaddr&0x7000) != 0x7000)
                    ppu.vaddr += 0x1000;
                else
                {
                    uint16_t y;
                    ppu.vaddr &= 0x8fff;
                    y = (ppu.vaddr&0x03e0)>>5;
                    if(y==29)
                    {
                        y = 0;
                        ppu.vaddr ^= 0x800;
                    }
                    else
                    if(y==31)
                        y = 0;
                    else
                        y++;

                    ppu.vaddr &= 0xfc1f;
                    ppu.vaddr |= (y<<5);
                }
            }

            // copy X
            if(ppu.cycle==257)
            {
                ppu.vaddr &= 0xfbe0;
                ppu.vaddr |= (ppu.tmp&0x41f);
            }
        }

        // evaluate sprites
        if(ppu.cycle==257)
        {
            //if(VISIBLE_LINE)
                //printf("Eval sprites\n");
        }
    }

    // start vblank logic
    if(VBLANK_START)
    {
        // we're in vblank state
        ppu.status.r7 = 1;

        // notify cpu if nmi is enabled
        if(ppu.ctrl.r7)
            cpu_nmi_notify();
    }

    // end vblank logic
    if(VBLANK_END)
    {
        ppu.status.r7 = 0; // vblank bit
        ppu.status.r6 = 0; // sprite 0 hit
        ppu.status.r5 = 0; // sprite overflow

        swap_buffers();
    }
}
