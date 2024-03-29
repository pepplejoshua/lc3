#pragma once

typedef uint16_t u16;
#define MEM_MAX (1 << 16) // 65536 locations in memory

// 65536 locations each 16 bits wide
// 65536 * (16 / 8)
// 131072 bytes = 128 KB
u16 mem[MEM_MAX];

enum {
  R_R0 = 0,
  R_R1,
  R_R2,
  R_R3,
  R_R4,
  R_R5,
  R_R6,
  R_R7,
  R_PC, // program counter register
  R_COND,
  R_COUNT
};

enum {
  FL_POS = 1 << 0, // P = 001 or 1
  FL_ZRO = 1 << 1, // Z = 010 or 2
  FL_NEG = 1 << 2, // N = 100 or 4
};

// 10 registers
// - 8 general purpose registers
// - 1 program counter register
// - 1 condition register
u16 reg[R_COUNT];

enum {
  Op_Br = 0,  // branch
  Op_Add,     // add
  Op_Ld,      // load
  Op_St,      // store
  Op_Jsr,     // jump to register
  Op_And,     // bitwise and
  Op_Ldr,     // load from register
  Op_Str,     // store register
  Op_RTI,     // UNUSED
  Op_Not,     // bitwise not
  Op_Ldi,     // load indirect
  Op_Sti,     // store indirect
  Op_Jmp,     // jump
  Op_RES,     // reserved (UNUSED)
  Op_Lea,     // load effective address
  Op_Trap,    // execute trap
};

enum {
  TRAP_GET_C = 0x20, // get character from keyboard, not echoed into terminal
  TRAP_OUT_C = 0x21, // output a character to terminal
  TRAP_PUT_S = 0x22, // output a word string
  TRAP_IN_C = 0x23, // get character from keyboard, echoed onto the terminal
  TRAP_PUT_SP = 0x24, // output a byte string
  TRAP_HALT = 0x25, // halt the program
};