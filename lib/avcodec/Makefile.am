## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Jul  3 12:37:30 2005.
# $Id: Makefile.am,v 1.28 2005/07/03 03:44:43 sian Exp $

if USE_AVCODEC
noinst_LTLIBRARIES = libavcodec.la
endif
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib
EXTRA_DIST = \
	i386/fdct_mmx.c i386/cputest.c \
	i386/dsputil_mmx.c i386/mpegvideo_mmx.c i386/mpegvideo_mmx_template.c \
	i386/idct_mmx.c i386/motion_est_mmx.c \
	i386/simple_idct_mmx.c i386/fft_sse.c \
	i386/dsputil_mmx_avg.h i386/dsputil_mmx_rnd.h i386/mmx.h \
	svq3.c mdec.c motion_est_template.c wmv2.c

DEFS = -DHAVE_AV_CONFIG_H

if USE_MMX
DEFS += -DHAVE_MMX -Dasm=__asm -Dvolatile=__volatile
# We want many registers... so you must omit frame pointer.
CFLAGS_POST = -Wno-error -Wno-unused -O4 -ffast-math -fomit-frame-pointer $(CFLAGS_POST_SSE)
#CFLAGS_POST = -Wno-error -Wno-unused -O -fomit-frame-pointer $(CFLAGS_POST_SSE)
MMX_SRCS = \
	i386/fdct_mmx.c i386/cputest.c \
	i386/dsputil_mmx.c i386/mpegvideo_mmx.c \
	i386/idct_mmx.c i386/motion_est_mmx.c \
	i386/simple_idct_mmx.c i386/fft_sse.c \
	i386/vp3dsp_mmx.c i386/vp3dsp_sse2.c
else
CFLAGS_POST = -Wno-unused
MMX_SRCS =
endif

if USE_SSE
CFLAGS_POST_SSE = -msse
DEFS += -DHAVE_BUILTIN_VECTOR
endif

# # armv4l specific stuff
# ifeq ($(TARGET_ARCH_ARMV4L),yes)
# ASM_OBJS += armv4l/jrevdct_arm.o armv4l/simple_idct_arm.o
# OBJS += armv4l/dsputil_arm.o armv4l/mpegvideo_arm.o
# endif

# # sun mediaLib specific stuff
# # currently only works when libavcodec is used in mplayer
# ifeq ($(HAVE_MLIB),yes)
# OBJS += mlib/dsputil_mlib.o
# CFLAGS += $(MLIB_INC)
# endif

# # alpha specific stuff
# ifeq ($(TARGET_ARCH_ALPHA),yes)
# OBJS += alpha/dsputil_alpha.o alpha/mpegvideo_alpha.o \
#         alpha/simple_idct_alpha.o alpha/motion_est_alpha.o
# ASM_OBJS += alpha/dsputil_alpha_asm.o alpha/motion_est_mvi_asm.o
# CFLAGS += -fforce-addr -freduce-all-givs
# endif

# ifeq ($(TARGET_ARCH_POWERPC),yes)
# OBJS += ppc/dsputil_ppc.o ppc/mpegvideo_ppc.o
# endif

# ifeq ($(TARGET_MMI),yes)
# OBJS += ps2/dsputil_mmi.o ps2/idct_mmi.o ps2/mpegvideo_mmi.o
# endif

# ifeq ($(TARGET_ALTIVEC),yes)
# OBJS += ppc/dsputil_altivec.o ppc/mpegvideo_altivec.o ppc/idct_altivec.o \
#         ppc/fft_altivec.o ppc/gmc_altivec.o ppc/fdct_altivec.o
# endif

# ifeq ($(TARGET_ARCH_SH4),yes)
# OBJS+= sh4/idct_sh4.o sh4/dsputil_sh4.o sh4/dsputil_align.o
# endif

noinst_HEADERS = \
	ac3.h \
	bitstream.h \
	common.h \
	golomb.h \
	mpeg12data.h \
	mpegvideo.h \
	rational.h \
	truemotion1data.h \
	ac3tab.h \
	dsputil.h \
	h263data.h \
	mpeg4data.h \
	msmpeg4data.h \
	simple_idct.h \
	vp3data.h \
	avcodec.h \
	dvdata.h \
	h261data.h \
	h264data.h \
	indeo2data.h \
	mpegaudio.h \
	oggvorbis.h \
	sp5x.h \
	wmadata.h \
	bswap.h \
	faandct.h \
	imgconvert_template.h \
	mpegaudiodectab.h \
	ra144.h \
	svq1_cb.h \
	cabac.h \
	indeo3data.h \
	mpegaudiotab.h \
	ra288.h \
	svq1_vlc.h \
	integer.h \
	ulti_cb.h \
	rangecoder.h \
	vc9data.h \
	vorbis.h

#	a52dec.c	ac3dec.c	ac3enc.c
#	amr.c		faac.c		faad.c
#	mp3lameaudio.c	dtsdec.c	xvidff.c
#	x264.c		libgsm.c
libavcodec_la_SOURCES = \
	4xm.c \
	8bps.c \
	aasc.c \
	adpcm.c \
	adx.c \
	alac.c \
	allcodecs.c \
	asv1.c \
	bitstream.c \
	cabac.c \
	cinepak.c \
	cljr.c \
	cyuv.c \
	dpcm.c \
	dsputil.c \
	dv.c \
	dvbsub.c \
	dvdsub.c \
	error_resilience.c \
	eval.c \
	faandct.c \
	fdctref.c \
	fft.c \
	ffv1.c \
	flac.c \
	flicvideo.c \
	fraps.c \
	g726.c \
	golomb.c \
	h261.c \
	h263.c \
	h263dec.c \
	h264.c \
	h264idct.c \
	huffyuv.c \
	idcinvideo.c \
	imgconvert.c \
	imgresample.c \
	indeo2.c \
	indeo3.c \
	integer.c \
	interplayvideo.c \
	jfdctfst.c \
	jfdctint.c \
	jrevdct.c \
	lcl.c \
	loco.c \
	mace.c \
	mdct.c \
	mem.c \
	mjpeg.c \
	motion_est.c \
	mpeg12.c \
	mpegaudio.c \
	mpegaudiodec.c \
	mpegvideo.c \
	msmpeg4.c \
	msrle.c \
	msvideo1.c \
	oggtheora.c \
	oggvorbis.c \
	opts.c \
	parser.c \
	pcm.c \
	png.c \
	pnm.c \
	pthread.c \
	qdrw.c \
	qpeg.c \
	qtrle.c \
	ra144.c \
	ra288.c \
	rangecoder.c \
	ratecontrol.c \
	rational.c \
	raw.c \
	resample.c \
	resample2.c \
	roqvideo.c \
	rpza.c \
	rv10.c \
	shorten.c \
	simple_idct.c \
	smc.c \
	snow.c \
	sonic.c \
	svq1.c \
	truemotion1.c \
	tscc.c \
	ulti.c \
	utils.c \
	vc9.c \
	vcr1.c \
	vmdav.c \
	vorbis.c \
	vp3.c \
	vp3dsp.c \
	vqavideo.c \
	wmadec.c \
	wnv1.c \
	ws-snd1.c \
	xan.c \
	xl.c \
	xvmcvideo.c \
	$(MMX_SRCS)
