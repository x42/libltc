lib_LTLIBRARIES = libltc.la
include_HEADERS = ltc.h

libltc_la_SOURCES=ltc.c config.h decoder.h decoder.c encoder.h encoder.c timecode.c
libltc_la_LDFLAGS=@LIBLTC_LDFLAGS@ -version-info @VERSION_INFO@
libltc_la_LIBADD=-lm
libltc_la_CFLAGS=@LIBLTC_CFLAGS@
