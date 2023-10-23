Simple Plugin-based Graphic Loader Enfle
========================================

WARNING
-------
*WARNING*: Enfle is _VERY_ old software.  There might be vulnerabilities
unfixed.  Some functionality doesn't work well now. e.g. no wall paper
support on GNOME environment.


Metadata
--------
(C) Copyright 1998-2023 by Hiroshi Takekawa.

Last Modified: Mon Oct 23 16:05:45 2023.


License
-------
- SPDX-License-Identifier: GPL-2.0-only
- The moral right of the author has been asserted.
- GPLv2 grants the right of _copying, distribution and modification_ of this software.  I hereby explicitly grant the _use_ of this software freely.  The disclaimer is also applied for using.
- The Japanese copyright law enforces me to have the moral right and I cannot abandon or revoke it.  I hereby explicitly permit to use or modify this software for your own need, _unless_ the modification is beneath my dignity.
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
  work of the Independent JPEG Group.

* libjpeg is only linked, not distributed with this archive.  You
  _MUST_ install it to use.

* The Graphics Interchange Format (c) is
  the Copyright property of CompuServe Incorporated.
  GIF (sm) is a Service Mark property of CompuServe Incorporated.

* libungif is only linked, not distributed with this archive.
  Moreover, libungif is not used by default.
  Now that the patent was expired, enfle has its own gif loader plugin.

Integrated library by other authors.  Thanks to all authors.

* lib/mpeg2: libmpeg2-0.5.1 for MPEG decoding.
    libmpeg2 by Aaron Holtzman and others.  For this part, you can
    apply GPL2 or later version.

* plugins/archiver/rar/unrar: unrarsrc-5.7.3 for rar handling.
    You cannot use the code to develop a RAR (WinRAR) compatible archiver.
    Read plugins/archiver/rar/unrar/license.txt.


Blurb
-----

OK, legal part is over.  The rest is fun.

Enfle is a clipped form of Enfleurage.  Enfleurage means: A process of
extracting perfumes by exposing absorbents, as fixed oils or fats, to
the exhalations of the flowers.  It is used for plants whose volatile
oils are too delicate to be separated by distillation. (Webster's
Revised Unabridged Dictionary (1913)).

This software aims to view many pictures with ease.  You can view
various formatted pictures and movies with several effects.

Formats you can view (which can be supported with proper libraries):

* BMP, GIF, JPEG, PCX, PNG, PNM, XBM, XPM, TGA, JPEG2000, WEBP
* animated GIF
* mng (old extention for png)
* mpeg (mpeg1, mpeg2)
* avi, ogg, ogm, (codec supported by avcodec, vorbis)

This software has a plugin architecture.  You can write plugins to
support new formatted pictures and movies.  Also, you can read
regular archive files, such as tar, gz, bz2, rar, and so on.


Requirements
------------

X server's depth _SHOULD_ be 16 or 24 (bpp 24, 32).  Other depths are
unsupported for now.  If you don't see a correct colored-image in these
supported depths, let me know with xdpyinfo's output.  When use in
depth 16, color mask should be 0xf800, 0x7e0, 0x1f.  Other masks might
be supported sometime.

These environments are checked:

* Linux (kernel 2.2/2.4/2.6/3.x/4.x/5.x + glibc-2.1-2.32, x86(-64))
* FreeBSD (4.1R, x86)

Other similar environments should work.  Please let me know if you try
on the same/other environments.


Compile
-------

GCC is the primary compiler. clang/llvm should be able to compile.

Basically, type './configure && make', and that's all.  If you get some
error messages, please let me know with config.log, your environment, and/or
such useful information to debug.

After successfully compiled, type 'sudo make install'.

You can pass several options to configure script.  You can see help
message by typing './configure --help'.

Plugins are compiled as shared objects.  A plugin's filename should end with
.so. If you get not .so but only .a, the chances are your system cannot /
doesn't create shared objects.


Command line
------------

Enfle is _very_ simple software (which doesn't use some toolkit or
something).  Pass filenames to arguments which you want to view.  If you
pass directory name, then files under that directory will be added
recursively.  Supported archives(such as .tar.gz) can be directly specified.

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

Enfle will also quit when it reaches the end of the pictures.

If you have a wheel on mouse, you can use it for Left, Right click.


Customize
---------

There is the configuration file in
$prefix/share/enfle/enfle.rc (normally, /usr/local/share/enfle/enfle.rc
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

        Hiroshi Takekawa <sian.ht@gmail.com>
                         <sian@big.or.jp>
                         <sian@fennel.org>


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
