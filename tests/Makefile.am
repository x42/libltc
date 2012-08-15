check_PROGRAMS = ltcencode ltcdecode

CLEANFILES = output.raw

LIBLTCDIR =../src
INCLUDES = -I$(srcdir)/$(LIBLTCDIR)

ltcdecode_SOURCES = ltcdecode.c
ltcdecode_LDADD = $(LIBLTCDIR)/libltc.la -lm
ltcdecode_CFLAGS=-g -Wall

ltcencode_SOURCES = ltcencode.c
ltcencode_CFLAGS=-g -Wall
ltcencode_LDADD = $(LIBLTCDIR)/libltc.la -lm

check: $(check_PROGRAMS)
	 date
	 uname -a
	 ./ltcencode output.raw
	 ./ltcdecode output.raw | diff -q expect_48k_2sec.txt -
	 @echo "-----------------------------------------------------------------"
	 ./ltcencode output.raw 192000
	 ./ltcdecode output.raw 7680 | diff -q expect_96k_2sec.txt -
	 @echo "-----------------------------------------------------------------"
	 @echo "  ${PACKAGE}-${VERSION} passed all tests."
	 @echo "-----------------------------------------------------------------"
