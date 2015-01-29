bin_PROGRAMS = eibd
AM_CPPFLAGS=-I$(top_srcdir)/eibd/include -I$(top_srcdir)/eibd/libserver -I$(top_srcdir)/eibd/backend -I$(top_srcdir)/common -I$(top_srcdir)/eibd/usb $(PTH_CPPFLAGS) $(XML_CPPFLAGS) $(XSLT_CPPFLAGS)
eibd_LDADD=-L$(top_srcdir)/common -L$(top_srcdir)/eibd/libserver -L$(top_srcdir)/eibd/backend -leibbackend -leibstack -leibcommon ../usb/libeibdusb.a $(LIBUSB_LIBS) $(PTH_LDFLAGS) $(PTH_LIBS) $(XML_LIBS) $(XSLT_LIBS) 
BACKEND_CONF= b-EIBNETIP.h b-FT12.h b-PEI16.h b-PEI16s.h b-TPUART.h b-TPUARTs.h b-EIBNETIPTUNNEL.h b-USB.h
eibd_SOURCES=eibd.cpp eibdstate.cpp layer2conf.h layer2create.h $(BACKEND_CONF)

