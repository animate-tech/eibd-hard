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

#include <errno.h>
#include <unistd.h>
#include "server.h"
#include "client.h"
#include "busmonitor.h"
#include "connection.h"
#include "managementclient.h"
#include "groupcacheclient.h"
#include "state.h"
#include "xmlccwrap.h"
#include "config.h"
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>


ClientConnection::ClientConnection (Server * s, Layer3 * l3,
				    Logs * tr,  int inquemaxlen, int outquemaxlen, int peerquemaxlen,
				    int fd, struct sockaddr *addr) :
  Thread(tr,PTH_PRIO_STD, "client connection"),
  created(pth_timeout(0,0))
{
  TRACEPRINTF (Loggers(), 8, this, "ClientConnection Init %s", get_addr_str(addr));
  this->fd = fd;
  this->l3 = l3;
  if (addr)
    this->addr = *addr;
  else
    this->addr.sa_family = AF_UNSPEC;

  this->s = s;

  buf = 0;
  buflen = 0;
  size=0;
}

ClientConnection::~ClientConnection ()
{
  TRACEPRINTF (Loggers(), 8, this, "ClientConnection closed %s", get_addr_str(&addr));
  s->deregister (this);
  if (buf)
    delete[]buf;
  close (fd);
}

const
void ClientConnection::LogBusMon(bool start) const
{
  INFOLOG (Loggers(), LOG_INFO, this,
      " %s busmonitor from %s", (start ? "starting" : "finished" ), get_addr_str(&addr));
}

void
ClientConnection::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      if (readmessage (stop) == -1)
	break;
      int msg = EIBTYPE (buf);

      switch (msg)
	{
	case EIB_OPEN_BUSMONITOR:
	  {
            LogBusMon(true);
	    A_Busmonitor busmon (this, l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength());
	    busmon.Do (stop);
	    LogBusMon(false);
	  }
	  break;

	case EIB_OPEN_BUSMONITOR_TEXT:
	  {
            LogBusMon(true);
	    A_Text_Busmonitor busmon (this, l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength());
	    busmon.Do (stop);
            LogBusMon(false);
	  }
	  break;

	case EIB_OPEN_VBUSMONITOR:
	  {
            LogBusMon(true);
	    A_Busmonitor busmon (this, l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength(), 1);
	    busmon.Do (stop);
            LogBusMon(false);
	  }
	  break;

	case EIB_OPEN_VBUSMONITOR_TEXT:
	  {
            LogBusMon(true);
	    A_Text_Busmonitor busmon (this, l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength(), 1);
	    busmon.Do (stop);
	    LogBusMon(false);

	  }
	  break;

	case EIB_OPEN_T_BROADCAST:
	  {
	    A_Broadcast cl (l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength(), this);
	    cl.Do (stop);
	  }
	  break;

	case EIB_OPEN_T_GROUP:
	  {
	    A_Group cl (l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength(), this);
	    cl.Do (stop);
	  }
	  break;

	case EIB_OPEN_T_INDIVIDUAL:
	  {
	    A_Individual cl (l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength(), this);
	    cl.Do (stop);
	  }
	  break;

	case EIB_OPEN_T_TPDU:
	  {
	    A_TPDU cl (l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength(), this);
	    cl.Do (stop);
	  }
	  break;

	case EIB_OPEN_T_CONNECTION:
	  {
	    A_Connection cl (l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength(), this);
	    cl.Do (stop);
	  }
	  break;

	case EIB_OPEN_GROUPCON:
	  {
	    A_GroupSocket cl (l3, Loggers(), s->maxInQueueLength(), s->maxOutQueueLength(), s->maxPeerQueueLength(), this);
	    cl.Do (stop);
	  }
	  break;

	case EIB_M_INDIVIDUAL_ADDRESS_READ:
	  ReadIndividualAddresses (l3, Loggers(), this, stop);
	  break;

	case EIB_PROG_MODE:
	  ChangeProgMode (l3, Loggers(), this, stop);
	  break;

	case EIB_MASK_VERSION:
	  GetMaskVersion (l3, Loggers(), this, stop);
	  break;

	case EIB_M_INDIVIDUAL_ADDRESS_WRITE:
	  WriteIndividualAddress (l3, Loggers(), this, stop);
	  break;

	case EIB_MC_CONNECTION:
	  ManagementConnection (l3, Loggers(), this, stop);
	  break;

	case EIB_MC_INDIVIDUAL:
	  ManagementIndividual (l3, Loggers(), this, stop);
	  break;

	case EIB_LOAD_IMAGE:
	  LoadImage (l3, Loggers(), this, stop);
	  break;

	case EIB_CACHE_ENABLE:
	case EIB_CACHE_DISABLE:
	case EIB_CACHE_CLEAR:
	case EIB_CACHE_REMOVE:
	case EIB_CACHE_READ:
	case EIB_CACHE_READ_NOWAIT:
	case EIB_CACHE_LAST_UPDATES:
#ifdef HAVE_GROUPCACHE
	  GroupCacheRequest (l3, Loggers(), this, stop);
#else
	  sendreject (stop);
#endif
	  break;

	case EIB_RESET_CONNECTION:
	  sendreject (stop, EIB_RESET_CONNECTION);
	  EIBSETTYPE (buf, EIB_INVALID_REQUEST);
	  break;

	case EIB_STATE_REQ_THREADS:
	  this->s->Daemon()->StateThreads(l3, Loggers(), this, stop);
	  break;

	case EIB_STATE_REQ_BACKENDS:
	  this->s->Daemon()->StateBackends(l3, Loggers(), this, stop);
	  break;

	case EIB_STATE_REQ_SERVERS:
	  this->s->Daemon()->StateServers(l3, Loggers(), this, stop);
	  break;

	default:
	  WARNLOGSHAPE (Loggers(), LOG_NOTICE, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
			"Received unknown request %d from %s",
			(int) msg,
			(const char *) inet_ntoa(((struct sockaddr_in *) &this->addr)->sin_addr));

	  ++stat_recverr;
	  sendreject (stop);
	}
      if (EIBTYPE (buf) == EIB_RESET_CONNECTION)
	sendreject (stop, EIB_RESET_CONNECTION);
    }
  pth_event_free (stop, PTH_FREE_THIS);
  StopDelete ();
}

int
ClientConnection::sendreject (pth_event_t stop)
{
  uchar buf[2];
  EIBSETTYPE (buf, EIB_INVALID_REQUEST);
  return sendmessage (2, buf, stop);
}

int
ClientConnection::sendreject (pth_event_t stop, int type)
{
  uchar buf[2];
  EIBSETTYPE (buf, type);
  return sendmessage (2, buf, stop);
}

int
ClientConnection::sendmessage (int size, const uchar * msg, pth_event_t stop)
{
  int i;
  int start;
  uchar head[2];
  assert (size >= 2);

  Loggers()->TracePacket (8, this, "SendMessage", size, msg);
  head[0] = (size >> 8) & 0xff;
  head[1] = (size) & 0xff;

  i = pth_write_ev (fd, head, 2, stop);
  if (i != 2) {
    ++stat_senderr;
    return -1;
  }

  start = 0;
lp:
  i = pth_write_ev (fd, msg + start, size - start, stop);
  if (i <= 0) {
    ++stat_senderr;
    return -1;
  }
  start += i;
  if (start < size)
    goto lp;
  ++stat_packets_sent;
  return 0;
}

int
ClientConnection::readmessage (pth_event_t stop)
{
  uchar head[2];
  int i;
  unsigned start;

  i = pth_read_ev (fd, &head, 2, stop);
  if (i != 2) {
      ++stat_recverr;
      return -1;
  }

  size = (head[0] << 8) | (head[1]);
  if (size < 2) {
    ++stat_recverr;
    return -1;
  }

  if (size > buflen)
    {
      if (buf)
	delete[]buf;
      buf = new uchar[size];
      buflen = size;
    }

  start = 0;
lp:
  i = pth_read_ev (fd, buf + start, size - start, stop);
  if (i <= 0) {
    ++stat_recverr;
    return -1;
  }

  start += i;
  if (start < size)
    goto lp;

  Loggers()->TracePacket (8, this, "RecvMessage", size, buf);
  ++stat_packets_received;
  return 0;
}

Element * ClientConnection::_xml(Element *parent) const
{
  Element *p=parent->addElement(XMLCLIENTELEMENT);
  char buf[64];

  p->addAttribute(XMLCLIENTTYPEATTR,this->addr.sa_family == AF_INET ? "IP" : "");
  p->addAttribute(XMLCLIENTSTARTTIMEATTR,created.fmtString("%c"));
  p->addAttribute(XMLCLIENTSMESSAGESRECVATTR,*stat_packets_received);
  p->addAttribute(XMLCLIENTSMESSAGESSENTATTR,*stat_packets_sent);
  if (*stat_recverr)
    {
      p->addAttribute(XMLCLIENTSRECVERRATTR,*stat_recverr);
    }
  if (*stat_senderr)
    {
      p->addAttribute(XMLCLIENTSRECVERRATTR,*stat_senderr);
    }
  if (this->addr.sa_family==AF_INET)
    {
      struct sockaddr_in *sin = (struct sockaddr_in *) &this->addr;
      snprintf(buf,sizeof(buf)-1,"%s:%d", (const char *) inet_ntoa(sin->sin_addr),(int) ntohs(sin->sin_port));
      p->addAttribute(XMLCLIENTADDRESSATTR,buf);
    }
  return p;
}
