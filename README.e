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

Xサーバのdepthが 16, 24(bpp 24, 32)では動くと思われます。その他はいま
のところunsupportedです。今あげたdepthで表示できなかった場合はxdpyinfo
の出力と詳しい環境をお知らせください。なお、depth 16のmaskが
0xf800,0x7e0,0x1f でない場合ばけると思われます。今のところこれは仕様で
すが、そのうち改良されるかもしれません。

今のところ以下の環境での動作が確認されています。

Linux(2.4.0-test8 w/ glibc-2.1)
Linux(2.4.0-test11 w/ glibc-2.2)
FreeBSD-4.1R

私のメインの開発環境は2001/01/15現在 以下の通りです。

Kernel: Linux-2.4.0
CPU: Celeron/366 (running at 550)
X server: XFree86-4.0.2
Video: Matrox MillenniumII/AGP 8M
Compiler: gcc version 2.95.2 19991024 (release)
libc: glibc-2.2


3.コンパイル

コンパイルにはgccが必要です。コードにgccのextensionを使っています。
~~~~~~~~~~~~~~~~~~~~~~~~~~~
基本はconfigure して make するだけです。うまくいかない際には、どのよう
なエラーがでたか報告していただければ幸いです。configureが生成する
config.logなども参考になります。報告先は後述してあります。

makeが通ったら、make installするか、srcに移動して./enfleとして起動して
ください。

configureスクリプトの指定できるオプションは、./configure --helpで詳細
がみれます。MMXが使えて、かつ、nasmがinstallされていて、しかもdepth16
で使いたいという場合には、--enable-nasmとするといいかもしれません。

プラグインはshared objectとしてcompileされます。ファイル名は.soでおわっ
ている必要があります。compileは通るが.so以外になってしまって動かないと
いう方がいらっしゃいましたら御一報ください。ただし.aができている場合に
はshared objectが作れない/使えない環境である可能性もあります。

susieのpluginを使いたい場合には--enable-spiが必要です。
ungif pluginを使いたい場合には--with-ungifが必要です。

4.コマンドライン

Enfleは非常にシンプルなプログラムです。引数に表示したいファイル名を続
けて指定するだけです。ディレクトリを指定した場合はその下のファイルを再
帰的に追加します。対応してるアーカイブを指定した場合には、その中味も読
まれます。


5.使い方

左クリック,n,space	次の画像へ
右クリック,b		前の画像へ
中クリック,q		終了
f			フルスクリーンon/off
S(shift+s)		拡大アルゴリズムの変更(nointerpolate/bilinear)
m			2倍拡大
M(shift+m)		画像の長辺が画面いっぱいになるように拡大
Alt+m			画像の短辺が画面いっぱいになるように拡大

また、全ての画像を表示し終わっても終了します。

マウスのwheelにも対応しています。今のところ、前にまわすと次の画像、後
にまわると前の画像にうつります。

6.カスタマイズ

installすると$prefix/share/enfle/enfle.rc(だいたい
/usr/share/enfle/enfle.rcとかそのあたり)という設定ファイルがinstallさ
れます。カスタマイズするには、

mkdir ~/.enfle
cp /usr/share/enfle/enfle.rc ~/.enfle/config

とコピーして~/.enfle/configを書きかえてください。


7.その他

w3mの外部ビューアとしてenfleを使う方法

~/.mailcap に
image/*;enfle %s
なんて感じに書いてください。システム全体でenfleにしてやるぜ、という方は
/etc/mailcapにでも書いてください。


8.連絡先

バグの報告、御意見、御感想は歓迎します。Enfleに関するMLがあるので、そ
ちらをご利用ください。自動登録となります。講読するには、

 subscribe あなたの名前

とだけ本文に書いて、<enfle-ctl@fennel.org>に送ってください。

 例: subscribe Hiroshi Takekawa

その後折り返し届いたメールにしたがってconfirm(登録の確認)をしてくださ
い。投稿されたメールは私に各種権利があるものとします。(ログの公開、検
索の提供等を楽にするためくらいの意味しかないです)

どうしてもという場合には私に直接でも結構です。

        Hiroshi Takekawa <sian@big.or.jp>
                         <sian@fennel.org>


9.配布について

EnfleはGNU GENERAL PUBLIC LICENSE Version 2に従います。Version 2のみで、
それ以降のVersionにするオプションはいまのところありません。GPL2につい
てはCOPYINGをお読みください。補助として日本語訳COPYING.jもつけてありま
す。
