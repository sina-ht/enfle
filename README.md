Simple Plugin-based Graphic Loader Enfle
========================================

Metadeta
--------
(C) Copyright 1998-2018 by Hiroshi Takekawa.

Last Modified: Sun Apr 15 23:11:01 2018.


License
-------
- SPDX-License-Identifier: GPL-2.0-only
- GPLv2 grants the right of _copying, distribution and modification_ of this software.  I hereby explicitly grant the _use_ of this software freely.  The disclaimer is also applied for using.
- The Japanese copyright law enforces me to have the personal right and I cannot abandon or revoke it.  I hereby explicitly permit to use or modify this software for your own need, unless the modification is beneath my dignity.
- The software under this directory contain SPDX-License-Identifier: tag to simplify licensing (if not, apply GPL-2.0-only), of course these tags are legal bindings, though here I include the boilerplate.  Note that the license is GPLv2 only, no later option.

``` text
  This file is part of Enfle.

  Enfle is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.

  Enfle is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
```

ATTENTION: GPL version 2 *only*.  You cannot apply any later version.
This situation may change in the future.


Acknowledgement
---------------

* JPEG Loader plugin and JPEG Saver plugin are based in part on the
  work of the Independent JPEG Group

* libjpeg is only linked, not distributed with this archive.  You
  should install it.

* The Graphics Interchange Format (c) is
  the Copyright property of CompuServe Incorporated.
  GIF (sm) is a Service Mark property of CompuServe Incorporated.

* libungif is only linked, not distributed with this archive.
  Moreover, libungif is not used by default.
  Now that the patent was expired, enfle has its own gif loader plugin.

Integrated libraries by other authors.  Thanks to all authors.

* lib/mpglib: mpglib has been removed.

* lib/mpeg2: libmpeg2 from mpeg2dec-0.4.0 for MPEG decoding.
    libmpeg2 by Aaron Holtzman and others.  For this part, you can
    apply GPL2 or later version.

* lib/{avutil,avcodec}: libavcodec as of 20070719 has been removed.

* lib/mad: libmad-0.15.0 for mp3 decoding.
    libmad by Underbit Technologies, Inc.  For this part, you can
    apply GPL2 or later version.

* lib/j2k: libj2k has been removed.

* plugins/archiver/rar/unrar: unrarsrc-3.7.8 for rar handling.
    You cannot use the code to develop a RAR (WinRAR) compatible archiver.
    Read plugins/archiver/rar/unrar/license.txt.


Blurb
-----

OK, legal part is over.  The rest is fun.
(BTW, my English is rather bad.  Are there any volunteers for correction?)

Enfle is a clipped form of Enfleurage.  Enfleurage means: A process of
extracting perfumes by exposing absorbents, as fixed oils or fats, to
the exhalations of the flowers.  It is used for plants whose volatile
oils are too delicate to be separated by distillation. (Webster's
Revised Unabridged Dictionary (1913)).

This software aims to view many pictures with ease.  You can view
various formatted pictures and movies with several effects.

Formats you can view:

* BMP, GIF, JPEG, PCX, PNG, PNM, XBM, XPM, TGA, JPEG2000, WEBP
* animated GIF
* mng
* mpeg (mpeg1, mpeg2)
* avi,ogg,ogm, (codec supported by avcodec, vorbis)

This software has plugin architecture.  You can write plugins to
loader new formatted pictures and movies.  Also, you can read
regular archive files, such as tar, gz, bz2, and so on.

Some plugins use libraries which is not included in distribution.  You
should install corresponding libraries to use them.

* JPEG: jpegsrc-6b (libjpeg)
* PNG: libpng-1.0.2 or later (recommended: 1.2.22)
* gz: zlib-1.2.3 or later (Note: versions prior to 1.2.1 include the security hole)
* bz2: bzip2-1.0.0 or later (recommended: 1.0.3)
* ungif: libungif-4.1.4
* alsa: require ALSA_PCM_NEW_HW_PARAMS_API.
* ogg: libogg-1.1
* vorbis: libvorbis-1.0.1
* theora: libtheora-1.0-alpha4
* mng: libmng-1.0.0 or later (plugin unmaintained)
* libmpeg3: libmpeg3-1.2.2 or later (not supported now)
* avifile: avifile-0.53.5 or avifile-0.6 in CVS (not supported any more)
* mpeg_lib: mpeg_lib-1.3.1+patch (obsolete)


Requirements
------------

X server's depth should be 16 or 24(bpp 24, 32).  Other depths are
unsupported for now.  If you don't see a correct colored-image in these
supported depths, let me know with xdpyinfo's output.  When use in
depth 16, color mask should be 0xf800, 0x7e0, 0x1f.  Other masks might
be supported sometime.

These environments are checked:

* Linux (kernel 2.2/2.4/2.6/3.x/4.x + glibc-2.1-2.24, x86(-64))
* FreeBSD (4.1R, x86)

Other similar environments should work.  Please let me know if you try
on the same/other environments.

*WARNING*: Enfle is old software.  Some functionality don't worked well
now. e.g. no wall paper support on GNOME environment.


Compile
-------

GCC is mandatory for compile, or compilation will fail.
(With some effort, you can compile with icc(Intel C++ Compieler)).

Basically, type ./configure && make, that's all.  If you get some
error messages, please let me know with config.log, your environment,
or such useful information.

After successfully compiled, type sudo make install.

You can pass several options to configure script.  You can see help
message by typing ./configure --help.

Plugins are compiled as a shared object.  Filename should end with
.so. If you get not .so but only .a, the chances are your system
cannot/don't create shared object.


Command line
------------

Enfle is very simple software.  Pass filenames to arguments which you
want to view.  If you pass directory name, then files under that
directory will be added recursively.  Supported archives(such as
.tar.gz) can be directly specified.

* -C converts images by specified Saver plugin.  If the argument is omitted, PNG is used.
* -i specifies the pattern to include.
* -x specifies the pattern to exclude.
* -w sets the first image as wallpaper.  Will not work in gnome environment.
* -m specifies the method to magnify.
   0: normal 1: double 2: short 3: long
* -c specifies the string to configure, multiple allowed.
* -u specifies which UI plugin to use
* -v specifies which video plugin to use
* -a specifies which audio plugin to use
* -I shows detail information of loaded plugins
* -h shows help message
* -V shows version information
* -q disables plugin cache
* -N updates plugin cache
* -X specifies the threshold width of images to be displayed
* -Y specifies the threshold height of images to be displayed
* -d reads the directory wherein the specified file resides.
* -s starts slideshow, optionally accepts interval timer in second.


Usage
-----

* Left click,n,space: next image
* Right click,b: previous image
* Center click,N (shift+n): next archive
* B (shift+b): previous archive
* Ctrl+n: 5th next image
* Ctrl+b: 5th previous image
* Alt+n: 5th next archive
* Alt+b: 5th previous archive
* d: delete displaying file from list
* D(shift+d): delete displaying file
* q: quit
* f: full screen on/off
* a: Auto forward mode
* s: Slideshow
* S(shift+s): toggle magnification algorithm(nointerpolate/bilinear)
* C-s: save the image displayed as PNG.
* Alt-s: save the image displayed as specified in config.
* h,v: flip horizontally or vertically respecitively.
* l,r: rotate by a right angle.
* m: magnify x2
* M(shift+m): magnify according to longer edge.
* Alt-m: magnify according to shorter edge.
* 1-7: do gamma correction with 2.2, 1.7, 1.45, 1, 1/1.45, 1/1.7, 1/2.2.

Enfle will also quit when all pictures are viewed.

If you have wheel on mouse, you can use it for Left, Right click.


Customize
---------

There is the configuration file in
$prefix/share/enfle/enfle.rc(normally, /usr/local/share/enfle/enfle.rc
or like).  If you'd like to customize, copy and edit it.

``` shell
$ mkdir ~/.enfle
$ cp /usr/local/share/enfle/enfle.rc ~/.enfle/config
```

### Customizing the caption

Use caption_template to customize the window caption.
Default: %p %f %xx%y

Legends:

* %%: % itself
* %) or %>: Indicates the magnification algorithm used.
* %x and %y: Dimension of the displayed image
* %f: Replaced by the format description of the image
* %F: Simplified format description
* %p: The path to the file displayed
* %P: The full path
* %N: The name of the software
* %i: The index of the current file in the current archive
* %I: The number of the files in the current archive


Contact
-------

All bug reporting and comments are welcome.

        Hiroshi Takekawa <sian@big.or.jp>
                         <sian@fennel.org>


Distribution
------------

Enfle is distributed under GNU GENERAL PUBLIC LICENSE Version 2.  See
COPYING in detail.  You cannot apply any later version without my
explicit permission.  This situation may change.


Disclaimer
----------

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
