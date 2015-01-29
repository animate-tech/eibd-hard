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

#include "eibnetrouter.h"
#include "emi.h"
#include "config.h"
#include "eibtypes.h"
#include "eibpriority.h"

const char EIBNetIPRouter::indropmsg[] = "EIBNetIPRouter: incoming queue length exceeded, dropping packet";
const char EIBNetIPRouter::outdropmsg[] = "EIBNetIPRouter: outgoing queue length exceeded, dropping packet";


Element * EIBNetIPRouter::_xml(Element *parent) const
{
  Element *n = parent->addElement(XMLBACKENDELEMENT);
  n->addAttribute(XMLBACKENDELEMENTTYPEATTR, _str());
  n->addAttribute(XMLBACKENDSTATUSATTR, "?");
  outqueue._xml(n);
  if (ipnetfilters.size())
    {
      std::string r="";
      for (IPv4NetList::const_iterator i=ipnetfilters.begin(); i!=ipnetfilters.end(); i++)
      {
          if (i!=ipnetfilters.begin())  {
            r += ",";
          }
          std::string s(*i);
          r += s;
      }
      n->addAttribute(XMLSERVERSUBNETFILTERS,r.c_str());
    }
  return n;
}

EIBNetIPRouter::EIBNetIPRouter (const char *multicastaddr, int port,
				eibaddr_t a, Logs * tr, int inquemaxlen, int outquemaxlen, int peerquemaxlen,
				IPv4NetList &ipnetfilters) :
				Layer2Interface(tr,0),
				Thread(tr,PTH_PRIO_STD, "EIBNetIPRouter"),
				ipnetfilters(ipnetfilters)
{
  struct sockaddr_in baddr;
  struct ip_mreq mcfg;
  TRACEPRINTF (Thread::Loggers(), 2, this, "Open");
  addr = a;
  mode = 0;
  vmode = 0;
  memset (&baddr, 0, sizeof (baddr));
#ifdef HAVE_SOCKADDR_IN_LEN
  baddr.sin_len = sizeof (baddr);
#endif
  baddr.sin_family = AF_INET;
  baddr.sin_port = htons (port);
  baddr.sin_addr.s_addr = htonl (INADDR_ANY);
  pth_sem_init (&out_signal);
  sock = new EIBNetIPSocket (baddr, 1, Thread::Loggers(), inquemaxlen, outquemaxlen, peerquemaxlen,ipnetfilters);
  if (!sock->init ())
    {
      delete sock;
      sock = 0;
      throw Exception (DEV_OPEN_FAIL);
    }
  sock->recvall = 2;
  if (GetHostIP (&sock->sendaddr, multicastaddr) == 0)
    {
		  delete sock;
	      sock = 0;
      ERRORLOGSHAPE (Thread::Loggers(), LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                "Cannot get Host IP");
      throw Exception (DEV_OPEN_FAIL);
    }
  sock->sendaddr.sin_port = htons (port);
  if (!GetSourceAddress (&sock->sendaddr, &sock->localaddr))
    return;
  sock->localaddr.sin_port = sock->sendaddr.sin_port;

  mcfg.imr_multiaddr = sock->sendaddr.sin_addr;
  mcfg.imr_interface.s_addr = htonl (INADDR_ANY);
  if (!sock->SetMulticast (mcfg, tr))
    {
      delete sock;
      sock = 0;
      throw Exception (DEV_OPEN_FAIL);
    }
  Start ();
  TRACEPRINTF (Thread::Loggers(), 2, this, "Opened");
}

EIBNetIPRouter::~EIBNetIPRouter ()
{
  TRACEPRINTF (Thread::Loggers(), 2, this, "Destroy");
  Stop ();
  while (!outqueue.isempty ())
    delete outqueue.get ();
  if (sock)
    delete sock;
}

bool
EIBNetIPRouter::init ()
{
  return sock != 0;
}

bool
EIBNetIPRouter::Send_L_Data (LPDU * l)
{
  TRACEPRINTF (Thread::Loggers(), 2, this, "Send %s", l->Decode ()());
  if (l->getType () != L_Data)
    {
      delete l;
      return false;
    }
  L_Data_PDU *l1 = (L_Data_PDU *) l;
  EIBNetIPPacket p;
  p.data = L_Data_ToCEMI (0x29, *l1);
  p.service = ROUTING_INDICATION;
  sock->Send (p);
  if (vmode)
    {
      L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
      l2->pdu.set (l->ToPacket ());
      outqueue.put (l2);
      pth_sem_inc (&out_signal, 1);
    }
  outqueue.put (l);
  pth_sem_inc (&out_signal, 1);
  return true;
}

LPDU *
EIBNetIPRouter::Get_L_Data (pth_event_t stop)
{
  pth_event_t getwait = pth_event (PTH_EVENT_SEM, &out_signal);
  if (stop != NULL)
    pth_event_concat (getwait, stop, NULL);

  pth_wait (getwait);

  pth_event_isolate (getwait);
  if (stop)
    pth_event_isolate(stop);

  bool s=pth_event_status (getwait) == PTH_STATUS_OCCURRED;
  pth_event_free (getwait, PTH_FREE_THIS);

  if (s)
    {
      pth_sem_dec (&out_signal);
      LPDU *l = outqueue.get ();
      TRACEPRINTF (Thread::Loggers(), 2, this, "Recv %s", l->Decode ()());
      return l;
    }
  else
    return 0;
}


void
EIBNetIPRouter::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      EIBNetIPPacket *p = sock->Get (stop);
      if (p)
	{
	  if (p->service != ROUTING_INDICATION)
	    {
	      delete p;
	      continue;
	    }
	  if (p->data () < 2 || p->data[0] != 0x29)
	    {
	      delete p;
	      continue;
	    }
	  const CArray data = p->data;
	  delete p;
	  L_Data_PDU *c = CEMI_to_L_Data (data);
	  if (c)
	    {
	      TRACEPRINTF (Thread::Loggers(), 2, this, "Recv %s", c->Decode ()());
	      if (mode == 0)
		{
		  if (vmode)
		    {
		      L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
		      l2->pdu.set (c->ToPacket ());
		      outqueue.put (l2);
		      pth_sem_inc (&out_signal, 1);
		    }
		  outqueue.put (c);
		  pth_sem_inc (&out_signal, 1);
		  continue;
		}
	      L_Busmonitor_PDU *p1 = new L_Busmonitor_PDU;
	      p1->pdu = c->ToPacket ();
	      delete c;
	      outqueue.put (p1);
	      pth_sem_inc (&out_signal, 1);
	      continue;
	    }
	}
    }
  pth_event_free (stop, PTH_FREE_THIS);
}

bool
EIBNetIPRouter::addAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPRouter::addGroupAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPRouter::removeAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPRouter::removeGroupAddress (eibaddr_t addr)
{
  return 1;
}

bool
EIBNetIPRouter::openVBusmonitor ()
{
  vmode = 1;
  return 1;
}

bool
EIBNetIPRouter::closeVBusmonitor ()
{
  vmode = 0;
  return 1;
}

bool
EIBNetIPRouter::enterBusmonitor ()
{
  mode = 1;
  return 1;
}

bool
EIBNetIPRouter::leaveBusmonitor ()
{
  mode = 0;
  return 1;
}

bool
EIBNetIPRouter::Open ()
{
  mode = 0;
  return 1;
}

bool
EIBNetIPRouter::Close ()
{
  return 1;
}

eibaddr_t
EIBNetIPRouter::getDefaultAddr ()
{
  return addr;
}

bool
EIBNetIPRouter::Send_Queue_Empty ()
{
  return 1;
}


void
EIBNetIPRouter::logtic()
{
  INFOLOGSHAPE(Thread::Loggers(), LOG_INFO, Logging::DUPLICATESMAX1PERMIN, this,
      Logging::MSGNOHASH,
      "%s: sent %d pkts, %d receive errors, %d send errors, %d resets ",
      _str(),
      (int) *outqueue.stat_inserts,
      (int) *stat_recverr,
      (int) *stat_senderr,
      (int) *stat_resets);
}

