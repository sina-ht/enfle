## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.12 2001/01/18 17:02:04 sian Exp $

lib_LTLIBRARIES = libenfle.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

libenfle_la_SOURCES = \
	enfle-plugins.c enfle-plugins.h \
	enfle-plugin.c enfle-plugin.h enfle-plugintype.h \
	archiver.c archiver.h archiver-extra.h archiver-plugin.h \
	streamer.c streamer.h streamer-plugin.h \
	ui.c ui.h ui-extra.h ui-plugin.h \
	video.h video-plugin.h \
	audio.h audio-plugin.h \
	spi.c spi.h spi-private.h \
	loader.c loader.h loader-extra.h loader-plugin.h \
	player.c player.h player-extra.h player-plugin.h \
	utils.h \
	stream-utils.c stream-utils.h \
	stream.c stream.h \
	archive.c archive.h \
	movie.c movie.h \
	image_magnify.c \
	image.c image.h \
	memory.c memory.h

if USE_SPI
libenfle_la_LIBADD = @LIBADD_DLOPEN@ $(top_builddir)/lib/utils/libenfleutils.la $(top_builddir)/lib/dllloader/libdllloader.la
else
libenfle_la_LIBADD = @LIBADD_DLOPEN@ $(top_builddir)/lib/utils/libenfleutils.la
endif
