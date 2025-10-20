#include <stdio.h>
#include <stdlib.h>

#include "NES.h"

int main(int argc, char *argv[]) {
    printf("[DEBUG] Starting NES emulator\n");
    
    char *filePath = "Super Mario Bros (PC10)";
    // char *filePath = "Super Mario Bros. (World)";
    // char *filePath = "nestest";
    // char *filePath = "color_test";
    // char *filePath = "oam_read";
    // char *filePath = "oam3";
    // char *filePath = "oam_stress";
    // char *filePath = "sprite_ram";
    // char *filePath = "vram_access";
    // char *filePath = "tv";
    // char *filePath = "ppu_open_bus";
    // char *filePath = "01.basics";
    // char *filePath = "02.alignment";
    // char *filePath = "03.corners";
    // char *filePath = "cpu_interrupts";
    // char *filePath = "Baseball (VS) (Player 1 Mode) [a2]";
    // char *filePath = "Donkey Kong Jr. (JU)";
    // char *filePath = "";
    // char *filePath = "";
    // char *filePath = "";
    

    struct NES nes;
    printf("[DEBUG] About to initialize NES\n");
    NES_init(&nes, filePath);
    printf("[DEBUG] NES initialized\n");
    
    printf("[DEBUG] About to start NES\n");
    NES_start(&nes);
    printf("[DEBUG] NES started\n");
    
    return 0;
}