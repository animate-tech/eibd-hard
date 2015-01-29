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

#include "eibusb.h"
#include "emi.h"

LowLevelDriverInterface *
initUSBDriver (LowLevelDriverInterface * i, Logs * tr, int flags,
	       int inquemaxlen, int outquemaxlen,
	       int maxpacketsoutpersecond)
{
  // the whole USB reset code has been moved into the low level driver which is the
  // CORRECT place for it. Each time the USB interface gets plugged/unplugged it needs
  // to run this
  LowLevelDriverInterface *iface = NULL;
  iface = new USBConverterInterface(i, tr, flags, inquemaxlen, outquemaxlen,
      maxpacketsoutpersecond);
  // must be done by upper layers coming up to have a single logic
  // iface->SendReset();
  return iface;

}

Element * USBConverterInterface::_xml(Element *parent) const
{
  return i->_xml(parent);
}

USBConverterInterface::USBConverterInterface (LowLevelDriverInterface * iface,
					      Logs * tr,
					      int flags,
					      int inquemaxlen, int outquemaxlen,
					      int maxpacketsoutpersecond) :
  LowLevelDriverInterface(tr, flags, inquemaxlen, outquemaxlen,maxpacketsoutpersecond)
{
  i = iface;
  SetProxy(i,true); // proxy us including version
  TRACEPRINTF(Loggers(), 2, this, "Allocated @ %p proxy intf %p ", this, & Proxy());
}

USBConverterInterface::~USBConverterInterface ()
{
	if (i) {
		delete i;
	}
}

bool USBConverterInterface::init ()
{
  return i ? i->init () : false;
}

bool
USBConverterInterface::Send_Packet (CArray l, bool always)
{
  Loggers()->TracePacket (0, this, "USBConverterInterface Send_Packet", l);
  CArray out;
  int j, l1;

  if (!i) return false;

  l1 = l ();
  out.resize (64);
  if (l1 + 11 > 64)
    l1 = 64 - 11;
  for (j = 0; j < out (); j++)
    out[j] = 0;
  for (j = 0; j < l1; j++)
    out[j + 11] = l[j];
  out[0] = 1;
  out[1] = 0x13;
  out[2] = l1 + 8;
  out[3] = 0x0;
  out[4] = 0x08;
  out[5] = 0x00;
  out[6] = l1;
  out[7] = 0x01;
  switch (i->getEMIVer())
    {
    case vEMI1:
      out[8] = 0x01;
      break;
    case vEMI2:
      out[8] = 0x02;
      break;
    case vCEMI:
      out[8] = 0x03;
      break;
    }
  return i->Send_Packet (out,always);
}

CArray *
USBConverterInterface::Get_Packet (pth_event_t stop,bool readwhenstatusdown)
{
  if (!i) return NULL;
  CArray *res1 = i->Get_Packet (stop, readwhenstatusdown);
  if (res1)
    {
      CArray res = *res1;
      if (res () != 64)
	goto out;
      if (res[0] != 0x01)
	goto out;
      if ((res[1] & 0x0f) != 3)
	goto out;
      if (res[2] > 64 - 8)
	goto out;
      if (res[3] != 0)
	goto out;
      if (res[4] != 8)
	goto out;
      if (res[5] != 0)
	goto out;
      if (res[6] + 11 > 64)
	goto out;
      if (res[7] != 1)
	goto out;
      switch (i->getEMIVer())
	{
	case vEMI1:
	  if (res[8] != 1)
	    goto out;
	  break;
	case vEMI2:
	  if (res[8] != 2)
	    goto out;
	  break;
	case vCEMI:
	  if (res[8] != 3)
	    goto out;
	  break;
	default:
	  goto out;
	}
      res1->set (res.array () + 11, res[6]);
      Loggers()->TracePacket (0, this, "USBConverterInterface::Get_Packet", *res1);
    }
  return res1;

out:
  delete res1;
  return 0;
}

bool
USBConverterInterface::SendReset ()
{
  return i ? i->SendReset () : false;
}

EMIVer USBConverterInterface::getEMIVer ()
{
  return i ? i->getEMIVer() : vUnknown;
}

pth_sem_t *
USBConverterInterface::Send_Queue_Empty_Cond ()
{
  return i ? i->Send_Queue_Empty_Cond () : NULL;
}

bool
USBConverterInterface::Send_Queue_Empty ()
{
  return i ? i->Send_Queue_Empty () : true;
}

Element * USBLayer2Interface::_xml(Element *parent) const
{
  return emi ? emi->_xml(parent) : parent;
}

USBLayer2Interface::USBLayer2Interface (LowLevelDriverInterface * i,
					Logs * tr, int flags,
					int inquemaxlen, int outquemaxlen,
					int maxpacketsoutpersecond) :
					Layer2Interface(tr,flags)
{
  emi = NULL;
  iface = initUSBDriver(i, tr, flags, inquemaxlen,
      outquemaxlen, maxpacketsoutpersecond);
  if (iface)
    {
      iface->SendReset();
      emi = new EMILayer2Interface(iface, flags, Loggers(), inquemaxlen, outquemaxlen);
    }
  SetProxy(emi,true);
  TRACEPRINTF(Loggers(), 2, this, "Allocated @ %p emi to: %p proxying to emi", this,
      & Proxy());
}

USBLayer2Interface::~USBLayer2Interface ()
{
  if (emi)
    delete emi;
}

bool USBLayer2Interface::init ()
{
  return emi != 0;
}
bool USBLayer2Interface::addAddress (eibaddr_t addr)
{
  return emi->addAddress (addr);
}

bool USBLayer2Interface::addGroupAddress (eibaddr_t addr)
{
  return emi->addGroupAddress (addr);
}

bool USBLayer2Interface::removeAddress (eibaddr_t addr)
{
  return  emi->removeAddress (addr);
}

bool USBLayer2Interface::removeGroupAddress (eibaddr_t addr)
{
  return  emi->removeGroupAddress (addr);
}

eibaddr_t USBLayer2Interface::getDefaultAddr ()
{
  return  emi->getDefaultAddr ();
}

bool USBLayer2Interface::openVBusmonitor ()
{
  return  emi->openVBusmonitor ();
}

bool USBLayer2Interface::closeVBusmonitor ()
{
  return  emi->closeVBusmonitor ();
}

bool USBLayer2Interface::enterBusmonitor ()
{
  return  emi->enterBusmonitor ();
}

bool USBLayer2Interface::leaveBusmonitor ()
{
  return  emi->leaveBusmonitor ();
}

bool USBLayer2Interface::Open ()
{
  return  emi->Open ();
}

bool USBLayer2Interface::Close ()
{
  return  emi->Close ();
}

bool USBLayer2Interface::Send_Queue_Empty ()
{
  return  emi->Send_Queue_Empty ();
}

bool
USBLayer2Interface::Send_L_Data (LPDU * l)
{
  return  emi->Send_L_Data (l);
}

LPDU *
USBLayer2Interface::Get_L_Data (pth_event_t stop)
{
  return  emi->Get_L_Data (stop);
}
