## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan 17 22:39:22 2004.
# $Id: Makefile.am,v 1.3 2004/01/18 07:15:19 sian Exp $

AM_CFLAGS = -DLAYER1 -DLAYER2 -DLAYER3

noinst_LTLIBRARIES = libenflempglib.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

libenflempglib_la_SOURCES = \
	common.c \
	dct64_i386.c \
	decode_i386.c \
	huffman.h \
	interface.c \
	l2tables.h \
	layer1.c \
	layer2.c \
	layer3.c \
	mpg123.h \
	mpglib.h \
	tabinit.c \
	audiodecoder_mpglib.c

libenflempglib_la_LIBADD = -lm
