## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_audio_plugindir)

if HAVE_ESD
lib_LTLIBRARIES = audio_esd.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

audio_esd_la_LDFLAGS = -avoid-version -module
audio_esd_la_SOURCES = esd.c
audio_esd_la_LIBADD = @LIBADD_ESD@
