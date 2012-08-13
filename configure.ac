AC_PREREQ(2.57)
AC_INIT([libltc], [-], [robin@gareus.org], [], [http://ltcsmpte.sourceforge.net/])

AC_PATH_PROG(SED, sed, "", $PATH:/bin:/usr/bin:/usr/local/bin)
if test -z "$SED"; then
  AC_MSG_WARN([sed was not found])
fi

AC_MSG_CHECKING([libltc version])
VERSION=`$SED -ne 's/^#define LIBLTC_VERSION "\(.*\)"/\1/p' ${srcdir}/src/ltc.h 2>/dev/null`
AC_MSG_RESULT($VERSION)
if test -z "$VERSION"; then
  AC_MSG_ERROR([version number can not be retrieved from src/ltc.h])
fi

VERSION_CUR=`$SED -ne 's/^#define LIBLTC_CUR\s*\([0-9]*\)/\1/p' ${srcdir}/src/ltc.h 2>/dev/null`
VERSION_REV=`$SED -ne 's/^#define LIBLTC_REV\s*\([0-9]*\)/\1/p' ${srcdir}/src/ltc.h 2>/dev/null`
VERSION_AGE=`$SED -ne 's/^#define LIBLTC_AGE\s*\([0-9]*\)/\1/p' ${srcdir}/src/ltc.h 2>/dev/null`
VERSION_INFO=${VERSION_CUR}:${VERSION_REV}:${VERSION_AGE}

AC_CONFIG_SRCDIR([src/ltc.c])
dnl AC_CONFIG_TESTDIR([tests])
AC_CANONICAL_TARGET([])
AC_COPYRIGHT([Copyright (C) Robin Gareus 2007-2012])
AM_INIT_AUTOMAKE(libltc,$VERSION)
AM_CONFIG_HEADER(src/config.h)

AC_SUBST(VERSION)
AC_SUBST(VERSION_INFO)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL
AM_PROG_CC_C_O
AC_LIBTOOL_WIN32_DLL
AC_CONFIG_MACRO_DIR([m4])

dnl *** misc complier/linker flags ***
LIBSMPTE_CFLAGS="-Wall"
LIBSMPTE_LDFLAGS="-export-symbols-regex '^(SMPTE).*'"

AC_MSG_CHECKING([if -Wl,--as-needed works])
LFDLAGS_save=$LDFLAGS
LDFLAGS="$LDFLAGS -Wl,--as-needed"
AC_TRY_LINK([], [],
            [
             AC_MSG_RESULT([yes])
             LIBSMPTE_LDFLAGS="$LIBSMPTE_LDFLAGS -Wl,--as-needed"
            ], 
            [AC_MSG_RESULT([no])])
LDFLAGS=$LDFLAGS_save

AC_SUBST(LIBLTC_CFLAGS)
AC_SUBST(LIBLTC_LDFLAGS)

dnl *** configuration options ***
AC_ARG_ENABLE(enable_date,  AC_HELP_STRING([--enable-date],  [Compile with SMPTE date support (http://www.barney-wol.net/time/timecode.html)]))

AH_TEMPLATE([ENABLE_DATE], [Define as 1 to enabled SMPTE date support])

if test "x${enable_date}" == "xyes"; then
	AC_DEFINE(ENABLE_DATE)
else 
	enable_date=no
fi

AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/param.h sys/time.h time.h sys/mkdev.h sys/sysmacros.h string.h memory.h fcntl.h dirent.h sys/ndir.h ndir.h alloca.h locale.h stdint.h sys/times.h)

AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS
AC_FUNC_CLOSEDIR_VOID

dnl *** doxygen ***
AC_ARG_VAR(DOXYGEN, Doxygen)
AC_PATH_PROG(DOXYGEN, doxygen, no)

if test "$DOXYGEN" != "no"; then
  DOXMAKE='run "make dox" to generate API html reference: doc/html/index.html'
fi

dnl *** graphviz ***
dnl (needed for Doxyfile.in)
AC_ARG_VAR(DOT, The 'dot' program from graphviz)
AC_PATH_PROG(DOT, dot, no)
if test "$DOT" != "no"; then
  HAVEDOT=YES
  DOTPATH=$( dirname "$DOT" )
else
  HAVEDOT=NO
fi
AC_SUBST(HAVEDOT)
AC_SUBST(DOTPATH)
AC_SUBST(GMP_LIBS)


dnl subdirs="src doc tests"
subdirs="src"
AC_SUBST(subdirs)

dnl AC_OUTPUT(Makefile src/Makefile doc/Makefile tests/Makefile libltc.lsm ltc.pc Doxyfile)
AC_OUTPUT(Makefile src/Makefile ltc.pc)

AC_MSG_NOTICE([

 libltc configured:
 -----------------------

  version:             $VERSION
  interface revision:  $VERSION_INFO

  doxygen:             $DOXYGEN
  installation prefix: $prefix

  SMPTE date support : ${enable_date}

 type "make" followed my "make install" as root.
 run "make check" to perform selftests.
 $DOXMAKE
])
