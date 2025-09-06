#include <stdio.h>

#include "CPU6502.h"

CPU6502* cpu = NULL;

void CPU_init(CPU6502* instance)
{
    cpu = instance;
    cpu->a = 0x00;
    cpu->x = 0x00;
    cpu->y = 0x00;
    cpu->s = 0x00;
    cpu->p = 0x00;
    cpu->pc = 0x0000;
}

uint8_t CPU_get_flag(Flags flag)
{
    return (cpu->p & flag) ? 1 : 0;
}

void CPU_set_flag(Flags flag, bool value)
{
    if(value)
        cpu->p |= flag;
    else
        cpu->p &= ~flag;
}

void CPU_clock()
{

}

void CPU_reset()
{

}

uint8_t CPU_fetch()
{

}

uint8_t ADC()
{
    CPU_fetch();

    temp = (uint16_t) cpu->a + (uint16_t)fetched + (uint16_t)CPU_get_flag(cpu, C);

}

uint8_t AND()
{

}

uint8_t ASL()
{
    
}

uint8_t BCC()
{

}