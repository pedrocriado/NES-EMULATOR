#pragma once
#include <stdio.h>
#include <stdbool.h>

#include "Bus.h"

typedef enum
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

typedef struct CPU6502
{
    uint8_t a;		// Accumulator
    uint8_t x;		// X Register
    uint8_t y;	    // Y Register
    uint8_t p;      // Processor Status
    uint8_t s;		// Stack Pointer
    uint16_t pc;    // Program Counter
    
    struct Bus* bus;
} CPU6502;

void CPU_init();
void CPU_clock();
void CPU_reset();
void CPU_irq();
void CPU_nmi();

uint8_t CPU_get_flag(Flags flag);
void CPU_set_flag(Flags flag, bool value);

uint8_t CPU_fetch();

uint8_t fetched   = 0x00;
uint16_t temp      = 0x0000;
uint16_t addr_abs = 0x0000;
uint16_t addr_rel = 0x00;
uint8_t opcode    = 0x00;
uint8_t cycles    = 0;

typedef enum {
    ADC, AND, ASL, BCC, BCS, 
    BEQ, BIT, BMI, BNE, BPL, 
    BRK, BVC, BVS, CLC, CLD, 
    CLI, CLV, CMP, CPX, CPY, 
    DEC, DEX, DEY, EOR, INC, 
    INX, INY, JMP, JSR, LDA, 
    LDX, LDY, LSR, NOP, ORA, 
    PHA, PHP, PLA, PLP, ROL, 
    ROR, RTI, RTS, SBC, SEC, 
    SED, SEI, STA, STX, STY, 
    TAX, TAY, TSX, TXA, TXS, 
    TYA, XXX  
} Opcode;

typedef enum {
    IMP, IMM, ZP0, ZPX, ZPY, 
    IZX, IZY, ABS, ABX, ABY, 
    IND, REL 
} AddressMode;

typedef uint8_t (*OpcodeFunction)();
typedef uint8_t (*AddressModeFunction)();

typedef struct INSTRUCTION {
    char* name;
    OpcodeFunction operate;
    AddressModeFunction addrmode;
    uint8_t cycles;
} INSTRUCTION;

INSTRUCTION lookup[256];

lookup = 
{
	{  BRK, IMM, 7 },{  ORA, IZX, 6 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  NOP, IMP, 3 },{  ORA, ZP0, 3 },{  ASL, ZP0, 5 },{  XXX, IMP, 5 },{  PHP, IMP, 3 },{  ORA, IMM, 2 },{  ASL, IMP, 2 },{  XXX, IMP, 2 },{  NOP, IMP, 4 },{  ORA, ABS, 4 },{  ASL, ABS, 6 },{  XXX, IMP, 6 },
	{  BPL, REL, 2 },{  ORA, IZY, 5 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  NOP, IMP, 4 },{  ORA, ZPX, 4 },{  ASL, ZPX, 6 },{  XXX, IMP, 6 },{  CLC, IMP, 2 },{  ORA, ABY, 4 },{  NOP, IMP, 2 },{  XXX, IMP, 7 },{  NOP, IMP, 4 },{  ORA, ABX, 4 },{  ASL, ABX, 7 },{  XXX, IMP, 7 },
	{  JSR, ABS, 6 },{  AND, IZX, 6 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  BIT, ZP0, 3 },{  AND, ZP0, 3 },{  ROL, ZP0, 5 },{  XXX, IMP, 5 },{  PLP, IMP, 4 },{  AND, IMM, 2 },{  ROL, IMP, 2 },{  XXX, IMP, 2 },{  BIT, ABS, 4 },{  AND, ABS, 4 },{  ROL, ABS, 6 },{  XXX, IMP, 6 },
	{  BMI, REL, 2 },{  AND, IZY, 5 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  NOP, IMP, 4 },{  AND, ZPX, 4 },{  ROL, ZPX, 6 },{  XXX, IMP, 6 },{  SEC, IMP, 2 },{  AND, ABY, 4 },{  NOP, IMP, 2 },{  XXX, IMP, 7 },{  NOP, IMP, 4 },{  AND, ABX, 4 },{  ROL, ABX, 7 },{  XXX, IMP, 7 },
	{  RTI, IMP, 6 },{  EOR, IZX, 6 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  NOP, IMP, 3 },{  EOR, ZP0, 3 },{  LSR, ZP0, 5 },{  XXX, IMP, 5 },{  PHA, IMP, 3 },{  EOR, IMM, 2 },{  LSR, IMP, 2 },{  XXX, IMP, 2 },{  JMP, ABS, 3 },{  EOR, ABS, 4 },{  LSR, ABS, 6 },{  XXX, IMP, 6 },
	{  BVC, REL, 2 },{  EOR, IZY, 5 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  NOP, IMP, 4 },{  EOR, ZPX, 4 },{  LSR, ZPX, 6 },{  XXX, IMP, 6 },{  CLI, IMP, 2 },{  EOR, ABY, 4 },{  NOP, IMP, 2 },{  XXX, IMP, 7 },{  NOP, IMP, 4 },{  EOR, ABX, 4 },{  LSR, ABX, 7 },{  XXX, IMP, 7 },
    {  RTS, IMP, 6 },{  ADC, IZX, 6 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  NOP, IMP, 3 },{  ADC, ZP0, 3 },{  ROR, ZP0, 5 },{  XXX, IMP, 5 },{  PLA, IMP, 4 },{  ADC, IMM, 2 },{  ROR, IMP, 2 },{  XXX, IMP, 2 },{  JMP, IND, 5 },{  ADC, ABS, 4 },{  ROR, ABS, 6 },{  XXX, IMP, 6 },
	{  BVS, REL, 2 },{  ADC, IZY, 5 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  NOP, IMP, 4 },{  ADC, ZPX, 4 },{  ROR, ZPX, 6 },{  XXX, IMP, 6 },{  SEI, IMP, 2 },{  ADC, ABY, 4 },{  NOP, IMP, 2 },{  XXX, IMP, 7 },{  NOP, IMP, 4 },{  ADC, ABX, 4 },{  ROR, ABX, 7 },{  XXX, IMP, 7 },
	{  NOP, IMP, 2 },{  STA, IZX, 6 },{  NOP, IMP, 2 },{  XXX, IMP, 6 },{  STY, ZP0, 3 },{  STA, ZP0, 3 },{  STX, ZP0, 3 },{  XXX, IMP, 3 },{  DEY, IMP, 2 },{  NOP, IMP, 2 },{  TXA, IMP, 2 },{  XXX, IMP, 2 },{  STY, ABS, 4 },{  STA, ABS, 4 },{  STX, ABS, 4 },{  XXX, IMP, 4 },
	{  BCC, REL, 2 },{  STA, IZY, 6 },{  XXX, IMP, 2 },{  XXX, IMP, 6 },{  STY, ZPX, 4 },{  STA, ZPX, 4 },{  STX, ZPY, 4 },{  XXX, IMP, 4 },{  TYA, IMP, 2 },{  STA, ABY, 5 },{  TXS, IMP, 2 },{  XXX, IMP, 5 },{  NOP, IMP, 5 },{  STA, ABX, 5 },{  XXX, IMP, 5 },{  XXX, IMP, 5 },
	{  LDY, IMM, 2 },{  LDA, IZX, 6 },{  LDX, IMM, 2 },{  XXX, IMP, 6 },{  LDY, ZP0, 3 },{  LDA, ZP0, 3 },{  LDX, ZP0, 3 },{  XXX, IMP, 3 },{  TAY, IMP, 2 },{  LDA, IMM, 2 },{  TAX, IMP, 2 },{  XXX, IMP, 2 },{  LDY, ABS, 4 },{  LDA, ABS, 4 },{  LDX, ABS, 4 },{  XXX, IMP, 4 },
	{  BCS, REL, 2 },{  LDA, IZY, 5 },{  XXX, IMP, 2 },{  XXX, IMP, 5 },{  LDY, ZPX, 4 },{  LDA, ZPX, 4 },{  LDX, ZPY, 4 },{  XXX, IMP, 4 },{  CLV, IMP, 2 },{  LDA, ABY, 4 },{  TSX, IMP, 2 },{  XXX, IMP, 4 },{  LDY, ABX, 4 },{  LDA, ABX, 4 },{  LDX, ABY, 4 },{  XXX, IMP, 4 },
	{  CPY, IMM, 2 },{  CMP, IZX, 6 },{  NOP, IMP, 2 },{  XXX, IMP, 8 },{  CPY, ZP0, 3 },{  CMP, ZP0, 3 },{  DEC, ZP0, 5 },{  XXX, IMP, 5 },{  INY, IMP, 2 },{  CMP, IMM, 2 },{  DEX, IMP, 2 },{  XXX, IMP, 2 },{  CPY, ABS, 4 },{  CMP, ABS, 4 },{  DEC, ABS, 6 },{  XXX, IMP, 6 },
	{  BNE, REL, 2 },{  CMP, IZY, 5 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  NOP, IMP, 4 },{  CMP, ZPX, 4 },{  DEC, ZPX, 6 },{  XXX, IMP, 6 },{  CLD, IMP, 2 },{  CMP, ABY, 4 },{  NOP, IMP, 2 },{  XXX, IMP, 7 },{  NOP, IMP, 4 },{  CMP, ABX, 4 },{  DEC, ABX, 7 },{  XXX, IMP, 7 },
	{  CPX, IMM, 2 },{  SBC, IZX, 6 },{  NOP, IMP, 2 },{  XXX, IMP, 8 },{  CPX, ZP0, 3 },{  SBC, ZP0, 3 },{  INC, ZP0, 5 },{  XXX, IMP, 5 },{  INX, IMP, 2 },{  SBC, IMM, 2 },{  NOP, IMP, 2 },{  SBC, IMP, 2 },{  CPX, ABS, 4 },{  SBC, ABS, 4 },{  INC, ABS, 6 },{  XXX, IMP, 6 },
	{  BEQ, REL, 2 },{  SBC, IZY, 5 },{  XXX, IMP, 2 },{  XXX, IMP, 8 },{  NOP, IMP, 4 },{  SBC, ZPX, 4 },{  INC, ZPX, 6 },{  XXX, IMP, 6 },{  SED, IMP, 2 },{  SBC, ABY, 4 },{  NOP, IMP, 2 },{  XXX, IMP, 7 },{  NOP, IMP, 4 },{  SBC, ABX, 4 },{  INC, ABX, 7 },{  XXX, IMP, 7 }
};

// Opcode functions
uint8_t ADC();	uint8_t AND();	
uint8_t ASL();	uint8_t BCC();  
uint8_t BCS();	uint8_t BEQ();	
uint8_t BIT();	uint8_t BMI();  
uint8_t BNE();	uint8_t BPL();	
uint8_t BRK();	uint8_t BVC();  
uint8_t BVS();	uint8_t CLC();	
uint8_t CLD();	uint8_t CLI();  
uint8_t CLV();	uint8_t CMP();	
uint8_t CPX();	uint8_t CPY();
uint8_t DEC();	uint8_t DEX();	
uint8_t DEY();	uint8_t EOR();  
uint8_t INC();	uint8_t INX();	
uint8_t INY();	uint8_t JMP();  
uint8_t JSR();	uint8_t LDA();	
uint8_t LDX();	uint8_t LDY();  
uint8_t LSR();	uint8_t NOP();	
uint8_t ORA();	uint8_t PHA();  
uint8_t PHP();	uint8_t PLA();	
uint8_t PLP();	uint8_t ROL();
uint8_t ROR();	uint8_t RTI();	
uint8_t RTS();	uint8_t SBC();  
uint8_t SEC();	uint8_t SED();	
uint8_t SEI();	uint8_t STA();  
uint8_t STX();	uint8_t STY();	
uint8_t TAX();	uint8_t TAY();  
uint8_t TSX();	uint8_t TXA();	
uint8_t TXS();	uint8_t TYA(); 
uint8_t XXX();

// Addressing modes functions
uint8_t IMP();	uint8_t IMM();	
uint8_t ZP0();	uint8_t ZPX();	
uint8_t ZPY();  uint8_t IZX();  
uint8_t IZY();	uint8_t ABS();	
uint8_t ABX();	uint8_t ABY();  
uint8_t IND();  uint8_t REL();	