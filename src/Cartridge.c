#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "Cartridge.h"
#include "Mappers/Mapper.h"

void Cartridge_init(Cartridge* cart)
{
    cart->file = NULL;
}

void Cartridge_load(Cartridge* cart, char* filePath)
{
    cart->file = fopen(filePath, "rb");
    if (cart->file == NULL) {
        exit(EXIT_FAILURE);
    }

    Cartridge_reset(cart);

    uint8_t header[INES_HEADER_SIZE];
    size_t res = fread(header, 1, INES_HEADER_SIZE, cart->file);
    if(res != INES_HEADER_SIZE)
    {
        exit(EXIT_FAILURE);
    }

    // Check if the file format is iNES
    bool iNESFormat=false;
    if (!(header[0] == 'N' && 
          header[1] == 'E' && 
          header[2] == 'S' && 
          header[3] == 0x1A))
    {
        exit(EXIT_FAILURE);
    } 

    // Check what kind of iNES file is being used
    if (iNESFormat == true)
    {
        switch(header[7] & 0x0C)
        {
        case 0x08:
            // NES 2.0
            cart->mapper->format = NES2;
        case 0x04:
            // Archaic iNES
            cart->mapper->format = ARCHAIC_INES;
        case 0x00:
            // iNES
            for(int i = 12; i < INES_HEADER_SIZE;i++)
            {
                if(header[i] != 0x00)
                    exit(EXIT_FAILURE);
            }
            cart->mapper->format = INES;
        default:
            // Archaic iNES or iNES 0.7
            cart->mapper->format = ARCHAIC_INES;
        }
    }
    
    // Store common information shared by all the file formats
    cart->prgChunks = header[4];
    cart->chrChunks = header[5];
    cart->mapperId = header[6] >> 4;
    cart->gameSave = (header[6] & 0x2);
    cart->hasTrainer = (header[6] & 0x4) != 0;
    if(cart->hasTrainer)
    {
        malloc(cart->trainer, 512);
        if (fread(cart->trainer, 1, 512, cart->file) != 512) {
            exit(EXIT_FAILURE);
        }
    }
    else
        cart->trainer = NULL;
    
    // Some mapper set different mirrors from the bit patterns
    // If uncommon mapper needs to be incorperate then work most 
    // be done here
    if(header[6] & 0x8)
        cart->mapper->mirror = FOUR_SCREEN; 
    else if(header[6] & 0x1)
        cart->mapper->mirror = HORIZONTAL;
    else if(!(header[6] & 0x1))
        cart->mapper->mirror = VERTICAL;

    switch(cart->mapper->format)
    {
    case ARCHAIC_INES:
        // I am not planning to be working on this file format
        // Atleast for now.
    case INES:
        cart->mapperId |= (header[7] & 0XF0);
        // Because the iNES format does not specify the submapper
        // the variable will just store it as a byte value that isn't possible
        cart->subMapper = 0xFF;

        if(header[10] & 0x10)
        {
            cart->prgRam = calloc(1, 0x2000 * header[8]);
        }

        cart->mapper->tv = (header[9] & 0x01) ? NTSC : PAL;
        
    case NES2: 
        
        cart->mapperId |= (header[7] & 0XF0);
        cart->mapperId |= ((uint16_t)(header[8] & 0x0F) << 8);
        cart->subMapper = header[8] >> 4;

        cart->prgChunks |= ((uint16_t)(header[9] & 0x0F) << 8);
        cart->chrChunks |= ((uint16_t)(header[9] & 0xF0) << 4);

        if(header[10] & 0x0F)
            cart->prgRam = calloc(1, (64 << (header[10] & 0x0F)));
            cart->hasPrgRam = true;
        if(header[10] & 0xF0)
            cart->prgNvRam = calloc(1, (64 << (header[10]  >> 4)));
            cart->hasPrgNvRam = true;
        if(header[11] & 0x0F)
            cart->chrRam = calloc(1, (64 << (header[11] & 0x0F)));
            cart->hasChrRam = true;

        switch (header[12] & 0x03)
        {
        case 0:
            cart->mapper->tv = NTSC;
            break;
        case 1:
            cart->mapper->tv = PAL;
            break;
        case 2:
            cart->mapper->tv = DUAL;    
            break;
        case 3:
            cart->mapper->tv = DENDY;
            break;
        }
    }
    fclose(cart->file);
}

void Cartridge_reset(Cartridge* cart)
{
    if(!cart) return;

    Mapper_reset(cart->mapper);

    if(!cart->trainer)
        free(cart->trainer);
    if(!cart->prgRom)
        free(cart->prgRom);
    if(!cart->prgRam)
        free(cart->prgRam);
    if(!cart->prgRam)
        free(cart->prgRam);
    if(!cart->chrRom)
        free(cart->chrRom);
    if(!cart->chrRam)
        free(cart->chrRam);

    memset(cart, 0, sizeof(Cartridge));
}

void Cartridge_free(Cartridge* cart)
{
    if(!cart) return;

    Mapper_free(cart->mapper);

    if(!cart->trainer)
        free(cart->trainer);
    if(!cart->prgRom)
        free(cart->prgRom);
    if(!cart->prgRam)
        free(cart->prgRam);
    if(!cart->chrRom)
        free(cart->chrRom);
    if(!cart->chrRam)
        free(cart->chrRam);

    free(cart);
}