## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Mon Sep 17 18:22:55 2001.
# $Id: Makefile.am,v 1.2 2001/09/18 05:23:26 sian Exp $

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
	mpg123.h \
	mpglib.h \
	tabinit.c

libenflempglib_la_LIBADD = -lm
