/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "localserver.h"
#include "eibtypes.h"
#include "eibpriority.h"

LocalServer::LocalServer (Layer3 * la3,
			  const char *path,
			  Logs * tr,
			  DaemonInstance *d,
			  int inquemaxlen, int outquemaxlen, int peerquemaxlen, int clientsmax,  IPv4NetList &ipnetfilters) :
  Server (la3, "Local EIBD Server", tr, d, inquemaxlen, outquemaxlen, peerquemaxlen, clientsmax, ipnetfilters)
{
  struct sockaddr_un addr;
  TRACEPRINTF (Loggers(), 8, this, "OpenLocalSocket");
  addr.sun_family = AF_LOCAL;
  strncpy (addr.sun_path, path, sizeof (addr.sun_path));

  fd = socket (AF_LOCAL, SOCK_STREAM, 0);
  if (fd == -1)
    {
      ERRORLOGSHAPE (Loggers(), LOG_CRIT, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Local Socket Init Fail");
      throw Exception (DEV_OPEN_FAIL);
    }

  unlink (path);
  if (bind (fd, (struct sockaddr *) &addr, sizeof (addr)) == -1)
    {
	  close (fd);
      fd = -1;
      ERRORLOGSHAPE (Loggers(), LOG_CRIT, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Local Socket Bind Fail");
      throw Exception (DEV_OPEN_FAIL);
    }


  // make sure everyone can access the socket

  if (chmod(path,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH)== -1)
    {
       close (fd);
      fd = -1;
      ERRORLOGSHAPE (Loggers(), LOG_CRIT, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Local Socket Permissions Setting Fail");
      throw Exception (DEV_OPEN_FAIL);
    }

  if (listen (fd, 10) == -1)
    {
      close (fd);
      fd = -1;
      ERRORLOGSHAPE (Loggers(), LOG_CRIT, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Local Socket Listen Fail");
      throw Exception (DEV_OPEN_FAIL);
    }

  TRACEPRINTF (Loggers(), 8, this, "Local Socket opened");
  Start ();
}

bool
LocalServer::init ()
{
  return fd != -1;
}

Element * LocalServer::_xml(Element *parent)
{
  Element *s=Server::_xml(parent);
  s->addAttribute(XMLSERVERTYPEATTR,"Local EIBD Socket");
  return s;
}
