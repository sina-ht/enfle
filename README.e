***************************************************

     Simple Plugin-based Graphic Loader Enfle
			*** DEVELOPING VERSION ***

 (C)Copyright 1998, 99, 2000, 2001 by Hiroshi Takekawa.

     Last Modified: Fri Feb  2 01:04:33 2001.

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

***************************************************

 This software is based in part on the work of the Independent JPEG Group

 -- OK, libjpeg is only linked, not distributed with this archive. You
 -- should install it.

 The Graphics Interchange Format(c) is
 the Copyright property of CompuServe Incorporated.
 GIF(sm) is a Service Mark property of CompuServe Incorporated.

 -- OK, libungif is only linked, not distributed with this
 -- archive. Moreover, libungif is not used by default, for avoiding
 -- possible patent infringement. It's up to you whether use it or not.

 This software includes mpglib by Michael Hipp.

 This software includes libmpeg2 by Aaron Holtzman. For this part, you
 can apply GPL2 or later version.

 This product includes software developed by or derived from software
 developed by Project Mayo.

 In effect, only libdivxdecore is derived from software developed by
 Project Mayo. OpenDivX plugin uses the library, but all other portions
 of this software does not.

 This archive contains libdivxdecore from Project Mayo. OpenDivX player
 plugin uses libdivxdecore and is released as a "Larger Work" under
 that license. Consistent with that license, OpenDivX plugin is
 released under the GNU General Public License. The OpenDivX license
 can be found at OpenDivX-LICENSE in source tree, or
 http://www.projectmayo.com/opendivx/docs.php.

***************************************************

OK, legal part is over. The rest is fun.

1. Blurb

This software aims to view many pictures just clicking. You can view
various formatted pictures and movies.

 Formats you can view:
 GIF
 JPEG
 PNG
 BMP
 spi(unstable, any formats which (some of) susie plugins support)
 animated GIF
 mng
 mpeg(unstable, mpeg1, mpeg2, including audio support)
 avi,asf(unstable, any formats which avifile supports)

This software has plugin architecture. You can write plugins to loader new formatted pictures and movies. Also, you can read irregular files, such as tar, gz,  bz2, and so on.

***
GIF decoder must perform LZW decompression. Unfortunately, it's
patented. (Yes, many think that patent doesn't cover
decompression. But Unisys won't permit). So, this software never
includes LZW decompression code, though I've written. UNGIF plugin
which uses libungif is included. But, I'm not sure that usage of this
library is legal. It's up to you whether use this or not. You must
explicit pass --with-ungif option to configure script.
***

Some plugins use libraries which is not included in distribution. You
should install corresponding libraries to use them.

JPEG: jpegsrc-6b (libjpeg)
PNG: libpng-1.0.2 or later (recommended: 1.0.9)
gz: zlib-1.1.3 or later
bz2: bzip2-1.0.0 or later
ungif: libungif-3.1.0 or later (recommended: 4.1.0)
mng: libmng
mpeg: libmpeg3 or mpeg_lib(needs patch)
avi: avifile(may need cvs version)

2. Requirements

X�����Ф�depth�� 16, 24(bpp 24, 32)�Ǥ�ư���Ȼפ��ޤ�������¾�Ϥ���
�ΤȤ���unsupported�Ǥ�����������depth��ɽ���Ǥ��ʤ��ä�����xdpyinfo
�ν��ϤȾܤ����Ķ����Τ餻�����������ʤ���depth 16��mask��
0xf800,0x7e0,0x1f �Ǥʤ����Ф���Ȼפ��ޤ������ΤȤ�������ϻ��ͤ�
���������Τ������ɤ���뤫�⤷��ޤ���

���ΤȤ����ʲ��δĶ��Ǥ�ư���ǧ����Ƥ��ޤ���

Linux(2.4.0-test8 w/ glibc-2.1)
Linux(2.4.0-test11 w/ glibc-2.2)
FreeBSD-4.1R

��Υᥤ��γ�ȯ�Ķ���2001/01/15���� �ʲ����̤�Ǥ���

Kernel: Linux-2.4.0
CPU: Celeron/366 (running at 550)
X server: XFree86-4.0.2
Video: Matrox MillenniumII/AGP 8M
Compiler: gcc version 2.95.2 19991024 (release)
libc: glibc-2.2


3.����ѥ���

����ѥ���ˤ�gcc��ɬ�פǤ��������ɤ�gcc��extension��ȤäƤ��ޤ���
~~~~~~~~~~~~~~~~~~~~~~~~~~~
���ܤ�configure ���� make ��������Ǥ������ޤ������ʤ��ݤˤϡ��ɤΤ褦
�ʥ��顼���Ǥ�����𤷤Ƥ���������й����Ǥ���configure����������
config.log�ʤɤ⻲�ͤˤʤ�ޤ��������ϸ�Ҥ��Ƥ���ޤ���

make���̤ä��顢make install���뤫��src�˰�ư����./enfle�Ȥ��Ƶ�ư����
����������

configure������ץȤλ���Ǥ��륪�ץ����ϡ�./configure --help�Ǿܺ�
���ߤ�ޤ���MMX���Ȥ��ơ����ġ�nasm��install����Ƥ��ơ�������depth16
�ǻȤ������Ȥ������ˤϡ�--enable-nasm�Ȥ���Ȥ������⤷��ޤ���

�ץ饰�����shared object�Ȥ���compile����ޤ����ե�����̾��.so�Ǥ����
�Ƥ���ɬ�פ�����ޤ���compile���̤뤬.so�ʳ��ˤʤäƤ��ޤä�ư���ʤ���
������������ä��㤤�ޤ��������󤯤�������������.a���Ǥ��Ƥ������
��shared object�����ʤ�/�Ȥ��ʤ��Ķ��Ǥ����ǽ���⤢��ޤ���

susie��plugin��Ȥ��������ˤ�--enable-spi��ɬ�פǤ���
ungif plugin��Ȥ��������ˤ�--with-ungif��ɬ�פǤ���

4.���ޥ�ɥ饤��

Enfle�����˥���ץ�ʥץ������Ǥ���������ɽ���������ե�����̾��³
���ƻ��ꤹ������Ǥ����ǥ��쥯�ȥ����ꤷ�����Ϥ��β��Υե�������
��Ū���ɲä��ޤ����б����Ƥ륢�������֤���ꤷ�����ˤϡ�������̣����
�ޤ�ޤ���


5.�Ȥ���

������å�,n,space	���β�����
������å�,b		���β�����
�楯��å�,q		��λ
f			�ե륹���꡼��on/off
S(shift+s)		���祢�르�ꥺ����ѹ�(nointerpolate/bilinear)
m			2�ܳ���
M(shift+m)		������Ĺ�դ����̤��äѤ��ˤʤ�褦�˳���
Alt+m			������û�դ����̤��äѤ��ˤʤ�褦�˳���

�ޤ������Ƥβ�����ɽ��������äƤ⽪λ���ޤ���

�ޥ�����wheel�ˤ��б����Ƥ��ޤ������ΤȤ��������ˤޤ魯�ȼ��β�������
�ˤޤ������β����ˤ��Ĥ�ޤ���

6.�������ޥ���

install�����$prefix/share/enfle/enfle.rc(��������
/usr/share/enfle/enfle.rc�Ȥ����Τ�����)�Ȥ�������ե����뤬install��
��ޤ����������ޥ�������ˤϡ�

mkdir ~/.enfle
cp /usr/share/enfle/enfle.rc ~/.enfle/config

�ȥ��ԡ�����~/.enfle/config��񤭤����Ƥ���������


7.����¾

w3m�γ����ӥ塼���Ȥ���enfle��Ȥ���ˡ

~/.mailcap ��
image/*;enfle %s
�ʤ�ƴ����˽񤤤Ƥ��������������ƥ����Τ�enfle�ˤ��Ƥ�뤼���Ȥ�������
/etc/mailcap�ˤǤ�񤤤Ƥ���������


8.Ϣ����

�Х�����𡢸�ո����洶�ۤϴ��ޤ��ޤ���Enfle�˴ؤ���ML������Τǡ���
��������Ѥ�����������ư��Ͽ�Ȥʤ�ޤ������ɤ���ˤϡ�

 subscribe ���ʤ���̾��

�Ȥ�����ʸ�˽񤤤ơ�<enfle-ctl@fennel.org>�����äƤ���������

 ��: subscribe Hiroshi Takekawa

���θ��ޤ��֤��Ϥ����᡼��ˤ������ä�confirm(��Ͽ�γ�ǧ)�򤷤Ƥ�����
������Ƥ��줿�᡼��ϻ�˳Ƽ︢���������ΤȤ��ޤ���(�����θ�������
����������ڤˤ��뤿�᤯�餤�ΰ�̣�����ʤ��Ǥ�)

�ɤ����Ƥ�Ȥ������ˤϻ��ľ�ܤǤ�빽�Ǥ���

        Hiroshi Takekawa <sian@big.or.jp>
                         <sian@fennel.org>


9.���ۤˤĤ���

Enfle��GNU GENERAL PUBLIC LICENSE Version 2�˽����ޤ���Version 2�Τߤǡ�
����ʹߤ�Version�ˤ��륪�ץ����Ϥ��ޤΤȤ�������ޤ���GPL2�ˤĤ�
�Ƥ�COPYING���ɤߤ�������������Ȥ������ܸ���COPYING.j��Ĥ��Ƥ����
����