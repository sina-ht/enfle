## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2000/09/30 17:36:36 sian Exp $

lib_LTLIBRARIES = libenfle.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils

libenfle_la_SOURCES = \
	enfle-plugin.c enfle-plugin.h \
	archiver.c archiver.h archiver-plugin.h \
	streamer.c streamer.h streamer-plugin.h \
	ui.c ui.h ui-plugin.h \
	loader.c loader.h loader-plugin.h \
	utils.c utils.h \
	stream-utils.c stream-utils.h \
	stream.c stream.h \
	archive.c archive.h \
	image_magnify.c \
	image.c image.h
libenfle_la_LIBADD = $(top_builddir)/lib/utils/libenfleutils.la
