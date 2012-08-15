AUTOMAKE_OPTIONS = foreign

SUBDIRS = @subdirs@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ltc.pc

EXTRA_DIST= \
	ltc.pc.in Doxyfile.in \
	README.md ChangeLog \
	COPYING INSTALL AUTHORS

CLEANFILES = stamp-doxygen stamp-doc

dox: stamp-doxygen

stamp-doxygen: src/ltc.h doc/mainpage.dox Doxyfile
	$(DOXYGEN) Doxyfile
	cp doc/favicon.ico doc/html/
	touch stamp-doxygen
