AM_CPPFLAGS=-I$(top_srcdir)/eibd/libserver -I$(top_srcdir)/eibd/include -I$(top_srcdir)/common $(PTH_CPPFLAGS)  $(XML_CPPFLAGS) $(XSLT_CPPFLAGS)

bin_PROGRAMS=$(EIBNET)

if HAVE_SOURCE_INFO
EIBNET=eibnetdescribe eibnetsearch
else
EIBNET=
endif
eibnetdescribe_SOURCES=eibnetdescribe.cpp
eibnetdescribe_LDADD=-L$(top_srcdir)/common -L$(top_srcdir)/eibd/libserver  -leibstack -leibcommon $(PTH_LDFLAGS) $(PTH_LIBS) $(XML_LIBS) $(XSLT_LIBS)

eibnetsearch_SOURCES=eibnetsearch.cpp
eibnetsearch_LDADD=-L$(top_srcdir)/common -L$(top_srcdir)/eibd/libserver -leibstack -leibcommon $(PTH_LDFLAGS) $(PTH_LIBS) $(XML_LIBS) $(XSLT_LIBS)

