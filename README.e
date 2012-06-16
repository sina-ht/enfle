***************************************************************************

             Simple Plugin-based Graphic Loader Enfle

           (C) Copyright 1998-2012 by Hiroshi Takekawa.

             Last Modified: Sun Jun 17 00:31:49 2012.

***************************************************************************

Licensing Term

  This file is part of Enfle.

  Enfle is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.

  Enfle is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

  ATTENTION: GPL version 2 only. You cannot apply any later version.
  This situation may change in the future.

***************************************************************************

Acknowledgement

  JPEG Loader plugin and JPEG Saver plugin are based in part on the
  work of the Independent JPEG Group

  libjpeg is only linked, not distributed with this archive.  You
  should install it.

  The Graphics Interchange Format (c) is
  the Copyright property of CompuServe Incorporated.
  GIF (sm) is a Service Mark property of CompuServe Incorporated.

  libungif is only linked, not distributed with this archive.
  Moreover, libungif is not used by default.
  Now that the patent was expired, enfle has its own gif loader plugin.

Integrated libraries by other authors
  Thanks to all authors.

  lib/mpglib: mpglib from mpg123 for mp3 decoding
    mpglib by Michael Hipp.  Read lib/mpglib/README.

  lib/mpeg2: libmpeg2 from mpeg2dec-0.4.0 for MPEG decoding
    libmpeg2 by Aaron Holtzman and others.  For this part, you can
    apply GPL2 or later version.

  lib/{avutil,avcodec}: libavcodec as of 20070719 for a/v decoding
    libavcodec by Fabrice Bellard and others.  For this part, you can
    apply LGPL2 or any later version.  LGPL license is in
    lib/avcodec/COPYING.LIB.  System-installed libavcodec is also supported.

  lib/mad: libmad-0.15.0 for mp3 decoding
    libmad by Underbit Technologies, Inc.  For this part, you can
    apply GPL2 or later version.

  lib/j2k: libj2k for JPEG2000 decoding
    libj2k by David Janssens.  Read lib/j2k/LICENSE.

  plugins/archiver/rar/unrar: unrarsrc-3.7.8 for rar handling
    You cannot use the code to develop a RAR (WinRAR) compatible archiver.
    Read plugins/archiver/rar/unrar/license.txt.

***************************************************************************

OK, legal part is over.  The rest is fun.
BTW, my English is rather bad.  Are there any volunteers for correction?

Enfle is a clipped form of Enfleurage.  Enfleurage means: A process of
extracting perfumes by exposing absorbents, as fixed oils or fats, to
the exhalations of the flowers.  It is used for plants whose volatile
oils are too delicate to be separated by distillation. (Webster's
Revised Unabridged Dictionary (1913)).


1. Blurb

This software aims to view many pictures with ease.  You can view
various formatted pictures and movies with several effects.

 Formats you can view:
  BMP, GIF, JPEG, PCX, PNG, PNM, XBM, XPM, TGA, JPEG2000, WEBP
  spi(highly unstable, any formats which (some of) susie plugins support)
  animated GIF
  mng
  mpeg (mpeg1, mpeg2)
  avi,ogg,ogm(,asf,wmv) (codec supported by avcodec, vorbis, Windows DMO)

This software has plugin architecture.  You can write plugins to
loader new formatted pictures and movies.  Also, you can read
regular archive files, such as tar, gz, bz2, and so on.

Some plugins use libraries which is not included in distribution.  You
should install corresponding libraries to use them.

JPEG: jpegsrc-6b (libjpeg)
PNG: libpng-1.0.2 or later (recommended: 1.2.22)
gz: zlib-1.2.3 or later (Note: versions prior to 1.2.1 include the security hole)
bz2: bzip2-1.0.0 or later (recommended: 1.0.3)
ungif: libungif-4.1.4
alsa: require ALSA_PCM_NEW_HW_PARAMS_API.
ogg: libogg-1.1
vorbis: libvorbis-1.0.1
theora: libtheora-1.0-alpha4
divx: divx4linux-20030428 (old version will not work)
mng: libmng-1.0.0 or later (plugin unmaintained)
libmpeg3: libmpeg3-1.2.2 or later (not supported now)
avifile: avifile-0.53.5 or avifile-0.6 in CVS (not supported any more)
mpeg_lib: mpeg_lib-1.3.1+patch (obsolete)


2. Requirements

X server's depth should be 16 or 24(bpp 24, 32).  Other depths are
unsupported for now.  If you don't see a correct colored-image in these
supported depths, let me know with xdpyinfo's output.  When use in
depth 16, color mask should be 0xf800, 0x7e0, 0x1f.  Other masks might
be supported sometime.

These environments are checked:

Linux (kernel 2.2/2.4/2.6/3.0 + glibc-2.[1234567], x86(-64))
FreeBSD (4.1R, x86)

Other similar environments should work.  Please let me know if you try
on the same/other environments.


3. Compile

GCC is mandatory for compile, or compilation will fail.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
(With some effort, you can compile with icc(Intel C++ Compieler).

Basically, type ./configure && make, that's all.  If you get some
error messages, please let me know with config.log, your environment,
or such useful information.

After successfully compiled, type make install.

You can pass several options to configure script.  You can see help
message by typing ./configure --help.

Plugins are compiled as a shared object.  Filename should end with
.so. If you get not .so but only .a, the chances are your system
cannot/don't create shared object.

If you'd like to enable susie plugin extension, add --enable-spi.
If you'd like to enable libungif, add --with-ungif.

Put Susie plugins on the directory specified by spi/dir in
configuration file.  It might be able to decode pictures, or might
fail to decode, or even might fail to load.  Your mileage may
vary. Please inform me of your result.

Put Windows DMO DLL on the directory specified by dmo/dir in
configuration file.  It might be able to decode pictures, or might
fail to decode, or even might fail to load...

Compiling avcodec takes much time.  If you don't need this, disable it
by --disable-avcodec option.


4. Command line

Enfle is very simple software.  Pass filenames to arguments which you
want to view.  If you pass directory name, then files under that
directory will be added recursively.  Supported archives(such as
.tar.gz) can be directly specified.

-C converts images by specified Saver plugin.  If the argument is omitted, PNG is used.
-i specifies the pattern to include.
-x specifies the pattern to exclude.
-w sets the first image as wallpaper.  Will not work in gnome environment.
-m specifies the method to magnify.
   0: normal 1: double 2: short 3: long
-c specifies the string to configure, multiple allowed.
-u specifies which UI plugin to use
-v specifies which video plugin to use
-a specifies which audio plugin to use
-I shows detail information of loaded plugins
-h shows help message
-V shows version information
-q disables plugin cache
-N updates plugin cache
-X specifies the threshold width of images to be displayed
-Y specifies the threshold height of images to be displayed
-d reads the directory wherein the specified file resides.
-s starts slideshow, optionally accepts interval timer in second.


5. Usage

Left click,n,space		next image
Right click,b			previous image
Center click,N (shift+n)	next archive
B (shift+b)			previous archive
Ctrl+n				5th next image
Ctrl+b				5th previous image
Alt+n				5th next archive
Alt+b				5th previous archive
d			delete displaying file from list
D(shift+d)		delete displaying file
q			quit
f			full screen on/off
a			Auto forward mode
s			Slideshow
S(shift+s)		toggle magnification algorithm(nointerpolate/bilinear)
C-s			save the image displayed as PNG.
Alt-s			save the image displayed as specified in config.
h,v			flip horizontally or vertically respecitively.
l,r			rotate by a right angle.
m			magnify x2
M(shift+m)		magnify according to longer edge.
Alt-m			magnify according to shorter edge.
1-7			do gamma correction with 2.2, 1.7, 1.45, 1, 1/1.45, 1/1.7, 1/2.2.

Yes, Enfle will also quit when all pictures are viewed.

If you have wheel, you can use it for Left, Right click.


6. Customize

There is the configuration file in
$prefix/share/enfle/enfle.rc(normally, /usr/local/share/enfle/enfle.rc
or like).  If you'd like to customize, copy and edit it.

mkdir ~/.enfle
cp /usr/local/share/enfle/enfle.rc ~/.enfle/config

Customizing the caption

Use caption_template to customize the window caption.
Default: %p %f %xx%y

Legends:
%%: % itself
%) or %>: Indicates the magnification algorithm used.
%x and %y: Dimension of the displayed image
%f: Replaced by the format description of the image
%F: Simplified format description
%p: The path to the file displayed
%P: The full path
%N: The name of the software
%i: The index of the current file in the current archive
%I: The number of the files in the current archive


7. Contact

All bug reporting and comments are welcome.  Please subscribe to Enfle
ML and post to it.  To subscribe, write:

 subscribe Yourname

in body, send it to <enfle-ctl@fennel.org>.

 e.g. subscribe Hiroshi Takekawa

You will be requested to confirm your subscription.  Read it and
confirm.  I have the rights to read, convert, transfer, put in web
pages, all posted mails.  And any other necessary rights to provide an
ML search engine. (I just want to provide an ML search engine. Please
do not be alarmed.)

If you insist, you can send me directly.

        Hiroshi Takekawa <sian@big.or.jp>
                         <sian@fennel.org>


8. Distribution

Enfle is distributed under GNU GENERAL PUBLIC LICENSE Version 2.  See
COPYING in detail.  You cannot apply any later version without my
explicit permission.  This situation may change.
