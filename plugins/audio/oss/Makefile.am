## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_audio_plugindir)

if HAVE_OSS
lib_LTLIBRARIES = audio_oss.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle

audio_oss_la_LDFLAGS = -avoid-version -module
audio_oss_la_SOURCES = oss.c
