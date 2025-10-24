#include <stdio.h>
#include <stdlib.h>

#include "NES.h"

struct NES nes;

int main(int argc, char *argv[]) {
    printf("[DEBUG] Starting NES emulator\n");

    const char *filePath = NULL;
    if (argc > 1) {
        filePath = argv[1];
        printf("[DEBUG] Initial ROM argument detected: %s\n", filePath);
    } else {
        printf("[DEBUG] No ROM provided via command line. Use the File -> Open ROM menu.\n");
    }
    
    printf("[DEBUG] About to initialize NES\n");
    NES_init(&nes, filePath);
    printf("[DEBUG] NES initialized\n");
    
    printf("[DEBUG] About to start NES\n");
    NES_start(&nes);
    printf("[DEBUG] NES stopped\n");
    
    return 0;
}