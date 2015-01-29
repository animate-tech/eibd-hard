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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef EIBNET_SERVER_H
#define EIBNET_SERVER_H

#include "eibnetip.h"
#include "layer3.h"
#include <map>

typedef struct
{
  uchar channel; // also primary key of the map
  uchar sno;
  uchar rno;
  int state;
  int type;
  int no;
  bool nat;
  pth_event_t timeout;
  Queue < CArray > *out;
  struct sockaddr_in daddr;
  struct sockaddr_in caddr;
  pth_sem_t outsignal;
  pth_event_t outwait;
  pth_event_t sendtimeout;

  TimeVal created;
  UIntStatisticsCounter stat_senderr;
  UIntStatisticsCounter stat_recverr;
} ConnState;

class NATKey
{
private:
  eibaddr_t src;
  eibaddr_t dest;
public:
  NATKey( eibaddr_t _src, eibaddr_t _dest ) : src(_src), dest(_dest) {};
  bool operator==( const NATKey  & lhs ) const
      { return lhs.src == src && lhs.dest == dest; };
  bool operator<(const NATKey  & lhs ) const
    { return lhs.src < src || ( lhs.src == src && lhs.dest < dest); };
};

typedef struct
{
  eibaddr_t src;
  eibaddr_t dest;
  pth_event_t timeout;
} NATState;

class EIBnetServer:public L_Data_CallBack, public L_Busmonitor_CallBack,
  private Thread
  , public DroppableQueueInterface
{
  Layer3 *l3;
  EIBNetIPSocket *sock;
  int Port;
  bool tunnel;
  bool route;
  bool discover;
  int busmoncount;
  struct sockaddr_in maddr;
  typedef std::map < unsigned short, ConnState> connstatemap;
  connstatemap state;
  typedef std::map < NATKey, NATState> natstatemap;
  natstatemap natstate;

  int  peerqueuemaxlen;
  int clientsmax;

  const static char outdropmsg[], indropmsg[];

  void Run (pth_sem_t * stop);
  void Get_L_Data (L_Data_PDU * l);
  void Get_L_Busmonitor (L_Busmonitor_PDU * l);
private:
  void addBusmonitor ();
  void delBusmonitor ();
  int addClient (int type, const EIBnet_ConnectRequest & r1);
  int delClient( connstatemap::iterator, const char * );
  void addNAT (const L_Data_PDU & l);

  /** statistics */
  UIntStatisticsCounter stat_maxconcurrentclients;
  UIntStatisticsCounter stat_totalclients;
  UIntStatisticsCounter stat_clientsrejected;
  UIntStatisticsCounter stat_clientsauthfail;

  UIntStatisticsCounter stat_senderr;
  UIntStatisticsCounter stat_recverr;

public:
    EIBnetServer (const char *multicastaddr, int port, bool Tunnel,
		  bool Route, bool Discover, Layer3 * layer3,
		  Logs * tr,
		  int inquemaxlen, int outquemaxlen, int peerquemaxlen,
		  int clientsmax, IPv4NetList &ipnetfilters );
    virtual ~ EIBnetServer ();
  bool init ();

    const char *_str(void) const
      {
	return "EIBNet Server";
      };

    Element * _xml(Element *parent) const;

private:
    mutable pth_mutex_t datalock;
    bool TraceDataLockWait(pth_mutex_t *datalock) const;
    IPv4NetList ipnetfilters;
};

#endif
