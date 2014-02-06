AC_PREREQ(2.57)
AC_INIT([libltc], [-], [robin@gareus.org], [], [http://x42.github.com/libltc/])

AC_PATH_PROG(SED, sed, "", $PATH:/bin:/usr/bin:/usr/local/bin)
if test -z "$SED"; then
  AC_MSG_ERROR([sed was not found -- sed is requied to extract version information])
fi

AC_MSG_CHECKING([libltc version])
VERSION=`$SED -ne 's/^#define LIBLTC_VERSION "\(.*\)"/\1/p' ${srcdir}/src/ltc.h 2>/dev/null`
AC_MSG_RESULT($VERSION)
if test -z "$VERSION"; then
  AC_MSG_ERROR([version number can not be retrieved from src/ltc.h])
fi

VERSION_CUR=`$SED -ne 's/^#define LIBLTC_CUR *\([0-9]*\)/\1/p' ${srcdir}/src/ltc.h 2>/dev/null`
VERSION_REV=`$SED -ne 's/^#define LIBLTC_REV *\([0-9]*\)/\1/p' ${srcdir}/src/ltc.h 2>/dev/null`
VERSION_AGE=`$SED -ne 's/^#define LIBLTC_AGE *\([0-9]*\)/\1/p' ${srcdir}/src/ltc.h 2>/dev/null`
VERSION_INFO=${VERSION_CUR}:${VERSION_REV}:${VERSION_AGE}

AC_CONFIG_SRCDIR([src/ltc.c])
AC_CONFIG_TESTDIR([tests])
AC_CANONICAL_TARGET([])
AC_COPYRIGHT([Copyright (C) Robin Gareus 2006-2012])

AM_INIT_AUTOMAKE(libltc,$VERSION)
AC_CONFIG_HEADERS([src/config.h])

AC_CANONICAL_HOST

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_LIBTOOL_WIN32_DLL

dnl *** misc complier/linker flags ***
LIBLTC_CFLAGS="-Wall"
LIBLTC_LDFLAGS="-export-symbols-regex '^(ltc_).*'"

case $target_os in
  *mingw32*|*win*)
    LDFLAGS="-no-undefined"
		;;
  *)
    AC_MSG_CHECKING([if -Wl,--as-needed works])
    LDFLAGS_save=$LDFLAGS
    LDFLAGS="$LDFLAGS -Wl,--as-needed"
    AC_TRY_LINK([], [],
                [
                 AC_MSG_RESULT([yes])
                 LIBLTC_LDFLAGS="$LIBLTC_LDFLAGS -Wl,--as-needed"
                ],
                [AC_MSG_RESULT([no])])
    LDFLAGS=$LDFLAGS_save
  ;;
esac

dnl *** check for dependencies ***
AC_CHECK_HEADERS(stdio.h stdlib.h string.h unistd.h math.h stdint.h)

dnl *** check for doxygen ***
AC_ARG_VAR(DOXYGEN, Doxygen)
AC_PATH_PROG(DOXYGEN, doxygen, no)

if test "$DOXYGEN" != "no"; then
  DOXMAKE='run "make dox" to generate API html reference: doc/html/index.html'
fi

subdirs="src doc tests"

AC_SUBST(subdirs)
AC_SUBST(VERSION)
AC_SUBST(VERSION_INFO)
AC_SUBST(LIBLTC_CFLAGS)
AC_SUBST(LIBLTC_LDFLAGS)

AC_OUTPUT(Makefile src/Makefile doc/Makefile tests/Makefile ltc.pc Doxyfile)

AC_MSG_NOTICE([

 libltc configured:
 -----------------------

  version:             $VERSION
  interface revision:  $VERSION_INFO

  doxygen:             $DOXYGEN
  installation prefix: $prefix

 type "make" followed my "make install" as root.
 run "make check" to perform selftests.
 $DOXMAKE
])
