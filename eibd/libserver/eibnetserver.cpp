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

#include "eibnetserver.h"
#include "emi.h"
#include "eibtypes.h"
#include "eibpriority.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"

#define NAME "Z2 Gateway"

const char EIBnetServer::indropmsg[] =
    "EIBnetServer: incoming queue length exceeded, dropping packet";
const char EIBnetServer::outdropmsg[] =
    "EIBnetServer: outgoing queue length exceeded, dropping packet";

EIBnetServer::EIBnetServer(const char *multicastaddr, int port, bool Tunnel,
    bool Route, bool Discover, Layer3 * layer3, Logs * tr, int inquemaxlen,
    int outquemaxlen, int peerquemaxlen, int clientsmax,
    IPv4NetList &ipnetfilters) :
    DroppableQueueInterface(tr), Thread(tr, PTH_PRIO_STD, "EIBnetServer"), stat_maxconcurrentclients(
        0), stat_totalclients(0), stat_clientsrejected(0), stat_clientsauthfail(
        0), ipnetfilters(ipnetfilters)
{
  struct sockaddr_in baddr;
  struct ip_mreq mcfg;
  l3 = layer3;
  pth_mutex_init(&datalock);
  memset(&baddr, 0, sizeof(baddr));
#ifdef HAVE_SOCKADDR_IN_LEN
  baddr.sin_len = sizeof (baddr);
#endif
  TRACEPRINTF(Thread::Loggers(), 8, this, "Open");
  baddr.sin_family = AF_INET;
  baddr.sin_port = htons(port);
  baddr.sin_addr.s_addr = htonl(INADDR_ANY );

  if (GetHostIP(&maddr, multicastaddr) == 0)
    {
      ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR, Logging::DUPLICATESMAX1PER10SEC,
          this, Logging::MSGNOHASH, "Get Host IP for EIBNet Server Failed");
      sock = 0;
      return;
    }
  maddr.sin_port = htons(port);

  sock = new EIBNetIPSocket(baddr, 1, Thread::Loggers(), inquemaxlen,
      outquemaxlen, peerquemaxlen, ipnetfilters);
  if (!sock->init())
    {
      delete sock;
      sock = 0;
      return;
    }
  mcfg.imr_multiaddr = maddr.sin_addr;
  mcfg.imr_interface.s_addr = htonl(INADDR_ANY );
  if (!sock->SetMulticast(mcfg, tr))
    {
      delete sock;
      sock = 0;
      return;
    }
  sock->recvall = 2;
  if (!GetSourceAddress(&maddr, &sock->localaddr))
    {
      ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR, Logging::DUPLICATESMAX1PER10SEC,
          this, Logging::MSGNOHASH,
          "Get Source Address for EIBNet Server Failed");
      delete sock;
      sock = 0;
      return;
    }
  sock->localaddr.sin_port = htons(port);
  tunnel = Tunnel;
  route = Route;
  discover = Discover;
  Port = htons(port);
  if (route || tunnel)
    {
      if (!l3->registerBroadcastCallBack(this))
        {
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
              Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
              "Internal Error for EIBNet Server registerBroadcastCallBack");
          delete sock;
          sock = 0;
          return;
        }
      if (!l3->registerGroupCallBack(this, 0))
        {
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
              Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
              "Internal Error for EIBNet Server registerGroupCallBack");
          delete sock;
          sock = 0;
          return;
        }
      if (!l3->registerIndividualCallBack(this, Individual_Lock_None, 0, 0))
        {
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
              Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
              "Internal Error for EIBNet Server registerIndividualCallBack");
          delete sock;
          sock = 0;
          return;
        }
    }
  this->peerqueuemaxlen = peerquemaxlen;
  this->clientsmax = clientsmax > 255 ? 255 : clientsmax; //  only one byte channel
  busmoncount = 0;

  Start();
  TRACEPRINTF(Thread::Loggers(), 8, this, "Opened");
}

bool
EIBnetServer::TraceDataLockWait(pth_mutex_t *datalock) const
{
  TRACEPRINTF(Thread::Loggers(), 11, this, "Take: %s", name());
  return WaitForDataLock(datalock);
}

EIBnetServer::~EIBnetServer()
{
  natstatemap::const_iterator i;
  TRACEPRINTF(Thread::Loggers(), 8, this, "Exiting");
  if (!TraceDataLockWait(&datalock))
    assert(0);

  if (route || tunnel)
    {
      l3->deregisterBroadcastCallBack(this);
      l3->deregisterGroupCallBack(this, 0);
      l3->deregisterIndividualCallBack(this, 0, 0);
    }
  if (busmoncount)
    l3->deregisterVBusmonitor(this);
  Stop();
  for (i = natstate.begin(); i != natstate.end(); ++i)
    pth_event_free(i->second.timeout, PTH_FREE_THIS);
  if (sock)
    delete sock;
  ReleaseDataLock(&datalock);
}

bool
EIBnetServer::init()
{
  return sock != 0;
}

void
EIBnetServer::Get_L_Busmonitor(L_Busmonitor_PDU * l)
{
  if (!TraceDataLockWait(&datalock))
    return;
  connstatemap::iterator i;
  for (i = state.begin(); i != state.end(); ++i)
    {
      if (i->second.type == 1)
        {
          Put_On_Queue_Or_Drop<CArray, CArray>(*(i->second.out),
              Busmonitor_to_CEMI(0x2B, *l, (i->second.no)++),
              &i->second.outsignal, false, outdropmsg);
#if 0
          i->second.out->put (Busmonitor_to_CEMI (0x2B, *l, i->second.no++));
          pth_sem_inc (i->second.outsignal, 0);
#endif
        }
    }

  ReleaseDataLock(&datalock);
}

void
EIBnetServer::Get_L_Data(L_Data_PDU * l)
{
  if (!l)
    return; // just ignore empty packets, we do NOT go down when interface goes down
  if (!l->hopcount)
    {
      TRACEPRINTF(Thread::Loggers(), 8, this, "SendDrop");
      delete l;
      return;
    }
  if (l->object == this)
    {
      delete l;
      return;
    }
  l->hopcount--;

  if (!TraceDataLockWait(&datalock))
    return;

  if (route)
    {
      TRACEPRINTF(Thread::Loggers(), 8, this, "Send_Route %s", l->Decode ()());
      sock->sendaddr = maddr;
      EIBNetIPPacket p;
      p.service = ROUTING_INDICATION;
      if (l->dest == 0 && l->AddrType == IndividualAddress)
        {
          natstatemap::const_iterator i;
          int cnt = 0;
          for (i = natstate.begin(); i != natstate.end(); ++i)
            {
              if (i->second.dest == l->source)
                {
                  l->dest = i->second.src;
                  p.data = L_Data_ToCEMI(0x29, *l);
                  sock->Send(p);
                  l->dest = 0;
                  cnt++;
                }
            }
          if (!cnt)
            {
              p.data = L_Data_ToCEMI(0x29, *l);
              sock->Send(p);
            }
        }
      else
        {
          p.data = L_Data_ToCEMI(0x29, *l);
          sock->Send(p);
        }
    }
    {
      connstatemap::iterator i;
      for (i = state.begin(); i != state.end(); ++i)
        {
          if (i->second.type == 0)
            {
              Put_On_Queue_Or_Drop<CArray, CArray>(*(i->second.out),
                  L_Data_ToCEMI(0x29, *l), &i->second.outsignal, false,
                  outdropmsg);

#if 0
              i->second.out.put (L_Data_ToCEMI (0x29, *l));
              pth_sem_inc (i->second.outsignal, 0);
#endif
            }
        }
    }
  delete l;

  ReleaseDataLock(&datalock);
}

void
EIBnetServer::addBusmonitor()
{
  if (!TraceDataLockWait(&datalock))
    return;

  if (busmoncount == 0)
    {
      if (!l3->registerVBusmonitor(this))
        TRACEPRINTF(Thread::Loggers(), 8, this, "Registervbusmonitor failed");
      busmoncount++;
    }
  ReleaseDataLock(&datalock);
}

void
EIBnetServer::delBusmonitor()
{
  if (!TraceDataLockWait(&datalock))
    return;

  busmoncount--;
  if (busmoncount == 0)
    l3->deregisterVBusmonitor(this);

  ReleaseDataLock(&datalock);
}

#define EIBNET_CLIENTTIMEOUT 120  // 120 normally
int
EIBnetServer::addClient(int type, const EIBnet_ConnectRequest & r1)
{
  unsigned short i, pos;
  unsigned short id = 1;

  if (!TraceDataLockWait(&datalock))
    return -1;

  // first it's faster, second it's much more stable on clients
  // that come and go and step on each other
  id = rand() & 0xff;
  while (state.find(id) != state.end() && id < 255)
    id++;
  if (id > 255)
    {
      id--;
      while (state.find(id) != state.end() && id > 0)
        id--;
    }

  if (id < 0 || id > 255)
    return -1;

  pos = id;

  assert( state.find(pos) == state.end());

  state[pos].timeout = pth_event(PTH_EVENT_RTIME,
      pth_time(EIBNET_CLIENTTIMEOUT, 0));
  state[pos].out = new Queue<CArray>("client outgoing", peerqueuemaxlen);
  pth_sem_init(&state[pos].outsignal);
  state[pos].outwait = pth_event(PTH_EVENT_SEM, &state[pos].outsignal);
  state[pos].sendtimeout = pth_event(PTH_EVENT_RTIME, pth_time(1, 0));
  state[pos].created = pth_timeout(0, 0);
  state[pos].channel = id; // primary key
  state[pos].daddr = r1.daddr;
  state[pos].caddr = r1.caddr;
  state[pos].state = 0;
  state[pos].sno = 0;
  state[pos].rno = 0;
  state[pos].no = 1;
  state[pos].type = type;
  state[pos].nat = r1.nat;
  TRACEPRINTF(Thread::Loggers(), 8, this,
      "Added IP Client channel %d type %d from %s", state[pos].channel, state[pos].type, (const char *) inet_ntoa(state[pos].caddr.sin_addr));
  ++stat_totalclients;
  stat_maxconcurrentclients =
      *stat_maxconcurrentclients < state.size() ?
          state.size() : *stat_maxconcurrentclients;

  ReleaseDataLock(&datalock);
  return id;
}

int
EIBnetServer::delClient(connstatemap::iterator i, const char *reason)
{
  unsigned short id;
  // if already done, better isolate again, rather safe than sorry
  if (!TraceDataLockWait(&datalock))
    return -1;

  assert( i!= state.end());
  id = i->first;

  if (i->second.type == 1)
    delBusmonitor();

  pth_event_isolate(i->second.timeout);
  pth_event_isolate(i->second.sendtimeout);
  pth_event_isolate(i->second.outwait);
  TRACEPRINTF(Thread::Loggers(), 8, this,
      "Delete IP Client channel %d type %d from %s on %s request pending packets: %d", i->second.channel, i->second.type, (const char *) inet_ntoa(i->second.caddr.sin_addr), reason, i->second.out->len());

  while (!i->second.out->isempty())
    {
      i->second.out->get();
      pth_sem_dec(&i->second.outsignal);
    }
  pth_event_free(i->second.timeout, PTH_FREE_THIS);
  pth_event_free(i->second.sendtimeout, PTH_FREE_THIS);
  pth_event_free(i->second.outwait, PTH_FREE_THIS);
  delete i->second.out;
  i->second.out = NULL;

  state.erase(i->first);

  assert( state.find(id) == state.end());

  ReleaseDataLock(&datalock);
  return 0;
}

void
EIBnetServer::addNAT(const L_Data_PDU & l)
{
  natstatemap::const_iterator i;
  NATKey k(l.source, l.dest);

  if (!TraceDataLockWait(&datalock))
    return;

  if (l.AddrType != IndividualAddress)
    return;

  if (natstate.find(k) != natstate.end())
    {
      pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, i->second.timeout,
          pth_time(180, 0));

      ReleaseDataLock(&datalock);
      return;
    }
  natstate[k].src = l.source;
  natstate[k].dest = l.dest;
  natstate[k].timeout = pth_event(PTH_EVENT_RTIME, pth_time(180, 0));
  ReleaseDataLock(&datalock);
  return;
}

void
EIBnetServer::Run(pth_sem_t * stop1)
{
  EIBNetIPPacket *p1;
  EIBNetIPPacket p;
  connstatemap::iterator i;
  pth_event_t stop = pth_event(PTH_EVENT_SEM, stop1);

  while (pth_event_status(stop) != PTH_STATUS_OCCURRED)
    {
      TRACEPRINTF(Thread::Loggers(), 2, this, "Loop Begin");

      pth_event_isolate(stop);

      if (!TraceDataLockWait(&datalock))
        break; // means lock dropped out on stop

      for (i = state.begin(); i != state.end(); ++i)
        {
          pth_event_concat(stop, i->second.timeout, NULL);
          if (i->second.state)
            pth_event_concat(stop, i->second.sendtimeout, NULL);
          else
            pth_event_concat(stop, i->second.outwait, NULL);

        }

      natstatemap::const_iterator j;
      for (j = natstate.begin(); j != natstate.end(); ++j)
        pth_event_concat(stop, j->second.timeout, NULL);

      ReleaseDataLock(&datalock);

      p1 = sock->Get(stop);

      pth_event_isolate(stop);

      if (!TraceDataLockWait(&datalock))
        break; // means lock dropped out on stop

      for (i = state.begin(); i != state.end(); ++i)
        {
          pth_event_isolate(i->second.timeout);
          pth_event_isolate(i->second.sendtimeout);
          pth_event_isolate(i->second.outwait);
        }
      for (j = natstate.begin(); j != natstate.end(); ++j)
        pth_event_isolate(j->second.timeout);
      j = natstate.begin();
      while (j != natstate.end())
        {
          if (pth_event_status(j->second.timeout) == PTH_STATUS_OCCURRED)
            {
              pth_event_free(j->second.timeout, PTH_FREE_THIS);
              natstate.erase(j->first);
              j = natstate.begin();
            }
          ++j;
        }
      if (p1)
        {
          if (p1->service == SEARCH_REQUEST && discover)
            {
              EIBnet_SearchRequest r1;
              EIBnet_SearchResponse r2;
              DIB_service_Entry d;
              if (parseEIBnet_SearchRequest(*p1, r1))
                goto out;
              TRACEPRINTF(Thread::Loggers(), 8, this, "SEARCH");
              r2.KNXmedium = 2;
              r2.devicestatus = 0;
              r2.individual_addr = 0;
              r2.installid = 0;
              r2.multicastaddr = maddr.sin_addr;
              strcpy((char *) r2.name, NAME);
              d.version = 1;
              d.family = 2;
              if (discover)
                r2.services.add(d);
              d.family = 4;
              if (tunnel)
                r2.services.add(d);
              d.family = 5;
              if (route)
                r2.services.add(d);
              if (!GetSourceAddress(&r1.caddr, &r2.caddr))
                goto out;
              r2.caddr.sin_port = Port;
              sock->sendaddr = r1.caddr;
              sock->Send(r2.ToPacket());
            }
          if (p1->service == DESCRIPTION_REQUEST && discover)
            {
              EIBnet_DescriptionRequest r1;
              EIBnet_DescriptionResponse r2;
              DIB_service_Entry d;
              if (parseEIBnet_DescriptionRequest(*p1, r1))
                goto out;
              TRACEPRINTF(Thread::Loggers(), 8, this, "DESCRIBE");
              r2.KNXmedium = 2;
              r2.devicestatus = 0;
              r2.individual_addr = 0;
              r2.installid = 0;
              r2.multicastaddr = maddr.sin_addr;
              strcpy((char *) r2.name, NAME);
              d.version = 1;
              d.family = 2;
              if (discover)
                r2.services.add(d);
              d.family = 3;
              r2.services.add(d);
              d.family = 4;
              if (tunnel)
                r2.services.add(d);
              d.family = 5;
              if (route)
                r2.services.add(d);
              sock->sendaddr = r1.caddr;
              sock->Send(r2.ToPacket());
            }
          if (p1->service == ROUTING_INDICATION && route)
            {
              if (p1->data() < 2 || p1->data[0] != 0x29)
                goto out;
              const CArray data = p1->data;
              L_Data_PDU *c = CEMI_to_L_Data(data);
              if (c)
                {
                  TRACEPRINTF(Thread::Loggers(), 8, this,
                      "Recv_Route %s", c->Decode ()());
                  if (c->hopcount)
                    {
                      c->hopcount--;
                      addNAT(*c);
                      c->object = this;
                      l3->send_L_Data(c);
                    }
                  else
                    {
                      TRACEPRINTF(Thread::Loggers(), 8, this, "RecvDrop");
                      delete c;
                    }
                }
            }
          if (p1->service == CONNECTIONSTATE_REQUEST)
            {
              uchar res = 21;
              EIBnet_ConnectionStateRequest r1;
              EIBnet_ConnectionStateResponse r2;
              if (parseEIBnet_ConnectionStateRequest(*p1, r1))
                goto out;
              i = state.find(r1.channel);
              if (i != state.end())
                {
                  if (compareIPAddress(p1->src, i->second.caddr))
                    {
                      res = 0;
                      pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE,
                          i->second.timeout, pth_time(EIBNET_CLIENTTIMEOUT, 0));
                    }
                  else
                    {
                      WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                          Logging::DUPLICATESMAX1PER10SEC, this,
                          Logging::MSGNOHASH, "Invalid Control Address");
                    }
                }
              r2.channel = r1.channel;
              r2.status = res;
              sock->sendaddr = r1.caddr;
              sock->Send(r2.ToPacket());
            }
          if (p1->service == DISCONNECT_REQUEST)
            {
              uchar res = 0x21;
              EIBnet_DisconnectRequest r1;
              EIBnet_DisconnectResponse r2;
              if (parseEIBnet_DisconnectRequest(*p1, r1))
                goto out;
              TRACEPRINTF(Thread::Loggers(), 8, this, "DISCONNECT_REQUEST");
              i = state.find(r1.channel);
              if (i != state.end())
                {
                  if (compareIPAddress(p1->src, i->second.caddr))
                    {
                      res = 0;
                      delClient(i, "disconnect");
                      goto out;
                    }
                  else
                    {
                      WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                          Logging::DUPLICATESMAX1PER10SEC, this,
                          Logging::MSGNOHASH,
                          "Invalid control address from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));
                    }
                }
              r2.channel = r1.channel;
              r2.status = res;
              sock->sendaddr = r1.caddr;
              sock->Send(r2.ToPacket());
            }
          if (p1->service == CONNECTION_REQUEST)
            {
              EIBnet_ConnectRequest r1;
              EIBnet_ConnectResponse r2;

              if (state.size() >= clientsmax)
                {
                  ++stat_clientsrejected;
                  goto out;
                }

              if (parseEIBnet_ConnectRequest(*p1, r1))
                goto out;

              r2.status = 0x22;
              if (r1.CRI() == 3 && r1.CRI[0] == 4 && tunnel)
                {
                  r2.CRD.resize(3);
                  r2.CRD[0] = 0x04;
                  r2.CRD[1] = 0x00;
                  r2.CRD[2] = 0x00;
                  if (r1.CRI[1] == 0x02 || r1.CRI[1] == 0x80) // 0x80 is 4.4.2 in 3.8.4 tunnel, 0x02 is link layer
                    {
                      int id = addClient((r1.CRI[1] == 0x80) ? 1 : 0, r1);
                      if (id >= 0)
                        {
                          if (r1.CRI[1] == 0x80)
                            addBusmonitor();
                          r2.channel = id;
                          r2.status = 0;
                        }
                    }
                }
              if (r1.CRI() == 1 && r1.CRI[0] == 3) // 0x03 is management connection per 7.8 in 3.8.2
                {
                  r2.CRD.resize(1);
                  r2.CRD[0] = 0x03;
                  int id = addClient(2, r1);
                  if (id >= 0)
                    {
                      r2.channel = id;
                      r2.status = 0;
                    }
                }
              if (!GetSourceAddress(&r1.caddr, &r2.daddr))
                goto out;
              r2.daddr.sin_port = Port;
              r2.nat = r1.nat;
              sock->sendaddr = r1.caddr;
              sock->Send(r2.ToPacket());
            }
          if (p1->service == TUNNEL_REQUEST && tunnel)
            {
              connstatemap::iterator i;
              EIBnet_TunnelRequest r1;
              EIBnet_TunnelACK r2;
              if (parseEIBnet_TunnelRequest(*p1, r1))
                goto out;
              TRACEPRINTF(Thread::Loggers(), 8, this, "TUNNEL_REQ");
              i = state.find(r1.channel);
              if (i == state.end())
                goto out;
              // @todo: not good, warning necessary maybe

              if (!compareIPAddress(p1->src, i->second.daddr))
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Invalid destination address on tunnel request from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));

                  goto out;
                }
              if (i->second.rno == (r1.seqno + 1) & 0xff)
                {
                  r2.channel = r1.channel;
                  r2.seqno = r1.seqno;
                  sock->sendaddr = i->second.daddr;
                  sock->Send(r2.ToPacket());
                  goto out;
                }
              if (i->second.rno != r1.seqno)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Received wrong sequence number from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);
                  goto out;
                }
              r2.channel = r1.channel;
              r2.seqno = r1.seqno;
              if (i->second.type == 0)
                {
                  L_Data_PDU *c = CEMI_to_L_Data(r1.CEMI);
                  if (c)
                    {
                      r2.status = 0;
                      if (c->hopcount)
                        {
                          c->hopcount--;
                          if (r1.CEMI[0] == 0x11)
                            {
                              Put_On_Queue_Or_Drop<CArray, CArray>(
                                  *(i->second.out), L_Data_ToCEMI(0x2E, *c),
                                  &i->second.outsignal, false, outdropmsg);
#if 0
                              i->second.out.put (L_Data_ToCEMI (0x2E, *c));
                              pth_sem_inc (i->second.outsignal, 0);
#endif
                            }
                          c->object = this;
                          if (r1.CEMI[0] == 0x11 || r1.CEMI[0] == 0x29)
                            l3->send_L_Data(c);
                          else
                            delete c;
                        }
                      else
                        {
                          TRACEPRINTF(Thread::Loggers(), 9, this, "RecvDrop");
                          delete c;
                        }
                    }
                  else
                    r2.status = 0x29;

                }
              else
                r2.status = 0x29;
              i->second.rno++;
              if (i->second.rno > 0xff)
                i->second.rno = 0;
              sock->sendaddr = i->second.daddr;
              sock->Send(r2.ToPacket());
            }
          if (p1->service == TUNNEL_RESPONSE && tunnel)
            {
              connstatemap::iterator i;
              EIBnet_TunnelACK r1;
              if (parseEIBnet_TunnelACK(*p1, r1))
                goto out;
              TRACEPRINTF(Thread::Loggers(), 9, this, "TUNNEL_ACK");
              i = state.find(r1.channel);
              if (i == state.end())
                goto out;
              // @todo: not good, warning necessary maybe

              if (!compareIPAddress(p1->src, i->second.daddr))
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Invalid data endpoint on tunnel response from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  goto out;
                }
              if (i->second.sno != r1.seqno)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Received wrong sequence from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);
                  goto out;
                }
              if (r1.status != 0)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Wrong status %d from %s", r1.status, (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);
                  goto out;
                }
              if (!i->second.state)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Unexpected ACK from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);
                  goto out;
                }
              if (i->second.type != 0 && i->second.type != 1)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Unexpected Connection Type");
                  ++(i->second.stat_recverr);
                  goto out;
                }
              i->second.sno++;
              if (i->second.sno > 0xff)
                i->second.sno = 0;
              i->second.state = 0;
              i->second.out->get();
              pth_sem_dec(&i->second.outsignal);
            }
          if (p1->service == DEVICE_CONFIGURATION_REQUEST)
            {
              connstatemap::iterator i;
              EIBnet_ConfigRequest r1;
              EIBnet_ConfigACK r2;
              if (parseEIBnet_ConfigRequest(*p1, r1))
                goto out;
              TRACEPRINTF(Thread::Loggers(), 8, this, "CONFIG_REQ");
              i = state.find(r1.channel);
              if (i == state.end())
                goto out;
              // @todo: not good, warning necessary maybe
              reqf3: if (!compareIPAddress(p1->src, i->second.daddr))
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Invalid data endpoint from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);
                  goto out;
                }
              if (i->second.rno == (r1.seqno + 1) & 0xff)
                {
                  r2.channel = r1.channel;
                  r2.seqno = r1.seqno;
                  sock->sendaddr = i->second.daddr;
                  sock->Send(r2.ToPacket());
                  goto out;
                }
              if (i->second.rno != r1.seqno)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Wrong sequence %d<->%d from %s", r1.seqno, i->second.rno, (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);
                  goto out;
                }
              r2.channel = r1.channel;
              r2.seqno = r1.seqno;
              if (i->second.type == 2 && r1.CEMI() > 1)
                {
                  if (r1.CEMI[0] == 0xFC)
                    {
                      if (r1.CEMI() == 7)
                        {
                          CArray res, CEMI;
                          int obj = (r1.CEMI[1] << 8) | r1.CEMI[2];
                          int objno = r1.CEMI[3];
                          int prop = r1.CEMI[4];
                          int count = (r1.CEMI[5] >> 4) & 0x0f;
                          int start = (r1.CEMI[5] & 0x0f) | r1.CEMI[6];
                          res.resize(1);
                          res[0] = 0;
                          if (obj == 0 && objno == 0)
                            {
                              if (prop == 0)
                                {
                                  res.resize(2);
                                  res[0] = 0;
                                  res[1] = 0;
                                  start = 0;
                                }
                              else
                                count = 0;
                            }
                          else
                            count = 0;
                          CEMI.resize(6 + res());
                          CEMI[0] = 0xFB;
                          CEMI[1] = (obj >> 8) & 0xff;
                          CEMI[2] = obj & 0xff;
                          CEMI[3] = objno;
                          CEMI[4] = prop;
                          CEMI[5] = ((count & 0x0f) << 4) | (start >> 8);
                          CEMI[6] = start & 0xff;
                          CEMI.setpart(res, 7);
                          r2.status = 0x00;

                          Put_On_Queue_Or_Drop<CArray, CArray>(*(i->second.out),
                              CEMI, &i->second.outsignal, false, outdropmsg);
#if 0
                          i->second.out.put (CEMI);
                          pth_sem_inc (i->second.outsignal, 0);
#endif
                        }
                      else
                        r2.status = 0x26;
                    }
                  else
                    r2.status = 0x26;
                }
              else
                r2.status = 0x29;
              i->second.rno++;
              if (i->second.rno > 0xff)
                i->second.rno = 0;
              sock->sendaddr = i->second.daddr;
              sock->Send(r2.ToPacket());
            }
          if (p1->service == DEVICE_CONFIGURATION_ACK)
            {
              connstatemap::iterator i;
              EIBnet_ConfigACK r1;
              if (parseEIBnet_ConfigACK(*p1, r1))
                goto out;
              TRACEPRINTF(Thread::Loggers(), 8, this, "CONFIG_ACK");
              i = state.find(r1.channel);
              if (i == state.end())
                goto out;
              // @todo: not good, warning necessary maybe
              if (!compareIPAddress(p1->src, i->second.daddr))
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Invalid data endpoint from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);
                  goto out;
                }
              if (i->second.sno != r1.seqno)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Wrong sequence %d<->%d from %s", r1.seqno, i->second.sno, (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);

                  goto out;
                }
              if (r1.status != 0)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Wrong status %d from %s", r1.status, (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);

                  goto out;
                }
              if (!i->second.state)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Unexpected ACK from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);

                  goto out;
                }
              if (i->second.type != 2)
                {
                  WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                      Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                      "Unexpected Connection Type from %s", (const char *) inet_ntoa(i->second.caddr.sin_addr));
                  ++(i->second.stat_recverr);

                  goto out;
                }
              i->second.sno++;
              if (i->second.sno > 0xff)
                i->second.sno = 0;
              i->second.state = 0;
              i->second.out->get();
              pth_sem_dec(&i->second.outsignal);
            }
          out: delete p1;
        }

      i = state.begin();
      while (i != state.end())
        {
          if (pth_event_status(i->second.timeout) == PTH_STATUS_OCCURRED)
            {
              delClient(i, "timeout");
              i = state.begin();
            }
          else
            i++;
        }

      for (i = state.begin(); i != state.end(); i++)
        {
          if ((i->second.state
              && pth_event_status(i->second.sendtimeout) == PTH_STATUS_OCCURRED)
              || (!i->second.state && !i->second.out->isempty()))
            {
              TRACEPRINTF(Thread::Loggers(), 9, this,
                  "TunnelSend %d", i->second.channel);
              i->second.state++;
              if (i->second.state > 10)
                {
                  i->second.out->get();
                  pth_sem_dec(&i->second.outsignal);
                  i->second.state = 0;
                  ++(i->second.stat_senderr);
                  continue;
                }
              EIBNetIPPacket p;
              if (i->second.type == 2)
                {
                  EIBnet_ConfigRequest r;
                  r.channel = i->second.channel;
                  r.seqno = i->second.sno;
                  r.CEMI = i->second.out->top();
                  p = r.ToPacket();
                }
              else
                {
                  EIBnet_TunnelRequest r;
                  r.channel = i->second.channel;
                  r.seqno = i->second.sno;
                  r.CEMI = i->second.out->top();
                  p = r.ToPacket();
                }
              pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, i->second.sendtimeout,
                  pth_time(1, 0));
              sock->sendaddr = i->second.daddr;
              sock->Send(p);
            }
        }

      ReleaseDataLock(&datalock);

    }

  TraceDataLockWait(&datalock);
  i = state.begin();
  while (i != state.end())
    {
      EIBnet_DisconnectRequest r;
      r.channel = i->second.channel;
      if (!GetSourceAddress(&i->second.caddr, &r.caddr))
        {
          ++i;
          continue;
        }
      r.caddr.sin_port = Port;
      r.nat = i->second.nat;
      sock->sendaddr = i->second.caddr;
      sock->Send(r.ToPacket());

      delClient(i, "exit");
      i = state.begin();
    }
  pth_event_free(stop, PTH_FREE_THIS);
  ReleaseDataLock(&datalock);
}

Element *
EIBnetServer::_xml(Element *parent) const
{
  char buf[64];
  Element *p = parent->addElement(XMLSERVERELEMENT);

  if (!TraceDataLockWait(&datalock))
    return parent; // means lock dropped out on stop

  p->addAttribute(XMLSERVERTYPEATTR, "EIBNET/IP");
  snprintf(buf, sizeof(buf) - 1, "%s:%d",
      (const char *) inet_ntoa(maddr.sin_addr), (int) ntohs(maddr.sin_port));
  p->addAttribute(XMLSERVERADDRESSATTR, buf);

  // here dump all the state as clients
  p->addAttribute(XMLSERVERMAXCLIENTSATTR, *stat_maxconcurrentclients);
  p->addAttribute(XMLSERVERCLIENTSATTR, state.size());

  p->addAttribute(XMLSERVERCLIENTSTOTALATTR, *stat_totalclients);

  if (clientsmax)
    {
      p->addAttribute(XMLSERVERMAXALLOWCLIENTSATTR, clientsmax);
    }

  if (*stat_clientsrejected)
    {
      p->addAttribute(XMLSERVERCLIENTSREJECTEDATTR, *stat_clientsrejected);
    }
  if (*stat_clientsauthfail)
    {
      p->addAttribute(XMLSERVERCLIENTSAUTHFAILATTR, *stat_clientsauthfail);
    }

  for (connstatemap::const_iterator i = state.begin(); i != state.end(); i++)
    {
      Element *c = p->addElement(XMLCLIENTELEMENT);

      c->addAttribute(XMLCLIENTTYPEATTR, "EIBNET/IP");
      c->addAttribute(XMLCLIENTSTARTTIMEATTR,
          i->second.created.fmtString("%c"));

      if (*stat_recverr)
        {
          c->addAttribute(XMLCLIENTSRECVERRATTR, *i->second.stat_recverr);
        }
      if (*stat_senderr)
        {
          c->addAttribute(XMLCLIENTSRECVERRATTR, *i->second.stat_senderr);
        }

      const struct sockaddr_in *sin = &i->second.caddr;
      snprintf(buf, sizeof(buf) - 1, "%s:%d",
          (const char *) inet_ntoa(sin->sin_addr), (int) ntohs(sin->sin_port));
      c->addAttribute(XMLCLIENTADDRESSATTR, buf);

      i->second.out->_xml(c);
    }

  if (ipnetfilters.size())
    {
      std::string r = "";
      for (IPv4NetList::const_iterator i = ipnetfilters.begin();
          i != ipnetfilters.end(); i++)
        {
          if (i != ipnetfilters.begin())
            {
              r += ",";
            }
          std::string s(*i);
          r += s;
        }
      p->addAttribute(XMLSERVERSUBNETFILTERS, r.c_str());
    }

  ReleaseDataLock(&datalock);
  return p;
}

