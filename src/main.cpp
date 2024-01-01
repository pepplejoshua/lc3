#include <iostream>
#include "arch.h"

using std::cout;

int main(int argc, char** argv) {
    if (argc < 2) {
        // show usage
        cout << "lc [image_file]\n";
        exit(2);
    }
}
/*
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