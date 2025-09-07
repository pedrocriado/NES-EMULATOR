#include <stdio.h>

#include "CPU6502.h"

void CPU_init(CPU6502* cpu)
{
    cpu->a = 0x00;
    cpu->x = 0x00;
    cpu->y = 0x00;
    cpu->s = 0xFD;
    cpu->p = 0x04;
    cpu->pc = 0xFFFC;
    cpu->ic = (InstructionContext){0,0,0,0,0};
}  

uint8_t CPU_read(CPU6502* cpu, uint16_t addr)
{
    return Bus_read(cpu->bus, addr);
}

void CPU_write(CPU6502* cpu, uint16_t addr, uint8_t data)
{

}

uint8_t CPU_get_flag(CPU6502* cpu, Flags flag)
{
    return (cpu->p & flag) ? 1 : 0;
}

void CPU_set_flag(CPU6502* cpu, Flags flag, bool value)
{
    if(value)
        cpu->p |= flag;
    else
        cpu->p &= ~flag;
}

void CPU_clock(CPU6502* cpu)
{
    if (cpu->ic.cycles == 0)
    {
        
    }
    cpu->ic.cycles--;
}

void CPU_reset(CPU6502* cpu)
{

}

uint8_t CPU_fetch(CPU6502* cpu)
{
    if(!(lookup[cpu->ic.opcode].addrmode == &IMP))
        cpu->ic.fetched = CPU_read(cpu, cpu->ic.addr_abs);
    return cpu->ic.fetched;
}

// Addressing Mode Functions
uint8_t IMP(CPU6502* cpu)
{
    cpu->ic.fetched = cpu->a;
    return 0;
}
uint8_t IMM(CPU6502* cpu)
{
    cpu->ic.addr_abs = cpu->pc++;
    return 0;
}
uint8_t ZP0(CPU6502* cpu)
{
    cpu->ic.addr_abs = (uint16_t) CPU_read(cpu, cpu->pc++);
    cpu->ic.addr_abs &= 0x00FF;
    return 0;
}
uint8_t ZPX(CPU6502* cpu)
{
    cpu->ic.addr_abs = (uint16_t) CPU_read(cpu, cpu->pc++ + cpu->x);
    cpu->ic.addr_abs &= 0x00FF;
    return 0;
}
uint8_t ZPY(CPU6502* cpu)
{
    cpu->ic.addr_abs = (uint16_t) CPU_read(cpu, cpu->pc++ + cpu->y);
    cpu->ic.addr_abs &= 0x00FF;
    return 0;
}
uint8_t IZX(CPU6502* cpu)
{
    uint16_t t = (uint16_t) CPU_read(cpu, cpu->pc++);
    uint16_t low = CPU_read(cpu, (t + (uint16_t) cpu->x) & 0x00FF);
    uint16_t high = CPU_read(cpu, (t + (uint16_t) cpu->x + 1) & 0x00FF);

    cpu->ic.addr_abs = (high << 8) | low;

    return 0;
}  
uint8_t IZY(CPU6502* cpu)
{
    uint16_t t = (uint16_t) CPU_read(cpu, cpu->pc++);
    uint16_t low = CPU_read(cpu, t & 0x00FF);
    uint16_t high = CPU_read(cpu, (t + 1) & 0x00FF);

    cpu->ic.addr_abs = ((high << 8) | low) + cpu->y;

    if((cpu->ic.addr_abs & 0xFF00) != (high << 8))
        return 1;
    return 0;
}
uint8_t ABS(CPU6502* cpu)
{
    uint16_t low = CPU_read(cpu, cpu->pc++);
    uint16_t high = CPU_read(cpu, cpu->pc++);

    cpu->ic.addr_abs = (high << 8) | low;

    return 0;
}	
uint8_t ABX(CPU6502* cpu)
{
    uint16_t low = CPU_read(cpu, cpu->pc++);
    uint16_t high = CPU_read(cpu, cpu->pc++);

    cpu->ic.addr_abs = ((high << 8) | low) + cpu->x;
    
    if((cpu->ic.addr_abs & 0xFF00) != (high << 8))
        return 1;
    return 0;
}	
uint8_t ABY(CPU6502* cpu)
{
    uint16_t low = CPU_read(cpu, cpu->pc++);
    uint16_t high = CPU_read(cpu, cpu->pc++);

    cpu->ic.addr_abs = ((high << 8) | low) + cpu->y;
    
    if((cpu->ic.addr_abs & 0xFF00) != (high << 8))
        return 1;
    return 0;
} 
uint8_t IND(CPU6502* cpu)
{
    uint16_t low = CPU_read(cpu, cpu->pc++);
    uint16_t high = CPU_read(cpu, cpu->pc++);

    uint16_t ptr = (high << 8) | low;

    if(low == 0x00FF) // Simulate page boundary hardware bug
        cpu->ic.addr_abs = (CPU_read(cpu, ptr & 0xFF00) << 8) | CPU_read(cpu, ptr);
    else
        cpu->ic.addr_abs = (CPU_read(cpu, ptr + 1) << 8) | CPU_read(cpu, ptr);

    return 0;
}  
uint8_t REL(CPU6502* cpu)
{
    cpu->ic.addr_rel = (uint16_t) CPU_read(cpu, cpu->pc);
    cpu->pc++;

    if(cpu->ic.addr_rel & 0x80)
        cpu->ic.addr_rel |= 0xFF00;
    return 0;
}	

// Operation Functions
uint8_t ADC(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    uint16_t temp = (uint16_t) cpu->a + (uint16_t) cpu->ic.fetched + (uint16_t) CPU_get_flag(cpu, C);

    CPU_set_flag(cpu, C, temp > 255);
    CPU_set_flag(cpu, Z, (temp & 0x00FF) == 0);
    CPU_set_flag(cpu, V, (temp ^ cpu->a) & (temp ^ cpu->ic.fetched) & 0x80);
    CPU_set_flag(cpu, N, (temp & 0x80));

    cpu->a = temp & 0x00FF;

    return 1;
}

uint8_t AND(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    uint16_t temp = (uint16_t) cpu->a & (uint16_t) cpu->ic.fetched;

    CPU_set_flag(cpu, Z, (temp & 0x00FF) == 0);
    CPU_set_flag(cpu, N, (temp & 0x80));

    cpu->a = temp & 0x00FF;

    return 1;
}

uint8_t ASL(CPU6502* cpu)
{
    
}

uint8_t BCC(CPU6502* cpu)
{

}