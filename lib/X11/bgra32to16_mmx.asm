;;; bgra32to16_mmx.asm -- convert b8g8r8a8 to bbbbbggg gggrrrrr
;;; (C)Copyright 2000 by Hiroshi Takekawa
;;;  by coutesy of Intel MMX Application Notes AP553
;;; This file is not part of any software package.
;;;
;;; Last Modified: Tue Sep 12 05:42:46 2000.

;;; Assumption:
;;; The number of elements allocated for the source,destination must be
;;; divisible by 8. The number of rows X the number of columns does not
;;; need to be divisible by 8. This is to allow working on 8 pixels within
;;; the inner loop without having to post-process pixels after the loop.

segment		.data align=16 class=DATA use32

rgbMulFactor	dd 0x20000004, 0x20000004
rgbMask1	dd 0x00f800f8, 0x00f800f8
rgbMask2	dd 0x0000fc00, 0x0000fc00

segment		.text align=16 class=CODE use32

;;; Description:
;;;  This is a rather optimized MMX routine to convert from BGRA32 data to
;;;  B5G6R5. The inner loop processes 8 pixels at a time and packs the 8
;;;  pixels represented as 8 DWORDs into 8 WORDs. The algorithm used for
;;;  each 2 pixels is as follows:
;;;
;;;  Step 1: read in 2 pixels as a quad word
;;;  Step 2: make a copy of the two pixels
;;;  Step 3: AND the 2 pixels with 0x00f800f800f800f8 to obtain a 
;;;                          quad word of:
;;;  00000000RRRRR00000000000BBBBB000 00000000rrrrr00000000000bbbbb000
;;; 
;;;  Step 4: PMADDWD this quad word by 0x2000000420000004 to obtain
;;;                          a quad word of:
;;;  00000000000RRRRR000000BBBBB00000 00000000000rrrrr000000bbbbb00000
;;; 
;;;  Step 5: AND the copy of the original pixels with 
;;;                          0x0000fc000000fc00 to obtain
;;;  0000000000000000GGGGGG0000000000 0000000000000000gggggg0000000000
;;; 
;;;  Step 6: OR the results of step 4 and step 5 to obtain
;;;  00000000000RRRRRGGGGGGBBBBB00000 00000000000rrrrrggggggbbbbb00000
;;; 
;;;  Step 7: SHIFT RIGHT by 5 bits to obtain
;;;  0000000000000000RRRRRGGGGGGBBBBB 0000000000000000rrrrrggggggbbbbb
;;; 
;;;  Step 8: When two pairs of pixels are converted, pack the results into
;;;          one register and then store them into the destination.

;;; void bgra32to16(unsigned char *dest, unsigned char *src, int width, int height);
global bgra32to16

	align	16

bgra32to16:
	push	ebx
	push	ecx
%assign _P 2*4
	mov	eax, [esp+_P+16]	; eax = height
	mov	ebx, [esp+_P+12]	; ebx = width
	imul	ebx			; eax = total number of pixels

	mov	ebx, [esp+_P+8]		; ebx = src
	sub	eax, 1

	mov	edx, [esp+_P+4]		; edx = dest
	and	eax, 0xfffffff8		; align eax to the last 8 pixel boundary

;;; Here, eax has the number of elements in the source. eax is then
;;; adjusted to contain the index to the last 8 pixel aligned
;;; pixel. Pointers to the source and destination are also set up.

;;; This section performs up to and including step 4 on pixels 0 and 1. It
;;; also performs up to and including step 5 on pixels 2 and 3.  This is
;;; done prior to entering the loop so that better loop efficiency is
;;; achieved. Better loop efficiency is achieved because these
;;; instructions are paired with other instruction at the end of the loop
;;; which could not be previously paired.

	movq	mm7, [rgbMulFactor]	; mm7 = pixel multiplication factor
	movq	mm6, [rgbMask2]		; mm6 = green pixel mask

	movq	mm2, [ebx+eax*4+8]	; mm2 = pixels 2 and 3
	movq	mm0, [ebx+eax*4+0]	; mm0 = pixels 0 and 1
	movq	mm3, mm2		; mm3 = pixels 2 and 3

	pand	mm3, [rgbMask1]		; mm3 = R and B of pixels 2 and 3
	movq	mm1, mm0		; mm1 = pixels 0 and 1

	pand	mm1, [rgbMask1]		; mm1 = R and B of pixels 0 and 1
	pmaddwd	mm3, mm7		; mm3 = shifted R and B of pixels 2 and 3

	pmaddwd	mm1, mm7		; mm1 = shifted R and B of pixels 0 and 1
	pand	mm2, mm6		; mm2 = G of pixels 2 and 3

;;; This section performs steps 1 through 8 for 4 pairs of pixels (or for
;;; a total of 8 pixels).

inner_loop:
	movq	mm4, [ebx+eax*4+24]	; mm4 = pixels 6 and 7
	pand	mm0, mm6		; mm0 = G of pixels 0 and 1

	movq	mm5, [ebx+eax*4+16]	; mm5 = pixels 4 and 5
	por	mm3, mm2		; mm3 = RGB of pixels 2 and 3

	psrld	mm3, 5			; mm3 = converted pixels 2 and 3
	por	mm1, mm0		; mm1 = RGB of pixels 0 and 1

	movq	mm0, mm4		; mm0 = pixels 6 and 7
	psrld	mm1, 5			; mm1 = converted pixels 0 and 1

	pand	mm0, [rgbMask1]		; mm0 = R and B of pixels 6 and 7

	;; store pixels 2 and 3
	;; cannot use packssdw; why no packusdw?
  	movd	ecx, mm3
    	mov	[edx+eax*2+4], cx
    	psrlq	mm3, 32
    	movd	ecx, mm3
    	mov	[edx+eax*2+6], cx

	movq	mm3, mm5		; copy pixels 4 and 5
	pmaddwd mm0, mm7		; SHIFT-OR pixels 6 and 7

	pand	mm3, [rgbMask1]		; get R and B of pixels 4 and 5
	pand	mm4, mm6		; get G of pixels 6 and 7

	;; store pixels 0 and 1
  	movd	ecx, mm1
  	mov	[edx+eax*2+0], cx
  	psrlq	mm1, 32
  	movd	ecx, mm1
  	mov	[edx+eax*2+2], cx

	pmaddwd	mm3, mm7		; SHIFT-OR pixels 4 and 5

	sub	eax, 8			; subtract 8 pixels from the index
	por	mm4, mm0		; get RGB of pixels 6 and 7

	pand	mm5, mm6		; get G of pixels 4 and 5
	psrld	mm4, 5			; mm4 = converted pixels 6 and 7

	movq	mm2, [ebx+eax*4+8]	; get pixels 2 and 3 for the next
	por	mm5, mm3		; get RGB of pixels 4 and 5

	movq	mm0, [ebx+eax*4+0]	; get pixels 0 and 1 for the next
	psrld	mm5, 5			; mm5 = converted pixels 4 and 5

	movq	mm3, mm2		; copy pixels 2 and 3
	movq	mm1, mm0		; copy pixels 0 and 1

	pand	mm3, [rgbMask1]		; get R and B of pixels 2 and 3

	;; store pixels 6 and 7
  	movd	ecx, mm4
  	mov	[edx+eax*2+28], cx
  	psrlq	mm4, 32
  	movd	ecx, mm4
  	mov	[edx+eax*2+30], cx

	pand	mm1, [rgbMask1]		; get R and B of pixels 0 and 1
	pand	mm2, mm6		; get G of pixels 2 and 3

	;; store pixels 4 and 5
  	movd	ecx, mm5
  	mov	[edx+eax*2+24], cx
  	psrlq	mm5, 32
  	movd	ecx, mm5
  	mov	[edx+eax*2+26], cx

	pmaddwd	mm3, mm7		; SHIFT-OR pixels 2 and 3
	pmaddwd	mm1, mm7		; SHIFT-OR pixels 0 and 1

	jge	near inner_loop

;;; process the rest

;  	pand	mm0, mm6		; mm0 = G of pixels 0 and 1
;  	por	mm3, mm2		; mm3 = RGB of pixels 2 and 3
;  	psrld	mm3, 5			; mm3 = converted pixels 2 and 3
;  	por	mm1, mm0		; mm1 = RGB of pixels 0 and 1
;  	psrld	mm1, 5			; mm1 = converted pixels 0 and 1
;  	;; store pixels 2 and 3
;    	movd	ecx, mm3
;      	mov	[edx+eax*2+4], cx
;      	psrlq	mm3, 32
;      	movd	ecx, mm3
;      	mov	[edx+eax*2+6], cx
;  	movd	[edx+eax*2+0], mm1	; store pixels 0 and 1
;  	;; store pixels 0 and 1
;    	movd	ecx, mm1
;    	mov	[edx+eax+2+0], cx
;    	psrlq	mm1, 32
;    	movd	ecx, mm1
;    	mov	[edx+eax*2+2], cx

        emms

	pop	ecx
	pop	ebx

	ret

	end
