#include <iostream>
#include "arch.h"

using std::cout;

extern int read_img(char *);
extern uint16_t mem_read(uint16_t);

uint16_t sext(uint16_t x, int bit_count) {
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
      uint16_t instr = mem_read(reg[R_PC]);
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
      uint16_t op = instr >> 12;

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

          uint16_t dst_msk = 0b0000111000000000;
          uint16_t sr1_msk = 0b0000000111000000;
          uint16_t mode_msk = 0b0000000000100000;

          auto mode = instr & mode_msk;
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