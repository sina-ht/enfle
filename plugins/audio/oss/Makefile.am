## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2000/12/18 16:59:26 sian Exp $

if HAVE_OSS
lib_LTLIBRARIES = audio_oss.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle

audio_oss_la_LDFLAGS = -module
audio_oss_la_SOURCES = oss.c
