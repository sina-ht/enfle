## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2000/12/24 15:14:49 sian Exp $

if HAVE_ESD
lib_LTLIBRARIES = audio_esd.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle

audio_esd_la_LDFLAGS = -module
audio_esd_la_SOURCES = esd.c
audio_esd_la_LIBADD = @LIBADD_ESD@
