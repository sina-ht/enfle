dnl acinclude.m4 -- Enfle local m4 macro
dnl
dnl (C)Copyright 2001 by Hiroshi Takekawa
dnl This file is part of Enfle.
dnl
dnl Last Modified: Mon Feb 19 21:10:36 2001.
dnl $Id: acinclude.m4,v 1.1 2001/02/19 16:41:49 sian Exp $
dnl
dnl Enfle is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU General Public License version 2 as
dnl published by the Free Software Foundation.
dnl
dnl Enfle is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

dnl for mpeg2dec
AC_DEFUN(ENFLE_MAX_ALIGN, [
  AC_MSG_CHECKING(maximum value for __attribute__((aligned())))
  for align_value in 2 4 8 16 32 64; do
    AC_TRY_COMPILE([],
      [ static char c __attribute__ ((aligned($align_value))) = 0; return c; ],
      [ align_value_max=$align_value ]
    )
  done
  if test "$align_value_max" != "0"; then
    AC_MSG_RESULT_UNQUOTED($align_value_max)
    AC_DEFINE_UNQUOTED(ATTRIBUTE_ALIGNED_MAX,
                       $align_value_max, [ Maximum number of supported alignment ])
  else
    AC_MSG_RESULT(no support)
  fi
])
