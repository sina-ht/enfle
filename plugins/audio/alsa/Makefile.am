## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.2 2001/10/14 12:33:50 sian Exp $

libdir = $(enfle_audio_plugindir)

if HAVE_ALSA
lib_LTLIBRARIES = audio_alsa.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

audio_alsa_la_LDFLAGS = -avoid-version -module
audio_alsa_la_SOURCES = alsa.c
audio_alsa_la_LIBADD = @LIBADD_ALSA@
