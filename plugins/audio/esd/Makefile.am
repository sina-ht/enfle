## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_audio_plugindir)

if HAVE_ESD
lib_LTLIBRARIES = audio_esd.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle

audio_esd_la_LDFLAGS = -avoid-version -module
audio_esd_la_SOURCES = esd.c
audio_esd_la_LIBADD = @LIBADD_ESD@
