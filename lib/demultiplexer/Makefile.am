## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Feb 22 01:23:45 2001.
# $Id: Makefile.am,v 1.2 2001/02/21 17:56:29 sian Exp $

if HAVE_PTHREAD
noinst_LTLIBRARIES = libdemultiplexer.la
endif

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)

libdemultiplexer_la_SOURCES = \
	demultiplexer.c demultiplexer.h \
	demultiplexer_mpeg.c demultiplexer_mpeg.h
