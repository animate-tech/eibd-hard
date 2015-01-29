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

#ifndef C_USB_H
#define C_USB_H

#include "eibusb.h"
#include "usbif.h"
#include "usb.h"
#include "layer2.h"
#include "ip/ipv4net.h"

#define USB_URL "usb:[bus[:device[:config[:interface]]]]\n"
#define USB_DOC "usb connects over a KNX USB interface\n\n"
#define USB_PREFIX "usb"
#define USB_CREATE Usb_Create
#define USB_CLEANUP USBEnd

inline Layer2Interface *
Usb_Create (const char *dev, int flags, Logs * t,
	    int inquemaxlen, int outquemaxlen,
	    int maxpacketsoutpersecond,
	    int peerquemaxlen,
            IPv4NetList &ipnetfilters)
{
  // printf ("%d %d %d %d\n", inquemaxlen, outquemaxlen, maxpacketsoutpersecond, peerquemaxlen);

  if (!USBInit (t)) {
	  ERRORLOGSHAPE (t, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, NULL, Logging::MSGNOHASH,
    						"Layer 2 interface failed: USBInit failed");
	  throw Exception (DEV_OPEN_FAIL);
  }
  return new USBLayer2Interface (new USBLowLevelDriver (dev, t, flags, inquemaxlen, outquemaxlen, maxpacketsoutpersecond, ipnetfilters),
      t, flags, inquemaxlen, outquemaxlen, maxpacketsoutpersecond);
}

#endif
