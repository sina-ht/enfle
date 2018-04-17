dnl Process this file with autoconf to produce a configure script.
dnl NOTE: autoconf must be 2.64 or later, automake should be 1.8.2 or later.
dnl
dnl (C)Copyright 1999-2018 by Hiroshi Takekawa
dnl This file is part of Enfle.
dnl
dnl Last Modified: Tue Apr 17 20:56:54 2018.
dnl
dnl SPDX-License-Identifier: GPL-2.0-only

AC_PREREQ(2.64)
AC_INIT([enfle],[20180407],[sian@big.or.jp])
AC_CONFIG_SRCDIR(src/enfle.c)
AC_CONFIG_HEADERS([enfle-config.h])
AM_INIT_AUTOMAKE(silent-rules)
AC_CANONICAL_HOST

dnl Maintainer mode (could be uncommented for production release.)
dnl AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_LN_S
dnl AC_PROG_RANLIB
AC_PROG_GCC_TRADITIONAL
AM_PROG_AS
AM_ICONV

dnl libtool
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(no)
_LT_SET_OPTION([LT_INIT],[dlopen])
AC_DIAGNOSE([obsolete],[AC_LIBTOOL_DLOPEN: Remove this warning and the call to _LT_SET_OPTION when you
put the `dlopen' option into LT_INIT's first parameter.])

LT_INIT

dnl for libmpeg2 and libavcodec
enable_x86=0
enable_ppc=0
enable_sparc=0
enable_alpha=0
enable_armv4l=0
enable_mlib=0
enable_mmi=0
enable_sh4=0
enable_bfin=0
lib_postfix=
case "$host_cpu" in
  i*86 )
    AC_DEFINE(ARCH_X86, 1, [ Define if you have X86 architecture. ])
    AC_DEFINE(ARCH_X86_32, 1, [ Define if you have X86 32bits architecture. ])
    arch_x86=yes
    ;;
  x86_64* )
    dnl AC_DEFINE(ARCH_X86, 1, [ Define if you have X86 architecture. ])
    dnl AC_DEFINE(ARCH_X86_64, 1, [ Define if you have X86 64bits architecture. ])
    dnl arch_x86_64=yes
    lib_postfix=64
    ;;
  powerpc* )
    AC_DEFINE(ARCH_PPC, 1, [ Define if you have PowerPC architecture. ])
    arch_ppc=yes
    enable_ppc=1
    ;;
  sparc* )
    AC_DEFINE(ARCH_SPARC, 1, [ Define if you have SPARC architecture. ])
    arch_sparc=yes
    enable_sparc=1
    ;;
  alpha* )
    AC_DEFINE(ARCH_ALPHA, 1, [ Define if you have ALPHA architecture. ])
    arch_alpha=yes
    enable_alpha=1
    ;;
  *)
    ;;
esac
AC_DEFINE_UNQUOTED(ENABLE_ARMV4L, $enable_armv4l, [ Define if you want to enable ARMV4L. ])
AC_DEFINE_UNQUOTED(ENABLE_MLIB, $enable_mlib, [ Define if you want to enable MLIB. ])
AC_DEFINE_UNQUOTED(ENABLE_SPARC, $enable_sparc, [ Define if you want to enable SPARC. ])
AC_DEFINE_UNQUOTED(ENABLE_ALPHA, $enable_alpha, [ Define if you want to enable ALPHA. ])
AC_DEFINE_UNQUOTED(ENABLE_POWERPC, $enable_ppc, [ Define if you want to enable POWERPC. ])
AC_DEFINE_UNQUOTED(ENABLE_MMI, $enable_mmi, [ Define if you want to enable MMI. ])
AC_DEFINE_UNQUOTED(ENABLE_SH4, $enable_sh4, [ Define if you want to enable SH4. ])
AC_DEFINE_UNQUOTED(ENABLE_BFIN, $enable_bfin, [ Define if you want to enable BFIN. ])

AM_CONDITIONAL(ARCH_X86, test "$arch_x86" = "yes")
AM_CONDITIONAL(ARCH_PPC, test "$arch_ppc" = "yes")
AM_CONDITIONAL(ARCH_SPARC, test "$arch_sparc" = "yes")
AM_CONDITIONAL(ARCH_ALPHA, test "$arch_alpha" = "yes")

dnl Checks for libraries.
if test -d /usr/local/lib$lib_postfix; then
  LDFLAGS="$LDFLAGS -L/usr/local/lib$lib_postfix"
  if test "${CC}" = "icc"; then
      CPPFLAGS="$CPPFLAGS -I/usr/local/include"
  else
    AC_MSG_CHECKING(if /usr/local/include has already been specified)
    if echo | cpp -v 2>&1 | grep "/usr/local/include" > /dev/null; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no, added)
      CPPFLAGS="$CPPFLAGS -I/usr/local/include"
    fi
  fi
fi

dnl plugin enablers

Plugins_dirs="loader saver video audio ui player effect archiver streamer audiodecoder videodecoder demultiplexer"
Plugins=""
dnl recognize available plugins
echo -n "checking for available plugins... "
search_topdir="$srcdir/plugins"
for kind in $Plugins_dirs; do
  search_dir="$search_topdir/$kind"
  for plugin_dir in $search_dir/*; do
    plugin=`basename $plugin_dir`
    makefilein="$plugin_dir/Makefile.in"
    if test -d "$plugin_dir" -a -f $makefilein ; then
      if test "${plugin}" != "ungif" -a "${plugin}" != "gif"; then
        ENFLE_APPEND(Plugins, "${kind}/${plugin}")
      fi
      eval "exists_${kind}_${plugin}=yes"
    fi
  done
done
echo "$Plugins"

dnl checks

available_dlopen=no
AC_CHECK_LIB(dl, dlopen, [ available_dlopen=yes; LIBADD_DLOPEN=-ldl; ], [
  AC_CHECK_LIB(c, dlopen, [ available_dlopen=yes; ]) ])
AM_CONDITIONAL(HAVE_DLOPEN, test "$available_dlopen" = "yes")
AC_SUBST(LIBADD_DLOPEN)

available_pow=no
AC_CHECK_HEADER(math.h, [
  AC_CHECK_LIB(m, pow, [ available_pow=yes; LIBADD_POW=-lm; ]) ])
available_pow=yes
LIBADD_POW=-lm

available_jpeg=no
AC_CHECK_HEADER(jpeglib.h, [
    AC_CHECK_LIB(jpeg, jpeg_finish_decompress, [
      available_jpeg=yes; LIBADD_JPEG=-ljpeg;
    ], [], $LIBADD_POW)
])
if test "$available_jpeg" != "yes"; then
  ENFLE_PLUGIN_DISABLE(loader/jpeg); ENFLE_PLUGIN_DISABLE(saver/jpeg);
fi
AM_CONDITIONAL(HAVE_JPEG, test "$available_jpeg" = "yes")
AC_SUBST(LIBADD_JPEG)

available_jasper=no
AC_CHECK_HEADER(jasper/jasper.h, [
    AC_CHECK_LIB(jasper, jas_init, [
      available_jasper=yes; LIBADD_JASPER=-ljasper;
    ], [], [-ljpeg])
])
if test "$available_jasper" != "yes"; then
  ENFLE_PLUGIN_DISABLE(loader/jasper);
fi
AM_CONDITIONAL(HAVE_JASPER, test "$available_jasper" = "yes")
AC_SUBST(LIBADD_JASPER)

available_mpeg_lib=no
AC_CHECK_HEADER(mpeg.h, [
  AC_CHECK_LIB(mpeg, OpenMPEG, [
    AC_CHECK_LIB(mpeg, OpenMPEGStream, [
      available_mpeg_lib=yes; LIBADD_MPEG_LIB=-lmpeg
    ], [
      AC_MSG_WARN([*** mpeg_lib requires a patch.]);
    ])
  ], [])
])
if test "$available_mpeg_lib" != "yes"; then
  ENFLE_PLUGIN_DISABLE(player/mpeg_lib)
fi
AM_CONDITIONAL(HAVE_MPEG_LIB, test "$available_mpeg_lib" = "yes")
AC_SUBST(LIBADD_MPEG_LIB)

available_pthread=no
AC_CHECK_HEADER(pthread.h, [
  AC_CHECK_LIB(pthread, pthread_create, [ available_pthread=yes; LIBADD_PTHREAD=-lpthread; ], [
    AC_CHECK_LIB(c_r, pthread_create, [ available_pthread=yes; LIBADD_PTHREAD=-lc_r; ], [
      AC_CHECK_FUNC(pthread_create, [ available_pthread=yes; ]) ]) ]) ])
if test "$available_pthread" = "yes"; then
  AC_DEFINE(USE_PTHREAD,,[ Define if you have libpthread ])
  dnl for avcodec
  AC_DEFINE(HAVE_PTHREAD,,[ Define if you have libpthread ])
  AC_DEFINE(HAVE_THREADS,,[ Define if you have libpthread ])
  AC_DEFINE(ENABLE_THREADS,1,[ Define if you have libpthread ])
  CFLAGS="$CFLAGS -pthread -D_REENTRANT"
else
  AC_DEFINE(ENABLE_THREADS,0,[ Define if you have libpthread ])
fi
AM_CONDITIONAL(HAVE_PTHREAD, test "$available_pthread" = "yes")
AC_SUBST(LIBADD_PTHREAD)

available_libmpeg3=no
dnl Some libmpeg3 needs -lm
if test "$available_pthread" = "yes"; then
  AC_CHECK_HEADER(mpeg3/libmpeg3.h, [
    AC_CHECK_LIB(mpeg3, mpeg3_open, [
      available_libmpeg3=yes; LIBADD_LIBMPEG3="$LIBADD_POW -lmpeg3 $LIBADD_PTHREAD";
    ], [], [$LIBADD_POW $LIBADD_PTHREAD])
  ], [
    AC_CHECK_HEADER(libmpeg3.h, [
      AC_MSG_WARN([*** please put libmpeg3 headers in (/usr/local/)include/mpeg3/.]);
    ])
  ])
fi
if test "$available_libmpeg3" != "yes"; then
  ENFLE_PLUGIN_DISABLE(player/libmpeg3)
fi
AM_CONDITIONAL(HAVE_LIBMPEG3, test "$available_libmpeg3" = "yes")
AC_SUBST(LIBADD_LIBMPEG3)

available_ogg=no
AC_CHECK_HEADER(ogg/ogg.h, [
  AC_CHECK_LIB(ogg, ogg_sync_init, [ available_ogg=yes; LIBADD_OGG=-logg; ]) ])
AM_CONDITIONAL(HAVE_OGG, test "$available_ogg" = "yes")
AC_SUBST(LIBADD_OGG)

available_vorbis=no
AC_CHECK_HEADER(vorbis/codec.h, [
  AC_CHECK_LIB(vorbis, vorbis_info_init, [
   available_vorbis=yes;
   LIBADD_VORBIS="-lvorbis -lvorbisenc";
   AC_DEFINE(CONFIG_LIBVORBIS, 1, [Define if you need this for avcodec.])
  ]) ])
AM_CONDITIONAL(HAVE_VORBIS, test "$available_vorbis" = "yes")
AC_SUBST(LIBADD_VORBIS)

LIBS_BACKUP=$LIBS
LIBS="$LIBS $LIBADD_OGG"
available_theora=no
AC_CHECK_HEADER(theora/theora.h, [
  AC_CHECK_LIB(theora, theora_info_init, [
   available_theora=yes;
   LIBADD_THEORA=-ltheora;
   AC_DEFINE(HAVE_LIBTHEORA, 1, [Define if you have libtheora.])
  ]) ])
AM_CONDITIONAL(HAVE_THEORA, test "$available_theora" = "yes")
AC_SUBST(LIBADD_THEORA)
LIBS=$LIBS_BACKUP

available_zlib=no
AC_CHECK_HEADER(zlib.h, [
  AC_CHECK_LIB(z, gzseek, [
   available_zlib=yes;
   LIBADD_ZLIB=-lz;
   AC_DEFINE(CONFIG_ZLIB, 1, [Define if you need this for avcodec.])
   AC_DEFINE(CONFIG_ZLIB_DECODER, 1, [Define if you need this for avcodec.])
  ]) ])
AM_CONDITIONAL(HAVE_ZLIB, test "$available_zlib" = "yes")
AC_SUBST(LIBADD_ZLIB)

available_png=no
if test "$available_zlib" = "yes"; then
  LIBADD_PNG=$LIBADD_ZLIB
  AC_CHECK_HEADER(libpng/png.h, [
   AC_DEFINE(HAVE_LIBPNG_PNG_H, 1, [ Define if you have the <libpng/png.h> header file. ])
   png_header_ok=yes
  ], [ png_header_ok=no ])
  if test "$png_header_ok" = yes; then
    AC_CHECK_LIB(png, png_get_IHDR, [
      available_png=yes; LIBADD_PNG="$LIBADD_POW -lpng $LIBADD_PNG";
      AC_DEFINE(CONFIG_PNG_DECODER, 1, [Define if you need this for avcodec.])
    ], [], [$LIBADD_POW $LIBADD_ZLIB])
  fi
fi
if test "$available_png" != "yes"; then
  ENFLE_PLUGIN_DISABLE(loader/png); ENFLE_PLUGIN_DISABLE(saver/png);
fi
AM_CONDITIONAL(HAVE_PNG, test "$available_png" = "yes")
AC_SUBST(LIBADD_PNG)

available_mng=no
if test "$available_zlib" = "yes" -a "$available_jpeg" = "yes"; then
  LIBADD_MNG="-lz -ljpeg -lm"
  AC_CHECK_HEADER(libmng.h, [
    AC_CHECK_LIB(mng, mng_initialize, [ available_mng=yes; LIBADD_MNG="$LIBADD_MNG -lmng"; ], [], $LIBADD_MNG )])
else
  AC_MSG_WARN([*** mng requires libz and libjpeg.])
fi
if test "$available_mng" != "yes"; then
  ENFLE_PLUGIN_DISABLE(player/mng)
fi
AM_CONDITIONAL(HAVE_MNG, test "$available_mng" = "yes")
AC_SUBST(LIBADD_MNG)

available_avifile=no
AC_ARG_WITH(avifile,
        [  --with-avifile=path     use avifile (default: no).],
	[ AC_MSG_WARN([*** avifile is marked as obsolete, probably not working.]);
          with_avifile=$withval ],
     	[ with_avifile=no ])
if test "$with_avifile" = "yes"; then
  for i in /usr/local/include /usr/include; do
    if test -d "$i/avifile"; then
      INCLUDEADD_AVIFILE="-I$i/avifile -I$i/avifile/wine"
    fi
  done
else
  if test -d "$with_avifile"; then
    INCLUDEADD_AVIFILE="-I$with_avifile -I$with_avifile/wine"
  fi
fi

CPPFLAGS_SAVE=$CPPFLAGS
CPPFLAGS="$INCLUDEADD_AVIFILE $CPPFLAGS"
AC_LANG([C++])
AC_PROG_CXX
AC_CHECK_HEADER(avifile.h, [ available_avifile=yes; LIBADD_AVIFILE="-laviplay" ])
LIBADD_STDCC=-lstdc++
AC_LANG([C])
if test "$available_avifile" != "yes"; then
  ENFLE_PLUGIN_DISABLE(player/avifile)
fi
AM_CONDITIONAL(HAVE_AVIFILE, test "$available_avifile" = "yes")
AC_SUBST(LIBADD_AVIFILE)
AC_SUBST(LIBADD_STDCC)
AC_SUBST(INCLUDEADD_AVIFILE)
CPPFLAGS=$CPPFLAGS_SAVE

available_bzlib=no
AC_CHECK_HEADER(bzlib.h, [
  AC_CHECK_LIB(bz2, BZ2_bzlibVersion, [
    available_bzlib=yes; LIBADD_BZLIB="-lbz2"; AC_DEFINE(BZAPI_NEEDS_PREFIX, 1, [Define if your bzlib needs BZ2_ prefix.])
  ], [
    AC_CHECK_LIB(bz2, bzlibVersion, [
      available_bzlib=yes; LIBADD_BZLIB="-lbz2";
    ])
  ])
])
if test "$available_bzlib" != "yes"; then
  ENFLE_PLUGIN_DISABLE(streamer/bz2)
fi
AM_CONDITIONAL(HAVE_BZLIB, test "$available_bzlib" = "yes")
AC_SUBST(LIBADD_BZLIB)

available_liblzma=no
AC_CHECK_HEADER(lzma.h, [
  AC_CHECK_LIB(lzma, lzma_stream_decoder, [
    available_liblzma=yes; LIBADD_XZLIB="-llzma" 
  ])
])
if test "$available_liblzma" != "yes"; then
  ENFLE_PLUGIN_DISABLE(streamer/xz)
fi
AM_CONDITIONAL(HAVE_XZLIB, test "$available_liblzma" = "yes")
AC_SUBST(LIBADD_XZLIB)

available_libarc=no
AC_CHECK_HEADER(libarc/arc.h, [
  AC_CHECK_LIB(arc, url_arc_open, [
    available_libarc=yes; LIBADD_LIBARC="-larc";
  ])
])
if test "$available_libarc" != "yes"; then
  ENFLE_PLUGIN_DISABLE(archiver/arc)
fi
AM_CONDITIONAL(HAVE_LIBARC, test "$available_libarc" = "yes")
AC_SUBST(LIBADD_LIBARC)

available_oss=no
AC_CHECK_HEADER(sys/soundcard.h, [
  AC_DEFINE(HAVE_SYS_SOUNDCARD_H, 1, [ Define if you have the <sys/soundcard.h> header file. ])
], [
  AC_CHECK_HEADER(machine/soundcard.h, [
    AC_DEFINE(HAVE_MACHINE_SOUNDCARD_H, 1, [ Define if you have the <machine/soundcard.h> header file. ])
  ], [
    AC_CHECK_HEADER(soundcard.h, [
      AC_DEFINE(HAVE_SOUNDCARD_H, 1, [ Define if you have the <soundcard.h> header file. ])
    ])
  ])
])
if test "${ac_cv_header_soundcard_h}" = "yes" -o \
        "${ac_cv_header_sys_soundcard_h}" = "yes" -o \
        "${ac_cv_header_machine_soundcard_h}" = "yes"; then
  available_oss=yes
fi
if test "$available_oss" != "yes"; then
  ENFLE_PLUGIN_DISABLE(audio/oss)
fi
AM_CONDITIONAL(HAVE_OSS, test "$available_oss" = "yes")

available_alsa=no
AC_CHECK_HEADER(alsa/asoundlib.h, [
  AC_CHECK_LIB(asound, snd_pcm_hw_params, [ available_alsa=yes; LIBADD_ALSA="-lasound" ], [], [-lm])])
if test "$available_alsa" != "yes"; then
  ENFLE_PLUGIN_DISABLE(audio/alsa)
fi
AM_CONDITIONAL(HAVE_ALSA, test "$available_alsa" = "yes")
AC_SUBST(LIBADD_ALSA)

available_esd=no
AC_CHECK_HEADER(esd.h,
  AC_CHECK_LIB(esd, esd_play_stream_fallback, [ available_esd=yes; LIBADD_ESD="-lesd" ], [], [-lm]))
if test "$available_esd" != "yes"; then
  ENFLE_PLUGIN_DISABLE(audio/esd)
fi
AM_CONDITIONAL(HAVE_ESD, test "$available_esd" = "yes")
AC_SUBST(LIBADD_ESD)

dnl AC_ARG_ENABLE(nasm,
dnl         [  --enable-nasm           use x86 assembler.])
dnl use_nasm=no
dnl if test "${enable_nasm-set}" = set; then
dnl   enable_nasm=no
dnl elif test "$enable_nasm" = "yes"; then
dnl   AC_PATH_PROG(nasm_path, nasm)
dnl   if test "$nasm_path" != ""; then
dnl     AC_DEFINE(WITH_NASM, 1, [Define if you want to use nasm.])
dnl     use_nasm=yes
dnl   else
dnl     AC_MSG_WARN([*** asm requested but the assembler nasm not found])
dnl   fi
dnl fi
dnl AM_CONDITIONAL(USE_NASM, test "$use_nasm" = "yes")

AC_ARG_ENABLE(mmx,
       [  --enable-mmx            Include MMX code(default yes for x86).])
if test "${enable_mmx-set}" = set; then
  case "$host_cpu" in
  i*86 )
    enable_mmx=yes ;;
  *)
    enable_mmx=no ;;
  esac
fi

if test "$enable_mmx" = "yes"; then
  AC_DEFINE(USE_MMX, 1, [ Define if you have and use MMX. ])
  AC_DEFINE(ENABLE_MMX, 1, [ Define if you have and use MMX. ])
else
  AC_DEFINE(USE_MMX, 0, [ Define if you have and use MMX. ])
  AC_DEFINE(ENABLE_MMX, 0, [ Define if you have and use MMX. ])
fi
AM_CONDITIONAL(USE_MMX, test "$enable_mmx" = "yes")

AC_ARG_ENABLE(mmx-bgra,
       [  --enable-mmx-bgra       Include MMX bgra32to16 code(default no, deprecated).])
if test "${enable_mmx_bgra-set}" = set; then
  enable_mmx_bgra=no
fi

test "$enable_mmx_bgra" = "yes" && AC_DEFINE(USE_MMX_BGRA, 1, [ Define if you use deprecated MMX bgra32to16 code. ])
AM_CONDITIONAL(USE_MMX_BGRA, test "$enable_mmx_bgra" = "yes")

AC_ARG_ENABLE(sse,
       [  --enable-sse            Include SSE code(default yes for x86).])
if test "${enable_sse-set}" = set; then
  case "$host_cpu" in
  i*86 )
    enable_sse=yes ;;
  *)
    enable_sse=no ;;
  esac
fi

test "$enable_sse" = "yes" && AC_DEFINE(USE_SSE, 1, [ Define if you have and use SSE. ])
AM_CONDITIONAL(USE_SSE, test "$enable_sse" = "yes")

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_STAT
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([byteswap.h errno.h fcntl.h float.h inttypes.h limits.h locale.h malloc.h memory.h stddef.h stdint.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/mount.h sys/param.h sys/socket.h sys/time.h unistd.h utime.h])

dnl Checks for X libraries.
AC_PATH_XTRA
# last resort
x_includes=${x_includes:-/usr/X11R6/include}
X_LIBS=${X_LIBS:--L/usr/X11R6/lib}
CPPFLAGS="$CPPFLAGS -I$x_includes"
LDFLAGS="$LDFLAGS $X_LIBS"

LIBS_BACKUP=$LIBS

LIBS="$X_PRELIBS $LIBS $LIBADD_DLOPEN"
AC_CHECK_LIB(X11, XOpenDisplay)
AM_CONDITIONAL(HAVE_X11, test "$have_x" = "yes")

dnl Checks for libraries which require X library.
available_shm=no
AC_CHECK_HEADER(sys/ipc.h, [
  AC_CHECK_HEADER(sys/shm.h, [
    AC_CHECK_HEADER(X11/extensions/XShm.h, [
      AC_CHECK_LIB(Xext, XShmCreateImage, [ available_shm=yes; LIBADD_SHM="-lXext"; ]) ], [], [#include <X11/Xlib.h>]) ]) ])
dnl AM_CONDITIONAL(HAVE_SHM, test "$available_shm" = "yes")
test "$available_shm" = "yes" && AC_DEFINE(USE_SHM,,[ Define if you have the MIT Shared Memory extension ])
AC_SUBST(LIBADD_SHM)

available_xv=no
AC_CHECK_HEADER(X11/extensions/Xvlib.h, [
  AC_CHECK_LIB(Xv, XvQueryExtension, [ available_xv=yes; LIBADD_XV="-lXext -lXv"; ],, -lXext) ], [], [#include <X11/Xlib.h>])
dnl AM_CONDITIONAL(HAVE_XV, test "$available_xv" = "yes")
test "$available_xv" = "yes" && AC_DEFINE(USE_XV,,[ Define if you have the Xv extension ])
AC_SUBST(LIBADD_XV)

dnl available_xvmc=no
dnl AC_CHECK_HEADER(X11/extensions/XvMClib.h, [
dnl   AC_CHECK_LIB(XvMC, XvMCQueryExtension, [ available_xvmc=yes; LIBADD_XVMC="-lXvMC"; ],, -lXext) ], [], [#include <X11/Xlib.h>])
dnl AM_CONDITIONAL(HAVE_XVMC, test "$available_xvmc" = "yes")
dnl test "$available_xvmc" = "yes" && AC_DEFINE(HAVE_XVMC,,[ Define if you have the XvMC extension ])
dnl AC_SUBST(LIBADD_XVMC)

available_ungif=no
AC_ARG_WITH(ungif,
        [  --with-ungif            build ungif plugin ])
if test "${with_ungif-set}" = "set"; then
  with_ungif=no
elif test "$with_ungif" = "yes"; then
  AC_CHECK_HEADER(gif_lib.h,
    AC_CHECK_LIB(ungif, DGifOpen, [ available_ungif=yes; LIBADD_UNGIF=-lungif; ]))
  AC_SUBST(LIBADD_UNGIF)
fi
if test "$available_ungif" != "yes"; then
  ENFLE_PLUGIN_DISABLE(loader/ungif); ENFLE_PLUGIN_DISABLE(player/ungif);
fi
AM_CONDITIONAL(HAVE_UNGIF, test "$available_ungif" = "yes")

available_gif=yes
AC_ARG_ENABLE(gif,
        [  --disable-gif           don't build gif plugin ])
if test "${enable_gif-set}" = "no"; then
  available_gif=no
fi
if test "$available_gif" != "yes"; then
  ENFLE_PLUGIN_DISABLE(loader/gif)
fi
AM_CONDITIONAL(HAVE_GIF, test "$available_gif" = "yes")

available_asf=no
AC_ARG_ENABLE(asf,
        [  --enable-asf            build asf plugin ])
if test "${enable_asf-set}" = "set"; then
  enable_asf=no
elif test "$enable_asf" = "yes"; then
  if test -f "$srcdir/plugins/demultiplexer/asf/asf.c"; then
    available_asf=yes
  else
    echo "*** ASF plugin source is not included in this distribution"
  fi
fi
if test "$available_asf" != "yes"; then
  ENFLE_PLUGIN_DISABLE(demultiplexer/asf)
fi
AM_CONDITIONAL(HAVE_ASF, test "$available_asf" = "yes")

available_webp=no
AC_CHECK_HEADER(webp/decode.h, [
    AC_CHECK_LIB(webp, WebPDecodeRGB, [
      available_webp=yes; LIBADD_WEBP=-lwebp;
    ], [], [])
])
if test "$available_webp" != "yes"; then
  ENFLE_PLUGIN_DISABLE(loader/webp);
fi
AM_CONDITIONAL(HAVE_WEBP, test "$available_webp" = "yes")
AC_SUBST(LIBADD_WEBP)

LIBS=$LIBS_BACKUP

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
AC_C_CONST
AC_C_RESTRICT
AC_C_VOLATILE
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_HEADER_TIME
AC_STRUCT_TM
ENFLE_MAX_ALIGN

dnl Checks for library functions.
AC_REPLACE_FNMATCH
AC_FUNC_CHOWN
dnl AC_FUNC_MALLOC
dnl AC_FUNC_REALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STRTOD
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_CHECK_FUNCS([floor getopt_long getpagesize gettimeofday mblen memalign memmove memset mkstemp munmap pow rint setlocale select sqrt strcasecmp strchr strdup strerror strncasecmp strrchr strstr strtol])

dnl debug option
AC_ARG_ENABLE(debug,
        [  --enable-debug          build a debugging version.])
if test "${enable_debug-set}" = set; then
  enable_debug=no
elif test "$enable_debug" = "yes"; then
  AC_DEFINE(DEBUG,, [Define if you want to debug.])
fi

dnl susie plugin support
AC_ARG_ENABLE(spi,
        [  --enable-spi            susie plugin support(i386 or later required).])
if test "${enable_spi-set}" = "set"; then
  enable_spi=no
elif test "$enable_spi" = yes; then
  AC_DEFINE(USE_SPI, 1, [Define if you want susie plugin support.])
fi

dnl dmo support
AC_ARG_ENABLE(dmo,
        [  --enable-dmo            dmo support(implies spi, i386 or later required).])
if test "${enable_dmo-set}" = "set"; then
  enable_dmo=no
elif test "$enable_dmo" = yes; then
  AC_DEFINE(USE_DMO, 1, [Define if you want dmo support.])
  dnl dmo depends on spi's dll loader
  enable_spi=yes
  AC_DEFINE(USE_SPI, 1, [Define if you want susie plugin support.])
fi

AM_CONDITIONAL(USE_SPI, test "$enable_spi" = "yes")
AM_CONDITIONAL(USE_DMO, test "$enable_dmo" = "yes")

dnl iconv needed for spi support(LoadString or so)
dnl AC_ARG_ENABLE(iconv,
dnl     [  --enable-iconv          Enable iconv support.],
dnl     [  enable_iconv=$enableval ],
dnl     [  enable_iconv=yes ])
dnl iconv_ok=no
dnl AC_CHECK_FUNC(iconv, [ iconv_ok=yes; LIBADD_ICONV= ], [
dnl   AC_CHECK_LIB(iconv, iconv, [ iconv_ok=yes; LIBADD_ICONV=-liconv ], [
dnl     AC_CHECK_LIB(giconv, iconv, [
dnl       iconv_ok=yes;
dnl       LIBADD_ICONV=-lgiconv;
dnl       AC_DEFINE(USE_GICONV,, [Define if you want to use libgiconv instead of libiconv.])
dnl     ])
dnl   ])
dnl ])
dnl AC_SUBST(LIBADD_ICONV)

dnl AC_MSG_CHECKING(if iconv support wanted)
dnl if test "$enable_iconv" = "yes"; then
dnl   if test "$iconv_ok" = "yes"; then
dnl     AC_DEFINE(USE_ICONV,, [Define if you want to use iconv().])
dnl     AC_MSG_RESULT(yes)
dnl   else
dnl     enable_iconv=no
dnl     AC_MSG_RESULT([yes, but lack of iconv functions])
dnl   fi
dnl else
dnl   AC_MSG_RESULT(no)
dnl fi

AC_ARG_ENABLE(avcodec,
        [  --enable-avcodec       enable avcodec plugin.])
if test "${enable_avcodec-set}" = "set"; then
  enable_avcodec=no
fi
if test "$enable_avcodec" = yes; then
save_LIBS=$LIBS
LIBS="$LIBS -lm"
AC_CHECK_FUNC(lrintf, [ available_lrintf=yes ], [])
LIBS=$save_LIBS
test "$available_lrintf" = "yes" && AC_DEFINE(HAVE_LRINTF, 1, [Define if you have lrintf().])

AC_DEFINE(CONFIG_DECODERS, 1, [Define if you need this for avcodec.])
AC_DEFINE(CONFIG_RISKY, 1, [Define if you need this for avcodec.])
AC_DEFINE(ENABLE_SMALL, 0, [Define if you need this for avcodec.])
else
  enable_avcodec=no
  ENFLE_PLUGIN_DISABLE(videodecoder/avcodec)
  ENFLE_PLUGIN_DISABLE(audiodecoder/avcodec)
fi
AM_CONDITIONAL(USE_AVCODEC, test "$enable_avcodec" = "yes")

ENFLE_PLUGIN_DISABLE(player/avcodec)
ENFLE_PLUGIN_DISABLE(saver/glic)

Static_plugin_objs=""
Static_plugin_proto=""
Static_plugin_add=""
AC_ARG_ENABLE(all-static,
    [  --enable-all-static     Link all plugins statically.],
    [ for p in $Plugins; do
        kind=`echo $p | sed 's/\/.*$//'`
        p=`echo $p | sed 's/^.*\///'`
        ENFLE_PLUGIN_ENABLE(Static_plugins, $p, $kind)
      done],
    [ Static_plugins=""])

ENFLE_LINKED_PLUGIN_ENABLER(loader, [        ])
ENFLE_LINKED_PLUGIN_ENABLER(saver, [         ])
ENFLE_LINKED_PLUGIN_ENABLER(video, [         ])
ENFLE_LINKED_PLUGIN_ENABLER(audio, [         ])
ENFLE_LINKED_PLUGIN_ENABLER(ui, [            ])
ENFLE_LINKED_PLUGIN_ENABLER(player, [        ])
ENFLE_LINKED_PLUGIN_ENABLER(effect, [        ])
ENFLE_LINKED_PLUGIN_ENABLER(archiver, [      ])
ENFLE_LINKED_PLUGIN_ENABLER(streamer, [      ])

echo "Dynamic plugins: $Plugins"
echo "Static plugins: $Static_plugins"
echo "Static plugin objs: $Static_plugin_objs"

ENFLE_APPEND(Static_plugin_add, do{}while(0))
AC_SUBST(Static_plugins)
AC_SUBST(Static_plugin_objs)
AC_DEFINE_UNQUOTED(STATIC_PLUGIN_PROTO, $Static_plugin_proto, [Define static plugin prototypes])
AC_DEFINE_UNQUOTED(STATIC_PLUGIN_ADD, $Static_plugin_add, [Define static plugin add calls])

dnl more warning
AC_ARG_ENABLE(warnings,
        [  --enable-warnings       enable more compiler warnings.])
enable_warnings=${enable_warnings-no}

dnl document
AC_ARG_ENABLE(doc,
        [  --enable-doc            enable document generation.])
enable_doc=${enable_doc-no}
AM_CONDITIONAL(MAN_PAGES_ENABLED, test "$enable_doc" = "yes")

dnl data directory path
enfle_datadir=${datadir}/enfle
AC_SUBST(enfle_datadir)

dnl plugin directory path
enfle_plugindir=${libdir}/enfle/plugins
enfle_archiver_plugindir=${enfle_plugindir}/archiver
enfle_audio_plugindir=${enfle_plugindir}/audio
enfle_audiodecoder_plugindir=${enfle_plugindir}/audiodecoder
enfle_demultiplexer_plugindir=${enfle_plugindir}/demultiplexer
enfle_effect_plugindir=${enfle_plugindir}/effect
enfle_loader_plugindir=${enfle_plugindir}/loader
enfle_saver_plugindir=${enfle_plugindir}/saver
enfle_player_plugindir=${enfle_plugindir}/player
enfle_streamer_plugindir=${enfle_plugindir}/streamer
enfle_ui_plugindir=${enfle_plugindir}/ui
enfle_video_plugindir=${enfle_plugindir}/video
enfle_videodecoder_plugindir=${enfle_plugindir}/videodecoder

AC_SUBST(enfle_plugindir)
AC_SUBST(enfle_archiver_plugindir)
AC_SUBST(enfle_audio_plugindir)
AC_SUBST(enfle_audiodecoder_plugindir)
AC_SUBST(enfle_demultiplexer_plugindir)
AC_SUBST(enfle_effect_plugindir)
AC_SUBST(enfle_loader_plugindir)
AC_SUBST(enfle_saver_plugindir)
AC_SUBST(enfle_player_plugindir)
AC_SUBST(enfle_streamer_plugindir)
AC_SUBST(enfle_ui_plugindir)
AC_SUBST(enfle_video_plugindir)
AC_SUBST(enfle_videodecoder_plugindir)

dnl do not put this line at earlier position to avoid confusing configure script.
if test "$CC" = "gcc" -o "$CC" = "/usr/bin/gcc" -o "$CC" = "/usr/local/bin/gcc"; then
  CFLAGS_warn="-Wall -Wno-trigraphs -D_GNU_SOURCE=1"
  if test "`uname -s`" = "SunOS"; then
    dnl Supress insignificant warnings
    CFLAGS="$CFLAGS_warn -Wno-implicit-int -pipe $CFLAGS"
  else
    test "$enable_debug" = "yes" && CFLAGS="-std=c9x $CFLAGS"
    CFLAGS="$CFLAGS_warn -pipe $CFLAGS"
  fi
  test "$enable_warnings" = "yes" && CFLAGS="$CFLAGS -Werror -Wstrict-prototypes -Wwrite-strings"
  CFLAGS="$CFLAGS \$(CFLAGS_POST)"
fi

dnl -std=c9x prefers __inline__
AC_C_INLINE

AC_CONFIG_FILES([
 Makefile
  doc/Makefile
  lib/Makefile
   lib/X11/Makefile
   lib/dllloader/Makefile
   lib/enfle/Makefile
   lib/utils/Makefile
  src/Makefile
  plugins/Makefile
   plugins/ui/Makefile
    plugins/ui/convert/Makefile
    plugins/ui/normal/Makefile
    plugins/ui/wallpaper/Makefile
   plugins/streamer/Makefile
    plugins/streamer/bz2/Makefile
    plugins/streamer/gz/Makefile
    plugins/streamer/xz/Makefile
   plugins/loader/Makefile
    plugins/loader/bmp/Makefile
    plugins/loader/jasper/Makefile
    plugins/loader/jpeg/Makefile
    plugins/loader/pcx/Makefile
    plugins/loader/png/Makefile
    plugins/loader/pnm/Makefile
    plugins/loader/tga/Makefile
    plugins/loader/ungif/Makefile
    plugins/loader/gif/Makefile
    plugins/loader/webp/Makefile
    plugins/loader/xbm/Makefile
    plugins/loader/xpm/Makefile
   plugins/archiver/Makefile
    plugins/archiver/arc/Makefile
    plugins/archiver/rar/Makefile
    plugins/archiver/tar/Makefile
   plugins/player/Makefile
    plugins/player/avifile/Makefile
    plugins/player/generic/Makefile
    plugins/player/libmpeg3/Makefile
    plugins/player/mng/Makefile
    plugins/player/mpeg_lib/Makefile
    plugins/player/ungif/Makefile
   plugins/video/Makefile
    plugins/video/Xlib/Makefile
   plugins/audio/Makefile
    plugins/audio/alsa/Makefile
    plugins/audio/esd/Makefile
    plugins/audio/oss/Makefile
   plugins/effect/Makefile
    plugins/effect/rotate/Makefile
    plugins/effect/gamma/Makefile
   plugins/saver/Makefile
    plugins/saver/jpeg/Makefile
    plugins/saver/png/Makefile
   plugins/audiodecoder/Makefile
    plugins/audiodecoder/avcodec/Makefile
    plugins/audiodecoder/vorbis/Makefile
   plugins/videodecoder/Makefile
    plugins/videodecoder/avcodec/Makefile
    plugins/videodecoder/raw/Makefile
   plugins/demultiplexer/Makefile
    plugins/demultiplexer/asf/Makefile
    plugins/demultiplexer/avi/Makefile
    plugins/demultiplexer/mp3/Makefile
    plugins/demultiplexer/mpeg/Makefile
    plugins/demultiplexer/ogg/Makefile
])

AC_OUTPUT
