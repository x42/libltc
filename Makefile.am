SUBDIRS = @subdirs@
EXTRA_DIST=ltc.pc.in Doxyfile.in testdata/smpte.raw COPYING

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ltc.pc

CLEANFILES = stamp-doxygen stamp-doc

dox: stamp-doxygen

stamp-doxygen: src/ltc.h doc/mainpage.dox Doxyfile
	$(DOXYGEN) Doxyfile
	touch stamp-doxygen
