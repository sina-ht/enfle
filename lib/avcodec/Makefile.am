## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Aug  6 16:36:52 2006.
# $Id: Makefile.am,v 1.42 2006/08/06 07:48:57 sian Exp $

AUTOMAKE_OPTIONS = nostdinc

if USE_AVCODEC
noinst_LTLIBRARIES = libavcodec.la
endif

AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/avutil -I$(top_srcdir)/lib/avcodec -I$(top_srcdir) -I$(top_builddir) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE
EXTRA_DIST = \
	i386/fdct_mmx.c i386/cputest.c \
	i386/dsputil_mmx.c i386/mpegvideo_mmx.c i386/mpegvideo_mmx_template.c \
	i386/idct_mmx.c i386/motion_est_mmx.c \
	i386/simple_idct_mmx.c i386/fft_sse.c \
	i386/dsputil_mmx_avg.h i386/dsputil_mmx_rnd.h i386/mmx.h \
	i386/h264dsp_mmx.c i386/dsputil_h264_template_mmx.c \
	svq3.c mdec.c motion_est_template.c wmv2.c jpeg_ls.c \
	gcc_fixes.h

DEFS = -prefer-non-pic -DHAVE_AV_CONFIG_H

if USE_MMX
DEFS += -DHAVE_MMX -Dasm=__asm -Dvolatile=__volatile
# We want many registers... so you must omit frame pointer.
CFLAGS_POST = -Wno-error -Wno-unused -O3 -ffast-math $(CFLAGS_POST_SSE)
#CFLAGS_POST = -Wno-error -Wno-unused -O -fomit-frame-pointer $(CFLAGS_POST_SSE)

# i386/cavsdsp_mmx.c
MMX_SRCS = \
	i386/fdct_mmx.c i386/cputest.c \
	i386/dsputil_mmx.c i386/mpegvideo_mmx.c \
	i386/idct_mmx.c i386/idct_mmx_xvid.c i386/motion_est_mmx.c \
	i386/simple_idct_mmx.c i386/fft_sse.c \
	i386/vp3dsp_mmx.c i386/vp3dsp_sse2.c \
	i386/snowdsp_mmx.c
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
# ASM_OBJS += armv4l/jrevdct_arm.o armv4l/simple_idct_arm.o armv4l/dsputil_arm_s.o
# OBJS += armv4l/dsputil_arm.o armv4l/mpegvideo_arm.o
# ifeq ($(TARGET_IWMMXT),yes)
# OBJS += armv4l/dsputil_iwmmxt.o armv4l/mpegvideo_iwmmxt.o
# endif
# endif

# # sun mediaLib specific stuff
# # currently only works when libavcodec is used in mplayer
# ifeq ($(HAVE_MLIB),yes)
# OBJS += mlib/dsputil_mlib.o
# CFLAGS += $(MLIB_INC)
# endif

# # Intel IPP specific stuff
# # currently only works when libavcodec is used in mplayer
# ifeq ($(HAVE_IPP),yes)
# CFLAGS += $(IPP_INC)
# endif

# # alpha specific stuff
# ifeq ($(TARGET_ARCH_ALPHA),yes)
# OBJS += alpha/dsputil_alpha.o alpha/mpegvideo_alpha.o \
# 	alpha/simple_idct_alpha.o alpha/motion_est_alpha.o
# ASM_OBJS += alpha/dsputil_alpha_asm.o alpha/motion_est_mvi_asm.o
# CFLAGS += -fforce-addr
# endif

# ifeq ($(TARGET_ARCH_POWERPC),yes)
# OBJS += ppc/dsputil_ppc.o ppc/mpegvideo_ppc.o
# endif

# ifeq ($(TARGET_MMI),yes)
# OBJS += ps2/dsputil_mmi.o ps2/idct_mmi.o ps2/mpegvideo_mmi.o
# endif

# ifeq ($(TARGET_ALTIVEC),yes)
# OBJS += ppc/dsputil_altivec.o ppc/mpegvideo_altivec.o ppc/idct_altivec.o \
#         ppc/fft_altivec.o ppc/gmc_altivec.o ppc/fdct_altivec.o \
#         ppc/dsputil_h264_altivec.o ppc/dsputil_snow_altivec.o
# endif

# ifeq ($(TARGET_ARCH_SH4),yes)
# OBJS+= sh4/idct_sh4.o sh4/dsputil_sh4.o sh4/dsputil_align.o
# endif

# ifeq ($(TARGET_ARCH_SPARC),yes)
# OBJS+=sparc/dsputil_vis.o
# sparc/%.o: sparc/%.c
# 	$(CC) -mcpu=ultrasparc -mtune=ultrasparc $(CFLAGS) -c -o $@ $<
# endif

# cavsdata.h
noinst_HEADERS = \
	ac3.h \
	ac3tab.h \
	avcodec.h \
	bitstream.h \
	cabac.h \
	config.h \
	cookdata.h \
	dsputil.h \
	dvdata.h \
	faandct.h \
	golomb.h \
	h261data.h \
	h263data.h \
	h264data.h \
	imgconvert_template.h \
	indeo2data.h \
	indeo3data.h \
	internal.h \
	lzo.h \
	mpeg12data.h \
	mpeg4data.h \
	mpegaudio.h \
	mpegaudiodectab.h \
	mpegaudiotab.h \
	mpegvideo.h \
	msmpeg4data.h \
	opt.h \
	qdm2data.h \
	ra144.h \
	ra288.h \
	rangecoder.h \
	rtjpeg.h \
	simple_idct.h \
	sp5x.h \
	svq1_cb.h \
	svq1_vlc.h \
	swscale.h \
	truemotion1data.h \
	truespeech_data.h \
	ulti_cb.h \
	vc1acdata.h \
	vc1data.h \
	vorbis.h \
	vp3data.h \
	wmadata.h

# a52dec.c ac3dec.c ac3enc.c amr.c apiexample.c
# beosthread.c
# cavs.c cavsdsp.c
# dct-test.c dtsdec.c
# faac.c faad.c fft-test.c
# libgsm.c
# mdec.c motion_est_template.c motion_test.c mp3lameaudio.c
# svq3.c
# w32thread.c
# wmv2.c
# x264.c xvidff.c
# vc9.c
libavcodec_la_SOURCES = \
	4xm.c \
	8bps.c \
	aasc.c \
	adpcm.c \
	adx.c \
	alac.c \
	allcodecs.c \
	asv1.c \
	avs.c \
	bitstream.c \
	bitstream_filter.c \
	bmp.c \
	cabac.c \
	cinepak.c \
	cljr.c \
	cscd.c \
	cook.c \
	cyuv.c \
	dpcm.c \
	dsputil.c \
	dv.c \
	dvbsub.c \
	dvbsubdec.c \
	dvdsub.c \
	dvdsubenc.c \
	error_resilience.c \
	eval.c \
	faandct.c \
	fdctref.c \
	fft.c \
	ffv1.c \
	flac.c \
	flashsv.c \
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
	interplayvideo.c \
	jfdctfst.c \
	jfdctint.c \
	jrevdct.c \
	kmvc.c \
	lcl.c \
	loco.c \
	lzo.c \
	mace.c \
	mdct.c \
	mjpeg.c \
	mmvideo.c \
	motion_est.c \
	mpeg12.c \
	mpegaudio.c \
	mpegaudiodec.c \
	mpegvideo.c \
	msmpeg4.c \
	msrle.c \
	msvideo1.c \
	nuv.c \
	oggvorbis.c \
	opt.c \
	parser.c \
	pcm.c \
	png.c \
	pnm.c \
	pthread.c \
	qdm2.c \
	qdrw.c \
	qpeg.c \
	qtrle.c \
	ra144.c \
	ra288.c \
	rangecoder.c \
	ratecontrol.c \
	raw.c \
	resample.c \
	resample2.c \
	roqvideo.c \
	rpza.c \
	rtjpeg.c \
	rv10.c \
	shorten.c \
	simple_idct.c \
	smacker.c \
	smc.c \
	snow.c \
	sonic.c \
	svq1.c \
	truemotion1.c \
	truemotion2.c \
	truespeech.c \
	tscc.c \
	tta.c \
	ulti.c \
	utils.c \
	vc1.c \
	vc1dsp.c \
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
	zmbv.c \
	$(MMX_SRCS)
