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
    Bus_write(cpu->bus, addr, data);
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
        uint8_t opcode = CPU_read(cpu, cpu->pc++);

        cpu->ic.cycles = lookup[opcode].cycles;

        uint8_t extra_cycles1 = lookup[opcode].operate();
        uint8_t extra_cycles2 = lookup[opcode].addrmode();

        cpu->ic.cycles += extra_cycles1 & extra_cycles2;
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
    cpu->ic.addr_rel = (uint16_t) CPU_read(cpu, cpu->pc++);

    if(cpu->ic.addr_rel & 0x80)
        cpu->ic.addr_rel |= 0xFF00;
    return 0;
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

    if(lookup[cpu->ic.opcode].operate == &ACC)
        cpu->a = temp & 0x00FF;
    else
        CPU_write(cpu, cpu->ic.addr_abs, temp & 0x00FF);

    return 0;
}

uint8_t BCC(CPU6502* cpu)
{

}
uint8_t BCS(CPU6502* cpu)
{

}	
uint8_t BEQ(CPU6502* cpu)
{

}
uint8_t BIT(CPU6502* cpu)
{

}	
uint8_t BMI(CPU6502* cpu) 
{

} 
uint8_t BNE(CPU6502* cpu)
{

}
uint8_t BPL(CPU6502* cpu)
{

}	
uint8_t BRK(CPU6502* cpu)
{

}
uint8_t BVC(CPU6502* cpu)
{

}  
uint8_t BVS(CPU6502* cpu)
{

}
uint8_t CLC(CPU6502* cpu)
{

}	
uint8_t CLD(CPU6502* cpu)
{

}
uint8_t CLI(CPU6502* cpu)
{

}  
uint8_t CLV(CPU6502* cpu)
{

}
uint8_t CMP(CPU6502* cpu)
{

}	
uint8_t CPX(CPU6502* cpu)
{

}
uint8_t CPY(CPU6502* cpu)
{

}
uint8_t DEC(CPU6502* cpu)
{

}
uint8_t DEX(CPU6502* cpu)
{

}	
uint8_t DEY(CPU6502* cpu)
{

}
uint8_t EOR(CPU6502* cpu)
{

}  
uint8_t INC(CPU6502* cpu)
{

}
uint8_t INX(CPU6502* cpu)
{

}	
uint8_t INY(CPU6502* cpu)
{

}
uint8_t JMP(CPU6502* cpu)
{

}  
uint8_t JSR(CPU6502* cpu)
{

}
uint8_t LDA(CPU6502* cpu)
{

}	
uint8_t LDX(CPU6502* cpu)
{

}
uint8_t LDY(CPU6502* cpu)
{

}  
uint8_t LSR(CPU6502* cpu)
{

}
uint8_t NOP(CPU6502* cpu)
{

}	
uint8_t ORA(CPU6502* cpu)
{

}
uint8_t PHA(CPU6502* cpu)
{

}  
uint8_t PHP(CPU6502* cpu)
{

}
uint8_t PLA(CPU6502* cpu)
{

}	
uint8_t PLP(CPU6502* cpu)
{

}
uint8_t ROL(CPU6502* cpu)
{

}
uint8_t ROR(CPU6502* cpu)
{

}
uint8_t RTI(CPU6502* cpu)
{

}	
uint8_t RTS(CPU6502* cpu)
{

}
uint8_t SBC(CPU6502* cpu)
{

}  
uint8_t SEC(CPU6502* cpu)
{

}
uint8_t SED(CPU6502* cpu)
{

}	
uint8_t SEI(CPU6502* cpu)
{

}
uint8_t STA(CPU6502* cpu)
{
    CPU_write(cpu, cpu->ic.addr_abs, cpu->a);

    return 0;
}  
uint8_t STX(CPU6502* cpu)
{
    CPU_write(cpu, cpu->ic.addr_abs, cpu->x);

    return 0;
}
uint8_t STY(CPU6502* cpu)
{
    CPU_write(cpu, cpu->ic.addr_abs, cpu->y);

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