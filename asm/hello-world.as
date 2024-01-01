.ORIG x3000                                 ; this is the address in memory where the program will be loaded
LEA R0, HELLO_STR                           ; load the address of the string into r0
PUTs                                        ; output the string pointed to by r0 to the console
HALT                                        ; halt the program
HELLO_STR .STRINGZ "Hello, World!"          ; store the string here in the program
.END                                        ; mark the end of the file