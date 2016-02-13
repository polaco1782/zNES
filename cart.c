#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "cart.h"
#include "mapper.h"

ines_t header;
Cart cart;

int load()
{
    FILE *f;

    //f=fopen("/home/cassiano/dk.nes","rb");
    //f=fopen("/home/cassiano/mario.nes","rb");
    //f=fopen("/home/cassiano/official.nes","rb");
    f=fopen("/home/cassiano/NEStress.NES","rb");
    //f=fopen("/home/cassiano/colors.nes","rb");
    //f=fopen("/home/cassiano/nestest.nes","rb");

    fread(&header,sizeof(header),1,f);

    if(header.nes == 0x1a53454e)
    {
        printf("Its a valid nes rom!\n");
        printf("PRG banks: %d\n",  header.prg_banks);
        printf("CHR banks: %d\n",  header.chr_banks);

        // read PRG banks to memory
        cart.prg = (uint8_t *)malloc(header.prg_banks*16384);
        fread(cart.prg,  16384,  header.prg_banks,  f);

        // read CHR rom banks to memory
        cart.chr = (uint8_t *)malloc(header.chr_banks*8192);
        fread(cart.chr,  8192,  header.chr_banks,  f);

        // cartridge mapper
        cart.mapper = ((header.control1>>4)|(header.control2&0xf0));
        printf("Mapper: %d\n",  cart.mapper);

        // mirroring
        cart.mirror = ((header.control1&1)|((header.control1>>3)&1)<<1);
        printf("Mirror: %d\n",  cart.mirror);

        cart.prgbanks = header.prg_banks;

        if(header.chr_banks == 0)
            cart.chr = (uint8_t *)malloc(8192);

        for(int i=0; i<header.chr_banks*8192; i++)
        {
            if(i==0)
                printf("0x%04X: ", i);

            printf("%02x ", cart.chr[i]);
            if((i+1)%32==0 && i>0)
                printf("\n0x%04X: ", i);
        }

        mapper_init();
    }
    else
    {
        printf("Bad ROM!\n");
        exit(-1);
    }
    
    fclose(f);
}