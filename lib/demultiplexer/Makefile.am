## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Tue Feb 20 21:48:35 2001.
# $Id: Makefile.am,v 1.1 2001/02/20 13:54:20 sian Exp $

if HAVE_PTHREAD
noinst_LTLIBRARIES = libdemultiplexer.la
endif

INCLUDES = -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle -I$(top_srcdir)

libdemultiplexer_la_SOURCES = \
	demultiplexer.c demultiplexer.h \
	demultiplexer_mpeg.c demultiplexer_mpeg.h
