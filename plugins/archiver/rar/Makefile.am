## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu May  3 21:15:20 2018.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = archiver/rar

libdir = $(enfle_archiver_plugindir)

lib_LTLIBRARIES = archiver_rar.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

CFLAGS_POST = -Wno-error

### From original makefile
#LIB_OBJ=filestr.o scantree.o dll.o qopen.o
#OBJECTS=rar.o strlist.o strfn.o pathfn.o smallfn.o global.o file.o filefn.o filcreat.o \
#	archive.o arcread.o unicode.o system.o isnt.o crypt.o crc.o rawread.o encname.o \
#	resource.o match.o timefn.o rdwrfn.o consio.o options.o errhnd.o rarvm.o secpassword.o \
#	rijndael.o getbits.o sha1.o sha256.o blake2s.o hash.o extinfo.o extract.o volume.o \
#  list.o find.o unpack.o headers.o threadpool.o rs16.o cmddata.o ui.o
#
### Distributed files
# unrar/savepos.hpp \
# unrar/ulinks.cpp \
# unrar/model.cpp unrar/model.hpp \
# unrar/crypt5.cpp \
# unrar/unpack50.cpp \
# unrar/compress.hpp \
# unrar/win32stm.cpp \
# unrar/rarpch.cpp \
# unrar/headers5.hpp \
# unrar/blake2sp.cpp \
# unrar/crypt1.cpp \
# unrar/hardlinks.cpp \
# unrar/unpack50mt.cpp \
# unrar/uowners.cpp \
# unrar/recvol.cpp unrar/recvol.hpp \
# unrar/win32acl.cpp \
# unrar/uiconsole.cpp \
# unrar/unpack20.cpp \
# unrar/unpack30.cpp \
# unrar/coder.cpp unrar/coder.hpp \
# unrar/os.hpp \
# unrar/array.hpp \
# unrar/rartypes.hpp \
# unrar/version.hpp \
# unrar/suballoc.cpp unrar/suballoc.hpp \
# unrar/uicommon.cpp \
# unrar/recvol5.cpp \
# unrar/unpack15.cpp \
# unrar/uisilent.cpp \
# unrar/unpack50frag.cpp \
# unrar/rs.cpp unrar/rs.hpp \
# unrar/arcmem.cpp unrar/arcmem.hpp \
# unrar/recvol3.cpp \
# unrar/threadmisc.cpp \
# unrar/crypt2.cpp \
# unrar/rawint.hpp \
# unrar/unpackinline.cpp \
# unrar/log.cpp unrar/log.hpp \
# unrar/crypt3.cpp \
# unrar/blake2s_sse.cpp \
# unrar/arccmt.cpp \
# unrar/rarlang.hpp \
# unrar/loclang.hpp \
# unrar/rardefs.hpp \
# unrar/raros.hpp \
# unrar/win32lnk.cpp
#
### Compiled files
# rar.o: unrar/rar.cpp unrar/rar.hpp
# strlist.o: unrar/strlist.cpp unrar/strlist.hpp
# strfn.o: unrar/strfn.cpp unrar/strfn.hpp
# pathfn.o: unrar/pathfn.cpp unrar/pathfn.hpp
# smallfn.o: unrar/smallfn.cpp unrar/smallfn.hpp
# global.o: unrar/global.cpp unrar/global.hpp
# file.o: unrar/file.cpp unrar/file.hpp
# filefn.o: unrar/filefn.cpp unrar/filefn.hpp
# filcreat.o: unrar/filcreat.cpp unrar/filcreat.hpp
# archive.o: unrar/archive.cpp unrar/archive.hpp
# arcread.o: unrar/arcread.cpp
# unicode.o: unrar/unicode.cpp unrar/unicode.hpp
# system.o: unrar/system.cpp unrar/system.hpp
# isnt.o: unrar/isnt.cpp unrar/isnt.hpp
# crypt.o: unrar/crypt.cpp unrar/crypt.hpp
# crc.o: unrar/crc.cpp unrar/crc.hpp
# rawread.o: unrar/rawread.cpp unrar/rawread.hpp
# encname.o: unrar/encname.cpp unrar/encname.hpp
# resource.o: unrar/resource.cpp unrar/resource.hpp
# match.o: unrar/match.cpp unrar/match.hpp
# timefn.o: unrar/timefn.cpp unrar/timefn.hpp
# rdwrfn.o: unrar/rdwrfn.cpp unrar/rdwrfn.hpp
# consio.o: unrar/consio.cpp unrar/consio.hpp
# options.o: unrar/options.cpp unrar/options.hpp
# errhnd.o: unrar/errhnd.cpp unrar/errhnd.hpp
# rarvm.o: unrar/rarvm.cpp unrar/rarvm.hpp
# secpassword.o: unrar/secpassword.cpp unrar/secpassword.hpp
# rijndael.o: unrar/rijndael.cpp unrar/rijndael.hpp
# getbits.o: unrar/getbits.cpp unrar/getbits.hpp
# sha1.o: unrar/sha1.cpp unrar/sha1.hpp
# sha256.o: unrar/sha256.cpp unrar/sha256.hpp
# blake2s.o: unrar/blake2s.cpp unrar/blake2s.hpp
# hash.o: unrar/hash.cpp unrar/hash.hpp
# extinfo.o: unrar/extinfo.cpp unrar/extinfo.hpp
# extract.o: unrar/extract.cpp unrar/extract.hpp
# volume.o: unrar/volume.cpp unrar/volume.hpp
# list.o: unrar/list.cpp unrar/list.hpp
# find.o: unrar/find.cpp unrar/find.hpp
# unpack.o: unrar/unpack.cpp unrar/unpack.hpp
# headers.o: unrar/headers.cpp unrar/headers.hpp
# threadpool.o: unrar/threadpool.cpp unrar/threadpool.hpp
# rs16.o: unrar/rs16.cpp unrar/rs16.hpp
# cmddata.o: unrar/cmddata.cpp unrar/cmddata.hpp
# ui.o: unrar/ui.cpp unrar/ui.hpp
# filestr.o: unrar/filestr.cpp unrar/filestr.hpp
# scantree.o: unrar/scantree.cpp unrar/scantree.hpp
# dll.o: unrar/dll.cpp unrar/dll.hpp
# qopen.o: unrar/qopen.cpp unrar/qopen.hpp

EXTRA_DIST = \
 unrar/savepos.hpp \
 unrar/ulinks.cpp \
 unrar/model.cpp unrar/model.hpp \
 unrar/crypt5.cpp \
 unrar/unpack50.cpp \
 unrar/compress.hpp \
 unrar/win32stm.cpp \
 unrar/rarpch.cpp \
 unrar/headers5.hpp \
 unrar/blake2sp.cpp \
 unrar/crypt1.cpp \
 unrar/hardlinks.cpp \
 unrar/unpack50mt.cpp \
 unrar/uowners.cpp \
 unrar/recvol.cpp unrar/recvol.hpp \
 unrar/win32acl.cpp \
 unrar/uiconsole.cpp \
 unrar/unpack20.cpp \
 unrar/unpack30.cpp \
 unrar/coder.cpp unrar/coder.hpp \
 unrar/os.hpp \
 unrar/array.hpp \
 unrar/rartypes.hpp \
 unrar/version.hpp \
 unrar/suballoc.cpp unrar/suballoc.hpp \
 unrar/uicommon.cpp \
 unrar/recvol5.cpp \
 unrar/unpack15.cpp \
 unrar/uisilent.cpp \
 unrar/unpack50frag.cpp \
 unrar/rs.cpp unrar/rs.hpp \
 unrar/arcmem.cpp unrar/arcmem.hpp \
 unrar/recvol3.cpp \
 unrar/threadmisc.cpp \
 unrar/crypt2.cpp \
 unrar/rawint.hpp \
 unrar/unpackinline.cpp \
 unrar/log.cpp unrar/log.hpp \
 unrar/crypt3.cpp \
 unrar/blake2s_sse.cpp \
 unrar/arccmt.cpp \
 unrar/rarlang.hpp \
 unrar/loclang.hpp \
 unrar/rardefs.hpp \
 unrar/raros.hpp \
 unrar/win32lnk.cpp \
 unrar/acknow.txt  unrar/license.txt  unrar/readme.txt

archiver_rar_la_LDFLAGS = $(MORE_LDFLAGS) -module
archiver_rar_la_CFLAGS = $(MORE_CFLAGS) -D_UNIX -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DRAR_SMP
archiver_rar_la_CXXFLAGS = $(MORE_CFLAGS) -DRARDLL -D_UNIX -DSILENT -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DRAR_SMP
archiver_rar_la_SOURCES = unrar.c \
	unrar/rar.cpp unrar/rar.hpp \
	unrar/strlist.cpp unrar/strlist.hpp \
	unrar/strfn.cpp unrar/strfn.hpp \
	unrar/pathfn.cpp unrar/pathfn.hpp \
	unrar/smallfn.cpp unrar/smallfn.hpp \
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
	unrar/errhnd.cpp unrar/errhnd.hpp \
	unrar/rarvm.cpp unrar/rarvm.hpp \
	unrar/secpassword.cpp unrar/secpassword.hpp \
	unrar/rijndael.cpp unrar/rijndael.hpp \
	unrar/getbits.cpp unrar/getbits.hpp \
	unrar/sha1.cpp unrar/sha1.hpp \
	unrar/sha256.cpp unrar/sha256.hpp \
	unrar/blake2s.cpp unrar/blake2s.hpp \
	unrar/hash.cpp unrar/hash.hpp \
	unrar/extinfo.cpp unrar/extinfo.hpp \
	unrar/extract.cpp unrar/extract.hpp \
	unrar/volume.cpp unrar/volume.hpp \
	unrar/list.cpp unrar/list.hpp \
	unrar/find.cpp unrar/find.hpp \
	unrar/unpack.cpp unrar/unpack.hpp \
	unrar/headers.cpp unrar/headers.hpp \
	unrar/threadpool.cpp unrar/threadpool.hpp \
	unrar/rs16.cpp unrar/rs16.hpp \
	unrar/cmddata.cpp unrar/cmddata.hpp \
	unrar/ui.cpp unrar/ui.hpp \
	unrar/filestr.cpp unrar/filestr.hpp \
	unrar/scantree.cpp unrar/scantree.hpp \
	unrar/dll.cpp unrar/dll.hpp \
	unrar/qopen.cpp unrar/qopen.hpp
