#include <stdint.h>
#include <SDL.h>

#include "Controller.h"

void Controller_init(JoyPad* ctrl)
{
    ctrl->state = 0;
    ctrl->idx = 0;
    ctrl->strobe = false;
}

void Controller_write(JoyPad* ctrl, uint8_t data)
{
    ctrl->strobe = data & 0x01;
    if(ctrl->strobe) ctrl->idx = 0;
}

uint8_t Controller_read(JoyPad* ctrl)
{
    if(ctrl->idx > 7) return 1;

    uint8_t res = (ctrl->state >> ctrl->idx) & 0x01;
    if(!ctrl->strobe) ctrl->idx++;

    return res;
}

uint8_t Controller_input_key(SDL_Scancode key)
{
    switch(key)
    {
        case SDL_SCANCODE_RIGHT: return 0x80;
        case SDL_SCANCODE_LEFT:  return 0x40;
        case SDL_SCANCODE_DOWN:  return 0x20;
        case SDL_SCANCODE_UP:    return 0x10;
        case SDL_SCANCODE_F:     return 0x08;
        case SDL_SCANCODE_G:     return 0x04;
        case SDL_SCANCODE_X:     return 0x02;
        case SDL_SCANCODE_Z:     return 0x01;
        default:                 return 0x00;
    }
}