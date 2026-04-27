/*--------------------------------------------------------------------*/
/* miniassembler.c                                                    */
/* Author: Bob Dondero, Donna Gabai                                   */
/*--------------------------------------------------------------------*/

#include "miniassembler.h"
#include <assert.h>
#include <stddef.h>

/*--------------------------------------------------------------------*/
/* Modify *puiDest in place,
   setting uiNumBits starting at uiDestStartBit (where 0 indicates
   the least significant bit) with bits taken from uiSrc,
   starting at uiSrcStartBit.
   uiSrcStartBit indicates the rightmost bit in the field.
   setField sets the appropriate bits in *puiDest to 1.
   setField never unsets any bits in *puiDest.                        */
static void setField(unsigned int uiSrc, unsigned int uiSrcStartBit,
                     unsigned int *puiDest, unsigned int uiDestStartBit,
                     unsigned int uiNumBits)
{
   unsigned int i;
   unsigned int curSrc;
   for (i = 0; i < uiNumBits; i++) {
      /*extracts the i + start bit by shifting it by i + start right and &-ing with 1*/
      curSrc = (uiSrc >> (i + uiSrcStartBit)) & 1U;
      *puiDest |= (curSrc << (i + uiDestStartBit));
   }
}

/*--------------------------------------------------------------------*/

unsigned int MiniAssembler_mov(unsigned int uiReg, int iImmed)
{
   unsigned int uiInstr;

   /* base instruction (only opc) for 32 bit version */
   uiInstr = 0x52800000;

   /* sets the Rd */
   setField(uiReg, 0, &uiInstr, 0, 5);
   /* sets imm */
   setField(iImmed, 0, &uiInstr, 5, 16);

   return uiInstr;
}

/*--------------------------------------------------------------------*/

unsigned int MiniAssembler_adr(unsigned int uiReg, unsigned long ulAddr,
   unsigned long ulAddrOfThisInstr)
{
   unsigned int uiInstr;
   unsigned int uiDisp;

   /* Base Instruction Code */
   uiInstr = 0x10000000;

   /* register to be inserted in instruction */
   setField(uiReg, 0, &uiInstr, 0, 5);

   /* displacement to be split into immlo and immhi and inserted */
   uiDisp = (unsigned int)(ulAddr - ulAddrOfThisInstr);

   setField(uiDisp, 0, &uiInstr, 29, 2);
   setField(uiDisp, 2, &uiInstr, 5, 19);

   return uiInstr;
}

/*--------------------------------------------------------------------*/

unsigned int MiniAssembler_strb(unsigned int uiFromReg,
   unsigned int uiToReg)
{
   unsigned int uiInstr;

   uiInstr = 0x39000000;

   setField(uiToReg, 0, &uiInstr, 5, 5);
   setField(uiFromReg, 0, &uiInstr, 0, 5);

   return uiInstr;
}

/*--------------------------------------------------------------------*/

unsigned int MiniAssembler_b(unsigned long ulAddr,
   unsigned long ulAddrOfThisInstr)
{
   unsigned int uiInstr;
   long relOffset;

   uiInstr = 0x14000000;
   relOffset = ((long) ulAddr - (long) ulAddrOfThisInstr)/4;

   setField((unsigned long)relOffset, 0, &uiInstr, 0, 26);

   return uiInstr;
}

/*--------------------------------------------------------------------*/

unsigned int MiniAssembler_bl(unsigned long ulAddr,
   unsigned long ulAddrOfThisInstr)
{
   unsigned int uiInstr;
   long lRelOffset;

   /* Base "bl .": bits 26-31 = 100101, bits 0-25 = imm26. The only
      difference from "b" is bit 31 (the "link" bit), which causes
      x30 to be set to the address of the next instruction before
      branching. */
   uiInstr = 0x94000000;

   /* The 26-bit signed offset is the PC-relative displacement
      divided by 4. Only the low 26 bits are written. */
   lRelOffset = ((long)ulAddr - (long)ulAddrOfThisInstr) / 4;
   setField((unsigned int)(unsigned long)lRelOffset, 0,
            &uiInstr, 0, 26);

   return uiInstr;
}

