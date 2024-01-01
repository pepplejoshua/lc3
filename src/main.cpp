#include <iostream>
#include "arch.h"

using std::cout;

extern int read_img(char *);
extern uint16_t mem_read(uint16_t);

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
      uint16_t op = instr >> 12;

      switch (op) {
        case Op_Add: {
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