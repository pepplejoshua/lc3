#include <iostream>
#include "arch.h"

using std::cout;

typedef uint16_t u16;

extern int read_img(char *);
extern u16 mem_read(u16);

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
      reg[R_PC]++; // move to next instruction
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
            // sign extend the last 5 bits if we are in
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
          break;
        }
        case Op_Not: {
          break;
        }
        case Op_Br: {
          break;
        }
        case Op_Jmp: {
          break;
        }
        case Op_Jsr: {
          break;
        }
        case Op_Ld: {
          break;
        }
        case Op_Ldi: {
          // load indirect
          // |  op  | dst | PC Offset |
          // | 1010 | 000 | 00000 0000 |
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
        case Op_Lea: {
          break;
        }
        case Op_St: {
          break;
        }
        case Op_Str: {
          break;
        }
        case Op_Trap: {
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