## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2000/10/09 20:25:38 sian Exp $

lib_LTLIBRARIES = libenfle.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils

libenfle_la_SOURCES = \
	enfle-plugin.c enfle-plugin.h \
	archiver.c archiver.h archiver-plugin.h \
	streamer.c streamer.h streamer-plugin.h \
	ui.c ui.h ui-plugin.h \
	loader.c loader.h loader-plugin.h \
	player.c player.h player-plugin.h \
	utils.c utils.h \
	stream-utils.c stream-utils.h \
	stream.c stream.h \
	archive.c archive.h \
	movie.c movie.h \
	image_magnify.c \
	image.c image.h
libenfle_la_LIBADD = $(top_builddir)/lib/utils/libenfleutils.la
