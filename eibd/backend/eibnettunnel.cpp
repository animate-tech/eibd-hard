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

#include "eibnettunnel.h"
#include "emi.h"

bool
EIBNetIPTunnel::addAddress (eibaddr_t addr)
{
  return 0;
}

bool
EIBNetIPTunnel::removeAddress (eibaddr_t addr)
{
  return 0;
}

bool
EIBNetIPTunnel::addGroupAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPTunnel::removeGroupAddress (eibaddr_t addr)
{
  return 1;
}

eibaddr_t
EIBNetIPTunnel::getDefaultAddr ()
{
  return 0;
}

const char EIBNetIPTunnel::indropmsg[] = "EIBNetIPTunnel: incoming queue length exceeded, dropping packet";
const char EIBNetIPTunnel::outdropmsg[] = "EIBNetIPTunnel: outgoing queue length exceeded, dropping packet";

EIBNetIPTunnel::EIBNetIPTunnel (const char *dest, int port, int sport,
					const char *srcip, int Dataport, int flags,
				Logs * tr, int inquemaxlen, int outquemaxlen, int peerquemaxlen,
				IPv4NetList &ipnetfilters) :
  outqueue("tunnel outgoing", outquemaxlen),
  inqueue("tunnel incoming", inquemaxlen),
  Layer2Interface(tr,0),
  Thread(tr,PTH_PRIO_STD, "EIBNetIPTunnel"),
  ipnetfilters(ipnetfilters)
{
  connmod = 0;
  addr = 0;
  TRACEPRINTF(Thread::Loggers(), 2, this, "Open");
  pth_sem_init(&insignal);
  pth_sem_init(&outsignal);
  noqueue = flags & FLAG_B_TUNNEL_NOQUEUE;
  sock = 0;
  if (!GetHostIP(&caddr, dest))
    return;
  caddr.sin_port = htons(port);
  if (!GetSourceAddress(&caddr, &raddr))
    return;
  raddr.sin_port = htons(sport);
  NAT = false;
  dataport = Dataport;
  sock = new EIBNetIPSocket(raddr, 0, Thread::Loggers(), inquemaxlen,
      outquemaxlen, peerquemaxlen, ipnetfilters);
  if (!sock->init())
    {
      delete sock;
      sock = 0;
      ERRORLOGSHAPE(tr, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, this,
          Logging::MSGNOHASH, "Cannot get tunnel host IP");
      throw Exception(DEV_OPEN_FAIL);
    }
  if (srcip)
    {
      if (!GetHostIP(&saddr, srcip))
        {
          delete sock;
          sock = 0;
          ERRORLOGSHAPE(tr, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, this,
              Logging::MSGNOHASH, "Cannot get tunnel source address");
          throw Exception(DEV_OPEN_FAIL);
        }
      saddr.sin_port = htons(sport);
      NAT = true;
    }
  else
    saddr = raddr;
  sock->sendaddr = caddr;
  sock->recvaddr = caddr;
  sock->recvall = 0;
  mode = 0;
  vmode = 0;
  support_busmonitor = 1;
  connect_busmonitor = 0;

  Start();
  TRACEPRINTF(Thread::Loggers(), 2, this, "Opened");
}

EIBNetIPTunnel::~EIBNetIPTunnel ()
{
  TRACEPRINTF (Thread::Loggers(), 2, this, "Close");
  Stop ();
  while (!outqueue.isempty ())
    delete outqueue.get ();
  if (sock)
    delete sock;
}

bool EIBNetIPTunnel::init ()
{
  return sock != 0;
}

bool
EIBNetIPTunnel::Send_L_Data (LPDU * l)
{
  TRACEPRINTF(Thread::Loggers(), 2, this, "Send %s", l->Decode ()());
  if (l->getType() != L_Data)
    {
      delete l;
      return false;
    }
  L_Data_PDU *l1 = (L_Data_PDU *) l;

  // careful logic, only return FALSE if l has to be dropped, CArray copies over
  if (Put_On_Queue_Or_Drop<CArray, CArray>(inqueue, L_Data_ToCEMI(0x11, *l1),
      &insignal, true, indropmsg))
    {
      if (Put_On_Queue_Or_Drop<LPDU *, LPDU *>(outqueue, l, &outsignal, true,
          outdropmsg))
        {
          if (vmode)
            {
              L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
              l2->pdu.set(l->ToPacket());
              if (!Put_On_Queue_Or_Drop<LPDU *, L_Busmonitor_PDU *>(outqueue,
                  l2, &outsignal, true, outdropmsg))
                {
                  // thta's ok, just drop the vmode ,rest already queued
                  delete l2;
                }
            }
          return true;
        }
      else
        {
          ++stat_senderr;
        }
      return false;
    }
  else
    {
      ++stat_senderr;
    }
  return false;
}

LPDU *
EIBNetIPTunnel::Get_L_Data (pth_event_t stop)
{

  if (Connection_Lost())
    {
      return NULL;
    }
  // wait for according link state change
  pth_event_t le =
      !Connection_Lost() ?
          Connection_Wait_Until_Lost() : Connection_Wait_Until_Up();
  pth_event_t getwait = pth_event (PTH_EVENT_SEM, &outsignal);

  pth_event_concat(le, getwait, NULL);
  if (stop != NULL)
    {
      pth_event_concat(getwait, stop, NULL);
    }

  TRACEPRINTF(Thread::Loggers(), 3, this, "Get_Packet waiting");

  pth_wait(le);

  pth_event_isolate(getwait);
  pth_event_isolate(le);
  if (stop)
    {
      pth_event_isolate(stop);
    }

  bool les = pth_event_status(le) == PTH_STATUS_OCCURRED;
  bool s = pth_event_status(getwait) == PTH_STATUS_OCCURRED;
  pth_event_free(le, PTH_FREE_THIS);
  pth_event_free (getwait, PTH_FREE_THIS);

    {
      outqueue.Lock();
      TRACEPRINTF(Thread::Loggers(), 3, this,
          "Get_Packet link event: %d wait-on-incoming-packet: %d incoming-queue: %d stop: %d",
          pth_event_status(le) == PTH_STATUS_OCCURRED,
          (int) outqueue.len(),
          (int) s,
          stop ? pth_event_status(stop) == PTH_STATUS_OCCURRED : -1);
      outqueue.Unlock();
    }

  if (!Connection_Lost()
      && s)
    {
      pth_sem_dec (&outsignal);
      LPDU *c = outqueue.get();

      return c;
    }

  TRACEPRINTF(Thread::Loggers(), 0, this,
      "Send empty as signal up (link loss/up or stop)");
  return NULL; // also if connection lost

}

bool
EIBNetIPTunnel::Send_Queue_Empty ()
{
  return inqueue.isempty ();
}

bool
EIBNetIPTunnel::openVBusmonitor ()
{
  vmode = 1;
  return 1;
}

bool
EIBNetIPTunnel::closeVBusmonitor ()
{
  vmode = 0;
  return 1;
}

bool
EIBNetIPTunnel::enterBusmonitor ()
{
  mode = 1;
  if (support_busmonitor)
    connect_busmonitor = 1;
  return Put_On_Queue_Or_Drop<CArray, CArray >(inqueue,  CArray(), &insignal, true, indropmsg);
#if 0
  inqueue.put (CArray ());
  pth_sem_inc (&insignal, 1);
#endif
}

bool
EIBNetIPTunnel::leaveBusmonitor ()
{
  mode = 0;
  connect_busmonitor = 0;
  return Put_On_Queue_Or_Drop<CArray, CArray >(inqueue,  CArray(), &insignal, true, indropmsg);
#if 0
  inqueue.put (CArray ());
  pth_sem_inc (&insignal, 1);
  return 1;
#endif
}

bool
EIBNetIPTunnel::Open ()
{
  return 1;
}

bool
EIBNetIPTunnel::Close ()
{
  return 1;
}

void EIBNetIPTunnel::TransitionToState(TunnelStates s,
                                       pth_event_t  timeout1)
{
  if (s!= _state) {
      struct sockaddr_in *sin = (struct sockaddr_in *) &this->caddr;
      if (s==TUNNEL_INIT) {
          ++ErrCounters::stat_resets;
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
              Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
              "Tunnel to server %s:%d lost", (const char *) inet_ntoa(sin->sin_addr),(int) ntohs(sin->sin_port));
      }
      else if (_state==TUNNEL_INIT) {
          INFOLOGSHAPE(Thread::Loggers(), LOG_INFO,
              Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
              "Tunnel to server %s:%d up and operational", (const char *) inet_ntoa(sin->sin_addr),(int) ntohs(sin->sin_port));
      }
      _state =s ;
      if (s!=TUNNEL_INIT)
        TransitionToUpState();
      else {
        pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout1,
                        pth_time(30, 0)); // restart in 30
        TransitionToDownState();
      }
  }
}

void
EIBNetIPTunnel::Run (pth_sem_t * stop1)
{
  int channel = -1;
  int rno = 0;
  int sno = 0;
  int retry = 0;
  int heartbeat = 0;
  int drop = 0;
  eibaddr_t myaddr;
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  pth_event_t input = pth_event (PTH_EVENT_SEM, &insignal);
  pth_event_t timeout = pth_event (PTH_EVENT_RTIME, pth_time (0, 0));
  pth_event_t timeout1 = pth_event (PTH_EVENT_RTIME, pth_time (10, 0));
  L_Data_PDU *c;

  EIBNetIPPacket p;
  EIBNetIPPacket *p1;
  EIBnet_ConnectRequest creq;
  creq.nat = saddr.sin_addr.s_addr == 0;
  EIBnet_ConnectResponse cresp;
  EIBnet_ConnectionStateRequest csreq;
  csreq.nat = saddr.sin_addr.s_addr == 0;
  EIBnet_ConnectionStateResponse csresp;
  EIBnet_TunnelRequest treq;
  EIBnet_TunnelACK tresp;
  EIBnet_DisconnectRequest dreq;
  dreq.nat = saddr.sin_addr.s_addr == 0;
  EIBnet_DisconnectResponse dresp;
  creq.caddr = saddr;
  creq.daddr = saddr;
  creq.CRI.resize (3);
  creq.CRI[0] = 0x04;
  creq.CRI[1] = 0x02;
  creq.CRI[2] = 0x00;
  p = creq.ToPacket ();
  sock->sendaddr = caddr;
  sock->Send (p);

  TransitionToState(TUNNEL_INIT, timeout1);
  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout1,
                          pth_time(1, 0)); // restart in 1
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      if (GetState() == TUNNEL_CONNECTED)
        pth_event_concat(stop, input, NULL);
      if (GetState() == TUNNEL_PACKET_SENT || GetState() == TUNNEL_EXPECT_ACK)
        pth_event_concat(stop, timeout, NULL);

      pth_event_concat(stop, timeout1, NULL);

      p1 = sock->Get(stop);
      pth_event_isolate(stop);
      pth_event_isolate(timeout);
      pth_event_isolate(timeout1);
      if (p1)
        {
          switch (p1->service)
            {
          case CONNECTION_RESPONSE:
            if (GetState() != TUNNEL_INIT)
              goto err;
            if (parseEIBnet_ConnectResponse(*p1, cresp))
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Recv wrong connection response");
                ++stat_recverr;
                break;
              }
            if (cresp.status != 0)
              {
                ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Connect failed with error %02X", cresp.status);
                ++stat_recverr;
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Connect failed with error %02X", cresp.status);
                if (cresp.status == 0x23 && support_busmonitor == 1
                    && connect_busmonitor == 1)
                  {
                    ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
                        Logging::DUPLICATESMAX1PER10SEC, this,
                        Logging::MSGNOHASH, "Disables busmonitor support");
                    support_busmonitor = 0;
                    connect_busmonitor = 0;
                    creq.CRI[1] = 0x02;
                    pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout1,
                        pth_time(10, 0));
                    p = creq.ToPacket();
                    WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                        Logging::DUPLICATESMAX1PER10SEC, this,
                        Logging::MSGNOHASH, "Connect Retry");
                    sock->sendaddr = caddr;
                    sock->Send(p);
                  }
                break;
              }
            if (cresp.CRD() != 3)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Recv wrong connection response");
                ++stat_recverr;
                break;
              }
            myaddr = (cresp.CRD[1] << 8) | cresp.CRD[2];
            daddr = cresp.daddr;
            if (!cresp.nat)
              {
                if (NAT)
                  daddr.sin_addr = caddr.sin_addr;
                if (dataport != -1)
                  daddr.sin_port = htons(dataport);
              }
            channel = cresp.channel;
            TransitionToState(TUNNEL_CONNECTED,timeout1);
            sno = 0;
            rno = 0;
            sock->recvaddr2 = daddr;
            sock->recvall = 3;
            pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout1,
                pth_time(30, 0));
            heartbeat = 0;
            break;

          case TUNNEL_REQUEST:
            if (GetState() == TUNNEL_INIT)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Not connected");
                ++stat_recverr;
                goto err;
              }
            if (parseEIBnet_TunnelRequest(*p1, treq))
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Invalid request");
                ++stat_recverr;
                break;
              }
            if (treq.channel != channel)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Not for our channel");
                ++stat_recverr;
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Not for us");
                break;
              }
            if (((treq.seqno + 1) & 0xff) == rno)
              {
                tresp.status = 0;
                tresp.channel = channel;
                tresp.seqno = treq.seqno;
                p = tresp.ToPacket();
                sock->sendaddr = daddr;
                sock->Send(p);
                sock->recvall = 0;
                break;
              }
            if (treq.seqno != rno)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Wrong sequence %d<->%d", treq.seqno, rno);
                ++stat_recverr;
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Wrong sequence %d<->%d", treq.seqno, rno);
                if (treq.seqno < rno)
                  treq.seqno += 0x100;
                if (treq.seqno >= rno + 5)
                  {
                    dreq.caddr = saddr;
                    dreq.channel = channel;
                    p = dreq.ToPacket();
                    sock->sendaddr = caddr;
                    sock->Send(p);
                    sock->recvall = 0;
                    TransitionToState(TUNNEL_INIT,timeout1);
                  }
                break;
              }
            rno++;
            if (rno > 0xff)
              rno = 0;
            tresp.status = 0;
            tresp.channel = channel;
            tresp.seqno = treq.seqno;
            p = tresp.ToPacket();
            sock->sendaddr = daddr;
            sock->Send(p);
            //Confirmation
            if (treq.CEMI[0] == 0x2E)
              {
                if (GetState() == TUNNEL_EXPECT_ACK)
                  TransitionToState(TUNNEL_CONNECTED,timeout1);
                break;
              }
            if (treq.CEMI[0] == 0x2B)
              {
                L_Busmonitor_PDU *l2 = CEMI_to_Busmonitor(treq.CEMI);
                if (!Put_On_Queue_Or_Drop<LPDU *, LPDU *>(outqueue, l2,
                    &outsignal, true, outdropmsg))
                  {
                    ++stat_senderr;
                    delete l2;
                  }
#if 0
                outqueue.put (l2);
                pth_sem_inc (&outsignal, 1);
#endif
                break;
              }
            if (treq.CEMI[0] != 0x29)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Unexpected CEMI Type %02X", treq.CEMI[0]);
                ++stat_recverr;
                break;
              }
            c = CEMI_to_L_Data(treq.CEMI);
            if (c)
              {

                TRACEPRINTF(Thread::Loggers(), 1, this,
                    "Recv %s", c->Decode ()());
                if (mode == 0)
                  {
                    if (vmode)
                      {
                        L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
                        l2->pdu.set(c->ToPacket());
                        if (!Put_On_Queue_Or_Drop<LPDU *, LPDU *>(outqueue, l2,
                            &outsignal, true, outdropmsg))
                          {
                            ++stat_senderr;
                            delete l2;
                          }
#if 0
                        outqueue.put (l2);
                        pth_sem_inc (&outsignal, 1);
#endif
                      }
                    if (c->AddrType == IndividualAddress && c->dest == myaddr)
                      c->dest = 0;
                    if (!Put_On_Queue_Or_Drop<LPDU *, LPDU *>(outqueue, c,
                        &outsignal, true, outdropmsg))
                      {
                        ++stat_senderr;
                        delete c;
                        c = NULL;
                      }
#if 0
                    outqueue.put (c);
                    pth_sem_inc (&outsignal, 1);
#endif
                    break;
                  }
                L_Busmonitor_PDU *p1 = new L_Busmonitor_PDU;
                p1->pdu = c->ToPacket();
                delete c;
                c = NULL;
                if (!Put_On_Queue_Or_Drop<LPDU *, LPDU *>(outqueue, p1,
                    &outsignal, true, outdropmsg))
                  {
                    ++stat_senderr;
                    delete p1;
                    p1 = NULL;
                  }
#if 0
                outqueue.put (p1);
                pth_sem_inc (&outsignal, 1);
#endif
                break;
              }
            WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                "Unknown CEMI");
            break;
          case TUNNEL_RESPONSE:
            if (GetState() == TUNNEL_INIT)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Not connected");
                ++stat_recverr;
                goto err;
              }
            if (parseEIBnet_TunnelACK(*p1, tresp))
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Invalid response");
                ++stat_recverr;
                break;
              }
            if (tresp.channel != channel)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Not for us");
                ++stat_recverr;
                break;
              }
            if (tresp.seqno != sno)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Wrong sequence %d<->%d", tresp.seqno, sno);
                ++stat_recverr;
                break;
              }
            if (tresp.status)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Error in ACK %d", tresp.status);
                ++stat_recverr;
                break;
              }
            if (GetState() == TUNNEL_PACKET_SENT)
              {
                sno++;
                if (sno > 0xff)
                  sno = 0;
                pth_sem_dec(&insignal);
                inqueue.get();
                if (noqueue)
                  {
                    TransitionToState(TUNNEL_EXPECT_ACK,timeout1);
                    pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout,
                        pth_time(1, 0));
                  }
                else
                  TransitionToState(TUNNEL_CONNECTED,timeout1);
                retry = 0;
                drop = 0;
              }
            else
              WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                  Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Unexpected ACK");
            break;
          case CONNECTIONSTATE_RESPONSE:
            if (parseEIBnet_ConnectionStateResponse(*p1, csresp))
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Invalid response");
                ++stat_recverr;
                break;
              }
            if (csresp.channel != channel)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Not for us");
                ++stat_recverr;
                break;
              }
            if (csresp.status == 0)
              {
                if (heartbeat > 0)
                  heartbeat--;
                else
                  {
                    WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                        Logging::DUPLICATESMAX1PER10SEC, this,
                        Logging::MSGNOHASH,
                        "Duplicate Connection State Response");
                    ++stat_recverr;

                  }
              }
            else if (csresp.status == 0x21)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Connection State Response not connected");
                ++stat_recverr;
                dreq.caddr = saddr;
                dreq.channel = channel;
                p = dreq.ToPacket();
                sock->sendaddr = caddr;
                sock->Send(p);
                sock->recvall = 0;
                TransitionToState(TUNNEL_INIT,timeout1);
              }
            else
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Connection State Response Error %02x", csresp.status);
                ++stat_recverr;
              }
            break;
          case DISCONNECT_REQUEST:
            if (GetState() == TUNNEL_INIT)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Not connected");
                ++stat_recverr;
                goto err;
              }
            if (parseEIBnet_DisconnectRequest(*p1, dreq))
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Invalid request");
                ++stat_recverr;
                break;
              }
            if (dreq.channel != channel)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Not for us");
                ++stat_recverr;
                break;
              }
            dresp.channel = channel;
            dresp.status = 0;
            p = dresp.ToPacket();
            Thread::Loggers()->TracePacket(1, this, "SendDis", p.data);
            sock->sendaddr = caddr;
            sock->Send(p);
            sock->recvall = 0;
            TransitionToState(TUNNEL_INIT,timeout1);
            break;
          case DISCONNECT_RESPONSE:
            if (GetState() == TUNNEL_INIT)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Response on not connected");
                ++stat_recverr;
                break;
              }
            if (parseEIBnet_DisconnectResponse(*p1, dresp))
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Invalid request");
                ++stat_recverr;
                break;
              }
            if (dresp.channel != channel)
              {
                WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Response not for us");
                ++stat_recverr;
                break;
              }
            TransitionToState(TUNNEL_INIT,timeout1);
            sock->recvall = 0;
            WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                "Disconnected");
            pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout1,
                pth_time(0, 100));
            connmod = 0;
            break;

          default:
            err:
            WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                "Recv unexpected service %04X", p1->service);
            break;
            }
          delete p1;
        }
      if (GetState() == TUNNEL_PACKET_SENT
          && pth_event_status(timeout) == PTH_STATUS_OCCURRED)
        {
          TransitionToState(TUNNEL_CONNECTED,timeout1);
          retry++;
          if (retry > 3)
            {
              WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                  Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Drop, too many retries");
              pth_sem_dec(&insignal);
              inqueue.get();
              retry = 0;
              ++stat_drops;

              drop++;
              if (drop >= 3)
                {
                  dreq.caddr = saddr;
                  dreq.channel = channel;
                  p = dreq.ToPacket();
                  sock->sendaddr = caddr;
                  sock->Send(p);
                  sock->recvall = 0;
                  TransitionToState(TUNNEL_INIT,timeout1);
                }
            }
        }
      if (GetState() == TUNNEL_EXPECT_ACK
          && pth_event_status(timeout) == PTH_STATUS_OCCURRED)
        TransitionToState(TUNNEL_CONNECTED,timeout1);
      if (GetState() != TUNNEL_INIT
          && pth_event_status(timeout1) == PTH_STATUS_OCCURRED)
        {
          pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout1,
              pth_time(30, 0));
          if (heartbeat < 5)
            {
              csreq.caddr = saddr;
              csreq.channel = channel;
              p = csreq.ToPacket();
              TRACEPRINTF(Thread::Loggers(), 1, this, "Heartbeat");
              sock->sendaddr = caddr;
              sock->Send(p);
              sock->sendaddr = daddr;
              heartbeat++;
            }
          else
            {
              TRACEPRINTF(Thread::Loggers(), 1, this,
                  "Disconnection because of errors");
              dreq.caddr = saddr;
              dreq.channel = channel;
              p = dreq.ToPacket();
              sock->sendaddr = caddr;
              if (channel != -1)
                sock->Send(p);
              sock->recvall = 0;
              TransitionToState(TUNNEL_INIT,timeout1);
            }
        }
      if (GetState() == TUNNEL_INIT
          && pth_event_status(timeout1) == PTH_STATUS_OCCURRED)
        {
          pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout1,
              pth_time(10, 0));
          creq.CRI[1] = (
              (connect_busmonitor && support_busmonitor) ? 0x80 : 0x02);
          p = creq.ToPacket();
          TRACEPRINTF(Thread::Loggers(), 1, this, "Connect retry");
          sock->sendaddr = caddr;
          sock->Send(p);
        }

      if (!inqueue.isempty() && inqueue.top()() == 0)
        {
          pth_sem_dec(&insignal);
          inqueue.get();
          if (support_busmonitor)
            {
              dreq.caddr = saddr;
              dreq.channel = channel;
              p = dreq.ToPacket();
              sock->sendaddr = caddr;
              sock->Send(p);
            }
        }

      if (!inqueue.isempty() && GetState() == TUNNEL_CONNECTED)
        {
          treq.channel = channel;
          treq.seqno = sno;
          treq.CEMI = inqueue.top();
          p = treq.ToPacket();
          Thread::Loggers()->TracePacket(1, this, "SendTunnel", p.data);
          sock->sendaddr = daddr;
          sock->Send(p);
          TransitionToState(TUNNEL_PACKET_SENT,timeout1);
          pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout, pth_time(1, 0));
        }
    }
out:
  dreq.caddr = saddr;
  dreq.channel = channel;
  p = dreq.ToPacket ();
  sock->sendaddr = caddr;
  if (channel != -1)
    sock->Send (p);

  pth_event_free (stop, PTH_FREE_THIS);
  pth_event_free (input, PTH_FREE_THIS);
  pth_event_free (timeout, PTH_FREE_THIS);
  pth_event_free (timeout1, PTH_FREE_THIS);
}


Element *
EIBNetIPTunnel::_xml(Element *parent) const
{
  static char buf[32];
  Element *n = parent->addElement(XMLBACKENDELEMENT);
  n->addAttribute(XMLBACKENDELEMENTTYPEATTR, _str());
  n->addAttribute(XMLBACKENDSTATUSATTR, Connection_Lost() ?  XMLSTATUSDOWN : XMLSTATUSUP );

  struct sockaddr_in *sin = (struct sockaddr_in *) &this->caddr;
  snprintf(buf,sizeof(buf)-1,"%s:%d", (const char *) inet_ntoa(sin->sin_addr),(int) ntohs(sin->sin_port));
  n->addAttribute(XMLBACKENDADDRESSATTR,buf);

  ErrCounters::_xml(n);
  outqueue._xml(n);
  inqueue._xml(n);
  return n;
}

void
EIBNetIPTunnel::logtic()
{
  INFOLOGSHAPE(Thread::Loggers(), LOG_INFO, Logging::DUPLICATESMAX1PERMIN, this,
      Logging::MSGNOHASH,
      "%s: received %d pkts, sent %d pkts, %d receive errors, %d send errors, %d resets ",
      _str(),
      (int) *inqueue.stat_inserts,
      (int) *outqueue.stat_inserts,
      (int) *stat_recverr,
      (int) *stat_senderr,
      (int) *stat_resets);
}
