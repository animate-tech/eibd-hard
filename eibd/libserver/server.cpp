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

#include <unistd.h>
#include "server.h"
#include "client.h"


Server::~Server ()
{
  pth_mutex_acquire(&this->lock,false,NULL);
  TRACEPRINTF (Loggers(), 8, this, "StopServer");
  Stop ();
  for (int i = 0; i < connections (); i++)
    connections[i]->StopDelete ();
  while (connections () != 0)
    pth_yield (0);

  if (fd != -1)
    close (fd);
  TRACEPRINTF (Loggers(), 8, this, "Server ended");
  pth_mutex_release(&this->lock);
}

bool
Server::deregister (ClientConnection * con)
{
  pth_mutex_acquire(&this->lock,false,NULL);
  for (unsigned i = 0; i < connections (); i++)
    if (connections[i] == con)
      {
	// update our counters
	stat_packets_sent += ( *con->stat_packets_sent );
	stat_packets_received += ( *con->stat_packets_received );
	stat_recverr	 += ( *con->stat_recverr );
	stat_senderr	 += ( *con->stat_senderr );
	connections.deletepart (i, 1);
	pth_mutex_release(&this->lock);
	return 1;
      }
  return 0;
  pth_mutex_release(&this->lock);
}

Server::Server (Layer3 * layer3,
		const char *threadname,
		Logs * tr,
	    DaemonInstance *d,
		int inquemaxlen, int outquemaxlen, int peerquemaxlen, int clientsmax,
	            IPv4NetList &ipnetfilters) :
  Thread(tr,PTH_PRIO_STD, threadname),
  stat_maxconcurrentclients(0),
  stat_totalclients(0),
  stat_clientsrejected(0),
  stat_clientsauthfail(0),
  stat_packets_sent(0),
  stat_packets_received(0),
  stat_recverr(0),
  stat_senderr(0),
  daemon(d),
  ipnetfilters(ipnetfilters)
{
  l3 = layer3;
  this->inqueuemaxlen = inquemaxlen;
  this->outqueuemaxlen = outquemaxlen;
  this->peerqueuemaxlen = peerqueuemaxlen;
  this->clientsmax = clientsmax;
  pth_mutex_init (&this->lock);
  fd = -1;
}

void
Server::Run (pth_sem_t * stop1)
{
  struct sockaddr addr;
  socklen_t l;

  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      int cfd;
      l = sizeof(struct sockaddr);
      cfd = pth_accept_ev(fd, &addr, &l, stop);

      if (cfd != -1)
        {
          ++stat_totalclients;

          pth_mutex_acquire(&this->lock, false, NULL);

          if (this->clientsmax && connections.len() > this->clientsmax)
            {
              WARNLOGSHAPE(Loggers(), LOG_WARNING,
                  Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Too many clients present, client request rejected");
              ++stat_clientsrejected;
              close(cfd);
              cfd = -1;
            }
          if (l>0 && ipnetfilters.size()>0 && addr.sa_family==AF_INET)
            {
              bool infilters=false;
              for (IPv4NetList::const_iterator i=ipnetfilters.begin(); i!=ipnetfilters.end() && !infilters; i++)
                {
                  infilters |= i->contains( IPv4(addr));
                }
              if (!infilters) {
                  WARNLOGSHAPE(Loggers(), LOG_WARNING,
                                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                                    "Client from addr %s rejected due to subnet filtering", get_addr_str(&addr));
                  ++stat_clientsrejected;
                  close(cfd);
                  cfd = -1;
              }
            }
          if (cfd != -1)
            {
              TRACEPRINTF(Loggers(), 8, this, "New Connection");
              setupConnection(cfd);
              ClientConnection *c = new ClientConnection(this, l3, Loggers(),
                  inqueuemaxlen, outqueuemaxlen, peerqueuemaxlen, cfd,
                  l > 0 ? &addr : NULL);
              connections.setpart(&c, connections(), 1);
              c->Start();
            }

          pth_mutex_release(&this->lock);
        }
  }
  pth_event_free (stop, PTH_FREE_THIS);
}

void
Server::setupConnection (int cfd)
{
  if (*stat_maxconcurrentclients < connections.len()) {
    ++stat_maxconcurrentclients;
  }
}

Element * Server::_xml(Element *parent)
{

  pth_mutex_acquire(&this->lock,false,NULL);

  Element *p=parent->addElement(XMLSERVERELEMENT);
  p->addAttribute(XMLSTATUSELEMENT,XMLSTATUSUP);

  p->addAttribute(XMLSERVERMAXCLIENTSATTR, *stat_maxconcurrentclients);
  p->addAttribute(XMLSERVERCLIENTSATTR, connections.len());

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


  UIntStatisticsCounter  stat_packets_sent_tmp(stat_packets_sent);
  UIntStatisticsCounter  stat_packets_received_tmp(stat_packets_received);
  UIntStatisticsCounter  stat_recverr_tmp(stat_recverr);
  UIntStatisticsCounter  stat_senderr_tmp(stat_senderr);

  // loop over all not joined connections && add up their counters up to now

  for (int i = 0; i < connections (); i++) {
    stat_packets_sent_tmp += connections[i]->stat_packets_sent;
    stat_packets_received_tmp += connections[i]->stat_packets_received;
    stat_recverr_tmp += connections[i]->stat_recverr;
    stat_senderr_tmp += connections[i]->stat_senderr;
    connections[i]->_xml(p);
  }

  p->addAttribute(XMLSERVERMESSAGESSENTATTR, *stat_packets_sent_tmp);
  p->addAttribute(XMLSERVERMESSAGESRECVATTR, *stat_packets_received_tmp);
  if (*stat_recverr_tmp)
    {
      p->addAttribute(XMLSERVERRECVERRATTR, *stat_recverr_tmp);
    }
  if (*stat_senderr_tmp)
    {
      p->addAttribute(XMLSERVERSENDERRATTR, *stat_senderr_tmp);
    }

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
      p->addAttribute(XMLSERVERSUBNETFILTERS,r.c_str());
    }

  pth_mutex_release(&this->lock);
  return p;
}

