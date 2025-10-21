#include <stdio.h>
#include <stdlib.h>

#include "NES.h"

int main(int argc, char *argv[]) {
    printf("[DEBUG] Starting NES emulator\n");
    
    // char *filePath = "Super Mario Bros (PC10)";
    char *filePath = "Super Mario Bros. (World)";
    // char *filePath = "nestest";
    // char *filePath = "color_test";
    // char *filePath = "oam_read";
    // char *filePath = "oam3";
    // char *filePath = "oam_stress";
    // char *filePath = "tv";
    // char *filePath = "ppu_open_bus";
    // char *filePath = "01.basics"; //passed
    // char *filePath = "02.alignment"; //passed
    // char *filePath = "03.corners"; //passed
    // char *filePath = "04.flip"; //passed
    // char *filePath = "05.left_clip"; //passed
    // char *filePath = "06.right_edge"; //passed
    // char *filePath = "07.screen_bottom"; //3
    // char *filePath = "08.double_height"; //passed
    // char *filePath = "09.timing_basics"; //nothing happend
    // char *filePath = "10.timing_order"; //nothing happend
    // char *filePath = "11.edge_timing"; //nothing happend
    // char *filePath = "";
    // char *filePath = "AccuracyCoin";
    // char *filePath = "cpu_interrupts";
    // char *filePath = "Baseball (VS) (Player 1 Mode) [a2]";
    // char *filePath = "Donkey Kong Jr. (JU)";
    // char *filePath = "F-1 Race (J) [p1]";
    // char *filePath = "scanline";

    // passed all tests
    // char *filePath = "palette_ram"; //passed
    // char *filePath = "power_up_palette"; //passed
    // char *filePath = "sprite_ram"; //passed
    // char *filePath = "vbl_clear_time"; //passed
    // char *filePath = "vram_access"; //passed
   
    
    // char *filePath = "cpu";
    // char *filePath = "DuckTales 2 (USA)";
    // char *filePath = "Castlevania (USA) (Rev 1)";
    // char *filePath = "Contra (USA)";
    
    // char *filePath = "official";
    // char *filePath = "instr_timing";
    // char *filePath = "Mega Man (USA)";
    // char *filePath = "cpu_dummy_reads";
    

    struct NES nes;
    printf("[DEBUG] About to initialize NES\n");
    NES_init(&nes, filePath);
    printf("[DEBUG] NES initialized\n");
    
    printf("[DEBUG] About to start NES\n");
    NES_start(&nes);
    printf("[DEBUG] NES started\n");
    
    return 0;
}