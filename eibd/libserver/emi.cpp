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

/*
 *   massive rewrite to support many features & stabilize
 *
 *    Embedded Controller software
 *    Copyright (c) 2006- Z2, GmbH, Switzerland
 *    All Rights Reserved
 *
 *    THE ACCOMPANYING PROGRAM IS PROPRIETARY SOFTWARE OF Z2, GmbH,
 *    AND CANNOT BE DISTRIBUTED, COPIED OR MODIFIED WITHOUT
 *    EXPRESS PERMISSION OF Z2, GmbH.
 *
 *    Z2, GmbH DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 *    SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 *    AND FITNESS, IN NO EVENT SHALL Z2, LLC BE LIABLE FOR ANY
 *    SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 *    IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 *    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 *    THIS SOFTWARE.
 *
 */

#include "emi.h"

CArray
L_Data_ToCEMI (uchar code, const L_Data_PDU & l1)
{
  uchar c;
  CArray pdu;
  assert (l1.data () >= 1);
  assert (l1.data () < 0xff);
  assert ((l1.hopcount & 0xf8) == 0);

  switch (l1.prio)
    {
    case PRIO_LOW:
      c = 0x3;
      break;
    case PRIO_NORMAL:
      c = 0x1;
      break;
    case PRIO_URGENT:
      c = 0x02;
      break;
    case PRIO_SYSTEM:
      c = 0x00;
      break;
    }
  pdu.resize (l1.data () + 9);
  pdu[0] = code;
  pdu[1] = 0x00;
  pdu[2] = 0x10 | (c << 2) | (l1.data () - 1 <= 0x0f ? 0x80 : 0x00);
  if (code == 0x29)
    pdu[2] |= (l1.repeated ? 0 : 0x20);
  else
    pdu[2] |= 0x20;
  pdu[3] =
    (l1.AddrType ==
     GroupAddress ? 0x80 : 0x00) | ((l1.hopcount & 0x7) << 4) | 0x0;
  pdu[4] = (l1.source >> 8) & 0xff;
  pdu[5] = (l1.source) & 0xff;
  pdu[6] = (l1.dest >> 8) & 0xff;
  pdu[7] = (l1.dest) & 0xff;
  pdu[8] = l1.data () - 1;
  pdu.setpart (l1.data.array (), 9, l1.data ());
  return pdu;
}

L_Data_PDU *
CEMI_to_L_Data (const CArray & data)
{
  L_Data_PDU c;
  if (data () < 2)
    return 0;
  unsigned start = data[1] + 2;
  if (data () < 7 + start)
    return 0;
  if (data () < 7 + start + data[6 + start] + 1)
    return 0;
  c.source = (data[start + 2] << 8) | (data[start + 3]);
  c.dest = (data[start + 4] << 8) | (data[start + 5]);
  c.data.set (data.array () + start + 7, data[6 + start] + 1);
  if (data[0] == 0x29)
    c.repeated = (data[start] & 0x20) ? 0 : 1;
  else
    c.repeated = 0;
  switch ((data[start] >> 2) & 0x3)
    {
    case 0:
      c.prio = PRIO_SYSTEM;
      break;
    case 1:
      c.prio = PRIO_URGENT;
      break;
    case 2:
      c.prio = PRIO_NORMAL;
      break;
    case 3:
      c.prio = PRIO_LOW;
      break;
    }
  c.hopcount = (data[start + 1] >> 4) & 0x07;
  c.AddrType = (data[start + 1] & 0x80) ? GroupAddress : IndividualAddress;
  if (!data[start] & 0x80 && data[start + 1] & 0x0f)
    return 0;
  return new L_Data_PDU (c);
}

L_Busmonitor_PDU *
CEMI_to_Busmonitor (const CArray & data)
{
  L_Busmonitor_PDU c;
  if (data () < 2)
    return 0;
  unsigned start = data[1] + 2;
  if (data () < 1 + start)
    return 0;
  c.pdu.set (data.array () + start, data () - start);
  return new L_Busmonitor_PDU (c);
}

CArray
Busmonitor_to_CEMI (uchar code, const L_Busmonitor_PDU & p, int no)
{
  CArray pdu;
  pdu.resize (p.pdu () + 6);
  pdu[0] = code;
  pdu[1] = 4;
  pdu[2] = 3;
  pdu[3] = 1;
  pdu[4] = 1;
  pdu[5] = no & 0x7;
  pdu.setpart (p.pdu, 6);
  return pdu;
}

CArray
L_Data_ToEMI (uchar code, const L_Data_PDU & l1)
{
  CArray pdu;
  uchar c;
  switch (l1.prio)
    {
    case PRIO_LOW:
      c = 0x3;
      break;
    case PRIO_NORMAL:
      c = 0x1;
      break;
    case PRIO_URGENT:
      c = 0x02;
      break;
    case PRIO_SYSTEM:
      c = 0x00;
      break;
    }
  pdu.resize (l1.data () + 7);
  pdu[0] = code;
  pdu[1] = c << 2;
  pdu[2] = 0;
  pdu[3] = 0;
  pdu[4] = (l1.dest >> 8) & 0xff;
  pdu[5] = (l1.dest) & 0xff;
  pdu[6] =
    (l1.hopcount & 0x07) << 4 | ((l1.data () - 1) & 0x0f) | (l1.AddrType ==
							     GroupAddress ?
							     0x80 : 0x00);
  pdu.setpart (l1.data.array (), 7, l1.data ());
  return pdu;
}

L_Data_PDU *
EMI_to_L_Data (const CArray & data)
{
  L_Data_PDU c;
  unsigned len;

  if (data () < 8)
    return 0;

  c.source = (data[2] << 8) | (data[3]);
  c.dest = (data[4] << 8) | (data[5]);
  switch ((data[1] >> 2) & 0x3)
    {
    case 0:
      c.prio = PRIO_SYSTEM;
      break;
    case 1:
      c.prio = PRIO_URGENT;
      break;
    case 2:
      c.prio = PRIO_NORMAL;
      break;
    case 3:
      c.prio = PRIO_LOW;
      break;
    }
  c.AddrType = (data[6] & 0x80) ? GroupAddress : IndividualAddress;
  len = (data[6] & 0x0f) + 1;
  if (len > data.len () - 7)
    len = data.len () - 7;
  c.data.set (data.array () + 7, len);
  c.hopcount = (data[6] >> 4) & 0x07;
  return new L_Data_PDU (c);
}

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "emi.h"
#include "eibtypes.h"
#include "eibpriority.h"

const char EMILayer2Interface::indropmsg[] = "EMILayer2Interface: incoming queue length exceeded, dropping packet";
const char EMILayer2Interface::outdropmsg[] = "EMILayer2Interface: outgoing queue length exceeded, dropping packet";

bool
EMILayer2Interface::addAddress (eibaddr_t addr)
{
  return 0;
}

bool
EMILayer2Interface::addGroupAddress (eibaddr_t addr)
{
  return 1;
}

bool
EMILayer2Interface::removeAddress (eibaddr_t addr)
{
  return 0;
}

bool
EMILayer2Interface::removeGroupAddress (eibaddr_t addr)
{
  return 1;
}

eibaddr_t
EMILayer2Interface::getDefaultAddr ()
{
  return 0;
}

bool
EMILayer2Interface::openVBusmonitor ()
{
  vmode = 1;
  return 1;
}

bool
EMILayer2Interface::closeVBusmonitor ()
{
  vmode = 0;
  return 1;
}

Element * EMILayer2Interface::_xml(Element *parent) const
{
  Element *n = parent->addElement(XMLBACKENDELEMENT);
  n->addAttribute(XMLBACKENDELEMENTTYPEATTR, _str());
  n->addAttribute(XMLBACKENDSTATUSATTR, Connection_Lost() ?  XMLSTATUSDOWN : XMLSTATUSUP );
  outqueue._xml(n);
  iface->_xml(n);
  return n;
}

EMILayer2Interface::EMILayer2Interface (LowLevelDriverInterface * i, int flags,
					  Logs * tr, int inquemaxlen, int outquemaxlen) :
  Thread(tr,PTH_PRIO_STD, "EMI L2"),
  Layer2Interface(tr,flags),
  outqueue("EMI L2 Output", outquemaxlen)
{
  def=0;
  iface = i;
  mode = 0;
  vmode = 0;
  pth_sem_init (&out_signal);
  SetProxy(i); // status from proxy but our own version
  TRACEPRINTF(Thread::Loggers(), 2, this, "Allocated @ %p proxying to: %p",
      this,
      &Proxy());
  iface->init();
  Start ();
}

bool
EMILayer2Interface::Wait_Until_Lost_Or_Sent(void)
{
  if (!iface)
    return false;
  pth_event_t e = pth_event(PTH_EVENT_SEM, iface->Send_Queue_Empty_Cond());
  pth_event_t le = Connection_Wait_Until_Lost();
  bool ret = false;

  TRACEPRINTF(Thread::Loggers(), 3, this, "enter: Wait_Until_Lost_Or_Sent");

    // until really lost on signal or all sent
  while (!ret && !Connection_Lost())
    {
      pth_event_concat(e, le, NULL);
      pth_wait(e);
      pth_event_isolate(le);
      pth_event_isolate(e);
      ret = (pth_event_status(le) != PTH_STATUS_OCCURRED
          && pth_event_status(e) == PTH_STATUS_OCCURRED);
    }

  pth_event_free(e, PTH_FREE_THIS);
  pth_event_free(le, PTH_FREE_THIS);

  TRACEPRINTF(Thread::Loggers(), 3, this, "exit: Wait_Until_Lost_Or_Sent");

  return ret;
}


bool
EMILayer2Interface::init ()
{
  return iface != 0;
}

EMILayer2Interface::~EMILayer2Interface ()
{
  TRACEPRINTF (Thread::Loggers(), 2, this, "Destroy");
  Close();
  Stop ();
  while (!outqueue.isempty ())
    delete outqueue.get ();
  if (iface) {
    delete iface;
    iface=NULL;
  }
}

bool
EMILayer2Interface::enterBusmonitor ()
{
  CArray t1, t2;
  const uchar t21[] =
    { 0xa9, 0x1E, 0x12, 0x34, 0x56, 0x78, 0x9a };
  const uchar t22[] =
    { 0xa9, 0x90, 0x18, 0x34, 0x45, 0x67, 0x8a };
  const uchar t11[] =
    { 0x46, 0x01, 0x00, 0x60, 0x90 };
  const uchar t12[] =
    { };
  uchar empty[] =
    { };

  EMIVer v = iface->getEMIVer();

  if (iface)
    {
      switch (v)
        {
      case vEMI2:
        t1 = CArray(t21, sizeof(t21));
        t2 = CArray(t22, sizeof(t22));
        break;
      case vEMI1:
        t1 = CArray(t11, sizeof(t11));
        t2 = CArray(t12, sizeof(t12));
        break;
      default:
        t1 = CArray(empty, sizeof(empty));
        t2 = CArray(empty, sizeof(empty));
        break;
        }

      TRACEPRINTF(Thread::Loggers(), 2, this, "OpenBusmon");
      if (mode != 0)
        return 0;
      if (t1.len())
        iface->Send_Packet(t1);
      if (t2.len())
        iface->Send_Packet(t2);

      mode = Wait_Until_Lost_Or_Sent() ? 1 : mode;
    }
  return mode == 1;
}

bool
EMILayer2Interface::leaveBusmonitor ()
{
  const uchar t21[] =
    { 0xa9, 0x1E, 0x12, 0x34, 0x56, 0x78, 0x9a };
  const uchar t11[] =
    { 0x46, 0x01, 0x00, 0x60, 0xc0 };
  uchar empty[] =
    { };

  EMIVer v = iface->getEMIVer();
  if (mode != 1)
    return 0;
  TRACEPRINTF(Thread::Loggers(), 2, this, "CloseBusmon");

  CArray t;

  switch (v)
    {
  case vEMI2:
    t = CArray(t21, sizeof(t21));
    break;
  case vEMI1:
    t = CArray(t11, sizeof(t11));
    break;
  default:
    t = CArray(empty, sizeof(empty));
    break;
    }

  if (t.len())
    iface->Send_Packet(t);

  mode = Wait_Until_Lost_Or_Sent() ? 0 : mode;

  return mode == 0;
}

bool
EMILayer2Interface::Open ()
{
  const uchar t21[] =
    { 0xa9, 0x1E, 0x12, 0x34, 0x56, 0x78, 0x9a };
  const uchar t22[] =
    { 0xa9, 0x00, 0x18, 0x34, 0x56, 0x78, 0x0a };
  const uchar t11[] =
    { 0x46, 0x01, 0x00, 0x60, 0x12 };
  const uchar t12[] =
    { };
  uchar empty[] =
    { };

  if (mode == 2)
    return true; // already open

  CArray t1, t2;

  if (iface)
    {
      if (iface->Connection_Lost())
        return false;

      EMIVer v = iface->getEMIVer();

      switch (v)
        {
          case vEMI2:
          t1 = CArray(t21,sizeof(t21));
          t2 = CArray(t22,sizeof(t22));
          break;
          case vEMI1:
          t1 = CArray(t11,sizeof(t11));
          t2 = CArray(t12,sizeof(t12));
          break;
          default:
            // we don't know the EMI, we can't reset!
            return false;
          break;
        }

      TRACEPRINTF(Thread::Loggers(), 2, this, "Opened L2, sending startup to lower layers");
      if (t1.len())
        iface->Send_Packet(t1);
      if (t2.len())
        iface->Send_Packet(t2);

      mode = Wait_Until_Lost_Or_Sent() ? 2 : mode;
    }
  return mode == 2;
}

bool
EMILayer2Interface::Close ()
{
  const uchar t21[] =
    { 0xa9, 0x1E, 0x12, 0x34, 0x56, 0x78, 0x9a };
  const uchar t11[] =
    { 0x46, 0x01, 0x00, 0x60, 0xc0 };
  uchar empty[] =
    { };

  CArray t;
  EMIVer v = iface->getEMIVer();

  TRACEPRINTF(Thread::Loggers(), 2, this, "Closing L2, sending shutdown to lower layers, lost connection: %d", Connection_Lost());
  if (iface && !Connection_Lost())
    {
      switch (v)
        {
      case vEMI2:
        t = CArray(t21, sizeof(t21));
        break;
      case vEMI1:
        t = CArray(t11, sizeof(t11));
        break;
      default:
        t = CArray(empty, sizeof(empty));
        break;
        }

      if (mode != 2)
        return false;
      TRACEPRINTF(Thread::Loggers(), 2, this, "Close L2");

      if (t.len())
        iface->Send_Packet(t);

      mode = Wait_Until_Lost_Or_Sent() ? 0 : mode;
    }
  return mode == 0;
}

bool
EMILayer2Interface::Send_Queue_Empty ()
{
  return iface->Send_Queue_Empty ();
}

bool
EMILayer2Interface::Send_L_Data (LPDU * l)
{
  TRACEPRINTF (Thread::Loggers(), 2, this, "Send %s", l->Decode ()());
  if (l->getType () != L_Data)
  {
    delete l;
    return true;
  }

  if (mode != 2) {
    if (! Open()) {
      return false;
    }
  }

  L_Data_PDU *l1 = (L_Data_PDU *) l;
  assert (l1->data () >= 1);
  /* discard long frames, as they are not supported by EMI 1/2 */
  if (l1->data () > 0x10)
    return false;
  assert (l1->data () <= 0x10);
  assert ((l1->hopcount & 0xf8) == 0);

  CArray pdu = L_Data_ToEMI (0x11, *l1);
  iface->Send_Packet (pdu);

  if (vmode)
  {
    L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
    l2->pdu.set (l->ToPacket ());

    if (!Put_On_Queue_Or_Drop<LPDU *, L_Busmonitor_PDU *>(outqueue, l2, &out_signal, true, outdropmsg))
      delete l2;
#if 0
    outqueue.put (l2);
    pth_sem_inc (&out_signal, 1);
#endif
  }
  return Put_On_Queue_Or_Drop<LPDU *, LPDU *>(outqueue, l, &out_signal, true, outdropmsg);
#if 0
  outqueue.put (l);
  pth_sem_inc (&out_signal, 1);
#endif
}

LPDU *
EMILayer2Interface::Get_L_Data (pth_event_t stop)
{
  pth_event_t le = Connection_Wait_Until_Lost(); // will give us proxy
  pth_event_t getwait = pth_event (PTH_EVENT_SEM, &out_signal);

  if (stop != NULL)
    {
      pth_event_concat(stop, getwait, NULL);
      pth_event_concat(stop, le, NULL);
    }

  pth_wait(stop);

  if (stop != NULL)
    {
      pth_event_isolate(stop);
    }

  pth_event_isolate(getwait);
  pth_event_isolate(le);

  pth_event_free(le, PTH_FREE_THIS);
  bool s= pth_event_status(getwait) == PTH_STATUS_OCCURRED;
  pth_event_free (getwait, PTH_FREE_THIS);

  if (s)
    {
      pth_sem_dec(&out_signal);
      LPDU *l = outqueue.get();
      TRACEPRINTF(Thread::Loggers(), 2, this, "Recv %s", l->Decode ()());
      return l;
    }
  return NULL; // upper layer, we lost connection
}

bool EMILayer2Interface::SendReset()
{
  ++stat_resets;
  if (iface) {
    return iface->SendReset(); // try to bring up again
  }
  return true;
}

void
EMILayer2Interface::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event(PTH_EVENT_SEM, stop1);
  unsigned long lastlowerversion=unknownVersion;
  unsigned int semvalue=0;

  while ( !semvalue )
    // bug in pthsem library, sometimes no event ! pth_event_status(stop) != PTH_STATUS_OCCURRED)
    {
      pth_sem_get_value(stop1, &semvalue);
      TRACEPRINTF(Thread::Loggers(), 11, this, "EMI 2 Loop Begin after checking semaphore x%lx event status: %d sem value: %d",
          (unsigned long) stop1,
          (unsigned int) pth_event_status(stop),
          (unsigned int) semvalue);

      EMIVer v = iface->getEMIVer();

      TRACEPRINTF(Thread::Loggers(), 11, this,
          "check transition to states %d %d %d", (int)Proxy().currentVersion(), (int)lastlowerversion, (int)Connection_Lost());

      // connection lower layer lost or gained, something changed
      if (Proxy().currentVersion() != lastlowerversion)
        {
          if (Connection_Lost())
            {
                {
                  TRACEPRINTF(Thread::Loggers(), 1, this,
                      "lost lower layer connection");
                  lastlowerversion = Proxy().currentVersion();
                  bumpVersion();
                  mode = 0; // well, I think we could loose all the bus monitor stuff as well
                }
              pth_sleep(1);
            }
          else
            {
              // may need reset
              TRACEPRINTF(Thread::Loggers(), 2, this,
                  "got lower layer connection, flags %d", (int) flags);

              if ( (flags & FLAG_B_RESET_ADDRESS_TABLE) && LowLevelDriver())
                {
                  // emi2 about 12 packets to write, give him some time
                  pth_event_t shortexchangetimer = pth_event(PTH_EVENT_RTIME, pth_time(5,0));

                  bool r = LowLevelDriver()->writeAddrTabSize(shortexchangetimer, 0, false); // vEMI2 is a full protocol that would need to be run in a state machine/thread
                  if (!r)
                    {
                      ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
                          Logging::DUPLICATESMAX1PERMIN, this,
                          Logging::MSGNOHASH,
                          "Low level interface address table write failed %s",
                          (pth_event_status(shortexchangetimer) == PTH_STATUS_OCCURRED) ? "on timeout" : "");
                    }
                  else
                    {
                      TRACEPRINTF(Thread::Loggers(), 2, this,
                          "interface address table set to zero size ");
                      INFOLOG(Thread::Loggers(), LOG_INFO, this,
                          "interface address table reset to zero size");
                    }
                  pth_event_isolate(shortexchangetimer);
                  pth_event_free(shortexchangetimer,PTH_FREE_THIS);
                }

              if (Open())
                {
                  lastlowerversion = Proxy().currentVersion();
                  bumpVersion();
                  TRACEPRINTF(Thread::Loggers(), 1, this,
                      "transition to up state");
                }
            }
          }

      CArray *c = iface->Get_Packet(stop);

      if (!c)
        {
          pth_sleep(1); // was a signal, debounce
        }
      else
        {

          if (c->len() == 1 && (*c)[0] == 0xA0 && mode == 2)
            {
              TRACEPRINTF(Thread::Loggers(), 2, this, "Reopen");
              mode = 0;
              Open();
            }
          if (c->len() == 1 && (*c)[0] == 0xA0 && mode == 1)
            {
              TRACEPRINTF(Thread::Loggers(), 2, this, "Reopen Busmonitor");
              mode = 0;
              enterBusmonitor();
            }
          if (c->len() && (*c)[0] == (v == vEMI2 ? 0x29 : 0x49) && mode == 2)
            {
              L_Data_PDU *p = EMI_to_L_Data(*c);
              if (p)
                {
                  delete c;
                  if (p->AddrType == IndividualAddress)
                    p->dest = 0;
                  TRACEPRINTF(Thread::Loggers(), 2, this,
                      "Recv %s", p->Decode ()());
                  if (vmode)
                    {
                      L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
                      l2->pdu.set(p->ToPacket());

                      Put_On_Queue_Or_Drop<LPDU *, L_Busmonitor_PDU *>(outqueue,
                          l2, &out_signal, true, outdropmsg);

#if 0
                      outqueue.put (l2);
                      pth_sem_inc (&out_signal, 1);
#endif
                    }

                  Put_On_Queue_Or_Drop<LPDU *, L_Data_PDU *>(outqueue, p,
                      &out_signal, true, outdropmsg);

#if 0
                  outqueue.put (p);
                  pth_sem_inc (&out_signal, 1);
#endif
                  continue;
                }
            }
          if (c->len() > 4 && (*c)[0] == (v == vEMI2 ? 0x2B : 0x49)
              && mode == 1)
            {
              L_Busmonitor_PDU *p = new L_Busmonitor_PDU;
              p->pdu.set(c->array() + 4, c->len() - 4);
              delete c;
              TRACEPRINTF(Thread::Loggers(), 2, this,
                  "Recv %s", p->Decode ()());

              Put_On_Queue_Or_Drop<LPDU *, L_Busmonitor_PDU *>(outqueue, p,
                  &out_signal, true, outdropmsg);

#if 0
              outqueue.put (p);
              pth_sem_inc (&out_signal, 1);
#endif
              continue;
            }
          delete c;
        }

    }
  pth_event_free(stop, PTH_FREE_THIS);
}

