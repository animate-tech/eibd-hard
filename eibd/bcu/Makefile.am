bin_PROGRAMS= bcuaddrtab
#bcuaddrtab bcuread
# support removed, code was not working anyway & it would need massive rewrite to work properly, bcuaddrtab partially rewritten
# but to get all the timings correctly on libusb all the layers must be brought up to where the stack is up as EMI(1/2)
AM_CPPFLAGS=-I$(top_srcdir)/common -I$(top_srcdir)/eibd/include -I$(top_srcdir)/eibd/libserver -I$(top_srcdir)/eibd/backend -I$(top_srcdir)/eibd/usb $(PTH_CPPFLAGS) $(XML_CPPFLAGS) $(XSLT_CPPFLAGS)
BACKEND_CONF= l-FT12.h l-PEI16.h l-PEI16s.h l-USB.h

noinst_LIBRARIES=libbcu.a
libbcu_a_SOURCES=addrtab.cpp addrtab.h lowlevelcreate.h lowlevelconf.h $(BACKEND_CONF)

bcuaddrtab_SOURCES=bcuaddrtab.cpp
bcuaddrtab_LDADD=-L$(top_srcdir)/common -L$(top_srcdir)/eibd/backend -L$(top_srcdir)/eibd/libserver libbcu.a -leibbackend -leibstack -leibcommon ../usb/libeibdusb.a $(LIBUSB_LIBS) $(PTH_LDFLAGS) $(PTH_LIBS) $(XML_LIBS) $(XSLT_LIBS)

bcuread_SOURCES=bcuread.cpp
bcuread_LDADD=-L$(top_srcdir)/common -L$(top_srcdir)/eibd/backend -L$(top_srcdir)/eibd/libserver libbcu.a -leibbackend -leibstack -leibcommon $../usb/libeibdusb.a (LIBUSB_LIBS) $(PTH_LDFLAGS) $(PTH_LIBS) $(XML_LIBS) $(XSLT_LIBS)

