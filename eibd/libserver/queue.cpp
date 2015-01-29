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

#include "common.h"
#include "eibtypes.h"
#include "eibpriority.h"
#include <iostream>

#include "layer4.h"
#include "eibnetip.h"

template < class T >
Element * Queue<T>::_xml(Element *parent) const
{
  Lock();
  Element *p=parent->addElement(XMLQUEUESTATELEMENT);
  p->addAttribute(XMLQUEUENAMEATTR,_name);
  p->addAttribute(XMLQUEUECURRENTLENATTR,  _len);
  if (maxlen)
    p->addAttribute(XMLQUEUEMAXLENATTR,  maxlen);
#if HAVE_QUEUESTATS
  p->addAttribute(XMLQUEUEMAXREACHLENATTR,  *stat_maxlen);
  p->addAttribute(XMLQUEUEINSERTSATTR,  *stat_inserts);
  if (*stat_drops)
    p->addAttribute(XMLQUEUEDROPSATTR,  *stat_drops);

  int cameandgo = *stat_inserts - _len - *stat_drops;
  if (cameandgo) {
    p->addAttribute(XMLQUEUEMAXDELAYATTR,
		    (int) (1000 * stat_longest_delay.sec() + stat_longest_delay.millisec()));
    p->addAttribute(XMLQUEUEMEANDELAYATTR,
		    (int) ((1000 * stat_delay_sum.sec() + stat_delay_sum.millisec()) / cameandgo));
    // std::cout << stat_longest_delay << "\n";
    // std::cout << stat_delay_sum << "\n";
    // std::cout << cameandgo << "\n";
    // std::cout << stat_delay_sum.sec() << "//" << stat_delay_sum.millisec() << "\n";
    // std::cout << "---\n";
  }

#endif
  Unlock();
  return p;
}

// instantiate queues
template class Queue < BroadcastComm >;
template class Queue < GroupAPDU >;
template class Queue < GroupComm >;
template class Queue < TpduComm >;
template class Queue < CArray >;
template class Queue < L_Data_PDU * >;


template class Queue < CArray * >;
template class Queue < LPDU * >;
template class Queue < struct _EIBNetIP_Send >;
template class Queue < EIBNetIPPacket >;
template class Queue < L_Busmonitor_PDU * >;
template class Queue < class Server * >;
