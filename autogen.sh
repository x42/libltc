#!/bin/sh

aclocal && autoheader && libtoolize --copy -f && autoconf && automake --gnu --add-missing --copy -f && ./configure $@
