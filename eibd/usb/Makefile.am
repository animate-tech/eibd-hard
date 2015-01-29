EXTRA_DIST=README COPYING

EIBD_USB = usb.cpp usb.h

if LINUX_API
AM_CFLAGS = -DTHREADS_POSIX 
TARGET = linux_usbfs.c linux_usbfs.h
else
TARGET =
endif

if HAVE_USB
if USE_SYSTEMLIBUSB
LIBUSB = $(EIBD_USB) 
else
LIBUSB = $(EIBD_USB) libusb.h libusbi.h core.c descriptor.c io.c sync.c os/threads_posix.h os/poll_posix.h $(TARGET) 
endif
PROG = findknxusb
else
LIBUSB =
PROG =
endif

bin_PROGRAMS= $(PROG)

findknxusb_SOURCES= findknxusb.cpp
noinst_LIBRARIES=libeibdusb.a
if USE_SYSTEMLIBUSB
findknxusb_LDADD=$(LIBUSB_LIBS) $(PTH_LIBS) $(PTH_LDFLAGS)
libeibdusb_a_SOURCES=$(EIBD_USB) dummy.cpp
else
findknxusb_LDADD=libeibdusb.a $(PTH_LIBS) $(PTH_LDFLAGS)
libeibdusb_a_SOURCES=$(LIBUSB) dummy.cpp
endif
AM_CPPFLAGS=-I$(top_srcdir)/eibd/include -I$(top_srcdir)/eibd/libserver -I$(top_srcdir)/common $(PTH_CPPFLAGS) $(XML_CPPFLAGS) $(XSLT_CPPFLAGS)
