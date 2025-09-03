#pragma once
#include <stdio.h>

#include "Bus.h"

typedef enum
{
    C = (1 << 0),	// Carry Bit
    Z = (1 << 1),	// Zero
    I = (1 << 2),	// Disable Interrupts
    D = (1 << 3),	// Decimal Mode (unused in this implementation)
    B = (1 << 4),	// Break
    U = (1 << 5),	// Unused
    V = (1 << 6),	// Overflow
    N = (1 << 7),	// Negative
} FLAGS6502;

typedef struct old6502
{
    Bus *bus;

} old6502;

void old6502_init(old6502* cpu, Bus* bus);
void old6502_reset(old6502* cpu);
