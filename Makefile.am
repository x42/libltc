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
# fix doxygen man-page
	sed -i 's/\([( ]\)-\([0-9][0-9d) ]\)/\1\\-\2/g' doc/man/man3/ltc.h.3
	touch stamp-doxygen
