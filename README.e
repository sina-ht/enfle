***************************************************************************

             Simple Plugin-based Graphic Loader Enfle

   (C) Copyright 1998, 99, 2000, 2001, 2002 by Hiroshi Takekawa.

             Last Modified: Wed Jul 31 22:54:49 2002.

 This file is part of Enfle.

 Enfle is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.

 Enfle is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

 ATTENTION: GPL version 2 only. You cannot apply any later
 version. This situation may change.

***************************************************************************

 JPEG Loader plugin and JPEG Saver plugin are based in part on the
 work of the Independent JPEG Group

 -- libjpeg is only linked, not distributed with this archive. You
 -- should install it.

 The Graphics Interchange Format (c) is
 the Copyright property of CompuServe Incorporated.
 GIF (sm) is a Service Mark property of CompuServe Incorporated.

 -- libungif is only linked, not distributed with this archive.  Moreover,
 -- libungif is not used by default, for avoiding possible patent
 -- infringement.  It's up to you whether use it or not.

 It seems I should include this sentence: "This product includes
 software developed by or derived from software developed by Project
 Mayo".  Strictly speaking, Enfle and OpenDivX plugin itself don't
 include any portion of software by them.  OpenDivX plugin will be
 dynamically linked with libdivxdecore.  I'm not sure this plugin is
 regarded as "Larger Work".  But it doesn't matter because this plugin
 can be and is released under GPL.  See
 plugins/player/opendivx/opendivx.c for further details.

 This archive includes mpglib by Michael Hipp.  Read lib/mpglib/README.

 This software includes libmpeg2 by Aaron Holtzman.  For this part,
 you can apply GPL2 or later version.

 This archive includes fnmatch.[ch] by FSF.  These files can be
 distributed under GPL2 or (at your option) any later version.

***************************************************

OK, legal part is over.  The rest is fun.
BTW, my English is rather bad.  Are there any volunteers for correction?

Enfle is a clipped form of Enfleurage.  Enfleurage means: A process of
extracting perfumes by exposing absorbents, as fixed oils or fats, to
the exhalations of the flowers.  It is used for plants whose volatile
oils are too delicate to be separated by distillation. (Webster's
Revised Unabridged Dictionary (1913)).


1. Blurb

This software aims to view many pictures just clicking.  You can view
various formatted pictures and movies with several effects.

 Formats you can view:
 BMP, GIF, JPEG, PCX, PNG, PNM, XBM, XPM
 spi(highly unstable, any formats which (some of) susie plugins support)
 animated GIF
 mng
 mpeg(mpeg1, mpeg2)
 avi,asf(avifile uses Windows DLL)
 DivX in AVI format supported by divx4linux

This software has plugin architecture.  You can write plugins to
loader new formatted pictures and movies.  Also, you can read
irregular files, such as tar, gz, bz2, and so on.

***
GIF decoder must perform LZW decompression.  Unfortunately, it's
patented.  (Many think that patent doesn't cover decompression.  But
Unisys won't permit).  So, this software never includes LZW
decompression code, though I've written.  UNGIF plugin which uses
libungif is included.  But, I'm not sure that usage of this library is
legal.  It's up to you whether use this or not.  You must explicit
pass --with-ungif option to configure script.
***

Some plugins use libraries which is not included in distribution.  You
should install corresponding libraries to use them.

JPEG: jpegsrc-6b (libjpeg)
PNG: libpng-1.0.2 or later (recommended: 1.0.9 or 1.2.3)
gz: zlib-1.1.4 or later (Note: versions prior to 1.1.3 include the security hole)
bz2: bzip2-1.0.0 or later
ungif: libungif-3.1.0 or later may work..., but even 4.1.0 has bug.
       (recommended: cvs version or ask me the patch)
mng: libmng-1.0.0 or later
libmpeg3: libmpeg3-1.2.2 or later
mpeg_lib: mpeg_lib-1.3.1+patch (if you want the patch, contact me)
avifile: avifile-0.53.5 or avifile-0.6 in CVS
opendivx: divx4linux-20010807, divx4linux501-20020418


2. Requirements

X server's depth should be 16 or 24(bpp 24, 32).  Other depths are
unsupported so far.  If you don't see a correct colored-image in these
supported depths, let me know with xdpyinfo's output.  When use in
depth 16, color mask should be 0xf800, 0x7e0, 0x1f.  Other masks might
be supported sometime.

These environments are checked:

Linux (kernel 2.2 or 2.4 + glibc-2.1 or 2.2, x86)
FreeBSD (4.1R, x86)

Other similar environments should work.  Please let me know if you try
on the same/other environments.

My main environment is:

Kernel: Linux-2.5.25
CPU: PentiumIII/1GHz or PentiumIII/700MHz
X server: XFree86-4.2.0
Video: Matrox Millennium G450 DH/AGP 16M or Silicon Motion, Inc. SM720 Lynx3DM
Compiler: gcc version 2.95.3 or 3.1
libc: glibc-2.2.5


3. Compile

GCC is mandatory for compile, or compilation will fail.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

Put Susie plugins on /usr/(local/)/lib/enfle/plugins/spis or such
where plugins are searched.  It might be able to decode pictures, or
might fail to decode, or even might fail to load.  Your mileage may
vary. Please inform me of your result.


4. Command line

Enfle is very simple software.  Pass filenames to arguments which you
want to view.  If you pass directory name, then files under that
directory will be added recursively.  Supported archives(such as
.tar.gz) can be directly specified.

-C converts images by specified Saver plugin.  If the argument is omitted, PNG is used.
-i specifies the pattern to include.
-x specifies the pattern to exclude.
-w sets the first image as wallpaper.
-m specifies the method to magnify.
   0: normal 1: double 2: short 3: long
-c specifies the string to configure, multiple allowed.
-u specifies which UI plugin to use
-v specifies which video plugin to use
-a specifies which audio plugin to use
-I shows detail information of loaded plugins
-h shows help message
-V shows version information


5. Usage

Left click,n,space	next image
Right click,b		previous image
Center click,N		next archive
d			delete displaying file from list
D(shift+d)		delete displaying file
q			quit
f			full screen on/off
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
cp /usr/share/enfle/enfle.rc ~/.enfle/config

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


9. Distribution

Enfle is distributed under GNU GENERAL PUBLIC LICENSE Version 2.  See
COPYING in detail.  You cannot apply any later version without my
explicit permission.  This situation may change.
