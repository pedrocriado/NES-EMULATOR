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
    // Registers
    uint8_t a;		// Accumulator
    uint8_t x;		// X Register
    uint8_t y;	    // Y Register
    uint8_t p;      // Processor Status
    uint8_t s;		// Stack Pointer
    uint16_t pc;    // Program Counter
    
    // Internal helper variables
    InstructionContext ic;

    // Connected Devices
    struct Bus* bus;
} CPU6502;

void CPU_init(CPU6502* cpu);
uint8_t CPU_read(CPU6502* cpu, uint16_t addr);
void CPU_write(CPU6502* cpu, uint16_t addr, uint8_t data);
void CPU_clock(CPU6502* cpu);
void CPU_reset(CPU6502* cpu);
void CPU_irq(CPU6502* cpu);
void CPU_nmi(CPU6502* cpu);

void CPU_branch_helper(CPU6502* cpu, bool condition);

uint8_t CPU_get_flag(CPU6502* cpu, Flags flag);
void CPU_set_flag(CPU6502* cpu, Flags flag, bool value);

uint8_t CPU_fetch(CPU6502* cpu);



// typedef enum {
//     ADC, AND, ASL, BCC, BCS, 
//     BEQ, BIT, BMI, BNE, BPL, 
//     BRK, BVC, BVS, CLC, CLD, 
//     CLI, CLV, CMP, CPX, CPY, 
//     DEC, DEX, DEY, EOR, INC, 
//     INX, INY, JMP, JSR, LDA, 
//     LDX, LDY, LSR, NOP, ORA, 
//     PHA, PHP, PLA, PLP, ROL, 
//     ROR, RTI, RTS, SBC, SEC, 
//     SED, SEI, STA, STX, STY, 
//     TAX, TAY, TSX, TXA, TXS, 
//     TYA, XXX  
// } Opcode;

// typedef enum {
//     IMP, IMM, ACC, ZP0, ZPX, 
//     ZPY, IZX, IZY, ABS, ABX, 
//     ABY, IND, REL 
// } AddressMode;

typedef uint8_t (*OpcodeFunction)(CPU6502* cpu);
typedef uint8_t (*AddressModeFunction)(CPU6502* cpu);

typedef struct INSTRUCTION {
    OpcodeFunction operate;
    AddressModeFunction addrmode;
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