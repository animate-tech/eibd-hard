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

#include "lowlevel.h"

const uchar EMI2_TLL[] = { 0xA9, 0x00, 0x12, 0x34, 0x56, 0x78, 0x0A };
const uchar EMI2_NORM[] = { 0xA9, 0x00, 0x12, 0x34, 0x56, 0x78, 0x8A };
const uchar EMI2_LCON[] = { 0x43, 0x00, 0x00, 0x00, 0x00, 0x00 };
const uchar EMI2_LDIS[] = { 0x44, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uchar EMI1_READ[] = { 0x4C, 0x01, 0x01, 0x16 };

const char *EMI2Str(const EMIVer v)
{
	switch (v) {
	case vEMI1:
		return "EMI1";
	case vEMI2:
		return "EMI2";
	case vCEMI:
		return "CEMI";
	default:
		break;
	}
	return "unknown";
}

static void
llwait (pth_event_t stop, LowLevelDriverInterface * iface)
{
  if (!iface->Send_Queue_Empty ())
    {
      pth_event_t
	e = pth_event (PTH_EVENT_SEM, iface->Send_Queue_Empty_Cond ());
      pth_event_concat(stop, e, NULL);
      pth_wait (e);
      pth_event_isolate(stop);
      pth_event_isolate(e);
      pth_event_free (e, PTH_FREE_THIS);
    }
}

LowLevelDriverInterface::LowLevelDriverInterface(Logs *t,
    int flags,
    int inquemaxlen,
    int outquemaxlen,
    int maxpacketsoutpersecond,
    char *inqueuename ,
    char *outqueuename ) :
DroppableQueueInterface(t), maxpktsoutpersec(maxpacketsoutpersecond),
inqueue(inqueuename, inquemaxlen),
outqueue(outqueuename, outquemaxlen),
ConnectionStateInterface(t,NULL)
{
    pth_sem_init (&in_signal);
    pth_sem_init (&out_signal);
    pth_sem_init (&send_empty);
    pth_sem_set_value (&send_empty, 1);
    pth_sem_set_value (&in_signal, 0);
    pth_sem_set_value (&out_signal, 0);

}

LowLevelDriverInterface::~LowLevelDriverInterface()
{
}

void LowLevelDriverInterface::DrainOut()
{
  outqueue.Lock();
  while (!outqueue.isempty()) {
      pth_sem_dec(&out_signal);
      outqueue.get();
  }
  outqueue.Unlock();
}

void LowLevelDriverInterface::DrainIn()
{
  inqueue.Lock();
  while (!inqueue.isempty())
    {
      pth_sem_dec(&in_signal);
      inqueue.get();
    }
  pth_sem_set_value(&send_empty, 1);
  inqueue.Unlock();
}

bool LowLevelDriverInterface::TransitionToDownState()
{
  ConnectionStateInterface::TransitionToDownState();
  TRACEPRINTF(Loggers(), 2, this, "transition to down state");
  DrainIn();
  DrainOut();
  pth_yield(NULL);
}

bool
LowLevelDriverInterface::Send_Queue_Empty ()
{
  return inqueue.isempty ();
}

pth_sem_t *
LowLevelDriverInterface::Send_Queue_Empty_Cond ()
{
  return &send_empty;
}

int
LowLevelDriverInterface::readEMI1Mem ( pth_event_t stop, memaddr_t addr, uchar len,
	     CArray & result)
{
  CArray *d1, d;
  d.resize (4);
  d[0] = 0x4C;
  d[1] = len;
  d[2] = (addr >> 8) & 0xff;
  d[3] = (addr) & 0xff;
  Send_Packet (d, true);
  llwait (stop, this);
  d1 = Get_Packet (stop);
  if (!d1)
    return 0;
  d = *d1;
  delete d1;
  if (d () != 4 + len)
    return 0;
  if (d[0] != 0x4B)
    return 0;
  if (d[1] != len)
    return 0;
  if (d[2] != ((addr >> 8) & 0xff))
    return 0;
  if (d[3] != ((addr) & 0xff))
    return 0;
  result.set (d.array () + 4, len);
  return 1;
}

int
LowLevelDriverInterface::writeEMI1Mem ( pth_event_t stop, memaddr_t addr, CArray data, bool reread)
{
  CArray d;
  d.resize(4 + data());
  d[0] = 0x46;
  d[1] = data.len() & 0xff;
  d[2] = (addr >> 8) & 0xff;
  d[3] = (addr) & 0xff;
  d.setpart(data, 4);
  Send_Packet(d, true);
  llwait (stop, this);
  if (reread)
    {
      llwait(stop, this);
      if (!readEMI1Mem(stop, addr, data(), d))
        return 0;
      return d == data;
    }
  return 1;
}

CArray *
LowLevelDriverInterface::Get_Packet (pth_event_t stop, bool readwhenstatusdown)
{
  // that can be reentrant, getwait per entrance
  if (Connection_Lost() && !readwhenstatusdown)
    {
      return NULL;
    }
  // wait for according link state change
  pth_event_t le = !Connection_Lost() ?  Connection_Wait_Until_Lost() : Connection_Wait_Until_Up() ;
  pth_event_t getwait = pth_event (PTH_EVENT_SEM, &out_signal);

  pth_event_isolate(getwait);
  pth_event_isolate(le);

  pth_event_concat(le, getwait, NULL);

  if (stop != NULL)
    {
      pth_event_concat(getwait, stop, NULL);
    }

  TRACEPRINTF(Loggers(), 3, this, "Get_Packet waiting");
#if 0
  {
    for (pth_event_t r=le; le; le=le->ev_next) {
        TRACEPRINTF(Loggers(), 3, this, "%p, ", le);
        pth_sleep(1);
    }
  }
#endif

  pth_wait(le);

  pth_event_isolate(getwait);
  pth_event_isolate(le);
  if (stop)
    {
      pth_event_isolate(stop);
    }

  bool les = pth_event_status(le) == PTH_STATUS_OCCURRED ;
  pth_event_free(le, PTH_FREE_THIS);
  bool s = pth_event_status(getwait) == PTH_STATUS_OCCURRED ;
  pth_event_free(getwait, PTH_FREE_THIS);


  {
    unsigned int v;
    outqueue.Lock();
    pth_sem_get_value(&out_signal,&v);
  TRACEPRINTF(Loggers(), 3, this, "Get_Packet link event: %d wait-on-incoming-packet: %d incoming-queue-len: %d incoming-queue-sem-count: %d stop: %d",
      pth_event_status(le) == PTH_STATUS_OCCURRED,
      (int) outqueue.len(),
      (int) v,
      (int) s,
      stop ? pth_event_status(stop) == PTH_STATUS_OCCURRED : -1);
    assert (v == outqueue.len());
    outqueue.Unlock();
  }

  if ( ( !Connection_Lost() || readwhenstatusdown)  && s)
    {
      pth_sem_dec(&out_signal);
      CArray *c = outqueue.get();
      return c;
    }

  TRACEPRINTF(Loggers(), 0, this, "Send empty as signal up (link loss/up or stop)");
  return NULL; // also if connection lost

}

int
LowLevelDriverInterface::readEMI2Mem ( pth_event_t stop, memaddr_t addr, uchar len,
	     CArray & result)
{
  CArray *d1, d;
  Send_Packet (CArray (EMI2_TLL, sizeof (EMI2_TLL)), true);
  Send_Packet (CArray (EMI2_LCON, sizeof (EMI2_LCON)), true);

  // ignore ACKs
  llwait (stop, this);
  d1 = Get_Packet (stop);
  if (d1)
    {
      d = *d1;
      delete d1;
      if (d () != 6)
	return 0;
      if (d[0] != 0x86)
	return 0;
    }
  else
    return 0;

  d.resize (11);
  d[0] = 0x41;
  d[1] = 0x00;
  d[2] = 0x00;
  d[3] = 0x00;
  d[4] = 0x00;
  d[5] = 0x00;
  d[6] = 0x03;
  d[7] = 0x02;
  d[8] = len & 0x0f;
  d[9] = (addr >> 8) & 0xff;
  d[10] = (addr) & 0xff;

  Send_Packet (d,true);
  llwait (stop, this);

  // ignore ACKs
  d1 = Get_Packet (stop);
  if (d1)
    {
      d = *d1;
      delete d1;
      if (d () != 11)
	return 0;
      if (d[0] != 0x8E)
	return 0;
    }
  else
    return 0;

  d1 = Get_Packet (stop);
  if (!d1)
    return 0;
  d = *d1;
  delete d1;
  if (d () != 11 + len)
    return 0;
  if (d[0] != 0x89)
    return 0;
  if (d[1] != 0x00)
    return 0;
  if (d[2] != 0x00)
    return 0;
  if (d[3] != 0x00)
    return 0;
  if (d[4] != 0x00)
    return 0;
  if (d[5] != 0x00)
    return 0;
  if (d[6] != 0x03 + len)
    return 0;
  if ((d[7] & 0x03) != 0x02)
    return 0;
  if (d[8] != (0x40 | len))
    return 0;
  if (d[9] != ((addr >> 8) & 0xff))
    return 0;
  if (d[10] != ((addr) & 0xff))
    return 0;
  result.set (d.array () + 11, len);
  Send_Packet (CArray (EMI2_LDIS, sizeof (EMI2_LDIS)),true);
  llwait (stop, this);

  d1 = Get_Packet (stop);
  if (!d1)
    return 0;
  else
    {
      d = *d1;
      delete d1;
      if (d () != 6)
	return 0;
      if (d[0] != 0x88)
	return 0;
    }
  Send_Packet (CArray (EMI2_NORM, sizeof (EMI2_NORM)),true);
  llwait (stop, this);
  return 1;
}

int
LowLevelDriverInterface::writeEMI2Mem ( pth_event_t stop, memaddr_t addr, CArray data, bool reread)
{
  CArray *d1, d;
  Send_Packet(CArray(EMI2_TLL, sizeof(EMI2_TLL)),true);

  Send_Packet(CArray(EMI2_LCON, sizeof(EMI2_LCON)),true);

  // ignore ACKs
  llwait (stop, this);

  d1 = Get_Packet(stop);
  if (d1)
    {
      d = *d1;
      delete d1;
      if (d() != 6)
        return 0;
      if (d[0] != 0x86)
        return 0;
    }
  else
    return 0;

  d.resize(11 + data());
  d[0] = 0x41;
  d[1] = 0x00;
  d[2] = 0x00;
  d[3] = 0x00;
  d[4] = 0x00;
  d[5] = 0x00;
  d[6] = 0x03;
  d[7] = 0x02;
  d[8] = (0x80 | (data() & 0x0f));
  d[9] = (addr >> 8) & 0xff;
  d[10] = (addr) & 0xff;
  d.setpart(data, 11);

  Send_Packet(d,true);
  llwait (stop, this);

  d1 = Get_Packet(stop);
  if (d1)
    {
      d = *d1;
      delete d1;
      if (d() != 11 + data())
        return 0;
      if (d[0] != 0x8E)
        return 0;
    }
  else
    return 0;

  Send_Packet(CArray(EMI2_LDIS, sizeof(EMI2_LDIS)),true);
  llwait (stop, this);

  d1 = Get_Packet(stop);
  if (!d1)
    return 0;
  else
    {
      d = *d1;
      delete d1;
      if (d() != 6)
        return 0;
      if (d[0] != 0x88)
        return 0;
    }

  Send_Packet(CArray(EMI2_NORM, sizeof(EMI2_NORM)),true);

  if (reread)
    {
      llwait( stop,this);

      if (!readEMI2Mem( stop,addr, data(), d))
        return 0;
      return d == data;
    }
  return 1;
}

int
LowLevelDriverInterface::readEMIMem ( pth_event_t stop,memaddr_t addr, uchar len,
	    CArray & result)
{
  switch (getEMIVer ())
    {
    case vEMI1:
      return readEMI1Mem (  stop,addr, len, result);
    case vEMI2:
      return readEMI2Mem (  stop,addr, len, result);

    default:
      return 0;
    }
}

int
LowLevelDriverInterface::writeEMIMem ( pth_event_t stop,memaddr_t addr, CArray data, bool reread)
{
  switch (getEMIVer ())
    {
    case vEMI1:
      return writeEMI1Mem (  stop,addr, data, reread);
    case vEMI2:
      return writeEMI2Mem (  stop,addr, data, reread);

    default:
      return 0;
    }
}

int
LowLevelDriverInterface::readAddrTabSize ( pth_event_t stop,uchar & result)
{
  CArray x;
  if (!readEMIMem (  stop,0x116, 1, x))
    return 0;
  result = x[0];
  return 1;
}

int
LowLevelDriverInterface::writeAddrTabSize ( pth_event_t stop,uchar size, bool reread)
{
  CArray x;
  x.resize (1);
  x[0] = size;
  return writeEMIMem (  stop,0x116, x, reread);
}

void
LowLevelDriverInterface::logtic()
{
  INFOLOGSHAPE(Loggers(), LOG_INFO, Logging::DUPLICATESMAX1PERMIN, this,
      Logging::MSGNOHASH,
      "%s: received %d pkts, sent %d pkts, %d receive errors, %d send errors, %d resets ",
      _str(),
      (int) *inqueue.stat_inserts,
      (int) *outqueue.stat_inserts,
      (int) *stat_recverr,
      (int) *stat_senderr,
      (int) *stat_resets);
}

