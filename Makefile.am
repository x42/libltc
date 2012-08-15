AUTOMAKE_OPTIONS = foreign
SUBDIRS = @subdirs@
EXTRA_DIST=ltc.pc.in Doxyfile.in COPYING INSTALL README.md \
	tests/example_encode.c \
	tests/timecode.raw tests/timecode.txt \
	tests/expect_48k_2sec.txt tests/expect_96k_2sec.txt

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ltc.pc

CLEANFILES = stamp-doxygen stamp-doc

dox: stamp-doxygen

stamp-doxygen: src/ltc.h doc/mainpage.dox Doxyfile
	$(DOXYGEN) Doxyfile
	cp doc/favicon.ico doc/html/
	touch stamp-doxygen
