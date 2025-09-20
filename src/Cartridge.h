#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <Mappers/Mapper.h>

#define INES_HEADER_SIZE 16

typedef struct Cartridge
{
    FILE* file;
    Mapper* mapper;

    uint16_t mapperId;
    uint8_t subMapper; // Might use in the future
    bool gameSave;

    uint16_t prgChunks;
    uint16_t chrChunks;

    bool hasPrgRam;
    bool hasChrRam;
    bool hasPrgNvRam;

    // ROM data
    uint8_t* trainer;
    bool hasTrainer;

    uint8_t* prgRom;
    uint8_t* chrRom;

    // RAM data
    uint8_t* prgRam;
    uint8_t* prgNvRam;
    uint8_t* chrRam;
    //uint8_t* chrNvRam; Probably won't implement it.

} Cartridge;

void Cartridge_init(Cartridge* cart);
void Cartridge_load(Cartridge* cart, char* filePath);
void Cartridge_free(Cartridge* cart);