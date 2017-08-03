## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Feb 12 23:56:12 2004.

if HAVE_PTHREAD
noinst_LTLIBRARIES = libdemultiplexer.la
endif

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)

libdemultiplexer_la_SOURCES = \
	demultiplexer_old.c demultiplexer_old.h \
	demultiplexer_old_mpeg.c demultiplexer_old_mpeg.h \
	libriff.c libriff.h \
	demultiplexer_old_avi.c demultiplexer_old_avi.h demultiplexer_old_avi_private.h
