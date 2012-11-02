#!/bin/sh

if test -z "$LIBTOOLIZE"; then
  if test "$(uname)" == "Darwin"; then
    LIBTOOLIZE="glibtoolize"
  else
    LIBTOOLIZE="libtoolize"
  fi
fi

aclocal && autoheader && $LIBTOOLIZE --copy -f && autoconf && automake --gnu --add-missing --copy -f && ./configure $@
