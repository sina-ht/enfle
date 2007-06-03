## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Tue May 29 01:16:08 2007.
# $Id: Makefile.am,v 1.58 2007/06/03 09:58:36 sian Exp $

AUTOMAKE_OPTIONS = nostdinc

if USE_AVCODEC
noinst_LTLIBRARIES = libavcodec.la
endif

AM_CPPFLAGS = -I$(top_srcdir)/lib/avcodec -I$(top_srcdir)/lib/avutil -I$(top_srcdir)/lib -I$(top_srcdir) -I$(top_builddir) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE
EXTRA_DIST = \
	i386/fdct_mmx.c i386/cputest.c \
	i386/dsputil_mmx.c i386/mpegvideo_mmx.c i386/mpegvideo_mmx_template.c \
	i386/idct_mmx.c i386/motion_est_mmx.c \
	i386/simple_idct_mmx.c i386/fft_sse.c \
	i386/dsputil_mmx_avg.h i386/dsputil_mmx_rnd.h i386/mmx.h \
	i386/h264dsp_mmx.c i386/dsputil_h264_template_mmx.c i386/mathops.h \
	i386/vp3dsp_mmx.h \
	i386/vp3dsp_sse2.h \
	i386/dsputil_mmx_qns.h \
	svq3.c mdec.c motion_est_template.c wmv2.c \
	gcc_fixes.h

DEFS = -prefer-non-pic -DHAVE_AV_CONFIG_H

if USE_MMX
DEFS += -DHAVE_MMX -Dasm=__asm -Dvolatile=__volatile
# We want many registers... so you must omit frame pointer.
CFLAGS_POST = -Wno-error -Wno-unused -O3 -ffast-math -fomit-frame-pointer $(CFLAGS_POST_SSE)
#CFLAGS_POST = -Wno-error -Wno-unused -O -fomit-frame-pointer $(CFLAGS_POST_SSE)

MMX_SRCS = \
	i386/cavsdsp_mmx.c \
	i386/fdct_mmx.c i386/cputest.c \
	i386/dsputil_mmx.c i386/mpegvideo_mmx.c \
	i386/idct_mmx.c i386/idct_mmx_xvid.c i386/motion_est_mmx.c \
	i386/simple_idct_mmx.c i386/fft_sse.c \
	i386/vp3dsp_mmx.c i386/vp3dsp_sse2.c \
	i386/snowdsp_mmx.c \
	i386/fft_3dn.c i386/fft_3dn2.c
else
CFLAGS_POST = -Wno-unused
MMX_SRCS =
endif

if USE_SSE
CFLAGS_POST_SSE = -msse
DEFS += -DHAVE_BUILTIN_VECTOR
endif

AM_CFLAGS = $(CFLAGS_POST)

# # armv4l specific stuff
# ifeq ($(TARGET_ARCH_ARMV4L),yes)
# ASM_OBJS += armv4l/jrevdct_arm.o armv4l/simple_idct_arm.o armv4l/dsputil_arm_s.o armv4l/simple_idct_armv6.S
# OBJS += armv4l/dsputil_arm.o armv4l/mpegvideo_arm.o
# ifeq ($(TARGET_IWMMXT),yes)
# OBJS += armv4l/dsputil_iwmmxt.o armv4l/mpegvideo_iwmmxt.o
# endif
## OBJS += libavcodec/armv4l/mpegvideo_armv5te.c
# endif

# # sun mediaLib specific stuff
# # currently only works when libavcodec is used in mplayer
# ifeq ($(HAVE_MLIB),yes)
# OBJS += mlib/dsputil_mlib.o
# AM_CFLAGS = $(MLIB_INC)
# endif

# # Intel IPP specific stuff
# # currently only works when libavcodec is used in mplayer
# ifeq ($(HAVE_IPP),yes)
# AM_CFLAGS = $(IPP_INC)
# endif

# # alpha specific stuff
# ifeq ($(TARGET_ARCH_ALPHA),yes)
# OBJS += alpha/dsputil_alpha.o alpha/mpegvideo_alpha.o \
# 	alpha/simple_idct_alpha.o alpha/motion_est_alpha.o
# ASM_OBJS += alpha/dsputil_alpha_asm.o alpha/motion_est_mvi_asm.o
# AM_CFLAGS = -fforce-addr
# endif

# ifeq ($(TARGET_ARCH_POWERPC),yes)
# OBJS += ppc/dsputil_ppc.o ppc/mpegvideo_ppc.o ppc/int_altivec.o
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

noinst_HEADERS = \
	aac_ac3_parser.h \
	ac3.h \
	ac3_parser.h \
	ac3tab.h \
	atrac3data.h \
	avcodec.h \
	bethsoftvideo.h \
	bitstream.h \
	bmp.h \
	bytestream.h \
	cabac.h \
	cavs.h \
	cavsdata.h \
	config.h \
	cookdata.h \
	dca.h \
	dcadata.h \
	dcahuff.h \
	dnxhddata.h \
	dsputil.h \
	dvdata.h \
	eval.h \
	faandct.h \
	golomb.h \
	h261.h \
	h261data.h \
	h263_parser.h \
	h263data.h \
	h264.h \
	h264_parser.h \
	h264data.h \
	imcdata.h \
	imgconvert_template.h \
	indeo2data.h \
	indeo3data.h \
	jpegls.h \
	jpeglsdec.h \
	lzw.h \
	mathops.h \
	mjpeg.h \
	mjpegdec.h \
	mjpegenc.h \
	mpcdata.h \
	mpeg12data.h \
	mpeg4data.h \
	mpeg4video_parser.h \
	mpegaudio.h \
	mpegaudiodata.h \
	mpegaudiodecheader.h \
	mpegaudiodectab.h \
	mpegaudiotab.h \
	mpegvideo.h \
	msmpeg4.h \
	msmpeg4data.h \
	msmpeg4tab.h \
	opt.h \
	parser.h \
	pnm.h \
	qdm2data.h \
	ra144.h \
	ra288.h \
	rangecoder.h \
	ratecontrol.h \
	rl.h \
	rle.h \
	rtjpeg.h \
	s3tc.h \
	sgi.h \
	simple_idct.h \
	snow.h \
	sp5x.h \
	svq1_cb.h \
	svq1_vlc.h \
	tiff.h \
	truemotion1data.h \
	truespeech_data.h \
	ulti_cb.h \
	vc1.h \
	vc1acdata.h \
	vc1data.h \
	vorbis.h \
	vorbis_enc_data.h \
	vp3data.h \
	vp5data.h \
	vp56.h \
	vp56data.h \
	vp6data.h \
	xiph.h \
	xvid_internal.h \
	xvmc_render.h \
	wma.h \
	wmadata.h

# a52dec.c ac3enc.c amr.c apiexample.c
# dct-test.c dtsdec.c
# faac.c faad.c fft-test.c libgsm.c
# mdec.c motion_est_template.c svq3.c wmv2.c
# mp3lameaudio.c
# beosthread.c w32thread.c, motion-test.c
# imgresample.c needs libswscale
# Encoders: bmpenc.c wmaenc.c x264.c and xvidff.c flacenc.c libtheoraenc.c zmbvenc.c flashsvenc.c targaenc.c sgienc.c lzwenc.c tiffenc.c roqaudioenc.c mjpegenc.c jpeglsenc.c pnmenc.c mpegaudioenc.c, ljpegenc.c, h261enc.c
# BSFs: remove_extradata_bsf.c, dump_extradata_bsf.c, noise_bsf.c, mp3_header_compress_bsf.c, imx_dump_header_bsf.c, mjpega_dump_header_bsf.c, mp3_header_decompress_bsf.c

libavcodec_la_SOURCES = \
	4xm.c \
	8bps.c \
	aasc.c \
	aac_parser.c \
	aac_ac3_parser.c \
	ac3.c \
	ac3_parser.c \
	ac3tab.c \
	adpcm.c \
	adx.c \
	alac.c \
	allcodecs.c \
	asv1.c \
	atrac3.c \
	audioconvert.c \
	avs.c \
	bethsoftvideo.c \
	bitstream.c \
	bitstream_filter.c \
	bmp.c \
	c93.c \
	cabac.c \
	cavs.c \
	cavs_parser.c \
	cavsdsp.c \
	cinepak.c \
	cljr.c \
	cscd.c \
	cook.c \
	cyuv.c \
	dca.c \
	dca_parser.c \
	dnxhddec.c \
	dpcm.c \
	dsicinav.c \
	dsputil.c \
	dv.c \
	dvbsub.c \
	dvbsub_parser.c \
	dvbsubdec.c \
	dvdsub_parser.c \
	dvdsubdec.c \
	dvdsubenc.c \
	dxa.c \
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
	gif.c \
	gifdec.c \
	golomb.c \
	h261.c \
	h261_parser.c \
	h261dec.c \
	h263.c \
	h263_parser.c \
	h263dec.c \
	h264.c \
	h264_parser.c \
	h264dsp.c \
	h264enc.c \
	h264idct.c \
	huffyuv.c \
	idcinvideo.c \
	imc.c \
	imgconvert.c \
	indeo2.c \
	indeo3.c \
	interplayvideo.c \
	jfdctfst.c \
	jfdctint.c \
	jpegls.c \
	jpeglsdec.c \
	jrevdct.c \
	kmvc.c \
	lcl.c \
	loco.c \
	lzw.c \
	mace.c \
	mdct.c \
	mjpeg.c \
	mjpegbdec.c \
	mjpegdec.c \
	mjpeg_parser.c \
	mmvideo.c \
	motion_est.c \
	mpc.c \
	mpeg12.c \
	mpeg4video_parser.c \
	mpegaudio.c \
	mpegaudio_parser.c \
	mpegaudiodata.c \
	mpegaudiodec.c \
	mpegaudiodecheader.c \
	mpegvideo.c \
	mpegvideo_parser.c \
	msmpeg4.c \
	msmpeg4data.c \
	msrle.c \
	msvideo1.c \
	nuv.c \
	oggvorbis.c \
	opt.c \
	parser.c \
	pcm.c \
	png.c \
	pnm.c \
	pnm_parser.c \
	pthread.c \
	ptx.c \
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
	rle.c \
	roqvideo.c \
	rpza.c \
	rtjpeg.c \
	rv10.c \
	s3tc.c \
	sgidec.c \
	shorten.c \
	simple_idct.c \
	smacker.c \
	smc.c \
	snow.c \
	sonic.c \
	sp5xdec.c \
	svq1.c \
	targa.c \
	tiertexseqv.c \
	tiff.c \
	truemotion1.c \
	truemotion2.c \
	truespeech.c \
	tscc.c \
	tta.c \
	txd.c \
	ulti.c \
	utils.c \
	vc1.c \
	vc1_parser.c \
	vc1data.c \
	vc1dsp.c \
	vcr1.c \
	vmdav.c \
	vmnc.c \
	vorbis.c \
	vorbis_data.c \
	vorbis_dec.c \
	vorbis_enc.c \
	vp3.c \
	vp3dsp.c \
	vp5.c \
	vp56.c \
	vp56data.c \
	vp6.c \
	vqavideo.c \
	wavpack.c \
	wma.c \
	wmadec.c \
	wnv1.c \
	ws-snd1.c \
	xan.c \
	xiph.c \
	xl.c \
	xvmcvideo.c \
	zmbv.c \
	$(MMX_SRCS)
