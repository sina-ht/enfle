## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_audio_plugindir)

if HAVE_OSS
lib_LTLIBRARIES = audio_oss.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

audio_oss_la_LDFLAGS = -avoid-version -module
audio_oss_la_SOURCES = oss.c
