## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Aug 29 15:37:46 2004.
# $Id: Makefile.am,v 1.7 2004/08/29 06:54:17 sian Exp $

AM_CFLAGS = -Dasm=__asm__
EXTRA_DIST = mad.h.sed CHANGES COPYRIGHT CREDITS README TODO VERSION

noinst_LTLIBRARIES = libenflemad.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

BUILT_SOURCES = mad.h
CLEANFILES = mad.h
#libenflemad_la_LIBADD = -lm
#libenflemad_la_LIBADD = @ASO_OBJS@
exported_headers = \
	version.h \
	fixed.h \
	bit.h \
	timer.h \
	stream.h \
	frame.h \
	synth.h \
	decoder.h

noinst_HEADERS = \
	$(exported_headers) \
	config.h \
	global.h \
	huffman.h \
	layer12.h \
	layer3.h \
	mad.h

libenflemad_la_SOURCES = \
	version.c \
	fixed.c \
	bit.c \
	timer.c \
	stream.c \
	frame.c \
	synth.c \
	decoder.c \
	layer12.c \
	layer3.c \
	huffman.c \
	D.dat \
	imdct_s.dat \
	qc_table.dat \
	rq_table.dat \
	sf_table.dat

EXTRA_libenflemad_la_SOURCES = imdct_l_arm.S

if ARCH_X86
FPM=-DFPM_INTEL
endif

if ARCH_SPARC
FPM=-DFPM_SPARC
endif

if ARCH_PPC
FPM=-DFPM_PPC
endif

$(srcdir)/mad.h: Makefile.am $(srcdir)/COPYRIGHT $(srcdir)/mad.h.sed $(exported_headers)
	(sed -e '1s|.*|/*|' -e '1b' -e '$$s|.*| */|' -e '$$b'  \
		-e 's/^.*/ *&/' $(srcdir)/COPYRIGHT; echo;  \
	echo "# ifdef __cplusplus";  \
	echo 'extern "C" {';  \
	echo "# endif"; echo;  \
	if [ ".$(FPM)" != "." ]; then  \
		echo ".$(FPM)" | sed -e 's|^\.-D|# define |'; echo;  \
	else \
		echo ".-DFPM_DEFAULT" | sed -e 's|^\.-D|# define |'; echo;  \
	fi;  \
	sed -ne 's/^# *define  *\(HAVE_.*_ASM\).*/# define \1/p'  \
		config.h; echo;  \
	sed -ne 's/^# *define  *OPT_\(SPEED\|ACCURACY\).*/# define OPT_\1/p'  \
		config.h; echo;  \
	sed -ne 's/^# *define  *\(SIZEOF_.*\)/# define \1/p'  \
		config.h; echo;  \
	for header in $(exported_headers); do  \
		echo;  \
		sed -n -f $(srcdir)/mad.h.sed $(srcdir)/$$header;  \
	done; echo;  \
	echo "# ifdef __cplusplus";  \
	echo '}';  \
	echo "# endif") >$@

all: $(srcdir)/mad.h
.c.s:
	$(COMPILE) -S $<
