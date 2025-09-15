#include <stdio.h>
#include <stdint.h>

#include "CPU6502.h"

INSTRUCTION lookup[256] = 
{
	{ BRK, IMM, 7 },{ ORA, IZX, 6 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 3 },{ ORA, ZP0, 3 },{ ASL, ZP0, 5 },{ XXX, IMP, 5 },{ PHP, IMP, 3 },{ ORA, IMM, 2 },{ ASL, ACC, 2 },{ XXX, IMP, 2 },{ NOP, IMP, 4 },{ ORA, ABS, 4 },{ ASL, ABS, 6 },{ XXX, IMP, 6 },
	{ BPL, REL, 2 },{ ORA, IZY, 5 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 4 },{ ORA, ZPX, 4 },{ ASL, ZPX, 6 },{ XXX, IMP, 6 },{ CLC, IMP, 2 },{ ORA, ABY, 4 },{ NOP, IMP, 2 },{ XXX, IMP, 7 },{ NOP, IMP, 4 },{ ORA, ABX, 4 },{ ASL, ABX, 7 },{ XXX, IMP, 7 },
	{ JSR, ABS, 6 },{ AND, IZX, 6 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ BIT, ZP0, 3 },{ AND, ZP0, 3 },{ ROL, ZP0, 5 },{ XXX, IMP, 5 },{ PLP, IMP, 4 },{ AND, IMM, 2 },{ ROL, ACC, 2 },{ XXX, IMP, 2 },{ BIT, ABS, 4 },{ AND, ABS, 4 },{ ROL, ABS, 6 },{ XXX, IMP, 6 },
	{ BMI, REL, 2 },{ AND, IZY, 5 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 4 },{ AND, ZPX, 4 },{ ROL, ZPX, 6 },{ XXX, IMP, 6 },{ SEC, IMP, 2 },{ AND, ABY, 4 },{ NOP, IMP, 2 },{ XXX, IMP, 7 },{ NOP, IMP, 4 },{ AND, ABX, 4 },{ ROL, ABX, 7 },{ XXX, IMP, 7 },
	{ RTI, IMP, 6 },{ EOR, IZX, 6 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 3 },{ EOR, ZP0, 3 },{ LSR, ZP0, 5 },{ XXX, IMP, 5 },{ PHA, IMP, 3 },{ EOR, IMM, 2 },{ LSR, ACC, 2 },{ XXX, IMP, 2 },{ JMP, ABS, 3 },{ EOR, ABS, 4 },{ LSR, ABS, 6 },{ XXX, IMP, 6 },
	{ BVC, REL, 2 },{ EOR, IZY, 5 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 4 },{ EOR, ZPX, 4 },{ LSR, ZPX, 6 },{ XXX, IMP, 6 },{ CLI, IMP, 2 },{ EOR, ABY, 4 },{ NOP, IMP, 2 },{ XXX, IMP, 7 },{ NOP, IMP, 4 },{ EOR, ABX, 4 },{ LSR, ABX, 7 },{ XXX, IMP, 7 },
    { RTS, IMP, 6 },{ ADC, IZX, 6 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 3 },{ ADC, ZP0, 3 },{ ROR, ZP0, 5 },{ XXX, IMP, 5 },{ PLA, IMP, 4 },{ ADC, IMM, 2 },{ ROR, ACC, 2 },{ XXX, IMP, 2 },{ JMP, IND, 5 },{ ADC, ABS, 4 },{ ROR, ABS, 6 },{ XXX, IMP, 6 },
	{ BVS, REL, 2 },{ ADC, IZY, 5 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 4 },{ ADC, ZPX, 4 },{ ROR, ZPX, 6 },{ XXX, IMP, 6 },{ SEI, IMP, 2 },{ ADC, ABY, 4 },{ NOP, IMP, 2 },{ XXX, IMP, 7 },{ NOP, IMP, 4 },{ ADC, ABX, 4 },{ ROR, ABX, 7 },{ XXX, IMP, 7 },
	{ NOP, IMP, 2 },{ STA, IZX, 6 },{ NOP, IMP, 2 },{ XXX, IMP, 6 },{ STY, ZP0, 3 },{ STA, ZP0, 3 },{ STX, ZP0, 3 },{ XXX, IMP, 3 },{ DEY, IMP, 2 },{ NOP, IMP, 2 },{ TXA, IMP, 2 },{ XXX, IMP, 2 },{ STY, ABS, 4 },{ STA, ABS, 4 },{ STX, ABS, 4 },{ XXX, IMP, 4 },
	{ BCC, REL, 2 },{ STA, IZY, 6 },{ XXX, IMP, 2 },{ XXX, IMP, 6 },{ STY, ZPX, 4 },{ STA, ZPX, 4 },{ STX, ZPY, 4 },{ XXX, IMP, 4 },{ TYA, IMP, 2 },{ STA, ABY, 5 },{ TXS, IMP, 2 },{ XXX, IMP, 5 },{ NOP, IMP, 5 },{ STA, ABX, 5 },{ XXX, IMP, 5 },{ XXX, IMP, 5 },
	{ LDY, IMM, 2 },{ LDA, IZX, 6 },{ LDX, IMM, 2 },{ XXX, IMP, 6 },{ LDY, ZP0, 3 },{ LDA, ZP0, 3 },{ LDX, ZP0, 3 },{ XXX, IMP, 3 },{ TAY, IMP, 2 },{ LDA, IMM, 2 },{ TAX, IMP, 2 },{ XXX, IMP, 2 },{ LDY, ABS, 4 },{ LDA, ABS, 4 },{ LDX, ABS, 4 },{ XXX, IMP, 4 },
	{ BCS, REL, 2 },{ LDA, IZY, 5 },{ XXX, IMP, 2 },{ XXX, IMP, 5 },{ LDY, ZPX, 4 },{ LDA, ZPX, 4 },{ LDX, ZPY, 4 },{ XXX, IMP, 4 },{ CLV, IMP, 2 },{ LDA, ABY, 4 },{ TSX, IMP, 2 },{ XXX, IMP, 4 },{ LDY, ABX, 4 },{ LDA, ABX, 4 },{ LDX, ABY, 4 },{ XXX, IMP, 4 },
	{ CPY, IMM, 2 },{ CMP, IZX, 6 },{ NOP, IMP, 2 },{ XXX, IMP, 8 },{ CPY, ZP0, 3 },{ CMP, ZP0, 3 },{ DEC, ZP0, 5 },{ XXX, IMP, 5 },{ INY, IMP, 2 },{ CMP, IMM, 2 },{ DEX, IMP, 2 },{ XXX, IMP, 2 },{ CPY, ABS, 4 },{ CMP, ABS, 4 },{ DEC, ABS, 6 },{ XXX, IMP, 6 },
	{ BNE, REL, 2 },{ CMP, IZY, 5 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 4 },{ CMP, ZPX, 4 },{ DEC, ZPX, 6 },{ XXX, IMP, 6 },{ CLD, IMP, 2 },{ CMP, ABY, 4 },{ NOP, IMP, 2 },{ XXX, IMP, 7 },{ NOP, IMP, 4 },{ CMP, ABX, 4 },{ DEC, ABX, 7 },{ XXX, IMP, 7 },
	{ CPX, IMM, 2 },{ SBC, IZX, 6 },{ NOP, IMP, 2 },{ XXX, IMP, 8 },{ CPX, ZP0, 3 },{ SBC, ZP0, 3 },{ INC, ZP0, 5 },{ XXX, IMP, 5 },{ INX, IMP, 2 },{ SBC, IMM, 2 },{ NOP, IMP, 2 },{ SBC, IMP, 2 },{ CPX, ABS, 4 },{ SBC, ABS, 4 },{ INC, ABS, 6 },{ XXX, IMP, 6 },
	{ BEQ, REL, 2 },{ SBC, IZY, 5 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 4 },{ SBC, ZPX, 4 },{ INC, ZPX, 6 },{ XXX, IMP, 6 },{ SED, IMP, 2 },{ SBC, ABY, 4 },{ NOP, IMP, 2 },{ XXX, IMP, 7 },{ NOP, IMP, 4 },{ SBC, ABX, 4 },{ INC, ABX, 7 },{ XXX, IMP, 7 }
};

void CPU_init(CPU6502* cpu)
{
    cpu->a = 0x00;
    cpu->x = 0x00;
    cpu->y = 0x00;
    cpu->s = 0xFD;
    //TODO cylces: add cycle-accurate IRQ timing.
    //Note: The effect of changing this flag is delayed 1 instruction
    cpu->p = I | U;

    uint8_t low = CPU_read(cpu, 0xFFFC);
    uint8_t high = CPU_read(cpu, 0xFFFD);
    cpu->pc = ((uint16_t)high << 8) | low;

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

        cpu->ic.cycles = lookup[opcode].cycle_cnt;

        uint8_t extra_cycles1 = lookup[opcode].operate(cpu);
        uint8_t extra_cycles2 = lookup[opcode].addrmode(cpu);

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

    uint8_t low = CPU_read(cpu, 0xFFFC);
    uint8_t high = CPU_read(cpu, 0xFFFD);
    cpu->pc = ((uint16_t)high << 8) | low;

    cpu->ic = (InstructionContext){0,0,0,0,7};
}

void CPU_irq(CPU6502* cpu)
{
    if(CPU_get_flag(cpu, I) == 0)
    {
        CPU_write(cpu, 0x0100 + cpu->s--, (cpu->pc >> 8) & 0x00FF);
        CPU_write(cpu, 0x0100 + cpu->s--, cpu->pc & 0x00FF);

        CPU_set_flag(cpu, B, 0);
        CPU_set_flag(cpu, U, 1);
        CPU_set_flag(cpu, I, 1);

        CPU_write(cpu, 0x0100 + cpu->s--, cpu->p);

        uint16_t low = CPU_read(cpu, 0xFFFE);
        uint16_t high = CPU_read(cpu, 0xFFFF);

        cpu->pc = (high << 8) | low;

        cpu->ic.cycles = 7;
    }
}

void CPU_nmi(CPU6502* cpu)
{
    CPU_write(cpu, 0x0100 + cpu->s--, (cpu->pc >> 8) & 0x00FF);
    CPU_write(cpu, 0x0100 + cpu->s--, cpu->pc & 0x00FF);

    CPU_set_flag(cpu, B, 0);
    CPU_set_flag(cpu, U, 1);
    CPU_set_flag(cpu, I, 1);

    CPU_write(cpu, 0x0100 + cpu->s--, cpu->p);

    uint16_t low = CPU_read(cpu, 0xFFFA);
    uint16_t high = CPU_read(cpu, 0xFFFB);

    cpu->pc = (high << 8) | low;

    cpu->ic.cycles = 8;
}

uint8_t CPU_fetch(CPU6502* cpu)
{
    if(!(lookup[cpu->ic.opcode].addrmode == &IMP)
       && !(lookup[cpu->ic.opcode].addrmode == &ACC))
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

    if(lookup[cpu->ic.opcode].operate == &ACC)
        cpu->a = temp & 0x00FF;
    else
        CPU_write(cpu, cpu->ic.addr_abs, temp & 0x00FF);

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
    CPU_write(cpu, 0x0100 + cpu->s--, (cpu->pc >> 8) & 0x00FF);
    CPU_write(cpu, 0x0100 + cpu->s--, cpu->pc & 0x00FF);

    CPU_set_flag(cpu, I, 1);
    CPU_set_flag(cpu, B, 1);

    CPU_write(cpu, 0x0100 + cpu->s--, cpu->p);

    CPU_set_flag(cpu, B, 0);

    cpu->pc = (uint16_t) (CPU_read(cpu, 0xFFFF) << 8) | (uint16_t) CPU_read(cpu, 0xFFFE);

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

    CPU_write(cpu, cpu->ic.addr_abs, memory &0x00FF);

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

    CPU_write(cpu, cpu->ic.addr_abs, memory);

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

    CPU_write(cpu, 0x0100 + cpu->s--, (cpu->pc >> 8) & 0x00FF);
    CPU_write(cpu, 0x0100 + cpu->s--, cpu->pc & 0x00FF);

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

    if(lookup[cpu->ic.opcode].addrmode == &ACC)
    {
        cpu->a = memory;
    }
    else
    {
        CPU_write(cpu, cpu->ic.addr_abs, memory);
    }

    return 0;
}
uint8_t NOP(CPU6502* cpu)
{
    //TODO actions:
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
    CPU_write(cpu, 0x0100 + cpu->s--, cpu->a);
    return 0;
}  
uint8_t PHP(CPU6502* cpu)
{

    CPU_write(cpu, 0x0100 + cpu->s--, cpu->p | B | U);

    return 0;
}
uint8_t PLA(CPU6502* cpu)
{
    cpu->s++;
    cpu->a = CPU_read(cpu, 0x0100 + cpu->s);

    CPU_set_flag(cpu, Z, cpu->a == 0);
    CPU_set_flag(cpu, N, cpu->a & 0x80);

    return 0;
}	
uint8_t PLP(CPU6502* cpu)
{
    //TODO cylces: add cycle-accurate IRQ timing.
    //Note: The effect of changing this flag is delayed 1 instruction
    cpu->s++;
    cpu->p= CPU_read(cpu, 0x0100 + cpu->s);

    return 0;
}
uint8_t ROL(CPU6502* cpu)
{
    uint16_t memory = CPU_fetch(cpu);
    
    memory = (memory << 1) | CPU_get_flag(cpu, C);

    CPU_set_flag(cpu, C, memory & 0xFF00);
    CPU_set_flag(cpu, Z, (memory & 0x00FF) == 0x0000);
    CPU_set_flag(cpu, N, memory & 0x0080);

    if(lookup[cpu->ic.opcode].addrmode == &ACC)
        cpu->a = memory & 0x00FF;
    else
        CPU_write(cpu, cpu->ic.addr_abs, memory & 0x00FF);

    return 0;
}
uint8_t ROR(CPU6502* cpu)
{
    uint16_t memory = CPU_fetch(cpu);

    CPU_set_flag(cpu, C, memory & 0x0001);

    memory = ((uint16_t)CPU_get_flag(cpu, C) << 7) | (memory >> 1);

    CPU_set_flag(cpu, Z, (memory & 0x00FF) == 0x0000);
    CPU_set_flag(cpu, N, memory & 0x0080);

    if(lookup[cpu->ic.opcode].addrmode == &ACC)
        cpu->a = memory & 0x00FF;
    else
        CPU_write(cpu, cpu->ic.addr_abs, memory & 0x00FF);

    return 0;
}
uint8_t RTI(CPU6502* cpu)
{
    cpu->s++;
    cpu->p = CPU_read(cpu, 0x0100 + cpu->s++);
    cpu->p &= ~B;
    cpu->p &= ~U;

    cpu->pc = (uint16_t)CPU_read(cpu, 0x0100 + cpu->s + 1) << 8 |
              (uint16_t)CPU_read(cpu, 0x0100 + cpu->s++);

    return 0;
}	
uint8_t RTS(CPU6502* cpu)
{
    cpu->s++;
    cpu->p = (uint16_t)CPU_read(cpu, 0x0100 + cpu->s++);
    cpu->p &= ~B;
    cpu->p &= ~U;

    cpu->pc = ((uint16_t)CPU_read(cpu, 0x0100 + cpu->s) << 8) |
              CPU_read(cpu, 0x0100 + cpu->s++);

    return 0;
}
uint8_t SBC(CPU6502* cpu)
{
    uint16_t memory = CPU_fetch(cpu);

    memory = memory ^ 0x00FF;

    uint16_t temp = (uint16_t)cpu->a + memory + (uint16_t)(~CPU_get_flag(cpu, C)); 

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