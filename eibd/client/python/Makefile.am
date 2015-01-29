CC = $(CC_FOR_BUILD)
CPPFLAGS = $(CPPFLAGS_FOR_BUILD)
CFLAGS = $(CFLAGS_FOR_BUILD)
LDFLAGS = $(LDFLAGS_FOR_BUILD)
LIBS = $(LIBS_FOR_BUILD)
EXEEXT = $(EXEEXT_FOR_BUILD)

AM_CPPFLAGS=-I$(top_srcdir)/eibd/include -I$(top_srcdir)/eibd/client -I$(top_builddir)/eibd/client

EXTRA_DIST = io.inc
CLEANFILES = gen.inc EIBConnection.py result.inc

result.inc : $(top_srcdir)/common/eibloadresult.h $(top_srcdir)/eibd/include/eibtypes.h
	cat $^  | egrep '#define.*(IMG_|XML|EIBD)' |sed -e 's/#define \+\(\(IMG_\|XML\|EIBD\)[A-Z_0-9]\+\) \+\([A-Z_a-z0-9\"\:\/\.\-]\+\)\(.*\)/\1 = \3/g' > $@

result2.inc : $(top_srcdir)/eibd/include/eibpriority.h
	cat $^  | egrep '.*= *[0-9]+.*' | sed -e 's/ *\([A-Z_0-9]\+\) *= *\([0-9]\).*/\1 = \2/g' > $@

noinst_PROGRAMS = gen

gen_SOURCES=gen.c arg.def def.def

pkgdata_DATA = EIBConnection.py

gen.inc : gen$(EXEEXT)
	./$< > $@

EIBConnection.py : io.inc gen.inc result.inc result2.inc
	cat $^ > $@

include_HEADERS= EIBConnection.py