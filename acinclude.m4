dnl acinclude.m4 -- Enfle local m4 macro
dnl
dnl (C)Copyright 1999, 2000, 2001, 2002 by Hiroshi Takekawa
dnl This file is part of Enfle.
dnl
dnl Last Modified: Sun Jun 23 15:11:25 2002.
dnl $Id: acinclude.m4,v 1.3 2002/08/02 14:00:28 sian Exp $
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

AC_DEFUN(ENFLE_APPEND, [ $1="$$1 $2" ])
AC_DEFUN(ENFLE_PREPEND, [ $1="$2 $$1" ])
AC_DEFUN(ENFLE_REMOVE, [ $1=`echo $$1 | sed "s%$2  *%%g"` ])

dnl ENFLE_PLUGIN_ENABLE(list, pluginname, type)
AC_DEFUN(ENFLE_PLUGIN_ENABLE, [
  pp=`echo $p | sed 's/^-//'`
  if test "$pp" = "$p"; then
    eval "count_$3=\`expr 0\$count_$3 + 1\`"
    ENFLE_APPEND($1, $3/$pp)
    if test "$1" = "Static_plugins"; then
      kind=$3
      eval "static_${kind}_${p}=yes"
      ENFLE_APPEND(Static_plugin_objs, [../plugins/${kind}/${p}/${kind}_${p}.la])
      ENFLE_APPEND(Static_plugin_proto, [void * ${kind}_${p}_entry(void);void ${kind}_${p}_exit(void *);])
      ENFLE_APPEND(Static_plugin_add, [enfle_plugins_add(eps, ${kind}_${p}_entry, ${kind}_${p}_exit, &type);])
    fi
  else
    eval "count_$3=\`expr 0\$count_$3 - 1\`"
    ENFLE_REMOVE($1, $3/$pp)
    if test "$1" = "Static_plugins"; then
      kind=$3
      eval "static_${kind}_${p}=no"
      ENFLE_REMOVE(Static_plugin_objs, [../plugins/${kind}/${p}/${kind}_${p}.la])
      ENFLE_REMOVE(Static_plugin_proto, [void * ${kind}_${p}_entry(void);void ${kind}_${p}_exit(void *);])
      ENFLE_REMOVE(Static_plugin_add, [enfle_plugins_add(eps, ${kind}_${p}_entry, ${kind}_${p}_exit, &type);])
    fi
  fi
])

dnl ENFLE_PLUGIN_ENABLER(list, -static or not, type, tabs)
AC_DEFUN(ENFLE_PLUGIN_ENABLER, [
AC_ARG_ENABLE($3$2,
    [  --enable-$3$2$4 Enable $3$2 plugins.],
    [ for p in $enableval; do
        ENFLE_PLUGIN_ENABLE($1, $p, $3)
      done])
])

dnl ENFLE_LINKED_PLUGIN_ENABLER(type, tabs)
AC_DEFUN(ENFLE_LINKED_PLUGIN_ENABLER, [
  ENFLE_PLUGIN_ENABLER(Static_plugins, -static, $1, [$2])
])

AC_DEFUN(ENFLE_REMOVE_DUP, [
  $1=`echo "$$1" | tr -s " " "\n" | sort | uniq | tr "\n" " " | sed 's/^ *//' | sed 's/ *$//'`
])

dnl for memory object and mpeg2dec
AC_DEFUN([ENFLE_MAX_ALIGN], [
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

AC_DEFUN([AM_WITH_DMALLOCTH], [
  AC_MSG_CHECKING(if malloc debugging with threading is wanted)
  AC_ARG_WITH(dmallocth,
    [  --with-dmallocth        use dmalloc with threading.],
    [ if test "$withval" = "yes"; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(WITH_DMALLOCTH, 1,
                  [Define if use the dmalloc debugging malloc package with threading])
        LIBS="$LIBS -ldmallocth"
      else
        AC_MSG_RESULT(no)
      fi
    ], [AC_MSG_RESULT(no)])
])
