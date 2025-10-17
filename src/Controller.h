#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum ButtonFlags
{
    BUTTON_A      = 0x01,
    BUTTON_B      = 0x02,
    BUTTON_SELECT = 0x04,
    BUTTON_START  = 0x08,
    BUTTON_UP     = 0x10,
    BUTTON_DOWN   = 0x20,
    BUTTON_LEFT   = 0x40,
    BUTTON_RIGHT  = 0x80
} ButtonFlags;

typedef struct JoyPad
{
    uint8_t status;
    uint8_t idx;
    bool strobe;
} JoyPad;

void Controller_init(JoyPad* ctrl);
void Controller_write(JoyPad* ctrl, uint8_t data);
uint8_t Controller_read(JoyPad* ctrl);