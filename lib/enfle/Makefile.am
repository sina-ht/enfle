## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.7 2000/11/20 12:54:04 sian Exp $

lib_LTLIBRARIES = libenfle.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/dllloader

libenfle_la_SOURCES = \
	enfle-plugins.c enfle-plugins.h \
	enfle-plugin.c enfle-plugin.h enfle-plugintype.h \
	archiver.c archiver.h archiver-extra.h archiver-plugin.h \
	streamer.c streamer.h streamer-plugin.h \
	ui.c ui.h ui-extra.h ui-plugin.h \
	video.h video-plugin.h \
	spi.c spi.h spi-private.h \
	loader.c loader.h loader-extra.h loader-plugin.h \
	player.c player.h player-plugin.h \
	utils.h \
	stream-utils.c stream-utils.h \
	stream.c stream.h \
	archive.c archive.h \
	movie.c movie.h \
	image_magnify.c \
	image.c image.h
libenfle_la_LIBADD = $(top_builddir)/lib/utils/libenfleutils.la $(top_builddir)/lib/dllloader/libdllloader.la
