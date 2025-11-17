#include <stdint.h>

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
