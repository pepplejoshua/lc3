#pragma once

#define MEM_MAX (1 << 16) // 65536 locations in memory

// 65536 locations each 16 bits wide
// 65536 * (16 / 8)
// 131072 bytes = 128 KB
uint16_t mem[MEM_MAX];

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
  FL_POS = 1 << 0, // P = 1
  FL_ZRO = 1 << 1, // Z = 2
  FL_NEG = 1 << 2, // N = 4
};

// 10 registers
// - 8 general purpose registers
// - 1 program counter register
// - 1 condition register
uint16_t reg[R_COUNT];

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
