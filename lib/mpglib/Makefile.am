## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan 27 00:35:20 2001.
# $Id: Makefile.am,v 1.1 2001/01/28 03:44:48 sian Exp $

AM_CFLAGS = -DLAYER1 -DLAYER2 -DLAYER3

noinst_LTLIBRARIES = libenflempglib.la
INCLUDES = -I$(top_srcdir)

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
	main.c \
	mpg123.h \
	mpglib.h \
	tabinit.c

libenflempglib_la_LIBADD = -lm
