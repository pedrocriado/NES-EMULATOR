#pragma once

#include <stdint.h>

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

typedef struct Mapper
{
    struct Cartridge* cart;

    uint16_t prgChunks;
    uint16_t chrChunks;

    Mirror mirror;
    TVSystem tv;
    FileFormat format;

    uint16_t name_table_map[4];

    void (*prg_write)(Mapper*, uint16_t, uint8_t);
    void (*chr_write)(Mapper*, uint16_t, uint8_t);
    uint8_t (*prg_read)(Mapper*, uint16_t);
    uint8_t (*chr_read)(Mapper*, uint16_t);
} Mapper;

inline void Mapper_reset(Mapper* mapper)
{
    if(!mapper) return;
    free(mapper);
}

inline void Mapper_free(Mapper* mapper)
{
    if(!mapper) return;
    memset(mapper, 0, sizeof(Mapper));
}

void set_mapper0(Mapper* mapper);
void set_mapper1(Mapper* mapper);
void set_mapper2(Mapper* mapper);
void set_mapper3(Mapper* mapper);
void set_mapper4(Mapper* mapper);
void set_mapper5(Mapper* mapper);
