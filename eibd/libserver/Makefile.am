lib_LTLIBRARIES = libeibstack.la
AM_CPPFLAGS=-I$(top_srcdir)/eibd/include -I$(top_srcdir)/common $(PTH_CPPFLAGS) $(XML_CPPFLAGS) $(XSLT_CPPFLAGS)

COMMON=classinterfaces.h classinterfaces.cpp exception.h queue.h queue.cpp common.h common.cpp threads.h threads.cpp trace.h trace.cpp c_format.h c_format.cpp timeval.h timeval.cpp
PDUs=lpdu.h lpdu.cpp tpdu.h tpdu.cpp apdu.h apdu.cpp 
CORE=lowlevel.h layer2.h layer2.cpp layer3.h layer3.cpp layer4.h layer4.cpp layer7.h layer7.cpp lowlevel.cpp 
MANAGEMENT=management.h management.cpp
FRONTEND_C=client.h client.cpp busmonitor.h busmonitor.cpp connection.h connection.cpp managementclient.h managementclient.cpp xmlccwrap.h xmlccwrap.cpp
FRONTEND=server.h server.cpp localserver.h localserver.cpp inetserver.h inetserver.cpp $(FRONTEND_C)
EMI= emi.h emi.cpp
EIBNETIP=eibnetip.cpp eibnetip.h eibnetserver.cpp eibnetserver.h
USB=eibusb.cpp eibusb.h
STATE=state.cpp state.h stateinterface.h stateinterface.cpp

libeibstack_la_SOURCES =$(COMMON) $(CORE) $(PDUs) $(MANAGEMENT) $(FRONTEND) $(EMI) $(EIBNETIP) $(USB) $(STATE)
libeibstack_la_LDFLAGS=-version-info 0:1:0 
