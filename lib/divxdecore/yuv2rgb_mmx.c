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
*
**/
// yuv2rgb_mmx.c //

/* MMX version of colourspace conversion   */

/* 12 Dec 2000  - John Funnell             */
/* (C) 2001 Project Mayo                   */
/* Port for GNU GCC by Hiroshi Takekawa <sian@big.or.jp> */


/* This is what we're doing:               */

/* Step 1.                                 */
/* Y -= 16                                 */
/* U -= 128                                */
/* V -= 128                                */

/* Step 2.                                 */
/* Y /= 219                                */
/* U /= 224                                */
/* V /= 224                                */

/* Step 3.                                 */
/* now we want the inverse of this matrix: */
/*  /  0.299  0.114  0.587  \              */
/* |  -0.169  0.500 -0.331   |             */
/*  \  0.500 -0.081 -0.419  /              */

/* which is, approximately:                */
/*  /  2568      0   3343  \               */
/* |   2568   f36e   e5e2   | / 65536 * 8  */
/*  \  2568   40cf      0  /               */
/* including the multiplies in Step 2      */


/***********************************************************/
/* The 24-bit version will write one byte beyond the frame */
/***********************************************************/


#ifdef WIN32
#include "portab.h"
#else
#include <inttypes.h>
#endif // WIN32

#include "yuv2rgb.h"


#define _USE_PREFETCH

#define __int64 uint64_t

#ifdef WIN32
static __int64 mmw_mult_Y   = 0x2568256825682568;
static __int64 mmw_mult_U_G = 0xf36ef36ef36ef36e;
static __int64 mmw_mult_U_B = 0x40cf40cf40cf40cf;
static __int64 mmw_mult_V_R = 0x3343334333433343;
static __int64 mmw_mult_V_G = 0xe5e2e5e2e5e2e5e2;

static __int64 mmb_0x10     = 0x1010101010101010;
static __int64 mmw_0x0080   = 0x0080008000800080;
static __int64 mmw_0x00ff   = 0x00ff00ff00ff00ff;

static __int64 mmw_cut_red		= 0x7c007c007c007c00;
static __int64 mmw_cut_green	= 0x03e003e003e003e0;
static __int64 mmw_cut_blue		= 0x001f001f001f001f;
#else
static __int64 mmw_mult_Y    __attribute__ ((aligned (8))) = 0x2568256825682568;
static __int64 mmw_mult_U_G  __attribute__ ((aligned (8))) = 0xf36ef36ef36ef36e;
static __int64 mmw_mult_U_B  __attribute__ ((aligned (8))) = 0x40cf40cf40cf40cf;
static __int64 mmw_mult_V_R  __attribute__ ((aligned (8))) = 0x3343334333433343;
static __int64 mmw_mult_V_G  __attribute__ ((aligned (8))) = 0xe5e2e5e2e5e2e5e2;

static __int64 mmb_0x10      __attribute__ ((aligned (8))) = 0x1010101010101010;
static __int64 mmw_0x0080    __attribute__ ((aligned (8))) = 0x0080008000800080;
static __int64 mmw_0x00ff    __attribute__ ((aligned (8))) = 0x00ff00ff00ff00ff;

static __int64 mmw_cut_red   __attribute__ ((aligned (8))) = 0x7c007c007c007c00;
static __int64 mmw_cut_green __attribute__ ((aligned (8))) = 0x03e003e003e003e0;
static __int64 mmw_cut_blue  __attribute__ ((aligned (8))) = 0x001f001f001f001f;
#endif

void
dummy(void)
{
  __int64 t;

  t = mmw_mult_Y;
  t = mmw_mult_U_G;
  t = mmw_mult_U_B;
  t = mmw_mult_V_R;
  t = mmw_mult_V_G;

  t = mmb_0x10;
  t = mmw_0x0080;
  t = mmw_0x00ff;

  t = mmw_cut_red;
  t = mmw_cut_green;
  t = mmw_cut_blue;
}

#ifdef YUV2RGB_32_MMX
/* all stride values are in _bytes_ */
void yuv2rgb_32(uint8_t *puc_y, int stride_y, 
                uint8_t *puc_u, uint8_t *puc_v, int stride_uv, 
                uint8_t *puc_out, int width_y, int height_y ) {

	int y, horiz_count;
	int stride_out = width_y * 4;

#ifdef WIN32
	puc_out = puc_out + stride_out * (height_y-2);

	horiz_count = -(width_y >> 3);

	for (y=0; y<(height_y-2); y++) {

	  _asm {
			push eax
			push ebx
			push ecx
			push edx
			push edi

			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
			
		horiz_loop:

			movd mm2, [ecx]
			pxor mm7, mm7

			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0

			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0

			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 

			psubusb mm0, mmb_0x10    ; mm0 -= 16

			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0

			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8

			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8

			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8

			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v

			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u

			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part

			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0

			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0

			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1

			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma

			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0

			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1

			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1

			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1

			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0

			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0

			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?

			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpcklbw mm1, mm0       ; mm1 = __g3__g2__g1__g0

			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpcklbw mm0, mm7       ; mm0 = r3b3r2b2r1b1r0b0

			movq mm2, mm0            ; mm2 = r3b3r2b2r1b1r0b0

			punpcklbw mm0, mm1       ; mm0 = __r1g1b1__r0g0b0
			punpckhbw mm2, mm1       ; mm2 = __r3g3b3__r2g2b2

			/* 32-bit save... */
			movq  [eax], mm0         ; eax[0] = __r1g1b1__r0g0b0
			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0

			movq 8[eax], mm2         ; eax[8] = __r3g3b3__r2g2b2

			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?

			punpckhbw mm1, mm0       ; mm1 = __g7__g6__g5__g4

			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpckhbw mm0, mm7       ; mm0 = r7b7r6b6r5b5r4b4

			movq mm2, mm0            ; mm2 = r7b7r6b6r5b5r4b4

			punpcklbw mm0, mm1       ; mm0 = __r5g5b5__r4g4b4
			punpckhbw mm2, mm1       ; mm2 = __r7g7b7__r6g6b6

			/* 32-bit save... */
			add ebx, 8               ; puc_y   += 8;
			add ecx, 4               ; puc_u   += 4;

			movq 16[eax], mm0        ; eax[16] = __r5g5b5__r4g4b4
			add edx, 4               ; puc_v   += 4;

			movq 24[eax], mm2        ; eax[24] = __r7g7b7__r6g6b6
			
			// 0 1 2 3 4 5 6 7 rgb save order

			add eax, 32              ; puc_out += 32

			inc edi
			jne horiz_loop			

			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax

			emms
						
		}
		puc_y   += stride_y;
		if (y%2) {
			puc_u   += stride_uv;
			puc_v   += stride_uv;
		}
		puc_out -= stride_out;
#else
	horiz_count = -(width_y >> 3);

	for (y=0; y<(height_y-2); y++) {
	
	  __asm__ __volatile__ (
			"push %%ebx\n\t"
			"mov %%esi, %%ebx\n"

			//"mov puc_out, %%eax\n\t"
			//"mov puc_y, %%ebx\n\t"
			//"mov puc_u, %%ecx\n\t"
			//"mov puc_v, %%edx\n\t"
			//"mov horiz_count, %%edi\n\t"

		"horiz_loop:\n\t"

			"movd (%%ecx), %%mm2\n\t"
			"pxor %%mm7, %%mm7\n\t"

			"movd (%%edx), %%mm3\n\t"
			"punpcklbw %%mm7, %%mm2\n\t"    // mm2 = __u3__u2__u1__u0

			"movq (%%ebx), %%mm0\n\t"       // mm0 = y7y6y5y4y3y2y1y0  
			"punpcklbw %%mm7, %%mm3\n\t"    // mm3 = __v3__v2__v1__v0

			"movq mmw_0x00ff, %%mm1\n\t"  // mm1 = 00ff00ff00ff00ff 

			"psubusb mmb_0x10, %%mm0\n\t" // mm0 -= 16

			"psubw mmw_0x0080, %%mm2\n\t" // mm2 -= 128
			"pand %%mm0, %%mm1\n\t"         // mm1 = __y6__y4__y2__y0

			"psubw mmw_0x0080, %%mm3\n\t" // mm3 -= 128
			"psllw $3, %%mm1\n\t"          // mm1 *= 8

			"psrlw $8, %%mm0\n\t"          // mm0 = __y7__y5__y3__y1
			"psllw $3, %%mm2\n\t"          // mm2 *= 8

			"pmulhw mmw_mult_Y, %%mm1\n\t" // mm1 *= luma coeff 
			"psllw $3, %%mm0\n\t" // mm0 *= 8

			"psllw $3, %%mm3\n\t" // mm3 *= 8
			"movq %%mm3, %%mm5\n\t" // mm5 = mm3 = v

			"pmulhw mmw_mult_V_R, %%mm5\n\t" // mm5 = red chroma
			"movq %%mm2, %%mm4\n\t" // mm4 = mm2 = u

			"pmulhw mmw_mult_Y, %%mm0\n\t" // mm0 *= luma coeff 
			"movq %%mm1, %%mm7\n\t" // even luma part

			"pmulhw mmw_mult_U_G, %%mm2\n\t" // mm2 *= u green coeff 
			"paddsw %%mm5, %%mm7\n\t" // mm7 = luma + chroma    __r6__r4__r2__r0

			"pmulhw mmw_mult_V_G, %%mm3\n\t" // mm3 *= v green coeff  
			"packuswb %%mm7, %%mm7\n\t" // mm7 = r6r4r2r0r6r4r2r0

			"pmulhw mmw_mult_U_B, %%mm4\n\t" // mm4 = blue chroma
			"paddsw %%mm0, %%mm5\n\t" // mm5 = luma + chroma    __r7__r5__r3__r1

			"packuswb %%mm5, %%mm5\n\t" // mm6 = r7r5r3r1r7r5r3r1
			"paddsw %%mm3, %%mm2\n\t" // mm2 = green chroma

			"movq %%mm1, %%mm3\n\t" // mm3 = __y6__y4__y2__y0
			"movq %%mm1, %%mm6\n\t" // mm6 = __y6__y4__y2__y0

			"paddsw %%mm4, %%mm3\n\t" // mm3 = luma + chroma    __b6__b4__b2__b0
			"paddsw %%mm2, %%mm6\n\t" // mm6 = luma + chroma    __g6__g4__g2__g0

			"punpcklbw %%mm5, %%mm7\n\t" // mm7 = r7r6r5r4r3r2r1r0
			"paddsw %%mm0, %%mm2\n\t" // odd luma part plus chroma part    __g7__g5__g3__g1

			"packuswb %%mm6, %%mm6\n\t" // mm2 = g6g4g2g0g6g4g2g0
			"packuswb %%mm2, %%mm2\n\t" // mm2 = g7g5g3g1g7g5g3g1

			"packuswb %%mm3, %%mm3\n\t" // mm3 = b6b4b2b0b6b4b2b0
			"paddsw %%mm0, %%mm4\n\t" // odd luma part plus chroma part    __b7__b5__b3__b1

			"packuswb %%mm4, %%mm4\n\t" // mm4 = b7b5b3b1b7b5b3b1
			"punpcklbw %%mm2, %%mm6\n\t" // mm6 = g7g6g5g4g3g2g1g0

			"punpcklbw %%mm4, %%mm3\n\t" // mm3 = b7b6b5b4b3b2b1b0

			/* 32-bit shuffle.... */
			"pxor %%mm0, %%mm0\n\t" // is this needed?

			"movq %%mm6, %%mm1\n\t" // mm1 = g7g6g5g4g3g2g1g0
			"punpcklbw %%mm0, %%mm1\n\t" // mm1 = __g3__g2__g1__g0

			"movq %%mm3, %%mm0\n\t" // mm0 = b7b6b5b4b3b2b1b0
			"punpcklbw %%mm7, %%mm0\n\t" // mm0 = r3b3r2b2r1b1r0b0

			"movq %%mm0, %%mm2\n\t" // mm2 = r3b3r2b2r1b1r0b0

			"punpcklbw %%mm1, %%mm0\n\t" // mm0 = __r1g1b1__r0g0b0
			"punpckhbw %%mm1, %%mm2\n\t" // mm2 = __r3g3b3__r2g2b2

			/* 32-bit save... */
			"movq %%mm0, (%%eax)\n\t" // eax[0] = __r1g1b1__r0g0b0
			"movq %%mm6, %%mm1\n\t" // mm1 = g7g6g5g4g3g2g1g0

			"movq %%mm2, 8(%%eax)\n\t" // eax[8] = __r3g3b3__r2g2b2

			/* 32-bit shuffle.... */
			"pxor %%mm0, %%mm0\n\t" // is this needed?

			"punpckhbw %%mm0, %%mm1\n\t" // mm1 = __g7__g6__g5__g4

			"movq %%mm3, %%mm0\n\t" // mm0 = b7b6b5b4b3b2b1b0
			"punpckhbw %%mm7, %%mm0\n\t" // mm0 = r7b7r6b6r5b5r4b4

			"movq %%mm0, %%mm2\n\t" // mm2 = r7b7r6b6r5b5r4b4

			"punpcklbw %%mm1, %%mm0\n\t" // mm0 = __r5g5b5__r4g4b4
			"punpckhbw %%mm1, %%mm2\n\t" // mm2 = __r7g7b7__r6g6b6

			/* 32-bit save... */
			"add $8, %%ebx\n\t" // puc_y   += 8;
			"add $4, %%ecx\n\t" // puc_u   += 4;

			"movq %%mm0, 16(%%eax)\n\t" // eax[16] = __r5g5b5__r4g4b4
			"add $4, %%edx\n\t" // puc_v   += 4;

			"movq %%mm2, 24(%%eax)\n\t" // eax[24] = __r7g7b7__r6g6b6

			// 0 1 2 3 4 5 6 7 rgb save order

			"add $32, %%eax\n\t" // puc_out += 32

			"inc %%edi\n\t"
			"jne horiz_loop\n\t"

			"pop %%ebx\n\t"

			"emms"
			:
			: "a" (puc_out),
			  "c" (puc_u),
			  "d" (puc_v),
			  "D" (horiz_count),
			  "S" (puc_y)
			);
		puc_y   += stride_y;
		if (y%2) {
			puc_u   += stride_uv;
			puc_v   += stride_uv;
		}
		puc_out += stride_out;
#endif
	}
}
#endif



#ifdef YUV2RGB_24_MMX
/* all stride values are in _bytes_ */
void yuv2rgb_24(uint8_t *puc_y, int stride_y, 
                uint8_t *puc_u, uint8_t *puc_v, int stride_uv, 
                uint8_t *puc_out, int width_y, int height_y ) {

	int y, horiz_count;
	int stride_out = width_y * 3;

#ifdef WIN32
	puc_out = puc_out + stride_out * (height_y-2); // go to the end of rgb buffer (upside down conversion)

	horiz_count = -(width_y >> 3);

	for (y=0; y<(height_y-1); y++) {
			
		_asm {
			push eax
			push ebx
			push ecx
			push edx
			push edi

			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
			
		horiz_loop:

			movd mm2, [ecx]
			pxor mm7, mm7

			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0

			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0

			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 

			psubusb mm0, mmb_0x10    ; mm0 -= 16

			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0

			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8

			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8

			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8

			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v

			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u

			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part

			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0

			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0

			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1

			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma

			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0

			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1

			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1

			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1

			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0

			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0

			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?

			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpcklbw mm1, mm0       ; mm1 = __g3__g2__g1__g0

			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpcklbw mm0, mm7       ; mm0 = r3b3r2b2r1b1r0b0

			movq mm2, mm0            ; mm2 = r3b3r2b2r1b1r0b0

			punpcklbw mm0, mm1       ; mm0 = __r1g1b1__r0g0b0
			punpckhbw mm2, mm1       ; mm2 = __r3g3b3__r2g2b2

			/* 24-bit shuffle and save... */
			movd   [eax], mm0        ; eax[0] = __r0g0b0
			psrlq mm0, 32            ; mm0 = __r1g1b1

			movd  3[eax], mm0        ; eax[3] = __r1g1b1

			movd  6[eax], mm2        ; eax[6] = __r2g2b2
			

			psrlq mm2, 32            ; mm2 = __r3g3b3
	
			movd  9[eax], mm2        ; eax[9] = __r3g3b3

			/* 32-bit shuffle.... */
			pxor mm0, mm0            ; is this needed?

			movq mm1, mm6            ; mm1 = g7g6g5g4g3g2g1g0
			punpckhbw mm1, mm0       ; mm1 = __g7__g6__g5__g4

			movq mm0, mm3            ; mm0 = b7b6b5b4b3b2b1b0
			punpckhbw mm0, mm7       ; mm0 = r7b7r6b6r5b5r4b4

			movq mm2, mm0            ; mm2 = r7b7r6b6r5b5r4b4

			punpcklbw mm0, mm1       ; mm0 = __r5g5b5__r4g4b4
			punpckhbw mm2, mm1       ; mm2 = __r7g7b7__r6g6b6

			/* 24-bit shuffle and save... */
			movd 12[eax], mm0        ; eax[12] = __r4g4b4
			psrlq mm0, 32            ; mm0 = __r5g5b5
			
			movd 15[eax], mm0        ; eax[15] = __r5g5b5
			add ebx, 8               ; puc_y   += 8;

			movd 18[eax], mm2        ; eax[18] = __r6g6b6
			psrlq mm2, 32            ; mm2 = __r7g7b7
			
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;

			movd 21[eax], mm2        ; eax[21] = __r7g7b7
			add eax, 24              ; puc_out += 24

			inc edi
			jne horiz_loop			

			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax

			emms
						
		}
		puc_y   += stride_y;
		if (y%2) {
			puc_u   += stride_uv;
			puc_v   += stride_uv;
		}
		puc_out -= stride_out; 
	}
#else
	puc_out = puc_out;

	horiz_count = -(width_y >> 3);

	for (y=0; y<(height_y-1); y++) {
	  __asm__ __volatile__ (
			"push %%ebx\n\t"
			"mov %%esi, %%ebx\n"

			//"mov eax, puc_out\n\t"
			//"mov ebx, puc_y\n\t"
			//"mov ecx, puc_u\n\t"
			//"mov edx, puc_v\n\t"
			//"mov edi, horiz_count\n\t"

		"horiz_loop24:\n\t"

			"movd (%%ecx), %%mm2\n\t"
			"pxor %%mm7, %%mm7\n\t"

			"movd (%%edx), %%mm3\n\t"
			"punpcklbw %%mm7, %%mm2\n\t" // mm2 = __u3__u2__u1__u0

			"movq (%%edx), %%mm0\n\t" // mm0 = y7y6y5y4y3y2y1y0  
			"punpcklbw %%mm7, %%mm3\n\t" // mm3 = __v3__v2__v1__v0

			"movq mmw_0x00ff, %%mm1\n\t" // mm1 = 00ff00ff00ff00ff 

			"psubusb mmb_0x10, %%mm0\n\t" // mm0 -= 16

			"psubw mmw_0x0080, %%mm2\n\t" // mm2 -= 128
			"pand %%mm0, %%mm1\n\t" // mm1 = __y6__y4__y2__y0

			"psubw mmw_0x0080, %%mm3\n\t" // mm3 -= 128
			"psllw $3, %%mm1\n\t" // mm1 *= 8

			"psrlw $8, %%mm0\n\t" // mm0 = __y7__y5__y3__y1
			"psllw $3, %%mm2\n\t" // mm2 *= 8

			"pmulhw mmw_mult_Y, %%mm1\n\t" // mm1 *= luma coeff 
			"psllw $3, %%mm0\n\t" // mm0 *= 8

			"psllw $3, %%mm3\n\t" // mm3 *= 8
			"movq %%mm3, %%mm5\n\t" // mm5 = mm3 = v

			"pmulhw mmw_mult_V_R, %%mm5\n\t" // mm5 = red chroma
			"movq %%mm2, %%mm4\n\t" // mm4 = mm2 = u

			"pmulhw mmw_mult_Y, %%mm0\n\t" // mm0 *= luma coeff 
			"movq %%mm1, %%mm7\n\t" // even luma part

			"pmulhw mmw_mult_U_G, %%mm2\n\t" // mm2 *= u green coeff 
			"paddsw %%mm5, %%mm7\n\t" // mm7 = luma + chroma    __r6__r4__r2__r0

			"pmulhw mmw_mult_V_G, %%mm3\n\t" // mm3 *= v green coeff  
			"packuswb %%mm7, %%mm7\n\t" // mm7 = r6r4r2r0r6r4r2r0

			"pmulhw mmw_mult_U_B, %%mm4\n\t" // mm4 = blue chroma
			"paddsw %%mm0, %%mm5\n\t" // mm5 = luma + chroma    __r7__r5__r3__r1

			"packuswb %%mm5, %%mm5\n\t" // mm6 = r7r5r3r1r7r5r3r1
			"paddsw %%mm3, %%mm2\n\t" // mm2 = green chroma

			"movq %%mm1, %%mm3\n\t" // mm3 = __y6__y4__y2__y0
			"movq %%mm1, %%mm6\n\t" // mm6 = __y6__y4__y2__y0

			"paddsw %%mm4, %%mm3\n\t" // mm3 = luma + chroma    __b6__b4__b2__b0
			"paddsw %%mm2, %%mm6\n\t" // mm6 = luma + chroma    __g6__g4__g2__g0
			
			"punpcklbw %%mm5, %%mm7\n\t" // mm7 = r7r6r5r4r3r2r1r0
			"paddsw %%mm0, %%mm2\n\t" // odd luma part plus chroma part    __g7__g5__g3__g1

			"packuswb %%mm6, %%mm6\n\t" // mm2 = g6g4g2g0g6g4g2g0
			"packuswb %%mm2, %%mm2\n\t" // mm2 = g7g5g3g1g7g5g3g1

			"packuswb %%mm3, %%mm3\n\t" // mm3 = b6b4b2b0b6b4b2b0
			"paddsw %%mm0, %%mm4\n\t" // odd luma part plus chroma part    __b7__b5__b3__b1

			"packuswb %%mm4, %%mm4\n\t" // mm4 = b7b5b3b1b7b5b3b1
			"punpcklbw %%mm2, %%mm6\n\t" // mm6 = g7g6g5g4g3g2g1g0

			"punpcklbw %%mm4, %%mm3\n\t" // mm3 = b7b6b5b4b3b2b1b0

			/* 32-bit shuffle.... */
			"pxor %%mm0, %%mm0\n\t" // is this needed?

			"movq %%mm6, %%mm1\n\t" // mm1 = g7g6g5g4g3g2g1g0
			"punpcklbw %%mm0, %%mm1\n\t" // mm1 = __g3__g2__g1__g0

			"movq %%mm3, %%mm0\n\t" // mm0 = b7b6b5b4b3b2b1b0
			"punpcklbw %%mm7, %%mm0\n\t" // mm0 = r3b3r2b2r1b1r0b0

			"movq %%mm0, %%mm2\n\t" // mm2 = r3b3r2b2r1b1r0b0

			"punpcklbw %%mm1, %%mm0\n\t" // mm0 = __r1g1b1__r0g0b0
			"punpckhbw %%mm1, %%mm2\n\t" // mm2 = __r3g3b3__r2g2b2

			/* 24-bit shuffle and save... */
			"movd  %%mm0, (%%eax)\n\t" // eax[0] = __r0g0b0
			"psrlq $32, %%mm0\n\t" // mm0 = __r1g1b1

			"movd  %%mm0, 3(%%eax)\n\t" // eax[3] = __r1g1b1

			"movd  %%mm2, 6(%%eax)\n\t" // eax[6] = __r2g2b2

			"psrlq $32, %%mm2\n\t" // mm2 = __r3g3b3

			"movd  %%mm2, 9(%%eax)\n\t" // eax[9] = __r3g3b3

			/* 32-bit shuffle.... */
			"pxor %%mm0, %%mm0\n\t" // is this needed?

			"movq %%mm6, %%mm1\n\t" // mm1 = g7g6g5g4g3g2g1g0
			"punpckhbw %%mm0, %%mm1\n\t" // mm1 = __g7__g6__g5__g4

			"movq %%mm3, %%mm0\n\t" // mm0 = b7b6b5b4b3b2b1b0
			"punpckhbw %%mm7, %%mm0\n\t" // mm0 = r7b7r6b6r5b5r4b4

			"movq %%mm0, %%mm2\n\t" // mm2 = r7b7r6b6r5b5r4b4

			"punpcklbw %%mm1, %%mm0\n\t" // mm0 = __r5g5b5__r4g4b4
			"punpckhbw %%mm1, %%mm2\n\t" // mm2 = __r7g7b7__r6g6b6

			/* 24-bit shuffle and save... */
			"movd %%mm0, 12(%%eax)\n\t" // eax[12] = __r4g4b4
			"psrlq $32, %%mm0\n\t" // mm0 = __r5g5b5

			"movd %%mm0, 15(%%eax)\n\t" // eax[15] = __r5g5b5
			"add $8, %%ebx\n\t" // puc_y   += 8;

			"movd %%mm2, 18(%%eax)\n\t" // eax[18] = __r6g6b6
			"psrlq $32, %%mm2\n\t" // mm2 = __r7g7b7

			"add $4, %%ecx\n\t" // puc_u   += 4;
			"add $4, %%edx\n\t" // puc_v   += 4;

			"movd %%mm2, 21(%%eax)\n\t" // eax[21] = __r7g7b7
			"add $24, %%eax\n\t" // puc_out += 24

			"inc %%edi\n\t"
			"jne horiz_loop24\n\t"

			"pop %%ebx\n\t"

			"emms\n\t"
			:
			: "a" (puc_out),
			  "c" (puc_u),
			  "d" (puc_v),
			  "D" (horiz_count),
			  "S" (puc_y)
			);
		puc_y   += stride_y;
		if (y%2) {
			puc_u   += stride_uv;
			puc_v   += stride_uv;
		}
		puc_out += stride_out; 
	}
#endif
}
#endif


/* 5 Jan 2001  - Andrea Graziani    */
/* this routine needs more testing  */

#ifdef YUV2RGB_16_MMX
/* all stride values are in _bytes_ */
void yuv2rgb_16(uint8_t *puc_y, int stride_y, 
                uint8_t *puc_u, uint8_t *puc_v, int stride_uv, 
                uint8_t *puc_out,	int width_y, int height_y ) {

	int y, horiz_count;
	int stride_out = width_y * 2;

#ifdef WIN32
	puc_out = puc_out + stride_out * (height_y-4); // go to the end of rgb buffer (upside down conversion)

	horiz_count = -(width_y >> 3);

	for (y=0; y<(height_y-4); y++) {
	
		_asm {
			push eax
			push ebx
			push ecx
			push edx
			push edi

			mov eax, puc_out       
			mov ebx, puc_y       
			mov ecx, puc_u       
			mov edx, puc_v
			mov edi, horiz_count
			
		horiz_loop:

			movd mm2, [ecx]
			pxor mm7, mm7

			movd mm3, [edx]
			punpcklbw mm2, mm7       ; mm2 = __u3__u2__u1__u0

			movq mm0, [ebx]          ; mm0 = y7y6y5y4y3y2y1y0  
			punpcklbw mm3, mm7       ; mm3 = __v3__v2__v1__v0

			movq mm1, mmw_0x00ff     ; mm1 = 00ff00ff00ff00ff 

			psubusb mm0, mmb_0x10    ; mm0 -= 16

			psubw mm2, mmw_0x0080    ; mm2 -= 128
			pand mm1, mm0            ; mm1 = __y6__y4__y2__y0

			psubw mm3, mmw_0x0080    ; mm3 -= 128
			psllw mm1, 3             ; mm1 *= 8

			psrlw mm0, 8             ; mm0 = __y7__y5__y3__y1
			psllw mm2, 3             ; mm2 *= 8

			pmulhw mm1, mmw_mult_Y   ; mm1 *= luma coeff 
			psllw mm0, 3             ; mm0 *= 8

			psllw mm3, 3             ; mm3 *= 8
			movq mm5, mm3            ; mm5 = mm3 = v

			pmulhw mm5, mmw_mult_V_R ; mm5 = red chroma
			movq mm4, mm2            ; mm4 = mm2 = u

			pmulhw mm0, mmw_mult_Y   ; mm0 *= luma coeff 
			movq mm7, mm1            ; even luma part

			pmulhw mm2, mmw_mult_U_G ; mm2 *= u green coeff 
			paddsw mm7, mm5          ; mm7 = luma + chroma    __r6__r4__r2__r0

			pmulhw mm3, mmw_mult_V_G ; mm3 *= v green coeff  
			packuswb mm7, mm7        ; mm7 = r6r4r2r0r6r4r2r0

			pmulhw mm4, mmw_mult_U_B ; mm4 = blue chroma
			paddsw mm5, mm0          ; mm5 = luma + chroma    __r7__r5__r3__r1

			packuswb mm5, mm5        ; mm6 = r7r5r3r1r7r5r3r1
			paddsw mm2, mm3          ; mm2 = green chroma

			movq mm3, mm1            ; mm3 = __y6__y4__y2__y0
			movq mm6, mm1            ; mm6 = __y6__y4__y2__y0

			paddsw mm3, mm4          ; mm3 = luma + chroma    __b6__b4__b2__b0
			paddsw mm6, mm2          ; mm6 = luma + chroma    __g6__g4__g2__g0
			
			punpcklbw mm7, mm5       ; mm7 = r7r6r5r4r3r2r1r0
			paddsw mm2, mm0          ; odd luma part plus chroma part    __g7__g5__g3__g1

			packuswb mm6, mm6        ; mm2 = g6g4g2g0g6g4g2g0
			packuswb mm2, mm2        ; mm2 = g7g5g3g1g7g5g3g1

			packuswb mm3, mm3        ; mm3 = b6b4b2b0b6b4b2b0
			paddsw mm4, mm0          ; odd luma part plus chroma part    __b7__b5__b3__b1

			packuswb mm4, mm4        ; mm4 = b7b5b3b1b7b5b3b1
			punpcklbw mm6, mm2       ; mm6 = g7g6g5g4g3g2g1g0

			punpcklbw mm3, mm4       ; mm3 = b7b6b5b4b3b2b1b0

			// merge rgb and write
			pxor mm0, mm0            ; is this needed?

			movq mm1, mm7						 ; mm1 = r7r6r5r4r3r2r1r0
			movq mm2, mm6						 ; mm2 = g7g6g5g4g3g2g1g0
			movq mm4, mm3						 ; mm4 = b7b6b5b4b3b2b1b0
/***/
			// reverse color order to output 64 bit at a time at the end (little endian)
			psrlq mm1, 8						 
			pand mm1, mmw_0x00ff     ; mm1 = __r7__r5__r3__r1
			pand mm7, mmw_0x00ff		 ; mm7 = __r6__r4__r2__r0
			psllq mm7, 8						 ; mm7 = r6__r4__r2__r0__
			por mm1, mm7						 ; mm1 = r6r7r4r5r2r3r0r1

			psrlq mm2, 8						 ; mm2 = __g7__g5__g3__g1
			pand mm2, mmw_0x00ff     ; mm1 = __r7__r5__r3__r1
			pand mm6, mmw_0x00ff		 ; mm6 = __g6__g4__g2__g0
			psllq mm6, 8						 ; mm6 = g6__g4__g2__g0__
			por mm2, mm6						 ; mm2 = g6g7g4g5g2g3g0g1

			psrlq mm4, 8						 ; mm4 = __b7__b5__b3__b1
			pand mm4, mmw_0x00ff     ; mm1 = __r7__r5__r3__r1
			pand mm3, mmw_0x00ff		 ; mm3 = __b6__b4__b2__b0
			psllq mm3, 8						 ; mm3 = b6__b4__b2__b0__
			por mm4, mm3						 ; mm4 = b6b7b4b5b2b3b0b1

			movq mm7, mm1						 ; mm7 = r6r7r4r5r2r3r0r1
			movq mm6, mm2						 ; mm6 = g6g7g4g5g2g3g0g1
			movq mm3, mm4						 ; mm3 = b6b7b4b5b2b3b0b1
/***
			// 1 0 3 2 5 4 7 6 wrong rgb save order!
***/
/***/
			// prepare to shuffle colors
			punpckhbw mm1, mm0			 ; mm1 = __r6__r7__r4__r5 red
			punpckhbw mm2, mm0			 ; mm2 = __g6__g7__g4__g5 green 
			punpckhbw mm4, mm0			 ; mm4 = __b6__b7__b4__b5 blue
			punpcklbw mm7, mm0			 ; mm7 = __r2__r3__r0__r1 red
			punpcklbw mm6, mm0			 ; mm6 = __g2__g3__g0__g1 green 
			punpcklbw mm3, mm0			 ; mm3 = __b2__b3__b0__b1 blue

			psllq mm1, 7						 ; shift red
			psllq mm7, 7						 
			psllq mm2, 2						 ; shift green
			psllq mm6, 2						 
			psrlq mm4, 3						 ; shift blue
			psrlq mm3, 3						 
			pand mm1, mmw_cut_red		 ; cut red
			pand mm7, mmw_cut_red
			pand mm2, mmw_cut_green	 ; cut green
			pand mm6, mmw_cut_green
			pand mm4, mmw_cut_blue	 ; cut blue ; is this needed?
			pand mm3, mmw_cut_blue

			// write 64 bit
			por mm6, mm3						 ; shuffle colors
			por mm6, mm1						 ; mm6 = rgb2rgb3rgb0rgb1
			movq [eax], mm6

			// write 64 bit
			por mm2, mm4						 ; shuffle colors
			por mm2, mm7						 ; mm2 = rgb6rgb7rgb4rgb5
			movq 8[eax], mm2

			// 0 1 2 3 4 5 6 7 rgb save order
/***/			
			add ebx, 8               ; puc_y   += 8;
			add ecx, 4               ; puc_u   += 4;
			add edx, 4               ; puc_v   += 4;
			add eax, 16              ; puc_out += 16 // wrote 16 bytes

			inc edi
			jne horiz_loop			

			pop edi 
			pop edx 
			pop ecx
			pop ebx 
			pop eax

			emms
						
		}
		puc_y   += stride_y;
		if (y%2) {
			puc_u   += stride_uv;
			puc_v   += stride_uv;
		}
		puc_out -= stride_out;
	}
#else
	puc_out = puc_out;

	horiz_count = -(width_y >> 3);

	for (y=0; y<(height_y-4); y++) {
	
	  __asm__ __volatile__ (
			"push %%ebx\n\t"
			"mov %%esi, %%ebx\n"

			//"mov eax, puc_out\n\t"
			//"mov ebx, puc_y\n\t"
			//"mov ecx, puc_u\n\t"
			//"mov edx, puc_v\n\t"
			//"mov edi, horiz_count\n\t"

		"horiz_loop_16:\n\t"

			"movd (%%ecx), %%mm2\n\t"
			"pxor %%mm7, %%mm7\n\t"

			"movd (%%edx), %%mm3\n\t"
			"punpcklbw %%mm7, %%mm2\n\t" // mm2 = __u3__u2__u1__u0

			"movq (%%ebx), %%mm0\n\t" // mm0 = y7y6y5y4y3y2y1y0  
			"punpcklbw %%mm7, %%mm3\n\t" // mm3 = __v3__v2__v1__v0

			"movq mmw_0x00ff, %%mm1\n\t" // mm1 = 00ff00ff00ff00ff 

			"psubusb mmb_0x10, %%mm0\n\t" // mm0 -= 16

			"psubw mmw_0x0080, %%mm2\n\t" // mm2 -= 128
			"pand %%mm0, %%mm1\n\t" // mm1 = __y6__y4__y2__y0

			"psubw mmw_0x0080, %%mm3\n\t" // mm3 -= 128
			"psllw $3, %%mm1\n\t" // mm1 *= 8

			"psrlw $8, %%mm0\n\t" // mm0 = __y7__y5__y3__y1
			"psllw $3, %%mm2\n\t" // mm2 *= 8

			"pmulhw mmw_mult_Y, %%mm1\n\t" // mm1 *= luma coeff 
			"psllw $3, %%mm0\n\t" // mm0 *= 8

			"psllw $3, %%mm3\n\t" // mm3 *= 8
			"movq %%mm3, %%mm5\n\t" // mm5 = mm3 = v

			"pmulhw mmw_mult_V_R, %%mm5\n\t" // mm5 = red chroma
			"movq %%mm2, %%mm4\n\t" // mm4 = mm2 = u

			"pmulhw mmw_mult_Y, %%mm0\n\t" // mm0 *= luma coeff 
			"movq %%mm1, %%mm7\n\t" // even luma part

			"pmulhw mmw_mult_U_G, %%mm2\n\t" // mm2 *= u green coeff 
			"paddsw %%mm5, %%mm7\n\t" // mm7 = luma + chroma    __r6__r4__r2__r0

			"pmulhw mmw_mult_V_G, %%mm3\n\t" // mm3 *= v green coeff  
			"packuswb %%mm7, %%mm7\n\t" // mm7 = r6r4r2r0r6r4r2r0

			"pmulhw mmw_mult_U_B, %%mm4\n\t" // mm4 = blue chroma
			"paddsw %%mm0, %%mm5\n\t" // mm5 = luma + chroma    __r7__r5__r3__r1

			"packuswb %%mm5, %%mm5\n\t" // mm6 = r7r5r3r1r7r5r3r1
			"paddsw %%mm3, %%mm2\n\t" // mm2 = green chroma

			"movq %%mm1, %%mm3\n\t" // mm3 = __y6__y4__y2__y0
			"movq %%mm1, %%mm6\n\t" // mm6 = __y6__y4__y2__y0

			"paddsw %%mm4, %%mm3\n\t" // mm3 = luma + chroma    __b6__b4__b2__b0
			"paddsw %%mm2, %%mm6\n\t" // mm6 = luma + chroma    __g6__g4__g2__g0

			"punpcklbw %%mm5, %%mm7\n\t" // mm7 = r7r6r5r4r3r2r1r0
			"paddsw %%mm0, %%mm2\n\t" // odd luma part plus chroma part    __g7__g5__g3__g1

			"packuswb %%mm6, %%mm6\n\t" // mm2 = g6g4g2g0g6g4g2g0
			"packuswb %%mm2, %%mm2\n\t" // mm2 = g7g5g3g1g7g5g3g1

			"packuswb %%mm3, %%mm3\n\t" // mm3 = b6b4b2b0b6b4b2b0
			"paddsw %%mm0, %%mm4\n\t" // odd luma part plus chroma part    __b7__b5__b3__b1

			"packuswb %%mm4, %%mm4\n\t" // mm4 = b7b5b3b1b7b5b3b1
			"punpcklbw %%mm2, %%mm6\n\t" // mm6 = g7g6g5g4g3g2g1g0

			"punpcklbw %%mm4, %%mm3\n\t" // mm3 = b7b6b5b4b3b2b1b0

			// merge rgb and write
			"pxor %%mm0, %%mm0\n\t" // is this needed?

			"movq %%mm7, %%mm1\n\t" // mm1 = r7r6r5r4r3r2r1r0
			"movq %%mm6, %%mm2\n\t" // mm2 = g7g6g5g4g3g2g1g0
			"movq %%mm3, %%mm4\n\t" // mm4 = b7b6b5b4b3b2b1b0

			// reverse color order to output 64 bit at a time at the end (little endian)
			"psrlq $8, %%mm1\n\t"
			"pand mmw_0x00ff, %%mm1\n\t" // mm1 = __r7__r5__r3__r1
			"pand mmw_0x00ff, %%mm7\n\t" // mm7 = __r6__r4__r2__r0
			"psllq $8, %%mm7\n\t" // mm7 = r6__r4__r2__r0__
			"por %%mm7, %%mm1\n\t" // mm1 = r6r7r4r5r2r3r0r1

			"psrlq $8, %%mm2\n\t" // mm2 = __g7__g5__g3__g1
			"pand mmw_0x00ff, %%mm2\n\t" // mm1 = __r7__r5__r3__r1
			"pand mmw_0x00ff, %%mm6\n\t" // mm6 = __g6__g4__g2__g0
			"psllq $8, %%mm6\n\t" // mm6 = g6__g4__g2__g0__
			"por %%mm6, %%mm2\n\t" // mm2 = g6g7g4g5g2g3g0g1

			"psrlq $8, %%mm4\n\t" // mm4 = __b7__b5__b3__b1
			"pand mmw_0x00ff, %%mm4\n\t" // mm1 = __r7__r5__r3__r1
			"pand mmw_0x00ff, %%mm3\n\t" // mm3 = __b6__b4__b2__b0
			"psllq $8, %%mm3\n\t" // mm3 = b6__b4__b2__b0__
			"por %%mm3, %%mm4\n\t" // mm4 = b6b7b4b5b2b3b0b1

			"movq %%mm1, %%mm7\n\t" // mm7 = r6r7r4r5r2r3r0r1
			"movq %%mm2, %%mm6\n\t" // mm6 = g6g7g4g5g2g3g0g1
			"movq %%mm4, %%mm3\n\t" // mm3 = b6b7b4b5b2b3b0b1

			// 1 0 3 2 5 4 7 6 wrong rgb save order!
			// prepare to shuffle colors
			"punpckhbw %%mm0, %%mm1\n\t" // mm1 = __r6__r7__r4__r5 red
			"punpckhbw %%mm0, %%mm2\n\t" // mm2 = __g6__g7__g4__g5 green 
			"punpckhbw %%mm0, %%mm4\n\t" // mm4 = __b6__b7__b4__b5 blue
			"punpcklbw %%mm0, %%mm7\n\t" // mm7 = __r2__r3__r0__r1 red
			"punpcklbw %%mm0, %%mm6\n\t" // mm6 = __g2__g3__g0__g1 green 
			"punpcklbw %%mm0, %%mm3\n\t" // mm3 = __b2__b3__b0__b1 blue

			"psllq $7, %%mm1\n\t" // shift red
			"psllq $7, %%mm7\n\t"
			"psllq $2, %%mm2\n\t" // shift green
			"psllq $2, %%mm6\n\t"
			"psrlq $3, %%mm4\n\t" // shift blue
			"psrlq $3, %%mm3\n\t"
			"pand mmw_cut_red, %%mm1\n\t" // cut red
			"pand mmw_cut_red, %%mm7\n\t"
			"pand mmw_cut_green, %%mm2\n\t" // cut green
			"pand mmw_cut_green, %%mm6\n\t"
			"pand mmw_cut_blue, %%mm4\n\t" // cut blue ; is this needed?
			"pand mmw_cut_blue, %%mm3\n\t"

			// write 64 bit
			"por %%mm3, %%mm6\n\t" // shuffle colors
			"por %%mm1, %%mm6\n\t" // mm6 = rgb2rgb3rgb0rgb1
			"movq %%mm6, (%%eax)\n\t"

			// write 64 bit
			"por %%mm4, %%mm2\n\t" // shuffle colors
			"por %%mm7, %%mm2\n\t" // mm2 = rgb6rgb7rgb4rgb5
			"movq %%mm2, 8(%%eax)\n\t"

			// 0 1 2 3 4 5 6 7 rgb save order

			"add $8, %%ebx\n\t" // puc_y   += 8;
			"add $4, %%ecx\n\t" // puc_u   += 4;
			"add $4, %%edx\n\t" // puc_v   += 4;
			"add $16, %%eax\n\t" // puc_out += 16 // wrote 16 bytes

			"inc %%edi\n\t"
			"jne horiz_loop_16\n\t"

			"pop %%ebx\n\t"

			"emms"
			:
			: "a" (puc_out),
			"c" (puc_u),
			"d" (puc_v),
			"D" (horiz_count),
			"S" (puc_y)
			);
		puc_y   += stride_y;
		if (y%2) {
			puc_u   += stride_uv;
			puc_v   += stride_uv;
		}
		puc_out += stride_out;
	}
#endif
}
#endif
