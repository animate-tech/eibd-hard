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

#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include "layer3.h"
#include "state.h"
#include "ip/ipv4net.h"

class ClientConnection;
/** implements the frontend (but opens no connection) */
class Server:protected Thread, public StateInterface
{
  /** daemon */
  DaemonInstance *daemon;
  /** Layer 3 interface */
  Layer3 *l3;
  /** open client connections*/
  Array < ClientConnection * > connections;

  /** statistics */
  UIntStatisticsCounter stat_maxconcurrentclients;
  UIntStatisticsCounter stat_totalclients;
  UIntStatisticsCounter stat_clientsrejected;
  UIntStatisticsCounter stat_clientsauthfail;

  UIntStatisticsCounter  stat_packets_sent;
  UIntStatisticsCounter  stat_packets_received;
  UIntStatisticsCounter  stat_recverr;
  UIntStatisticsCounter  stat_senderr;

  pth_mutex_t lock;

  void Run (pth_sem_t * stop);

  int  peerqueuemaxlen;
  int  outqueuemaxlen;
  int  inqueuemaxlen;

  int  clientsmax;  //< maximum concurrent clients, 0 means anything goes
protected:
    /** server socket */
  int fd;

  virtual void setupConnection (int cfd);

  Server (Layer3 * l3,
	  const char *threadname,
	  Logs * tr,
	  DaemonInstance *d,
	  int inquemaxlen,
	  int outquemaxlen,
	  int peerquemaxlen,
	  int clientsmax,
	  IPv4NetList &ipnetfilters);
public:
  virtual ~ Server ();
  virtual bool init () = 0;
   /** deregister client connection */
  bool deregister (ClientConnection * con);
  int  maxInQueueLength(void) { return inqueuemaxlen; }
  int  maxOutQueueLength(void) { return outqueuemaxlen; }
  int  maxPeerQueueLength(void) { return peerqueuemaxlen; }

  virtual Element *_xml(Element *parent);

  DaemonInstance *Daemon(void ) const { return daemon; } ;
private:
  IPv4NetList ipnetfilters;

};

#endif
