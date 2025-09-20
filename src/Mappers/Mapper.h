#pragma once

#include <stdint.h>

struct Cartridge;

typedef struct Mapper
{
    Cartridge* cart;

    uint16_t prgChunks;
    uint16_t chrChunks;

    Mirror mirror;
    TVSystem tv;
    FileFormat format;

    uint16_t name_table_map[4];

} Mapper;

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

inline void Mapper_free(Mapper* mapper)
{
    if(!mapper) return;
    free(mapper);
}

inline void Mapper_free(Mapper* mapper)
{
    if(!mapper) return;
    memset(mapper, 0, sizeof(Mapper));
}