## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Mon Sep 17 19:49:13 2007.
# $Id: Makefile.am,v 1.1 2007/10/20 13:41:55 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = archiver/rar

libdir = $(enfle_archiver_plugindir)

lib_LTLIBRARIES = archiver_rar.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

EXTRA_DIST = unrar/arccmt.cpp unrar/array.hpp unrar/beosea.cpp unrar/coder.cpp unrar/coder.hpp unrar/compress.hpp unrar/headers.hpp unrar/loclang.hpp unrar/log.cpp unrar/log.hpp unrar/model.cpp unrar/model.hpp unrar/os.hpp unrar/os2ea.cpp unrar/rardefs.hpp unrar/rarfn.hpp unrar/rarlang.hpp unrar/raros.hpp unrar/rartypes.hpp unrar/rarvmtbl.cpp unrar/recvol.cpp unrar/recvol.hpp unrar/rs.cpp unrar/rs.hpp unrar/smallfn.cpp unrar/smallfn.hpp unrar/suballoc.cpp unrar/suballoc.hpp unrar/unios2.cpp unrar/unpack15.cpp unrar/unpack20.cpp unrar/uowners.cpp unrar/version.hpp unrar/win32acl.cpp unrar/win32stm.cpp

CFLAGS_POST = -Wno-error

archiver_rar_la_LDFLAGS = $(MORE_LDFLAGS) -module
archiver_rar_la_CFLAGS = $(MORE_CFLAGS) -D_UNIX
archiver_rar_la_CXXFLAGS = $(MORE_CFLAGS) -DRARDLL -D_UNIX -DSILENT
archiver_rar_la_SOURCES = unrar.c \
	unrar/rar.cpp unrar/rar.hpp \
	unrar/strlist.cpp unrar/strlist.hpp \
	unrar/strfn.cpp unrar/strfn.hpp \
	unrar/pathfn.cpp unrar/pathfn.hpp \
	unrar/int64.cpp unrar/int64.hpp \
	unrar/savepos.cpp unrar/savepos.hpp \
	unrar/global.cpp unrar/global.hpp \
	unrar/file.cpp unrar/file.hpp \
	unrar/filefn.cpp unrar/filefn.hpp \
	unrar/filcreat.cpp unrar/filcreat.hpp \
	unrar/archive.cpp unrar/archive.hpp \
	unrar/arcread.cpp \
	unrar/unicode.cpp unrar/unicode.hpp \
	unrar/system.cpp unrar/system.hpp \
	unrar/isnt.cpp unrar/isnt.hpp \
	unrar/crypt.cpp unrar/crypt.hpp \
	unrar/crc.cpp unrar/crc.hpp \
	unrar/rawread.cpp unrar/rawread.hpp \
	unrar/encname.cpp unrar/encname.hpp \
	unrar/resource.cpp unrar/resource.hpp \
	unrar/match.cpp unrar/match.hpp \
	unrar/timefn.cpp unrar/timefn.hpp \
	unrar/rdwrfn.cpp unrar/rdwrfn.hpp \
	unrar/consio.cpp unrar/consio.hpp \
	unrar/options.cpp unrar/options.hpp \
	unrar/ulinks.cpp unrar/ulinks.hpp \
	unrar/errhnd.cpp unrar/errhnd.hpp \
	unrar/rarvm.cpp unrar/rarvm.hpp \
	unrar/rijndael.cpp unrar/rijndael.hpp \
	unrar/getbits.cpp unrar/getbits.hpp \
	unrar/sha1.cpp unrar/sha1.hpp \
	unrar/extinfo.cpp unrar/extinfo.hpp \
	unrar/extract.cpp unrar/extract.hpp \
	unrar/volume.cpp unrar/volume.hpp \
	unrar/list.cpp unrar/list.hpp \
	unrar/find.cpp unrar/find.hpp \
	unrar/unpack.cpp unrar/unpack.hpp \
	unrar/cmddata.cpp unrar/cmddata.hpp \
	unrar/filestr.cpp unrar/filestr.hpp \
	unrar/scantree.cpp unrar/scantree.hpp \
	unrar/dll.cpp unrar/dll.hpp
