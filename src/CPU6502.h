#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "Bus.h"

typedef enum Flags
{
    C = (1 << 0),	// Carry Bit
    Z = (1 << 1),	// Zero
    I = (1 << 2),	// Disable Interrupts
    D = (1 << 3),	// Decimal Mode
    B = (1 << 4),	// Break
    U = (1 << 5),	// Unused
    V = (1 << 6),	// Overflow
    N = (1 << 7),	// Negative
} Flags;

typedef struct InstructionContext{
    uint8_t fetched;
    uint16_t addr_abs;
    uint16_t addr_rel;
    uint8_t opcode;
    uint8_t cycles;
} InstructionContext;

struct Bus;

typedef struct CPU6502
{
    struct Bus* bus;

    // Registers
    uint8_t a;		// Accumulator
    uint8_t x;		// X Register
    uint8_t y;	    // Y Register
    uint8_t p;      // Processor Status
    uint8_t s;		// Stack Pointer
    uint16_t pc;    // Program Counter
    
    // Internal helper variables
    InstructionContext ic;

    // Interrupts
    bool nmi, irq;
    bool irqDisable;
    bool irqDisableLatch;
    uint8_t irqDelay;
    bool suppressNmiOnce;
} CPU6502;

void CPU_init(CPU6502* cpu);
void CPU_free(CPU6502* cpu);
void CPU_clock(CPU6502* cpu);
void CPU_reset(CPU6502* cpu);
void CPU_irq(CPU6502* cpu);
void CPU_nmi(CPU6502* cpu);

void CPU_branch_helper(CPU6502* cpu, bool condition);

uint8_t CPU_get_flag(CPU6502* cpu, Flags flag);
void CPU_set_flag(CPU6502* cpu, Flags flag, bool value);

uint8_t CPU_fetch(CPU6502* cpu);

typedef struct INSTRUCTION {
    uint8_t (*operate)(CPU6502* cpu);
    uint8_t (*addrmode)(CPU6502* cpu);
    uint8_t cycle_cnt;
} INSTRUCTION;

// Opcode functions
uint8_t ADC(CPU6502* cpu);	uint8_t AND(CPU6502* cpu);	
uint8_t ASL(CPU6502* cpu);	uint8_t BCC(CPU6502* cpu);  
uint8_t BCS(CPU6502* cpu);	uint8_t BEQ(CPU6502* cpu);	
uint8_t BIT(CPU6502* cpu);	uint8_t BMI(CPU6502* cpu);  
uint8_t BNE(CPU6502* cpu);	uint8_t BPL(CPU6502* cpu);	
uint8_t BRK(CPU6502* cpu);	uint8_t BVC(CPU6502* cpu);  
uint8_t BVS(CPU6502* cpu);	uint8_t CLC(CPU6502* cpu);	
uint8_t CLD(CPU6502* cpu);	uint8_t CLI(CPU6502* cpu);  
uint8_t CLV(CPU6502* cpu);	uint8_t CMP(CPU6502* cpu);	
uint8_t CPX(CPU6502* cpu);	uint8_t CPY(CPU6502* cpu);
uint8_t DEC(CPU6502* cpu);	uint8_t DEX(CPU6502* cpu);	
uint8_t DEY(CPU6502* cpu);	uint8_t EOR(CPU6502* cpu);  
uint8_t INC(CPU6502* cpu);	uint8_t INX(CPU6502* cpu);	
uint8_t INY(CPU6502* cpu);	uint8_t JMP(CPU6502* cpu);  
uint8_t JSR(CPU6502* cpu);	uint8_t LDA(CPU6502* cpu);	
uint8_t LDX(CPU6502* cpu);	uint8_t LDY(CPU6502* cpu);  
uint8_t LSR(CPU6502* cpu);	uint8_t NOP(CPU6502* cpu);	
uint8_t ORA(CPU6502* cpu);	uint8_t PHA(CPU6502* cpu);  
uint8_t PHP(CPU6502* cpu);	uint8_t PLA(CPU6502* cpu);	
uint8_t PLP(CPU6502* cpu);	uint8_t ROL(CPU6502* cpu);
uint8_t ROR(CPU6502* cpu);	uint8_t RTI(CPU6502* cpu);	
uint8_t RTS(CPU6502* cpu);	uint8_t SBC(CPU6502* cpu);  
uint8_t SEC(CPU6502* cpu);	uint8_t SED(CPU6502* cpu);	
uint8_t SEI(CPU6502* cpu);	uint8_t STA(CPU6502* cpu);  
uint8_t STX(CPU6502* cpu);	uint8_t STY(CPU6502* cpu);	
uint8_t TAX(CPU6502* cpu);	uint8_t TAY(CPU6502* cpu);  
uint8_t TSX(CPU6502* cpu);	uint8_t TXA(CPU6502* cpu);	
uint8_t TXS(CPU6502* cpu);	uint8_t TYA(CPU6502* cpu);  
uint8_t XXX(CPU6502* cpu);

// Addressing modes functions
uint8_t IMP(CPU6502* cpu);	uint8_t ACC(CPU6502* cpu);
uint8_t IMM(CPU6502* cpu);	uint8_t ZP0(CPU6502* cpu);	
uint8_t ZPX(CPU6502* cpu);	uint8_t ZPY(CPU6502* cpu);  
uint8_t IZX(CPU6502* cpu);  uint8_t IZY(CPU6502* cpu);	
uint8_t ABS(CPU6502* cpu);	uint8_t ABX(CPU6502* cpu);	
uint8_t ABY(CPU6502* cpu);  uint8_t IND(CPU6502* cpu);  
uint8_t REL(CPU6502* cpu);	

static const INSTRUCTION lookup[256] = 
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
	{ BEQ, REL, 2 },{ SBC, IZY, 5 },{ XXX, IMP, 2 },{ XXX, IMP, 8 },{ NOP, IMP, 4 },{ SBC, ZPX, 4 },{ INC, ZPX, 6 },{ XXX, IMP, 6 },{ SED, IMP, 2 },{ SBC, ABY, 4 },{ NOP, IMP, 2 },{ XXX, IMP, 7 },{ NOP, IMP, 4 },{ SBC, ABX, 4 },{ INC, ABX, 7 },{ XXX, IMP, 7 },
};