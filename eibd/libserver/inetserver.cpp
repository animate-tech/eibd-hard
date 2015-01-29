/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include "inetserver.h"
#include "eibtypes.h"
#include "eibpriority.h"

InetServer::InetServer (Layer3 * la3, Logs *tr,
		   DaemonInstance *d, int port,
			int inquemaxlen, int outquemaxlen, int peerquemaxlen, int clientsmax,  IPv4NetList ipnetfilters):
  Server (la3, "Inet Server", tr, d, inquemaxlen, outquemaxlen, peerquemaxlen, clientsmax, ipnetfilters)
{
  struct sockaddr_in addrcopy;
  int reuse = 1;
  TRACEPRINTF (Loggers(), 8, this, "OpenInetSocket %d", port);
  memset (&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons (port);
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  addrcopy=addr;

  fd = socket (AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
    {
      ERRORLOGSHAPE (Loggers(), LOG_CRIT, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Socket Open for INet Server Failed");
      throw Exception (DEV_OPEN_FAIL);
    }

  setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse));

  if (bind (fd, (struct sockaddr *) &addrcopy, sizeof (addrcopy)) == -1)
    {
      close (fd);
      fd = -1;
      ERRORLOGSHAPE (Loggers(), LOG_CRIT, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                "Socket Bind for INet Server Failed");
      throw Exception (DEV_OPEN_FAIL);
    }

  if (listen (fd, 10) == -1)
    {
      close (fd);
      fd = -1;
      ERRORLOGSHAPE (Loggers(), LOG_CRIT, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Listen Failed");
      throw Exception (DEV_OPEN_FAIL);
    }

  TRACEPRINTF (Loggers(), 8, this, "InetSocket opened");
  Start ();
}

bool
InetServer::init ()
{
  return fd != -1;
}
void
InetServer::setupConnection (int cfd)
{
  int val = 1;
  Server::setupConnection(cfd);
  setsockopt (cfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof (val));
}

Element *InetServer::_xml(Element *parent)
{
  char buf[64];
  Element *s = Server::_xml(parent);
  s->addAttribute(XMLSERVERTYPEATTR,"IP EIBD Protocol");
  snprintf(buf,sizeof(buf)-1,"%s:%u", (const char *) inet_ntoa(addr.sin_addr),(unsigned int) ntohs(addr.sin_port));
  s->addAttribute(XMLSERVERADDRESSATTR,buf);
  return parent;
}
