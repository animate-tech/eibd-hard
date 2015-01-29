/*
    EIBD client library
    Copyright (C) 2007- written by Z2 GmbH, T. Przygienda <prz@net4u.ch>
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License,
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any
    restriction coming from the use of this file. (The General Public
    License restrictions do apply in other respects; for example, they
    cover modification of the file, and distribution when not linked into
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "state.h"
#include <stdio.h>
#include "xmlccwrap.h"
#include <config.h>
#include <threads.h>

void
DaemonInstance::StateThreads (Layer3 * l3, Logs * t, ClientConnection * c,
              pth_event_t stop)
{
  uchar buf[3];
  EIBSETTYPE (buf, EIB_STATE_REQ_THREADS);
  buf[2] = 0;

  c->sendmessage (sizeof(buf), buf, stop);

  pth_ctrl(PTH_CTRL_DUMPSTATE, stdout);
  for ( Thread::pthset::iterator i= Thread::get_running_threads().begin(); i!= Thread::get_running_threads().end(); ++i)
    {
      unsigned char *n;
      static pth_time_t t;
      static pth_attr_t tr;
      int dispatches;
      tr=pth_attr_of(*i);
      pth_attr_get(tr,PTH_ATTR_NAME,&n);
      pth_attr_get(tr,PTH_ATTR_TIME_RAN,&t);
      pth_attr_get(tr,PTH_ATTR_DISPATCHES,&dispatches);
      printf("0x%lx %32s:%6d:\t", (unsigned long) *i, n, (int) dispatches);
      printf("%ld.%ld\n", (unsigned long) t.tv_sec,
                          (unsigned long) t.tv_usec/1000);
    }
}

void DaemonInstance::StateBackends(Layer3 * l3, Logs * t, ClientConnection * c,
		   pth_event_t stop)
{
  /* generate XML output of the daemon state */
  XMLTree xmltree;
  CArray erg;

  StateWrapper(xmltree,erg,l3,t,c,stop,1);
}

void DaemonInstance::StateServers(Layer3 * l3, Logs * t, ClientConnection * c,
			pth_event_t stop)
{
  /* generate XML output of the daemon state */
  XMLTree xmltree;
  CArray erg;

  StateWrapper(xmltree,erg,l3,t,c,stop,2);
}


