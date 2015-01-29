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

#ifndef EIBDINST_H
#define EIBDINST_H

#include "config.h"
#include "layer2.h"
#include "eibnetserver.h"
#include "localserver.h"
#include "inetserver.h"
#include "state.h"

class EIBDInstance : public DaemonInstance {
public:

  InetServer  *inetserver;
  LocalServer *localserver;

#ifdef HAVE_EIBNETIPSERVER
  EIBnetServer *serv;
#endif

  EIBDInstance(void) : inetserver(NULL), localserver(NULL)
#ifdef HAVE_EIBNETIPSERVER
    , serv(0)
#endif
{ };
  ~EIBDInstance(void) { };


  void StateWrapper(
  		  XMLTree &xmltree,
  		  CArray &erg,
  		  Layer3 * l3,
  		  Logs * t,
  		  ClientConnection * c,
  		  pth_event_t &stop,
  		  int which);

};

#endif
