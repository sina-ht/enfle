### bgra32to16_mmx.s -- convert b8g8r8a8 to bbbbbggg gggrrrrr
### (C)Copyright 2000, 2001 by Hiroshi Takekawa
###  by coutesy of Intel MMX Application Notes AP553
### This file is not part of any software package.
###
### Last Modified: Mon Sep 17 18:11:52 2001.

### Assumption:
### The number of elements allocated for the source,destination must be
### divisible by 8. The number of rows X the number of columns does not
### need to be divisible by 8. This is to allow working on 8 pixels within
### the inner loop without having to post-process pixels after the loop.

.section .rodata	
rgbMulFactor:	.long 0x20000004, 0x20000004
rgbMask1:	.long 0x00f800f8, 0x00f800f8
rgbMask2:	.long 0x0000fc00, 0x0000fc00

### Description:
###  This is a rather optimized MMX routine to convert from BGRA32 data to
###  B5G6R5. The inner loop processes 8 pixels at a time and packs the 8
###  pixels represented as 8 DWORDs into 8 WORDs. The algorithm used for
###  each 2 pixels is as follows:
###
###  Step 1: read in 2 pixels as a quad word
###  Step 2: make a copy of the two pixels
###  Step 3: AND the 2 pixels with 0x00f800f800f800f8 to obtain a 
###                          quad word of:
###  00000000RRRRR00000000000BBBBB000 00000000rrrrr00000000000bbbbb000
### 
###  Step 4: PMADDWD this quad word by 0x2000000420000004 to obtain
###                          a quad word of:
###  00000000000RRRRR000000BBBBB00000 00000000000rrrrr000000bbbbb00000
### 
###  Step 5: AND the copy of the original pixels with 
###                          0x0000fc000000fc00 to obtain
###  0000000000000000GGGGGG0000000000 0000000000000000gggggg0000000000
### 
###  Step 6: OR the results of step 4 and step 5 to obtain
###  00000000000RRRRRGGGGGGBBBBB00000 00000000000rrrrrggggggbbbbb00000
### 
###  Step 7: SHIFT RIGHT by 5 bits to obtain
###  0000000000000000RRRRRGGGGGGBBBBB 0000000000000000rrrrrggggggbbbbb
### 
###  Step 8: When two pairs of pixels are converted, pack the results into
###          one register and then store them into the destination.

### void bgra32to16_mmx(unsigned char *dest, unsigned char *src, unsigned int width, unsigned int height);
.text
.global bgra32to16_mmx

        .align 16

offset = 2*4

bgra32to16_mmx: 
        pushl   %ebx
        pushl   %ecx
        movl    offset+16(%esp),%eax        # eax = height
        movl    offset+12(%esp),%ebx        # ebx = width
        imull   %ebx                        # eax = total number of pixels

        movl    offset+8(%esp),%ebx         # ebx = src
        subl    $1,%eax

        movl    offset+4(%esp),%edx         # edx = dest
        andl    $0xfffffff8,%eax            # align eax to the last 8 pixel boundary

### Here, eax has the number of elements in the source. eax is then
### adjusted to contain the index to the last 8 pixel aligned
### pixel. Pointers to the source and destination are also set up.

### This section performs up to and including step 4 on pixels 0 and 1. It
### also performs up to and including step 5 on pixels 2 and 3.  This is
### done prior to entering the loop so that better loop efficiency is
### achieved. Better loop efficiency is achieved because these
### instructions are paired with other instruction at the end of the loop
### which could not be previously paired.

        movq    rgbMulFactor,%mm7       # mm7 = pixel multiplication factor
        movq    rgbMask2,%mm6           # mm6 = green pixel mask

        movq    8(%ebx,%eax,4),%mm2     # mm2 = pixels 2 and 3
        movq    0(%ebx,%eax,4),%mm0     # mm0 = pixels 0 and 1
        movq    %mm2,%mm3               # mm3 = pixels 2 and 3

        pand    rgbMask1,%mm3           # mm3 = R and B of pixels 2 and 3
        movq    %mm0,%mm1               # mm1 = pixels 0 and 1

        pand    rgbMask1,%mm1           # mm1 = R and B of pixels 0 and 1
        pmaddwd %mm7,%mm3               # mm3 = shifted R and B of pixels 2 and 3

        pmaddwd %mm7,%mm1               # mm1 = shifted R and B of pixels 0 and 1
        pand    %mm6,%mm2               # mm2 = G of pixels 2 and 3

### This section performs steps 1 through 8 for 4 pairs of pixels (or for
### a total of 8 pixels).

inner_loop: 
        movq    24(%ebx,%eax,4),%mm4    # mm4 = pixels 6 and 7
        pand    %mm6,%mm0               # mm0 = G of pixels 0 and 1

        movq    16(%ebx,%eax,4),%mm5    # mm5 = pixels 4 and 5
        por     %mm2,%mm3               # mm3 = RGB of pixels 2 and 3

        psrld   $5,%mm3                 # mm3 = converted pixels 2 and 3
        por     %mm0,%mm1               # mm1 = RGB of pixels 0 and 1

        movq    %mm4,%mm0               # mm0 = pixels 6 and 7
        psrld   $5,%mm1                 # mm1 = converted pixels 0 and 1

        pand    rgbMask1,%mm0           # mm0 = R and B of pixels 6 and 7

        ## store pixels 2 and 3
        ## cannot use packssdw. why no packusdw?
        movd    %mm3,%ecx
        movw    %cx,4(%edx,%eax,2)
        psrlq   $32,%mm3
        movd    %mm3,%ecx
        movw    %cx,6(%edx,%eax,2)

        movq    %mm5,%mm3               # copy pixels 4 and 5
        pmaddwd %mm7,%mm0               # SHIFT-OR pixels 6 and 7

        pand    rgbMask1,%mm3           # get R and B of pixels 4 and 5
        pand    %mm6,%mm4               # get G of pixels 6 and 7

        ## store pixels 0 and 1
        movd    %mm1,%ecx
        movw    %cx,0(%edx,%eax,2)
        psrlq   $32,%mm1
        movd    %mm1,%ecx
        movw    %cx,2(%edx,%eax,2)

        pmaddwd %mm7,%mm3               # SHIFT-OR pixels 4 and 5

        subl    $8,%eax                 # subtract 8 pixels from the index
        por     %mm0,%mm4               # get RGB of pixels 6 and 7

        pand    %mm6,%mm5               # get G of pixels 4 and 5
        psrld   $5,%mm4                 # mm4 = converted pixels 6 and 7

        movq    8(%ebx,%eax,4),%mm2     # get pixels 2 and 3 for the next
        por     %mm3,%mm5               # get RGB of pixels 4 and 5

        movq    0(%ebx,%eax,4),%mm0     # get pixels 0 and 1 for the next
        psrld   $5,%mm5                 # mm5 = converted pixels 4 and 5

        movq    %mm2,%mm3               # copy pixels 2 and 3
        movq    %mm0,%mm1               # copy pixels 0 and 1

        pand    rgbMask1,%mm3           # get R and B of pixels 2 and 3

        ## store pixels 6 and 7
        movd    %mm4,%ecx
        movw    %cx,28(%edx,%eax,2)
        psrlq   $32,%mm4
        movd    %mm4,%ecx
        movw    %cx,30(%edx,%eax,2)

        pand    rgbMask1,%mm1           # get R and B of pixels 0 and 1
        pand    %mm6,%mm2               # get G of pixels 2 and 3

        ## store pixels 4 and 5
        movd    %mm5,%ecx
        movw    %cx,24(%edx,%eax,2)
        psrlq   $32,%mm5
        movd    %mm5,%ecx
        movw    %cx,26(%edx,%eax,2)

        pmaddwd %mm7,%mm3               # SHIFT-OR pixels 2 and 3
        pmaddwd %mm7,%mm1               # SHIFT-OR pixels 0 and 1

        jge     inner_loop

### process the rest

#       pand    mm0, mm6                ; mm0 = G of pixels 0 and 1
#       por     mm3, mm2                ; mm3 = RGB of pixels 2 and 3
#       psrld   mm3, 5                  ; mm3 = converted pixels 2 and 3
#       por     mm1, mm0                ; mm1 = RGB of pixels 0 and 1
#       psrld   mm1, 5                  ; mm1 = converted pixels 0 and 1
#       ;; store pixels 2 and 3
#       movd    ecx, mm3
#       mov     [edx+eax*2+4], cx
#       psrlq   mm3, 32
#       movd    ecx, mm3
#       mov     [edx+eax*2+6], cx
#       movd    [edx+eax*2+0], mm1      ; store pixels 0 and 1
#       ;; store pixels 0 and 1
#       movd    ecx, mm1
#       mov     [edx+eax+2+0], cx
#       psrlq   mm1, 32
#       movd    ecx, mm1
#       mov     [edx+eax*2+2], cx

        emms

        popl    %ecx
        popl    %ebx

        ret
