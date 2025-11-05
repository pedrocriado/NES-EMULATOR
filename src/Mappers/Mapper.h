#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> 
#include <string.h>

struct Cartridge;

typedef enum Mirror
{
    HORIZONTAL,
    VERTICAL,
    ONE_SCREEN_LOWER,
    ONE_SCREEN_UPPER,
    FOUR_SCREEN,
} Mirror;

typedef enum FileFormat
{
    ARCHAIC_INES,
    INES,
    NES2,
} FileFormat;

typedef enum TVSystem
{
    NTSC,
    DUAL,
    PAL,
    DENDY,
} TVSystem;

typedef struct MMC1_REG
{
    uint8_t SR;
    uint8_t control;
    uint8_t chrBank0;
    uint8_t chrBank1;
    uint8_t prgBank;
} MMC1_REG;

typedef struct Mapper
{
    struct Cartridge* cart;

    uint16_t prgChunks;
    uint16_t chrChunks;

    uint8_t* prgRom;
    uint8_t* chrRom;

    int prgClamp, chrClamp, prgPointerOffset, chrPointerOffset;

    MMC1_REG mmc1;

    bool hasPrgRam;
    bool hasChrRam;
    bool hasPrgNvRam;

    Mirror mirror;
    TVSystem tv;
    FileFormat format;

    uint16_t name_table_map[4];

    void (*prg_write)(struct Mapper*, uint16_t, uint8_t);
    void (*chr_write)(struct Mapper*, uint16_t, uint8_t);
    uint8_t (*prg_read)(struct Mapper*, uint16_t);
    uint8_t (*chr_read)(struct Mapper*, uint16_t);
} Mapper;

inline void Mapper_reset(Mapper* mapper)
{
    if(!mapper) return;
    memset(mapper, 0, sizeof(Mapper));
}

inline void Mapper_free(Mapper* mapper)
{
    if(!mapper) return;
    free(mapper);
}

inline void Mapper_set_mirror(Mapper* mapper, Mirror mirror)
{
    // Setting the mirror for the mapper
    switch (mirror) {
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
}

void set_mapper0(Mapper* mapper, struct Cartridge* cart);
void set_mapper1(Mapper* mapper, struct Cartridge* cart);
void set_mapper2(Mapper* mapper, struct Cartridge* cart);
void set_mapper3(Mapper* mapper, struct Cartridge* cart);
void set_mapper4(Mapper* mapper, struct Cartridge* cart);
void set_mapper5(Mapper* mapper, struct Cartridge* cart);
