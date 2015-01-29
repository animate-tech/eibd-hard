if HAVE_FT12
FT12 = ft12.h ft12.cpp lowlatency.h lowlatency.cpp
else
FT12 =
endif

if HAVE_EIBNETIP
EIBNETIP = eibnetrouter.h eibnetrouter.cpp
else
EIBNETIP =
endif

if HAVE_EIBNETIPTUNNEL
EIBNETIPTUNNEL = eibnettunnel.h eibnettunnel.cpp
else
EIBNETIPTUNNEL =
endif

if HAVE_USB
USB = usbif.h usbif.cpp
else
USB =
endif

lib_LTLIBRARIES = libeibbackend.la
AM_CPPFLAGS=-I$(top_srcdir)/eibd/libserver -I$(top_srcdir)/eibd/include -I$(top_srcdir)/common -I$(top_srcdir)/eibd/usb $(PTH_CPPFLAGS) $(XML_CPPFLAGS) $(XSLT_CPPFLAGS)

libeibbackend_la_SOURCES= $(FT12) $(PEI16) $(TPUART) $(PEI16s) $(TPUARTs) $(EIBNETIP) $(EIBNETIPTUNNEL) $(USB) dummy.cpp
libeibbackend_la_LDFLAGS=-version-info 0:1:0

#libeibbackend_a_SOURCES= $(libeibbackend_la_SOURCES)
#noinst_LIBRARIES=libeibbackend.a
