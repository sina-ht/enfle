/**************************************************************************
 *                                                                        *
 * This code has been developed by John Funnell. This software is an      *
 * implementation of a part of one or more MPEG-4 Video tools as          *
 * specified in ISO/IEC 14496-2 standard.  Those intending to use this    *
 * software module in hardware or software products are advised that its  *
 * use may infringe existing patents or copyrights, and any such use      *
 * would be at such party's own risk.  The original developer of this     *
 * software module and his/her company, and subsequent editors and their  *
 * companies (including Project Mayo), will have no liability for use of  *
 * this software or modifications or derivatives thereof.                 *
 *                                                                        *
 * Project Mayo gives users of the Codec a license to this software       *
 * module or modifications thereof for use in hardware or software        *
 * products claiming conformance to the MPEG-4 Video Standard as          *
 * described in the Open DivX license.                                    *
 *                                                                        *
 * The complete Open DivX license can be found at                         *
 * http://www.projectmayo.com/opendivx/license.php                        *
 *                                                                        *
 **************************************************************************/
/**
*  Copyright (C) 2001 - Project Mayo
 *
 * John Funnell
 *
 * DivX Advanced Research Center <darc@projectmayo.com>
**/
// transferIDCT_mmx.c //

/* routines to transfer iDCT block result into the output picture */

/* this routine still needs to be optimised for pipelining        */
/* just haven't got time to finish the optimisations here ;-)     */

/* inline masm is used here so not compatible with gnu            */
/* Port for GNU GCC by Hiroshi Takekawa <sian@big.or.jp>          */


//#define _TEST_TRANSFER

#ifdef _TEST_TRANSFER
#include <stdio.h>
#endif


#ifdef WIN32
#include "portab.h"
#else
#include <inttypes.h>
#endif // WIN32

void transferIDCT_add(int16_t *sourceS16, uint8_t *destU8, int stride) {
	#ifdef _TEST_TRANSFER
	uint8_t reference_dest[64];
	int x, y, sum16;

	/* populate reference_dest[] with the correct result... */
	for (y=0; y<8; y++) {
		for (x=0; x<8; x++) {
			sum16 = (destU8[stride*y + x] + sourceS16[8*y + x]);
			if      (sum16 > 255) reference_dest[8*y + x] = 255;
			else if (sum16 <   0) reference_dest[8*y + x] =   0;
			else                  reference_dest[8*y + x] = (uint8_t)sum16;
		}
	}
	#endif

#ifdef WIN32
	_asm {

	; not sure about the state handling here - there must be a better way
	push eax
	push ebx
	push edi

	mov eax, sourceS16           ;  parameter 1, *sourceS16
	mov ebx, destU8              ;  parameter 2, *destU8
	mov edi, stride              ;  parameter 3, stride
	pxor mm7, mm7                ;  set mm7 = 0

; lines 0 to 7 all scheduled in together
	movq mm0,  qword ptr [ebx]   ;  eight bytes of destination into mm4
	movq mm1,  mm0               ;  eight bytes of destination into mm0
	punpcklbw mm0, mm7           ;  unpack first 4 bytes from dest into mm4
	punpckhbw mm1, mm7           ;  unpack next 4 bytes from dest into mm5
	paddsw mm0, qword ptr [eax]  ;  add source and destination
	paddsw mm1, qword ptr [eax+8];  add source and destination
	packuswb mm0, mm1            ;  pack mm0 and mm1 into mm0
	movq  qword ptr  [ebx], mm0  ;  copy output to destination
	add ebx, edi                 ;  add +stride to dest ptr

	movq mm2,  qword ptr [ebx]   ;  eight bytes of destination into mm4
	movq mm3,  mm2               ;  eight bytes of destination into mm3
	punpcklbw mm2, mm7           ;  unpack first 4 bytes from dest into mm4
	punpckhbw mm3, mm7           ;  unpack next 4 bytes from dest into mm5
	paddsw mm2, qword ptr [eax+16]  ;  add source and destination
	paddsw mm3, qword ptr [eax+24];  add source and destination
	packuswb mm2, mm3            ;  pack mm0 and mm1 into mm0
	movq  qword ptr  [ebx], mm2  ;  copy output to destination
	add ebx, edi                 ;  add +stride to dest ptr

	movq mm4,  qword ptr [ebx]   ;  eight bytes of destination into mm4
	movq mm5,  mm4               ;  eight bytes of destination into mm5
	punpcklbw mm4, mm7           ;  unpack first 4 bytes from dest into mm4
	punpckhbw mm5, mm7           ;  unpack next 4 bytes from dest into mm5
	paddsw mm4, qword ptr [eax+32]  ;  add source and destination
	paddsw mm5, qword ptr [eax+40];  add source and destination
	packuswb mm4, mm5            ;  pack mm0 and mm1 into mm0
	movq  qword ptr  [ebx], mm4  ;  copy output to destination
	add ebx, edi                 ;  add +stride to dest ptr

	movq mm0,  qword ptr [ebx]   ;  eight bytes of destination into mm4
	movq mm1,  qword ptr [ebx]   ;  eight bytes of destination into mm5
	punpcklbw mm0, mm7           ;  unpack first 4 bytes from dest into mm4
	punpckhbw mm1, mm7           ;  unpack next 4 bytes from dest into mm5
	paddsw mm0, qword ptr [eax+48]  ;  add source and destination
	paddsw mm1, qword ptr [eax+56];  add source and destination
	packuswb mm0, mm1            ;  pack mm0 and mm1 into mm0
	add eax, 64                  ;  add +64 to source ptr                
	movq  qword ptr  [ebx], mm0  ;  copy output to destination
	add ebx, edi                 ;  add +stride to dest ptr

	movq mm2,  qword ptr [ebx]   ;  eight bytes of destination into mm4
	movq mm3,  mm2               ;  eight bytes of destination into mm3
	punpcklbw mm2, mm7           ;  unpack first 4 bytes from dest into mm4
	punpckhbw mm3, mm7           ;  unpack next 4 bytes from dest into mm5
	paddsw mm2, qword ptr [eax]  ;  add source and destination
	paddsw mm3, qword ptr [eax+8];  add source and destination
	packuswb mm2, mm3            ;  pack mm0 and mm1 into mm0
	add eax, 16                  ;  add +16 to source ptr                
	movq  qword ptr  [ebx], mm2  ;  copy output to destination
	add ebx, edi                 ;  add +stride to dest ptr

	movq mm4,  qword ptr [ebx]   ;  eight bytes of destination into mm4
	movq mm5,  mm4               ;  eight bytes of destination into mm5
	punpcklbw mm4, mm7           ;  unpack first 4 bytes from dest into mm4
	punpckhbw mm5, mm7           ;  unpack next 4 bytes from dest into mm5
	paddsw mm4, qword ptr [eax]  ;  add source and destination
	paddsw mm5, qword ptr [eax+8];  add source and destination
	packuswb mm4, mm5            ;  pack mm0 and mm1 into mm0
	add eax, 16                  ;  add +16 to source ptr                
	movq  qword ptr  [ebx], mm4  ;  copy output to destination
	add ebx, edi                 ;  add +stride to dest ptr

	movq mm0,  qword ptr [ebx]   ;  eight bytes of destination into mm4
	movq mm1,  mm0               ;  eight bytes of destination into mm1
	punpcklbw mm0, mm7           ;  unpack first 4 bytes from dest into mm4
	punpckhbw mm1, mm7           ;  unpack next 4 bytes from dest into mm5
	paddsw mm0, qword ptr [eax]  ;  add source and destination
	paddsw mm1, qword ptr [eax+8];  add source and destination
	packuswb mm0, mm1            ;  pack mm0 and mm1 into mm0
	add eax, 16                  ;  add +16 to source ptr                
	movq  qword ptr  [ebx], mm0  ;  copy output to destination
	add ebx, edi                 ;  add +stride to dest ptr

	movq mm2,  qword ptr [ebx]   ;  eight bytes of destination into mm4
	movq mm3,  mm2               ;  eight bytes of destination into mm3
	punpcklbw mm2, mm7           ;  unpack first 4 bytes from dest into mm4
	punpckhbw mm3, mm7           ;  unpack next 4 bytes from dest into mm5
	paddsw mm2, qword ptr [eax]  ;  add source and destination
	paddsw mm3, qword ptr [eax+8];  add source and destination
	packuswb mm2, mm3            ;  pack mm0 and mm1 into mm0
	movq  qword ptr  [ebx], mm2  ;  copy output to destination

	pop edi
	pop ebx 
	pop eax

	emms


	}
#else
	__asm__ __volatile__ (
	//"movl      sourceS16, %%eax\n\t"   // parameter 1, *sourceS16
	//"movl      destU8, %%ecx\n\t"      // parameter 2, *destU8
	//"movl      stride, %%edi\n\t"      // parameter 3, stride
	"pxor         %%mm7,  %%mm7\n\t"     // set mm7 = 0

	// lines 0 to 7 all scheduled in together  
	"movq        (%%ecx), %%mm0\n\t"  // eight bytes of destination into mm4
	"movq         %%mm0,  %%mm1\n\t"  // eight bytes of destination into mm0
	"punpcklbw    %%mm7,  %%mm0\n\t"  // unpack first 4 bytes from dest into mm4
	"punpckhbw    %%mm7,  %%mm1\n\t"  // unpack next 4 bytes from dest into mm5
	"paddsw      (%%eax), %%mm0\n\t"  // add source and destination
	"paddsw     8(%%eax), %%mm1\n\t"  // add source and destination
	"packuswb     %%mm1,  %%mm0\n\t"  // pack mm0 and mm1 into mm0
	"movq         %%mm0, (%%ecx)\n\t" // copy output to destination
	"addl         %%edi,  %%ecx\n\t"  // add +stride to dest ptr

	"movq        (%%ecx), %%mm2\n\t"  // eight bytes of destination into mm4
	"movq         %%mm2,  %%mm3\n\t"  // eight bytes of destination into mm3
	"punpcklbw    %%mm7,  %%mm2\n\t"  // unpack first 4 bytes from dest into mm4
	"punpckhbw    %%mm7,  %%mm3\n\t"  // unpack next 4 bytes from dest into mm5
	"paddsw    16(%%eax), %%mm2\n\t"  // add source and destination
	"paddsw    24(%%eax), %%mm3\n\t"  // add source and destination
	"packuswb     %%mm3,  %%mm2\n\t"  // pack mm0 and mm1 into mm0
	"movq         %%mm2, (%%ecx)\n\t" // copy output to destination
	"addl         %%edi,  %%ecx\n\t"  // add +stride to dest ptr

	"movq        (%%ecx), %%mm4\n\t"  // eight bytes of destination into mm4
	"movq         %%mm4,  %%mm5\n\t"  // eight bytes of destination into mm5
	"punpcklbw    %%mm7,  %%mm4\n\t"  // unpack first 4 bytes from dest into mm4
	"punpckhbw    %%mm7,  %%mm5\n\t"  // unpack next 4 bytes from dest into mm5
	"paddsw    32(%%eax), %%mm4\n\t"  // add source and destination
	"paddsw    40(%%eax), %%mm5\n\t"  // add source and destination
	"packuswb     %%mm5,  %%mm4\n\t"  // pack mm0 and mm1 into mm0
	"movq         %%mm4, (%%ecx)\n\t" // copy output to destination
	"addl         %%edi,  %%ecx\n\t"  // add +stride to dest ptr

	"movq        (%%ecx), %%mm0\n\t"  // eight bytes of destination into mm4
	"movq        (%%ecx), %%mm1\n\t"  // eight bytes of destination into mm5
	"punpcklbw    %%mm7,  %%mm0\n\t"  // unpack first 4 bytes from dest into mm4
	"punpckhbw    %%mm7,  %%mm1\n\t"  // unpack next 4 bytes from dest into mm5
	"paddsw    48(%%eax), %%mm0\n\t"  // add source and destination
	"paddsw    56(%%eax), %%mm1\n\t"  // add source and destination
	"packuswb     %%mm1,  %%mm0\n\t"  // pack mm0 and mm1 into mm0
	"addl           $64,  %%eax\n\t"  // add +64 to source ptr
	"movq         %%mm0, (%%ecx)\n\t" // copy output to destination
	"addl         %%edi,  %%ecx\n\t"  // add +stride to dest ptr

	"movq       (%%ecx), %%mm2\n\t"   // eight bytes of destination into mm4
	"movq        %%mm2,  %%mm3\n\t"   // eight bytes of destination into mm3
	"punpcklbw   %%mm7,  %%mm2\n\t"   // unpack first 4 bytes from dest into mm4
	"punpckhbw   %%mm7,  %%mm3\n\t"   // unpack next 4 bytes from dest into mm5
	"paddsw     (%%eax), %%mm2\n\t"   // add source and destination
	"paddsw    8(%%eax), %%mm3\n\t"   // add source and destination
	"packuswb    %%mm3,  %%mm2\n\t"   // pack mm0 and mm1 into mm0
	"addl          $16,  %%eax\n\t"   // add +16 to source ptr
	"movq        %%mm2, (%%ecx)\n\t"  // copy output to destination
	"addl        %%edi,  %%ecx\n\t"   // add +stride to dest ptr

	"movq       (%%ecx), %%mm4\n\t"   // eight bytes of destination into mm4
	"movq        %%mm4,  %%mm5\n\t"   // eight bytes of destination into mm5
	"punpcklbw   %%mm7,  %%mm4\n\t"   // unpack first 4 bytes from dest into mm4
	"punpckhbw   %%mm7,  %%mm5\n\t"   // unpack next 4 bytes from dest into mm5
	"paddsw     (%%eax), %%mm4\n\t"   // add source and destination
	"paddsw    8(%%eax), %%mm5\n\t"   // add source and destination
	"packuswb    %%mm5,  %%mm4\n\t"   // pack mm0 and mm1 into mm0
	"addl          $16,  %%eax\n\t"   // add +16 to source ptr
	"movq        %%mm4, (%%ecx)\n\t"  // copy output to destination
	"addl        %%edi,  %%ecx\n\t"   // add +stride to dest ptr

	"movq       (%%ecx), %%mm0\n\t"   // eight bytes of destination into mm4
	"movq        %%mm0,  %%mm1\n\t"   // eight bytes of destination into mm1
	"punpcklbw   %%mm7,  %%mm0\n\t"   // unpack first 4 bytes from dest into mm4
	"punpckhbw   %%mm7,  %%mm1\n\t"   // unpack next 4 bytes from dest into mm5
	"paddsw     (%%eax), %%mm0\n\t"   // add source and destination
	"paddsw    8(%%eax), %%mm1\n\t"   // add source and destination
	"packuswb    %%mm1,  %%mm0\n\t"   // pack mm0 and mm1 into mm0
	"addl          $16,  %%eax\n\t"   // add +16 to source ptr
	"movq        %%mm0, (%%ecx)\n\t"  // copy output to destination
	"addl        %%edi,  %%ecx\n\t"   // add +stride to dest ptr

	"movq       (%%ecx), %%mm2\n\t"   // eight bytes of destination into mm4
	"movq        %%mm2,  %%mm3\n\t"   // eight bytes of destination into mm3
	"punpcklbw   %%mm7,  %%mm2\n\t"   // unpack first 4 bytes from dest into mm4
	"punpckhbw   %%mm7,  %%mm3\n\t"   // unpack next 4 bytes from dest into mm5
	"paddsw     (%%eax), %%mm2\n\t"   // add source and destination
	"paddsw    8(%%eax), %%mm3\n\t"   // add source and destination
	"packuswb    %%mm3,  %%mm2\n\t"   // pack mm0 and mm1 into mm0
	"movq        %%mm2, (%%ecx)\n\t"  // copy output to destination

	"emms"
	:
	: "a" (sourceS16),
	  "c" (destU8),
	  "D" (stride)
	);
#endif

	#ifdef _TEST_TRANSFER
	/* check destination against reference_dest[]... */
	for (y=0; y<8; y++) {
		for (x=0; x<8; x++) {
			if (reference_dest[8*y + x] != destU8[stride*y + x]) printf("transferIDCT_add() is broken\n");
		}
	}
	#endif
  
}

void transferIDCT_copy(int16_t *sourceS16, uint8_t *destU8, int stride) {
	#ifdef _TEST_TRANSFER
	int x, y, clipped;
	#endif
	
#ifdef WIN32
	_asm {

	; not sure about the state handling here - there must be a better way
	push eax
	push ebx
	push edi

	mov eax, sourceS16           ;  parameter 1, *sourceS16
	mov ebx, destU8              ;  parameter 2, *destU8
	mov edi, stride              ;  parameter 3, stride

; lines 0 to 7 schedueled into each other...
	movq mm0, qword ptr [eax]       ;  move first four words into mm0

	packuswb mm0, qword ptr [eax+8] ;  pack mm0 and the next four words into mm0

	movq mm1, qword ptr [eax+16]    ;  move first four words into mm1

	packuswb mm1, qword ptr [eax+24];  pack mm0 and the next four words into mm1

	movq mm2, qword ptr [eax+32]    ;  move first four words into mm2

	packuswb mm2, qword ptr [eax+40];  pack mm0 and the next four words into mm2

	movq mm3, qword ptr [eax+48]    ;  move first four words into mm3

	packuswb mm3, qword ptr [eax+56] ;  pack mm3 and the next four words into mm3

	movq qword ptr [ebx], mm0       ;  copy output to destination
	add ebx, edi                    ;  add +stride to dest ptr

	movq qword ptr [ebx], mm1       ;  copy output to destination
	add ebx, edi                    ;  add +stride to dest ptr

	movq qword ptr [ebx], mm2       ;  copy output to destination
	add ebx, edi                    ;  add +stride to dest ptr

	movq qword ptr [ebx], mm3       ;  copy output to destination
	add ebx, edi                    ;  add +stride to dest ptr
	
	movq mm0, qword ptr [eax+64]    ;  move first four words into mm0
	add eax, 64                     ;  add 64 to source ptr                

	packuswb mm0, qword ptr [eax+8] ;  pack mm0 and the next four words into mm0

	movq mm1, qword ptr [eax+16]    ;  move first four words into mm1

	packuswb mm1, qword ptr [eax+24];  pack mm0 and the next four words into mm1

	movq mm2, qword ptr [eax+32]    ;  move first four words into mm2

	packuswb mm2, qword ptr [eax+40];  pack mm0 and the next four words into mm2

	movq mm3, qword ptr [eax+48]    ;  move first four words into mm3

	packuswb mm3, qword ptr [eax+56];  pack mm3 and the next four words into mm3

	movq qword ptr [ebx], mm0       ;  copy output to destination
	add ebx, edi                    ;  add +stride to dest ptr

	movq qword ptr [ebx], mm1       ;  copy output to destination
	add ebx, edi                    ;  add +stride to dest ptr

	movq qword ptr [ebx], mm2       ;  copy output to destination
	add ebx, edi                    ;  add +stride to dest ptr

	movq qword ptr [ebx], mm3       ;  copy output to destination

	pop edi
	pop ebx 
	pop eax

	emms

	}
#else
	__asm__ __volatile__ (
	//"mov    sourceS16, %%eax\n\t"   // parameter 1, *sourceS16
	//"mov       destU8, %%ecx\n\t"   // parameter 2, *destU8
	//"mov       stride, %%edi\n\t"   // parameter 3, stride

	// lines 0 to 7 schedueled into each other...
	"movq       (%%eax), %%mm0\n\t"   // move first four words into mm0\n\t"
	"packuswb  8(%%eax), %%mm0\n\t"   // pack mm0 and the next four words into mm0\n\t"
	"movq     16(%%eax), %%mm1\n\t"   // move first four words into mm1\n\t"
	"packuswb 24(%%eax), %%mm1\n\t"   // pack mm0 and the next four words into mm1\n\t"
	"movq     32(%%eax), %%mm2\n\t"   // move first four words into mm2\n\t"
	"packuswb 40(%%eax), %%mm2\n\t"   // pack mm0 and the next four words into mm2\n\t"
	"movq     48(%%eax), %%mm3\n\t"   // move first four words into mm3\n\t"
	"packuswb 56(%%eax), %%mm3\n\t"   // pack mm3 and the next four words into mm3\n\t"
	"movq        %%mm0, (%%ecx)\n\t"  // copy output to destination\n\t"
	"add         %%edi,  %%ecx\n\t"   // add +stride to dest ptr\n\t"
	"movq        %%mm1, (%%ecx)\n\t"  // copy output to destination\n\t"
	"add         %%edi,  %%ecx\n\t"   // add +stride to dest ptr\n\t"
	"movq        %%mm2, (%%ecx)\n\t"  // copy output to destination\n\t"
	"add         %%edi,  %%ecx\n\t"   // add +stride to dest ptr\n\t"
	"movq        %%mm3, (%%ecx)\n\t"  // copy output to destination\n\t"
	"add         %%edi,  %%ecx\n\t"   // add +stride to dest ptr\n\t"
	"movq     64(%%eax), %%mm0\n\t"   // move first four words into mm0\n\t"
	"add           $64,  %%eax\n\t"   // add 64 to source ptr\n\t"

	"packuswb  8(%%eax), %%mm0\n\t"   // pack mm0 and the next four words into mm0\n\t"
	"movq     16(%%eax), %%mm1\n\t"   // move first four words into mm1\n\t"
	"packuswb 24(%%eax), %%mm1\n\t"   // pack mm0 and the next four words into mm1\n\t"
	"movq     32(%%eax), %%mm2\n\t"   // move first four words into mm2\n\t"
	"packuswb 40(%%eax), %%mm2\n\t"   // pack mm0 and the next four words into mm2\n\t"
	"movq     48(%%eax), %%mm3\n\t"   // move first four words into mm3\n\t"
	"packuswb 56(%%eax), %%mm3\n\t"   // pack mm3 and the next four words into mm3\n\t"
	"movq        %%mm0, (%%ecx)\n\t"  // copy output to destination\n\t"
	"addl        %%edi,  %%ecx\n\t"   // add +stride to dest ptr\n\t"
	"movq        %%mm1, (%%ecx)\n\t"  // copy output to destination\n\t"
	"addl        %%edi,  %%ecx\n\t"   // add +stride to dest ptr\n\t"
	"movq        %%mm2, (%%ecx)\n\t"  // copy output to destination\n\t"
	"addl        %%edi,  %%ecx\n\t"   // add +stride to dest ptr\n\t"
	"movq        %%mm3, (%%ecx)\n\t"  // copy output to destination\n\t"

	"emms"
	:
	: "a" (sourceS16),
	  "c" (destU8),
	  "D" (stride)
	);
#endif

	#ifdef _TEST_TRANSFER
	for (y=0; y<8; y++) {
		for (x=0; x<8; x++) {
			clipped = sourceS16[8*y + x];
			if (clipped > 255) clipped = 255;
			if (clipped <   0) clipped =   0;
			if (clipped != destU8[stride*y+x]) printf("transferIDCT_copy() is broken\n");
		}
	}
	#endif

}
