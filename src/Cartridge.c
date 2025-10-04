#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "Cartridge.h"
#include "Mappers/Mapper.h"
#include "Mappers/Mapper0.c"

void Cartridge_init(Cartridge* cart, Mapper* mapper)
{
    cart = calloc(1, sizeof(Cartridge));
    if (cart == NULL)
    {
        exit(EXIT_FAILURE);
    }
    cart->mapper = mapper;
    cart->mapper = calloc(1, sizeof(Mapper));
    if (cart == NULL)
    {
        exit(EXIT_FAILURE);
    }
}

void Cartridge_load(Cartridge* cart, char* fileName)
{
    FILE* file;
    
    // Reset pointers
    memset(cart, 0, sizeof(Cartridge));
    memset(cart->mapper, 0, sizeof(Mapper));

    // Set mapper variable for eaiser mapper use.
    Mapper* mapper = &cart->mapper;

    // Finding the path to the save file
    snprintf(cart->nesFilePath, 
             sizeof(cart->nesFilePath), 
             "%s%s", NES_FILE_PATH, fileName);
    
    file = fopen(cart->nesFilePath, "rb");
    if (file == NULL) {
        exit(EXIT_FAILURE);
    }

    Cartridge_reset(cart);    

    uint8_t header[INES_HEADER_SIZE];
    size_t res = fread(header, 1, INES_HEADER_SIZE, file);
    if(res != INES_HEADER_SIZE)
    {
        exit(EXIT_FAILURE);
    }

    // Check if the file format is iNES
    if (!(header[0] == 'N' && 
          header[1] == 'E' && 
          header[2] == 'S' && 
          header[3] == 0x1A))
    {
        exit(EXIT_FAILURE);
    } 

    // Check what kind of iNES file is being used
    switch(header[7] & 0x0C)
    {
    case 0x08:
        // NES 2.0
        mapper->format = NES2;
    case 0x04:
        // Archaic iNES
        mapper->format = ARCHAIC_INES;
    case 0x00:
        // iNES
        for(int i = 12; i < INES_HEADER_SIZE;i++)
        {
            if(header[i] != 0x00)
                exit(EXIT_FAILURE);
        }
        mapper->format = INES;
    default:
        // Archaic iNES or iNES 0.7
        mapper->format = ARCHAIC_INES;
    }
    
    // Store common information shared by all the file formats
    cart->prgChunks = header[4];
    cart->prgRom = malloc(cart->prgChunks * 0x4000);
    cart->chrChunks = header[5];
    
    if(cart->chrChunks > 0)
    {
        cart->chrRom = malloc(cart->chrChunks * 0x2000);
    }

    cart->mapperId = header[6] >> 4;
    cart->gameSave = (header[6] & 0x2);
    cart->hasTrainer = (header[6] & 0x4) != 0;
    if(cart->hasTrainer)
    {
        cart->trainer = malloc(512);
        if (fread(cart->trainer, 1, 512, file) != 512)
            exit(EXIT_FAILURE);
    }
    else
        
    
    // Some mapper set different mirrors from the bit patterns
    // If uncommon mapper needs to be incorperate then work most 
    // be done here
    if(header[6] & 0x8)
        mapper->mirror = FOUR_SCREEN; 
    else if(header[6] & 0x1 == 1)
        mapper->mirror = VERTICAL;
    else if(header[6] & 0x1 == 0)
        mapper->mirror = HORIZONTAL;

    switch(mapper->format)
    {
    case ARCHAIC_INES:
        // I am not planning to be working on this file format
        // Atleast for now.

        break;

    case INES:
        cart->mapperId |= (header[7] & 0XF0);
        // Because the iNES format does not specify the submapper
        // the variable will just store it as a byte value that isn't possible
        cart->subMapper = 0xFF;

        if(header[10] & 0x10)
        {
            cart->prgRam = calloc(1, 0x2000 * header[8]);
        }

        mapper->tv = (header[9] & 0x01) ? NTSC : PAL;
        
        break;

    case NES2: 
        
        cart->mapperId |= (header[7] & 0XF0);
        cart->mapperId |= ((uint16_t)(header[8] & 0x0F) << 8);
        cart->subMapper = header[8] >> 4;

        cart->prgChunks |= ((uint16_t)(header[9] & 0x0F) << 8);
        cart->chrChunks |= ((uint16_t)(header[9] & 0xF0) << 4);

        if(header[10] & 0x0F)
        {
            cart->prgRam = calloc(1, (64 << (header[10] & 0x0F)));
            mapper->hasPrgRam = cart->hasPrgRam = true;
        }
        if(header[10] & 0xF0)
        {
            cart->prgNvRamChunks = (header[10]  >> 4);
            cart->prgNvRam = calloc(1, (64 << (header[10]  >> 4)));
            snprintf(cart->savePath, 
                    sizeof(cart->savePath), 
                    "%s%s.sav", fileName);
            Cartridge_save_load(cart);
            mapper->hasPrgNvRam = cart->hasPrgNvRam = true;
        }
        if(header[11] & 0x0F)
        {
            cart->chrRam = calloc(1, (64 << (header[11] & 0x0F)));
            mapper->hasChrRam = cart->hasChrRam = true;
        }

        // The main difference between this tv formats is the frame rates
        // NTSC - 60 fps | PAL - 50 fps
        switch (header[12] & 0x03)
        {
        case 0:
            mapper->tv = NTSC;
            break;
        case 1:
            mapper->tv = PAL;
            break;
        case 2:
            mapper->tv = DUAL;    
            break;
        case 3:
            mapper->tv = DENDY;
            break;
        }

        break;
    }

    // Setting the mirror for the mapper
    switch (mapper->mirror) {
        case HORIZONTAL:
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2000;
            mapper->name_table_map[2] = 0x2400;
            mapper->name_table_map[3] = 0x2400;
            break;
        case VERTICAL:
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2400;
            mapper->name_table_map[2] = 0x2000;
            mapper->name_table_map[3] = 0x2400;
            break;
        case ONE_SCREEN_LOWER:
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2000;
            mapper->name_table_map[2] = 0x2000;
            mapper->name_table_map[3] = 0x2000;
            break;
        case ONE_SCREEN_UPPER:
            mapper->name_table_map[0] = 0x2400;
            mapper->name_table_map[1] = 0x2400;
            mapper->name_table_map[2] = 0x2400;
            mapper->name_table_map[3] = 0x2400;
            break;
        case FOUR_SCREEN:
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2400;
            mapper->name_table_map[2] = 0x2800;
            mapper->name_table_map[3] = 0x2C00;
            break;
        default:
            // Default to horizontal if unsupported
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2000;
            mapper->name_table_map[2] = 0x2400;
            mapper->name_table_map[3] = 0x2400;
            break;
    }
    // Set the proper mapper given the id
    switch (cart->mapperId)
    {
    case NROM:
        set_mapper0(mapper);
        break;
    case MMC1:
        set_mapper1(mapper);
        break;
    case UXROM:
        set_mapper2(mapper);
        break;
    case CNROM:
        set_mapper3(mapper);
        break;
    case MMC3:
        set_mapper4(mapper);
        break;
    case MMC5:
        set_mapper5(mapper);
        break;
    }
    fclose(file);
}

void Cartridge_save_load(Cartridge* cart)
{
    if(cart->prgNvRam)
    {
        FILE* saveFile = fopen(cart->savePath, "rb");
        if (saveFile)
        {
            fread(cart->prgNvRam, 1, cart->prgNvRamChunks, saveFile);
            fclose(saveFile);
        }
    }
}

void Cartridge_save(Cartridge* cart)
{
    if(cart->prgNvRam)
    {
        FILE* saveFile = fopen(cart->savePath, "wb");
        if (saveFile)
        {
            fwrite(cart->prgNvRam, 1, cart->prgNvRamChunks, saveFile);
            fclose(saveFile);
        }
    }
}

void Cartridge_reset(Cartridge* cart)
{
    if(!cart) return;

    Mapper_reset(cart->mapper);

    if(cart->trainer)
        free(cart->trainer);
    if(cart->prgRom)
        free(cart->prgRom);
    if(cart->prgRam)
        free(cart->prgRam);
    if(cart->prgRam)
        free(cart->prgRam);
    if(cart->chrRom)
        free(cart->chrRom);
    if(cart->chrRam)
        free(cart->chrRam);

    // Check if game needs to be saved.
    if(cart->prgNvRam)
    {
        Cartridge_save(cart);
        free(cart->prgNvRam);
    }

    memset(cart, 0, sizeof(Cartridge));
}

void Cartridge_free(Cartridge* cart)
{
    if(!cart) return;

    Mapper_free(cart->mapper);

    if(cart->trainer)
        free(cart->trainer);
    if(cart->prgRom)
        free(cart->prgRom);
    if(cart->prgRam)
        free(cart->prgRam);
    if(cart->chrRom)
        free(cart->chrRom);
    if(cart->chrRam)
        free(cart->chrRam);
    
    // Check if game needs to be saved.
    if(cart->prgNvRam)
    {
        Cartridge_save(cart);
        free(cart->prgNvRam);
    }

    free(cart);
}