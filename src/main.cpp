#include <iostream>
#include "arch.h"

using std::cout;

extern int read_img(char *);
extern u16 mem_read(u16);
extern void mem_write(u16 addr, u16 val);

u16 sext(u16 x, int bit_count) {
  // e.g: 01111 or 15 (which is also 5 bits) to 16 bits
  // 00000 & 1 (check if it is negative) = 0
  // since it isn't, it is returned unchanged

  // more applicably,
  // e.g: 10101 or -11 (which is 5 bits) to 16 bits
  // 00001 & 1 (check if it is negative) = 1
  if  ((x >> (bit_count - 1)) & 1) {
    // since it is, then we can extend it with 1s
    // 0xFFFF = 1111 1111 1111 1111
    // when we shift it left by bit count, in this case
    // we get:
    // 1111 1111 1111 1111 << 5
    // = 1111 1111 1110 0000
    // then we |= to imprint the lower 5 bits of x
    // so x = 1111 1111 1111 0101, which is also -11
    // which we can return.
    x |= (0xFFFF << bit_count);
  }
  return x;
}

void update_flags(u16 res_reg) {
  if (reg[res_reg] == 0)
    reg[R_COND] = FL_ZRO;
  else if (reg[res_reg] >> 15) // leftmost bit is 1, so negative
    reg[R_COND] = FL_NEG;
  else
    reg[R_COND] = FL_POS;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        // show usage
        cout << "lc [img_file img_file_2 ...]\n";
        exit(2);
    }

    // load and process images
    for (int j = 1; j < argc; ++j) {
      if (!read_img(argv[j])) {
        cout << "failed to load image: " << argv[j] << "\n";
        exit(1);
      }
    }

    // init R_COND to FL_ZRO flag
    reg[R_COND] = FL_ZRO;

    // set the program counter to start of user space
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    bool running = true;
    while (running) {
      // read current instruction
      u16 instr = mem_read(reg[R_PC]);
      // move to next instruction
      reg[R_PC]++;
      // |  op  |
      // | abcd | 0000 | 0000 | 0000 |
      // so we move right shift the instruction right by 12
      // and we get the opcode:
      //                      |  op  |
      // | 0000 | 0000 | 0000 | abcd |
      // notes on bitwise shifts:
      // https://gist.github.com/pepplejoshua/853145d5c89c200894e3a00cd662508a
      // some throw away code decoding 16 bit instructions:
      // https://gist.github.com/pepplejoshua/490fe6d201b29f940ff2564956453a75
      u16 op = instr >> 12;

      switch (op) {
        case Op_Add: {
          // Register mode
          // dst => destination register
          // sr1 => source register 1
          // F = 0 => register mode
          // sr2 => source register 2
          // |  op  | dst | sr1 | F | 00 | sr2 |
          // | 0001 | 000 | 000 | 0 | 00 | 000 |
          //
          // OR
          //
          // Immediate mode
          // dst => destination register
          // sr1 => source register 1
          // F = 1 => immediate mode
          // imm5 => an immediate 2's comp number -16..=15
          // |  op  | dst | sr1 | F |  imm5 |
          // | 0001 | 000 | 000 | 0 | 00000 |

          // dest
          u16 r0 = (instr >> 9) & 0x7;
          // sr1
          u16 r1 = (instr >> 6) & 0x7;
          // flag
          u16 imm_f = (instr >> 5) && 0x1;

          if (imm_f) {
            // sign extend the last 5 bits since we are in
            // immediate mode
            u16 imm5 = sext(instr & 0x1F, 5);
            reg[r0] = reg[r1] + imm5;
          } else {
            // we are in register mode
            u16 r2 = instr & 0x7;
            reg[r0] = reg[r1] + reg[r2];
          }

          update_flags(r0);
          break;
        }
        case Op_And: {
          // Register mode
          // dst => destination register
          // sr1 => source register 1
          // F = 0 => register mode
          // sr2 => source register 2
          // |  op  | dst | sr1 | F | 00 | sr2 |
          // | 0101 | 000 | 000 | 0 | 00 | 000 |
          //
          // OR
          //
          // Immediate mode
          // dst => destination register
          // sr1 => source register 1
          // F = 1 => immediate mode
          // imm5 => an immediate 2's comp number -16..=15
          // |  op  | dst | sr1 | F |  imm5 |
          // | 0101 | 000 | 000 | 0 | 00000 |

          // dest
          u16 r0 = (instr >> 9) & 0x7;
          // sr1
          u16 r1 = (instr >> 6) & 0x7;
          // flag
          u16 imm_f = (instr >> 5) && 0x1;

          if (imm_f) {
            // sign extend the last 5 bits since we are in
            // immediate mode
            u16 imm5 = sext(instr & 0x1F, 5);
            reg[r0] = reg[r1] & imm5;
          } else {
            // we are in register mode
            u16 r2 = instr & 0x7;
            reg[r0] = reg[r1] & reg[r2];
          }

          update_flags(r0);
          break;
        }
        case Op_Not: {
          // |  op  | dst | src |
          // | 1001 | 010 | 000 | 111111 |
          u16 dst = (instr >> 9) & 0x7;
          u16 src = (instr >> 6) & 0x7;
          reg[dst] = ~reg[src];
          update_flags(dst);
          break;
        }
        case Op_Br: {
          // |  op  | n | z | p |   PC Offset |
          // | 0000 | 0 | 0 | 0 | 000_000_000 |
          // n => if last operation produced negative result
          // z => if last operation produced 0
          // p => if last operation produced positive result
          // all flags can be enabled together or only 1 can be
          // enabled
          // if all flags are enabled, that is an unconditional
          // branch
          u16 flags = (instr >> 9) & 0x7;

          if (flags & reg[R_COND]) {
            u16 pc_offset = sext(instr & 0x1FF, 9);
            reg[R_PC] = reg[R_PC] + pc_offset;
          }
          break;
        }
        case Op_Jmp: {
          // Jmp mode
          // reg => any registers that isn't R_R7 or 0b111
          // PC <- reg
          // |  op  | 000 | reg | 000000 |
          // | 1100 | 000 | 000 | 000000 |
          // Above instruction is:
          // Jmp R0  =>  PC <- R0
          //
          // OR
          //
          // Ret mode
          // Special case of Jmp where we get linkage back to
          // the next instruction of the caller of a subroutine
          // PC <- R7
          // |  op  | 000 | reg | 000000 |
          // | 1100 | 000 | 111 | 000000 |
          // Ret => PC <- R7
          u16 base_reg = (instr >> 6) && 0x7;
          reg[R_PC] = reg[base_reg];
          break;
        }
        case Op_Jsr: {
          // Jump to Subroutine
          // To be combined with RET instruction (special Jmp)
          // it stores the location of the instruction following it
          // in R7 so we can return to it. Essentially tracking call site
          // information so callees can return to callers
          //
          // Jsr mode
          // |  op  | F |   PC Offset |
          // | 0100 | 1 | 00000000000 |
          // 1. R7 = PC
          // 2. PC += sext(PC Offset)
          // OR
          //
          // Jsrr mode
          // |  op  | F | 00 | reg | 000000 |
          // | 0100 | 0 | 00 | 000 | 000000 |
          // 1. R7 = PC
          // 2. PC = reg

          // check which mode the instruction is in
          u16 flag = (instr >> 11) & 1;
          reg[R_R7] = reg[R_PC];

          // Jsr mode
          if (flag) {
            // collect the lower 11 bits and sign extend them
            u16 pc_offset = sext(instr & 0x07FF, 11);
            reg[R_PC] += pc_offset;
          } else {
            // Jsrr mode
            u16 base_reg = (instr >> 6) && 0x7;
            reg[R_PC] = reg[base_reg];
          }
          break;
        }
        case Op_Ld: {
          // loads the contents of a memory location (relative to PC)
          // into destination register
          // |  op  | dst |   PC Offset |
          // | 0010 | 010 | 000_000_000 |
          // dst => R2
          // R2 <- mem[PC + sext(PC Offset)]

          u16 dst = (instr >> 9) & 0x7;
          u16 pc_offset = sext(instr & 0x1FF, 9);
          reg[dst] = mem_read(reg[R_PC] + pc_offset);

          update_flags(dst);
          break;
        }
        case Op_Ldi: {
          // load indirect
          // |  op  | dst |   PC Offset |
          // | 1010 | 000 | 000_000_000 |
          // dest => destination register
          // PC Offset => immediate with 9 bits
          // 1.     +offset
          //     PC ======> addr_a (gives us an addr in mem)
          //
          // 2.  mem[addr_a] ===> addr_b (gives us an addr in mem)
          //
          // 3.  mem[addr_b] ===> val (gives us the value to load)
          //
          // r0 = mem[addr_b] |
          // r0 = mem[mem[addr_a]] |
          // r0 = mem[mem[PC + offset]]

          u16 r0 = (instr >> 9) & 0x7;
          u16 pc_offset = sext(instr & 0x1FF, 9);
          reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));

          update_flags(r0);
          break;
        }
        case Op_Ldr: {
          // dst <- mem[reg + offset]
          // reg <- base register to offset from
          // |  op  | dst | reg | offset |
          // | 0110 | 010 | 000 | 000000 |
          u16 dst = (instr >> 9) & 0x7;
          u16 b_reg = (instr >> 6) & 0x7;
          u16 offset = sext(instr & 0x3F, 6);
          reg[dst] = mem_read(reg[b_reg] + offset);
          update_flags(dst);
          break;
        }
        case Op_Lea: {
          // load effective address gets the address, not the content
          // at the address, unlike Ld which gets the content
          // dst <- PC + PC offset
          // |  op  | dst |   PC Offset |
          // | 1110 | 010 | 000_000_000 |
          // Lea R4, LABEL
          // R4 <- addr(LABEL)

          u16 dst = (instr >> 6) & 0x7;
          u16 pc_offset = sext(instr & 0x1FF, 9);
          reg[dst] = reg[R_PC] + pc_offset;
          update_flags(dst);
          break;
        }
        case Op_St: {
          // store to memory
          // |  op  | src |   PC Offset |
          // | 0011 | 000 | 000_000_000 |
          // mem[PC + PC Offset] = src
          u16 src = (instr >> 9) & 0x7;
          u16 pc_offset = sext(instr & 0x1FF, 9);
          mem_write(reg[R_PC] + pc_offset, reg[src]);
          break;
        }
        case Op_Sti: {
          // store indirect, the counterpart of Ldi
          // |  op  | src |   PC Offset |
          // | 1011 | 001 | 000_000_000 |
          // src => source register
          // PC Offset => immediate with 9 bits
          // 1.     +offset
          //     PC ======> addr_a (gives us an addr in mem)
          //
          // 2.  mem[addr_a] ===> addr_b (gives us an addr in mem)
          //
          // 3.  mem[addr_b] ===> val (gives us the location to store at)
          //
          // mem[addr_b] = r1 |
          // mem[mem[addr_a]] = r1 |
          // mem[mem[PC + offset]] = r1

          u16 src = (instr >> 9) & 0x7;
          u16 pc_offset = sext(instr & 0x1FF, 9);
          u16 addr_a = reg[R_PC] + pc_offset;
          u16 addr_b = mem_read(addr_a);
          mem_write(addr_b, reg[src]);
          break;
        }
        case Op_Str: {
          // mem[reg + offset] <- src
          // reg <- base register to offset from
          // src <- register whose contents are to be stored
          // |  op  | src | reg | offset |
          // | 0111 | 001 | 000 | 000000 |
          u16 src = (instr >> 9) & 0x7;
          u16 b_reg = (instr >> 6) & 0x7;
          u16 offset = sext(instr & 0x3f, 6);
          mem_write(reg[b_reg] + offset, reg[src]);
          break;
        }
        case Op_Trap: {
          // |  op  | 0000 | trapvect |
          // | 1111 | 0000 | xxxxxxxx |
          // this is for trap routines for performing
          // fairly common routines
          // the value of the 8 bits of trapvect will determine
          // which trap routine will be called
          // we will also track the PC for the next instruction so we
          // can return to it
          u16 trapvect8 = (instr & 0xFF);

          // track the return point
          reg[R_R7] = reg[R_PC];
          switch (trapvect8) {
            case TRAP_GET_C: {
              break;
            }
            case TRAP_OUT_C: {
              break;
            }
            case TRAP_PUT_S: {
              // used to output a null terminated string
              // similar to printf in C
              // R0 contains the start address of the string,
              // one character per memory locations. When we see
              // an x0000 in a memory location.
              // this means each character is stored in 16 bits,
              // not 8.

              // we get to the start of the string by directly
              // offsetting the pointer of the memory
              u16 *c = mem + reg[R_R0];

              // not x0000
              while (*c) {
                // convert the value at c to a character and print it
                putc((char) *c, stdout);
                ++c; // go to next character
              }
              break;
            }
            case TRAP_IN_C: {
              break;
            }
            case TRAP_PUT_SP: {
              break;
            }
            case TRAP_HALT: {
              break;
            }
          }
          break;
        }
        case Op_RES:
        case Op_RTI:
        default: {
          break;
        }
      }
    }
}
/*
print hello world to console
------------------
  helloworld.lc:
------------------
.ORIG x3000         ; this is the address in memory where the program will be loaded
LEA R0, HELLO_STR   ; load the address of the HELLO_STR into R0
PUTs                ; output the string pointed to by R0 to the console
HALT                ; halt the program
HELLO_STR .STRINGZ "Hello World!" ; store the string here in the program
.END                ; mark the end of the file
------------------
*/

/*
the loop will continue till R0 is 10 because
R1 will only be negative while R0 is less than 10
-------------
  loops.lc:
-------------
AND R0, R0, 0         ; R0 = R0 & 0 (mask all bits to 0)
LOOP                  ; LOOP:
ADD R0, R0, 1         ;   R0 += 1
ADD R1, R0, -10       ;   R1 = R0 - 10
BRn LOOP              ;   IF (R_COND == FL_NEG): GOTO LOOP
-------------
*/