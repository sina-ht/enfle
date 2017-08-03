## Process this file with automake to produce Makefile.in
# Makefile.am for libdivxdecore

noinst_LTLIBRARIES = libdivxdecore.la
#INCLUDES =

if USE_MMX
AM_CFLAGS = -D_DECORE -DUSE_MMX
transferIDCT=transferIDCT_mmx.c
idct=idct.c idct_mmx.c
else
AM_CFLAGS = -D_DECORE
transferIDCT=transferIDCT.c
idct=idct.c
endif

libdivxdecore_la_SOURCES = \
	basic_prediction.c basic_prediction.h \
	decore.c decore.h \
	getbits.c \
	global.h \
	$(idct) idct.h \
	mp4_block.c mp4_block.h \
	mp4_decoder.c mp4_decoder.h \
	mp4_header.c mp4_header.h \
	mp4_mblock.c mp4_mblock.h \
	mp4_picture.c \
	mp4_predict.c mp4_predict.h \
	mp4_recon.c \
	mp4_vld.c mp4_vld.h \
	portab.h \
	postprocess.c postprocess.h \
	$(transferIDCT) transferIDCT.h \
	yuv2rgb.c yuv2rgb_mmx.c yuv2rgb.h
