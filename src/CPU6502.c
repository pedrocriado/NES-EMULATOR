#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "CPU6502.h"
#include "PPU.h"

void CPU_init(struct CPU6502* cpu)
{
    printf("[DEBUG] About to load NES CPU\n");
    cpu->a = cpu->x = cpu->y = 0x00;
    cpu->s = 0xFD;

    cpu->nmi = false; cpu->irq = false;

    cpu->p = I;

    //TODO cylces: add cycle-accurate IRQ timing.
    //Note: The effect of changing this flag is delayed 1 instruction

    uint8_t low = Bus_read(cpu->bus, 0xFFFC);
    uint8_t high = Bus_read(cpu->bus, 0xFFFD);
    
    cpu->pc = (high << 8) | low;
    cpu->ic = (InstructionContext){0,0,0,0,0};

    printf("[DEBUG] NES CPU loaded\n");
}

void CPU_free(CPU6502* cpu)
{
    //no malloc done, we skip
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
        cpu->ic.opcode = Bus_read(cpu->bus, cpu->pc++);

        cpu->ic.cycles = lookup[cpu->ic.opcode].cycle_cnt;

        uint8_t extra_cycles2 = lookup[cpu->ic.opcode].addrmode(cpu);
        uint8_t extra_cycles1 = lookup[cpu->ic.opcode].operate(cpu);

        cpu->ic.cycles += extra_cycles1 & extra_cycles2;
    }
    cpu->ic.cycles--;
}

void CPU_reset(CPU6502* cpu)
{
    cpu->s -= 3;
    //TODO cylces: add cycle-accurate IRQ timing.
    //Note: The effect of changing this flag is delayed 1 instruction
    cpu->p = cpu->p | I;

    uint8_t low = Bus_read(cpu->bus, 0xFFFC);
    uint8_t high = Bus_read(cpu->bus, 0xFFFD);
    cpu->pc = (high << 8) | low;

    cpu->ic = (InstructionContext){0,0,0,0,7};
}

void CPU_irq(CPU6502* cpu)
{
    if(CPU_get_flag(cpu, I) == 0)
    {
        Bus_write(cpu->bus, 0x0100 + cpu->s--, (cpu->pc >> 8) & 0x00FF);
        Bus_write(cpu->bus, 0x0100 + cpu->s--, cpu->pc & 0x00FF);

        CPU_set_flag(cpu, B, 0);
        CPU_set_flag(cpu, U, 1);
        CPU_set_flag(cpu, I, 1);

        Bus_write(cpu->bus, 0x0100 + cpu->s--, cpu->p);

        uint16_t low = Bus_read(cpu->bus, 0xFFFE);
        uint16_t high = Bus_read(cpu->bus, 0xFFFF);

        cpu->pc = (high << 8) | low;

        cpu->ic.cycles = 7;
    }
}

void CPU_nmi(CPU6502* cpu)
{
    Bus_write(cpu->bus, 0x0100 + cpu->s--, (cpu->pc >> 8) & 0x00FF);
    Bus_write(cpu->bus, 0x0100 + cpu->s--, cpu->pc & 0x00FF);

    CPU_set_flag(cpu, B, 0);
    CPU_set_flag(cpu, U, 1);
    CPU_set_flag(cpu, I, 1);

    Bus_write(cpu->bus, 0x0100 + cpu->s--, cpu->p);

    uint16_t low = Bus_read(cpu->bus, 0xFFFA);
    uint16_t high = Bus_read(cpu->bus, 0xFFFB);

    cpu->pc = (high << 8) | low;

    cpu->ic.cycles = 8;
}

uint8_t CPU_fetch(CPU6502* cpu)
{
    if(!(lookup[cpu->ic.opcode].addrmode == IMP)
       && !(lookup[cpu->ic.opcode].addrmode == ACC))
        cpu->ic.fetched = Bus_read(cpu->bus, cpu->ic.addr_abs);
    return cpu->ic.fetched;
}

// Addressing Mode Functions
uint8_t IMP(CPU6502* cpu)
{
    return 0;
}
uint8_t ACC(CPU6502* cpu)
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
    cpu->ic.addr_abs = Bus_read(cpu->bus, cpu->pc++);
    return 0;
}
uint8_t ZPX(CPU6502* cpu)
{
    cpu->ic.addr_abs = (Bus_read(cpu->bus, cpu->pc++) + cpu->x) & 0x00FF;
    return 0;
}
uint8_t ZPY(CPU6502* cpu)
{
    cpu->ic.addr_abs = (Bus_read(cpu->bus, cpu->pc++) + cpu->y) & 0x00FF;
    return 0;
}
uint8_t IZX(CPU6502* cpu)
{
    uint16_t temp = Bus_read(cpu->bus, cpu->pc++);
    uint16_t low = Bus_read(cpu->bus, (temp + (uint16_t) cpu->x) & 0x00FF);
    uint16_t high = Bus_read(cpu->bus, (temp + (uint16_t) cpu->x + 1) & 0x00FF);

    cpu->ic.addr_abs = (high << 8) | low;

    return 0;
}  
uint8_t IZY(CPU6502* cpu)
{
    uint8_t temp = Bus_read(cpu->bus, cpu->pc++);
    uint16_t low = Bus_read(cpu->bus, temp);
    uint16_t high = Bus_read(cpu->bus, (temp + 1) & 0x00FF);

    cpu->ic.addr_abs = ((high << 8) | low) + cpu->y;

    if((cpu->ic.addr_abs & 0xFF00) != (high << 8))
        return 1;
    return 0;
}
uint8_t ABS(CPU6502* cpu)
{
    uint16_t low = Bus_read(cpu->bus, cpu->pc++);
    uint16_t high = Bus_read(cpu->bus, cpu->pc++);

    cpu->ic.addr_abs = (high << 8) | low;

    return 0;
}	
uint8_t ABX(CPU6502* cpu)
{
    uint16_t low = Bus_read(cpu->bus, cpu->pc++);
    uint16_t high = Bus_read(cpu->bus, cpu->pc++);

    cpu->ic.addr_abs = ((high << 8) | low) + cpu->x;
    
    if((cpu->ic.addr_abs & 0xFF00) != (high << 8))
        return 1;
    return 0;
}	
uint8_t ABY(CPU6502* cpu)
{
    uint16_t low = Bus_read(cpu->bus, cpu->pc++);
    uint16_t high = Bus_read(cpu->bus, cpu->pc++);

    cpu->ic.addr_abs = ((high << 8) | low) + cpu->y;
    
    if((cpu->ic.addr_abs & 0xFF00) != (high << 8))
        return 1;
    return 0;
} 
uint8_t IND(CPU6502* cpu)
{
    uint16_t low = Bus_read(cpu->bus, cpu->pc++);
    uint16_t high = Bus_read(cpu->bus, cpu->pc++);

    uint16_t ptr = (high << 8) | low;

    if(low == 0x00FF) // Simulate page boundary hardware bug
        cpu->ic.addr_abs = (Bus_read(cpu->bus, ptr & 0xFF00) << 8) | Bus_read(cpu->bus, ptr);
    else
        cpu->ic.addr_abs = (Bus_read(cpu->bus, ptr + 1) << 8) | Bus_read(cpu->bus, ptr);

    return 0;
}  
uint8_t REL(CPU6502* cpu)
{
    cpu->ic.addr_rel = Bus_read(cpu->bus, cpu->pc++);

    if(cpu->ic.addr_rel & 0x80)
        cpu->ic.addr_rel |= 0xFF00;
    return 0;
}	

// Helper functions
void CPU_branch_helper(CPU6502* cpu, bool condition) {
    if (condition) {
        cpu->ic.cycles++;

        cpu->ic.addr_abs = cpu->pc + cpu->ic.addr_rel;

        if ((cpu->ic.addr_abs & 0xFF00) != (cpu->pc & 0xFF00))
            cpu->ic.cycles++;

        cpu->pc = cpu->ic.addr_abs;
    }
}

// Operation Functions
uint8_t ADC(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    uint16_t temp = (uint16_t) cpu->a + (uint16_t) memory + (uint16_t) CPU_get_flag(cpu, C);

    CPU_set_flag(cpu, C, temp > 255);
    CPU_set_flag(cpu, Z, (temp & 0x00FF) == 0);
    CPU_set_flag(cpu, V, (temp ^ cpu->a) & (temp ^ memory) & 0x80);
    CPU_set_flag(cpu, N, (temp & 0x80));

    cpu->a = temp & 0x00FF;

    return 1;
}

uint8_t AND(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    uint16_t temp = (uint16_t) cpu->a & (uint16_t) memory;

    CPU_set_flag(cpu, Z, (temp & 0x00FF) == 0);
    CPU_set_flag(cpu, N, (temp & 0x80));

    cpu->a = temp & 0x00FF;

    return 1;
}

uint8_t ASL(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    uint16_t temp = (uint16_t) memory << 1;

    CPU_set_flag(cpu, C, temp & 0xFF00);
    CPU_set_flag(cpu, Z, (temp & 0x00FF) == 0);
    CPU_set_flag(cpu, N, temp & 0x80);

    if(lookup[cpu->ic.opcode].addrmode == ACC)
    {
        cpu->a = temp & 0x00FF;
    }
        
    else    
    {
        Bus_write(cpu->bus, cpu->ic.addr_abs, temp & 0x00FF);
    }
        

    return 0;
}

uint8_t BCC(CPU6502* cpu)
{
    CPU_branch_helper(cpu, CPU_get_flag(cpu, C) == 0);
    return 0;
}
uint8_t BCS(CPU6502* cpu)
{
    CPU_branch_helper(cpu, CPU_get_flag(cpu, C) == 1);
    return 0;
}	
uint8_t BEQ(CPU6502* cpu)
{
    CPU_branch_helper(cpu, CPU_get_flag(cpu, Z) == 1);
    return 0;
}
uint8_t BIT(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    uint8_t temp = cpu->a & memory;

    CPU_set_flag(cpu, Z, temp == 0);
    CPU_set_flag(cpu, V, memory & 0x40);
    CPU_set_flag(cpu, N, memory & 0x80);
    
    return 0;
}	
uint8_t BMI(CPU6502* cpu) 
{
    CPU_branch_helper(cpu, CPU_get_flag(cpu, N) == 1);
    return 0;
} 
uint8_t BNE(CPU6502* cpu)
{
    CPU_branch_helper(cpu, CPU_get_flag(cpu, Z) == 0);
    return 0;
}
uint8_t BPL(CPU6502* cpu)
{
    CPU_branch_helper(cpu, CPU_get_flag(cpu, N) == 0);
    return 0;
}	
uint8_t BRK(CPU6502* cpu)
{
    cpu->pc++;
    Bus_write(cpu->bus, 0x0100 + cpu->s--, (cpu->pc >> 8) & 0x00FF);
    Bus_write(cpu->bus, 0x0100 + cpu->s--, cpu->pc & 0x00FF);

    CPU_set_flag(cpu, I, 1);
    CPU_set_flag(cpu, B, 1);

    Bus_write(cpu->bus, 0x0100 + cpu->s--, cpu->p);

    CPU_set_flag(cpu, B, 0);

    cpu->pc = (Bus_read(cpu->bus, 0xFFFF) << 8) | Bus_read(cpu->bus, 0xFFFE);

    return 0;
}
uint8_t BVC(CPU6502* cpu)
{
    CPU_branch_helper(cpu, CPU_get_flag(cpu, V) == 0);
    return 0;
}  
uint8_t BVS(CPU6502* cpu)
{
    CPU_branch_helper(cpu, CPU_get_flag(cpu, V) == 1);
    return 0;
}
uint8_t CLC(CPU6502* cpu)
{
    CPU_set_flag(cpu, C, 0);
    return 0;
}	
uint8_t CLD(CPU6502* cpu)
{
    CPU_set_flag(cpu, D, 0);
    return 0;
}
uint8_t CLI(CPU6502* cpu)
{
    //TODO cylces: add cycle-accurate IRQ timing.
    //Note: The effect of changing this flag is delayed 1 instruction
    CPU_set_flag(cpu, I, 0);
    return 0;
}  
uint8_t CLV(CPU6502* cpu)
{
    CPU_set_flag(cpu, V, 0);
    return 0;
}
uint8_t CMP(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    uint16_t temp = (uint16_t)cpu->a - (uint16_t)memory;

    CPU_set_flag(cpu, C, cpu->a >= memory);
    CPU_set_flag(cpu, Z, cpu->a == memory);
    CPU_set_flag(cpu, N, temp & 0x0080);

    return 1;
}	
uint8_t CPX(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    uint16_t temp = (uint16_t)cpu->x - (uint16_t)memory;

    CPU_set_flag(cpu, C, cpu->x >= memory);
    CPU_set_flag(cpu, Z, cpu->x == memory);
    CPU_set_flag(cpu, N, temp & 0x0080);

    return 0;
}
uint8_t CPY(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    uint16_t temp = (uint16_t)cpu->y - (uint16_t)memory;

    CPU_set_flag(cpu, C, cpu->y >= memory);
    CPU_set_flag(cpu, Z, cpu->y == memory);
    CPU_set_flag(cpu, N, temp & 0x0080);

    return 0;
}
uint8_t DEC(CPU6502* cpu)
{
    uint16_t memory = CPU_fetch(cpu) - 1;

    Bus_write(cpu->bus, cpu->ic.addr_abs, memory &0x00FF);

    CPU_set_flag(cpu, Z, (memory & 0x00FF) == 0);
    CPU_set_flag(cpu, N, memory & 0x0080);

    return 0;
}
uint8_t DEX(CPU6502* cpu)
{
    cpu->x--;

    CPU_set_flag(cpu, Z, cpu->x == 0x00);
    CPU_set_flag(cpu, N, cpu->x & 0x80);

    return 0;
}	
uint8_t DEY(CPU6502* cpu)
{
    cpu->y--;

    CPU_set_flag(cpu, Z, cpu->y == 0x00);
    CPU_set_flag(cpu, N, cpu->y & 0x80);

    return 0;
}
uint8_t EOR(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    cpu->a = cpu->a ^ memory;

    CPU_set_flag(cpu, Z, cpu->a == 0x00);
    CPU_set_flag(cpu, N, cpu->a & 0x80);

    return 1;
}  
uint8_t INC(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu) + 1;

    Bus_write(cpu->bus, cpu->ic.addr_abs, memory);

    CPU_set_flag(cpu, Z, memory == 0);
    CPU_set_flag(cpu, N, memory & 0x80);
    
    return 0;
}
uint8_t INX(CPU6502* cpu)
{
    cpu->x++;

    CPU_set_flag(cpu, Z, cpu->x == 0x00);
    CPU_set_flag(cpu, N, cpu->x & 0x80);

    return 0;
}	
uint8_t INY(CPU6502* cpu)
{
    cpu->y++;

    CPU_set_flag(cpu, Z, cpu->y == 0x00);
    CPU_set_flag(cpu, N, cpu->y & 0x80);

    return 0;
}
uint8_t JMP(CPU6502* cpu)
{
    cpu->pc = cpu->ic.addr_abs;

    return 0;
}  
uint8_t JSR(CPU6502* cpu)
{
    cpu->pc--;

    Bus_write(cpu->bus, 0x0100 + cpu->s--, (cpu->pc >> 8) & 0x00FF);
    Bus_write(cpu->bus, 0x0100 + cpu->s--, cpu->pc & 0x00FF);

    cpu->pc = cpu->ic.addr_abs;

    return 0;
}
uint8_t LDA(CPU6502* cpu)
{
    cpu->a = CPU_fetch(cpu);

    CPU_set_flag(cpu, Z, cpu->a == 0x00);
    CPU_set_flag(cpu, N, cpu->a & 0x80);

    return 1;
}	
uint8_t LDX(CPU6502* cpu)
{
    cpu->x = CPU_fetch(cpu);

    CPU_set_flag(cpu, Z, cpu->x == 0x00);
    CPU_set_flag(cpu, N, cpu->x & 0x80);

    return 1;
}
uint8_t LDY(CPU6502* cpu)
{
    cpu->y = CPU_fetch(cpu);

    CPU_set_flag(cpu, Z, cpu->y == 0x00);
    CPU_set_flag(cpu, N, cpu->y & 0x80);

    return 1;
}  
uint8_t LSR(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    CPU_set_flag(cpu, C, memory & 0x01);

    memory = memory >> 1;

    CPU_set_flag(cpu, Z, memory == 0);
    CPU_set_flag(cpu, N, memory & 0x80);

    if(lookup[cpu->ic.opcode].addrmode == ACC)
    {
        cpu->a = memory;
    }
    else
    {
        Bus_write(cpu->bus, cpu->ic.addr_abs, memory);
    }

    return 0;
}
uint8_t NOP(CPU6502* cpu)
{
    switch(cpu->ic.opcode)
    {
        case 0x04:
            return 0;
        case 0x0C:
            return 0;
        case 0x14:
            return 0;
        case 0x1A:
            return 0;
        case 0x1C:
            return 1;
        case 0x34:
            return 0;
        case 0x3A:
            return 0;
        case 0x3C:
            return 1;
        case 0x44:
            return 0;
        case 0x54:
            return 0;
        case 0x5A:
            return 0;
        case 0x5C:
            return 1;
        case 0x64:
            return 0;
        case 0x74:
            return 0;
        case 0x7A:
            return 0;
        case 0x7C:
            return 1;
        case 0x80:
            return 0;
        case 0x82:
            return 0;
        case 0x89:
            return 0;
        case 0xC2:
            return 0;
        case 0xD4:
            return 0;
        case 0xDA:
            return 0;
        case 0xDC:
            return 1;
        case 0xE2:
            return 0;
        case 0xEA:
            return 0;
        case 0xF4:
            return 0;
        case 0xFA:
            return 0;
        case 0xFC:
            return 1;
    }
    return 0;
}	
uint8_t ORA(CPU6502* cpu)
{
    uint8_t memory = CPU_fetch(cpu);

    cpu->a |= memory;

    CPU_set_flag(cpu, Z, cpu->a == 0);
    CPU_set_flag(cpu, N, cpu->a & 0x80);

    return 0;
}
uint8_t PHA(CPU6502* cpu)
{
    Bus_write(cpu->bus, 0x0100 + cpu->s--, cpu->a);
    return 0;
}  
uint8_t PHP(CPU6502* cpu)
{

    Bus_write(cpu->bus, 0x0100 + cpu->s--, cpu->p | B | U);

    CPU_set_flag(cpu, B, 0);

    return 0;
}
uint8_t PLA(CPU6502* cpu)
{
    cpu->s++;
    cpu->a = Bus_read(cpu->bus, 0x0100 + cpu->s);

    CPU_set_flag(cpu, Z, cpu->a == 0);
    CPU_set_flag(cpu, N, cpu->a & 0x80);

    return 0;
}	
uint8_t PLP(CPU6502* cpu)
{
    //TODO cylces: add cycle-accurate IRQ timing.
    //Note: The effect of changing this flag is delayed 1 instruction
    cpu->p= Bus_read(cpu->bus, 0x0100 + ++cpu->s);

    return 0;
}
uint8_t ROL(CPU6502* cpu)
{
    uint16_t memory = CPU_fetch(cpu);
    
    memory = (memory << 1) | CPU_get_flag(cpu, C);

    CPU_set_flag(cpu, C, memory & 0xFF00);
    CPU_set_flag(cpu, Z, (memory & 0x00FF) == 0x0000);
    CPU_set_flag(cpu, N, memory & 0x0080);

    if(lookup[cpu->ic.opcode].addrmode == ACC)
        cpu->a = memory & 0x00FF;
    else
        Bus_write(cpu->bus, cpu->ic.addr_abs, memory & 0x00FF);

    return 0;
}
uint8_t ROR(CPU6502* cpu)
{
    uint16_t memory = CPU_fetch(cpu);

    uint8_t old_carry = CPU_get_flag(cpu, C);
    
    CPU_set_flag(cpu, C, memory & 0x0001);
    memory = ((uint16_t)old_carry << 7) | (memory >> 1);

    CPU_set_flag(cpu, Z, (memory & 0x00FF) == 0x0000);
    CPU_set_flag(cpu, N, memory & 0x0080);

    if(lookup[cpu->ic.opcode].addrmode == ACC)
        cpu->a = memory & 0x00FF;
    else
        Bus_write(cpu->bus, cpu->ic.addr_abs, memory & 0x00FF);

    return 0;
}
uint8_t RTI(CPU6502* cpu)
{
    cpu->p = Bus_read(cpu->bus, 0x0100 + ++cpu->s);
    cpu->p &= ~B;
    cpu->p &= ~U;

    uint16_t low = Bus_read(cpu->bus, 0x0100 + ++cpu->s);
    uint16_t high = Bus_read(cpu->bus, 0x0100 + ++cpu->s);
    
    cpu->pc =  high << 8 | low;

    return 0;
}	
uint8_t RTS(CPU6502* cpu)
{
    uint16_t low = Bus_read(cpu->bus, 0x0100 + ++cpu->s);
    uint16_t high = Bus_read(cpu->bus, 0x0100 + ++cpu->s);

    cpu->pc = (high << 8) | low;
    cpu->pc++;

    return 0;
}
uint8_t SBC(CPU6502* cpu)
{
    uint16_t memory = CPU_fetch(cpu);

    memory = memory ^ 0x00FF;

    uint16_t temp = (uint16_t)cpu->a + memory + (uint16_t)CPU_get_flag(cpu, C); 

    CPU_set_flag(cpu, C, temp & 0xFF00);
    CPU_set_flag(cpu, Z, (temp & 0x00FF) == 0);
    CPU_set_flag(cpu, V, (temp ^ (uint16_t)cpu->a) & (temp ^ memory) & 0x0080);
    CPU_set_flag(cpu, N, temp & 0x0080);

    cpu->a = temp & 0x00FF;

    return 1;
}  
uint8_t SEC(CPU6502* cpu)
{
    CPU_set_flag(cpu, C, 1);

    return 0;
}
uint8_t SED(CPU6502* cpu)
{
    CPU_set_flag(cpu, D, 1);

    return 0;
}	
uint8_t SEI(CPU6502* cpu)
{
    //TODO cylces: add cycle-accurate IRQ timing.
    //Note: The effect of changing this flag is delayed 1 instruction
    CPU_set_flag(cpu, I, 1);

    return 0;
}
uint8_t STA(CPU6502* cpu)
{
    Bus_write(cpu->bus, cpu->ic.addr_abs, cpu->a);

    return 0;
}  
uint8_t STX(CPU6502* cpu)
{
    Bus_write(cpu->bus, cpu->ic.addr_abs, cpu->x);

    return 0;
}
uint8_t STY(CPU6502* cpu)
{
    Bus_write(cpu->bus, cpu->ic.addr_abs, cpu->y);

    return 0;
}	
uint8_t TAX(CPU6502* cpu)
{
    cpu->x = cpu->a;

    CPU_set_flag(cpu, Z, cpu->x == 0x00);
    CPU_set_flag(cpu, N, cpu->x & 0x80);

    return 0;
}
uint8_t TAY(CPU6502* cpu)
{
    cpu->y = cpu->a;

    CPU_set_flag(cpu, Z, cpu->y == 0x00);
    CPU_set_flag(cpu, N, cpu->y & 0x80);

    return 0;
}  
uint8_t TSX(CPU6502* cpu)
{
    cpu->x = cpu->s;

    CPU_set_flag(cpu, Z, cpu->x == 0x00);
    CPU_set_flag(cpu, N, cpu->x & 0x80);

    return 0;
}
uint8_t TXA(CPU6502* cpu)
{
    cpu->a = cpu->x;

    CPU_set_flag(cpu, Z, cpu->a == 0x00);
    CPU_set_flag(cpu, N, cpu->a & 0x80);

    return 0;
}	
uint8_t TXS(CPU6502* cpu)
{
    cpu->s = cpu->x;

    return 0;
}
uint8_t TYA(CPU6502* cpu)
{
    cpu->a = cpu->y;

    CPU_set_flag(cpu, Z, cpu->a == 0x00);
    CPU_set_flag(cpu, N, cpu->a & 0x80);

    return 0;
}  
uint8_t XXX(CPU6502* cpu)
{
    return 0;
}