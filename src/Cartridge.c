#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "Cartridge.h"
#include "Mappers/Mapper.h"

void Cartridge_load(Cartridge* cart, char* fileName)
{
    if (!cart) {
        printf("[ERROR] Cart is NULL\n");
        exit(EXIT_FAILURE);
    }

    printf("[DEBUG] Reseting Cartridge and Mapper data\n");

    Cartridge_reset(cart);

    printf("[DEBUG] Loading cartridge: %s\n", fileName);

    FILE* file;
    
    // Finding the path to the save file
    snprintf(cart->nesFilePath, 
             sizeof(cart->nesFilePath), 
             "%s%s.nes", NES_FILE_PATH, fileName);

    file = fopen(cart->nesFilePath, "rb");
    if (file == NULL) {
        printf("[ERROR] Failed to open ROM file: %s (errno: %d - %s)\n", 
               cart->nesFilePath, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("[DEBUG] Successfully opened ROM file\n");   
    
    uint8_t header[INES_HEADER_SIZE];
    size_t res = fread(header, 1, INES_HEADER_SIZE, file);
    if(res != INES_HEADER_SIZE)
    {
        printf("[DEBUG] NES file failed to load header\n");   
        exit(EXIT_FAILURE);
    }

    // Check if the file format is iNES
    if (!(header[0] == 'N' && 
          header[1] == 'E' && 
          header[2] == 'S' && 
          header[3] == 0x1A))
    {
        printf("[DEBUG] File type not recognized\n");   
        exit(EXIT_FAILURE);
    } 

    printf("[DEBUG] Checking file format\n"); 
    // Check what kind of iNES file is being used
    switch(header[7] & 0x0C)
    {
    case 0x08:
        // NES 2.0
        printf("[DEBUG] File format detected was NES 2.0\n");   
        cart->mapper.format = NES2;
        break;
    case 0x04:
        // Archaic iNES
        printf("[DEBUG] File format detected was Archaic NES\n");   
        cart->mapper.format = ARCHAIC_INES;
        break;
    case 0x00:
        // iNES
        for(int i = 12; i < INES_HEADER_SIZE;i++)
        {
            if(header[i] != 0x00)
                exit(EXIT_FAILURE);
        }
        printf("[DEBUG] File format detected was INES\n");   
        cart->mapper.format = INES;
        break;
    default:
        printf("[DEBUG] File format detected could be Archaic NES or INES\n");   
        cart->mapper.format = INES;
        for(int i = 12; i < INES_HEADER_SIZE;i++)
        {
            if(header[i] != 0x00)
                // Archaic iNES or iNES 0.7
                printf("[DEBUG] File format detected was Archaic INES\n");
                cart->mapper.format = ARCHAIC_INES;
                break;
        }
        break;
    }

    printf("[DEBUG] File format detected was: %d\n", cart->mapper.format);   

    // Store common information shared by all the file formats
    cart->prgChunks = header[4];
    cart->chrChunks = header[5];

    cart->mapperId = header[6] >> 4;
    cart->gameSave = (header[6] & 0x2);
    cart->hasTrainer = (header[6] & 0x4) != 0;
    if(cart->hasTrainer)
    {
        cart->trainer = malloc(512);
        if (fread(cart->trainer, 1, 512, file) != 512)
            exit(EXIT_FAILURE);
    }
    
    printf("[DEBUG] Loding memory into the program pointer\n"); 
    // Getting the Program Rom
    size_t prgSize = (cart->prgChunks * 0x4000);
    cart->prgRom = malloc(prgSize);
    if(fread(cart->prgRom, 1, prgSize, file) != prgSize)
        exit(EXIT_FAILURE);
    
    printf("[DEBUG] Loding memory into the character pointer\n"); 
    // Getting the Character Rom
    if(cart->chrChunks > 0)
    {
        size_t chrSize = (cart->chrChunks * 0x2000);
        cart->chrRom = malloc(chrSize);
        if(!cart->chrRom) {
            printf("[ERROR] Failed to allocate CHR ROM\n");
            exit(EXIT_FAILURE);
        }
        if(fread(cart->chrRom, 1, chrSize, file) != chrSize)
        {
            printf("[ERROR] Failed to read CHR ROM\n");
            exit(EXIT_FAILURE);
        }
    } 
    else 
    {
        // No CHR ROM means the game uses CHR RAM
        cart->hasChrRam = true;
        cart->chrRam = malloc(0x2000);  // 8KB of CHR RAM
        if (!cart->chrRam) {
            printf("[ERROR] Failed to allocate CHR RAM\n");
            exit(EXIT_FAILURE);
        }
        memset(cart->chrRam, 0, 0x2000);
        printf("[DEBUG] Using CHR RAM instead of CHR ROM\n");
    }

    // Some mapper set different mirrors from the bit patterns
    // If uncommon mapper needs to be incorporated then work must 
    // be done here
    if(header[6] & 0x8) {
        cart->mapper.mirror = FOUR_SCREEN;
    } else if((header[6] & 0x1) != 0) {
        cart->mapper.mirror = VERTICAL;
    } else {
        cart->mapper.mirror = HORIZONTAL;
    }

    switch(cart->mapper.format)
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

        cart->mapper.tv = (header[9] & 0x01) ? PAL : NTSC;
        
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
            cart->mapper.hasPrgRam = cart->hasPrgRam = true;
        }
        if(header[10] & 0xF0)
        {
            cart->prgNvRamChunks = (header[10]  >> 4);
            cart->prgNvRam = calloc(1, (64 << (header[10]  >> 4)));
            snprintf(cart->savePath, 
                    sizeof(cart->savePath), 
                    "%s%s.sav", NES_SAVE_PATH, fileName);
            Cartridge_save_load(cart);
            cart->mapper.hasPrgNvRam = cart->hasPrgNvRam = true;
        }
        if(header[11] & 0x0F)
        {
            cart->chrRam = calloc(1, (64 << (header[11] & 0x0F)));
            cart->mapper.hasChrRam = cart->hasChrRam = true;
        }

        // The main difference between this tv formats is the frame rates
        // NTSC - 60 fps | PAL - 50 fps
        switch (header[12] & 0x03)
        {
        case 0:
            cart->mapper.tv = NTSC;
            break;
        case 1:
            cart->mapper.tv = PAL;
            break;
        case 2:
            cart->mapper.tv = DUAL;    
            break;
        case 3:
            cart->mapper.tv = DENDY;
            break;
        }

        break;
    }

    cart->mapper.prgChunks = cart->prgChunks;
    cart->mapper.chrChunks = cart->chrChunks;
    cart->mapper.hasPrgRam = cart->hasPrgRam;
    cart->mapper.hasChrRam = cart->hasChrRam;
    cart->mapper.hasPrgNvRam = cart->hasPrgNvRam;
    cart->mapper.prgRom = cart->prgRom;
    cart->mapper.chrRom = cart->chrRom;

    printf("[DEBUG] Mapper ID: %d\n", cart->mapperId);

    // Setting the mirror for the mapper
    switch (cart->mapper.mirror) {
        case HORIZONTAL:
            printf("[DEBUG] Mirroring Horizontal\n");
            cart->mapper.name_table_map[0] = 0x2000;
            cart->mapper.name_table_map[1] = 0x2000;
            cart->mapper.name_table_map[2] = 0x2400;
            cart->mapper.name_table_map[3] = 0x2400;
            break;
        case VERTICAL:
            printf("[DEBUG] Mirroring Vertical\n");    
            cart->mapper.name_table_map[0] = 0x2000;
            cart->mapper.name_table_map[1] = 0x2400;
            cart->mapper.name_table_map[2] = 0x2000;
            cart->mapper.name_table_map[3] = 0x2400;
            break;
        case ONE_SCREEN_LOWER:
            printf("[DEBUG] Mirroring One Screen Lower\n");
            cart->mapper.name_table_map[0] = 0x2000;
            cart->mapper.name_table_map[1] = 0x2000;
            cart->mapper.name_table_map[2] = 0x2000;
            cart->mapper.name_table_map[3] = 0x2000;
            break;
        case ONE_SCREEN_UPPER:
            printf("[DEBUG] Mirroring One Screen Upper\n");
            cart->mapper.name_table_map[0] = 0x2400;
            cart->mapper.name_table_map[1] = 0x2400;
            cart->mapper.name_table_map[2] = 0x2400;
            cart->mapper.name_table_map[3] = 0x2400;
            break;
        case FOUR_SCREEN:
            printf("[DEBUG] Mirroring Four Screen\n");
            cart->mapper.name_table_map[0] = 0x2000;
            cart->mapper.name_table_map[1] = 0x2400;
            cart->mapper.name_table_map[2] = 0x2800;
            cart->mapper.name_table_map[3] = 0x2C00;
            break;
        default:
            // Default to horizontal if unsupported
            printf("[DEBUG] Mirroring Horizontal\n");
            cart->mapper.name_table_map[0] = 0x2000;
            cart->mapper.name_table_map[1] = 0x2000;
            cart->mapper.name_table_map[2] = 0x2400;
            cart->mapper.name_table_map[3] = 0x2400;
            break;
    }
    // Set the proper mapper given the id
    switch (cart->mapperId)
    {
    case NROM:
        set_mapper0(&cart->mapper, cart);
        printf("[DEBUG] Mapper 0 (NROM) initialized\n");
        break;
    case MMC1:
        //set_mapper1(mapper, cart);
        break;
    case UXROM:
        set_mapper2(&cart->mapper, cart);
        printf("[DEBUG] Mapper 2 (NxROM) initialized\n");
        break;
    case CNROM:
        //set_mapper3(mapper, cart);
        break;
    case MMC3:
        //set_mapper4(mapper, cart);
        break;
    case MMC5:
        //set_mapper5(mapper, cart);
        break;
    }
    fclose(file);
    printf("[DEBUG] Cartridge loaded successfully\n");
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

    memset(cart, 0, sizeof(Cartridge));
}

void Cartridge_free(Cartridge* cart)
{
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