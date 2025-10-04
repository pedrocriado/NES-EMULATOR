#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <Mappers/Mapper.h>

#define INES_HEADER_SIZE 16
#define NES_FILE_PATH "/nes_files/"
#define NES_SAVE_PATH "/nes_files/saves/"

typedef struct Cartridge
{
    char nesFilePath[260];
    char savePath[260]; 
    Mapper* mapper;

    uint16_t mapperId;
    uint8_t subMapper; // Might use in the future
    bool gameSave;

    uint16_t prgChunks;
    uint16_t chrChunks;
    uint8_t prgNvRamChunks;

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

typedef enum MapperId
{
    NROM = 0,
    MMC1 = 1,
    UXROM = 2,
    CNROM = 3,
    MMC3 = 4,
    MMC5 = 5,
    
} MapperId;

void Cartridge_init(Cartridge* cart, Mapper* mapper);
void Cartridge_load(Cartridge* cart, char* filePath);
void Cartridge_save_load(Cartridge* cart, char* filePath);
void Cartridge_save(Cartridge* cart);
void Cartridge_reset(Cartridge* cart);
void Cartridge_free(Cartridge* cart);