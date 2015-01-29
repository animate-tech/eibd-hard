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

#include "layer3.h"
extern "C" {
#include <inttypes.h>
#include <stdint.h>
};

Layer3::Layer3 (Layer2Interface * l2, Logs * tr, bool resettables, bool _droponl2loss, IPv4NetList &ipnetfilters) :
    Thread(tr, PTH_PRIO_STD, "L3 Connection"), ConnectionStateInterface(tr,l2),
    ipnetfilters(ipnetfilters)
{
  layer2 = l2;

  TRACEPRINTF(Thread::Loggers(), 2, this, "Allocated @ %p proxy to l2: %p",
        this,
        &Proxy());
  pth_mutex_init(&datalock);
  l2->Open ();
  mode = 0;
  resettables = resettables;
  droponl2loss=_droponl2loss;
  Start ();
}

Layer3::~Layer3 ()
{
  TRACEPRINTF (Loggers(), 3, this, "Close & Destroy");
  Stop ();

  if (mode)
    layer2->leaveBusmonitor();
  else
    layer2->Close();

  StopAllClients(true);
  delete layer2;
  layer2=NULL;
}

bool Layer3::TraceDataLockWait(pth_mutex_t *datalock)
{
  TRACEPRINTF (Loggers(), 3, this, "Take %p", datalock);
  return WaitForDataLock(datalock);
}

void Layer3::TraceDataLockRelease(pth_mutex_t *datalock)
{
  TRACEPRINTF (Loggers(), 3, this, "Release %p", datalock);
  ReleaseDataLock(datalock);
}

bool Layer3::StopAllClients(bool hard=false)
{
  if (!WaitForDataLock(&datalock))
    return false;

  mode = 0;
  int i;
  // send everyone a notification that we're going down
  Array < Busmonitor_Info >  acopy(busmonitor);
  for (i=0; i<acopy(); i++)
    {
      acopy[i].cb->Get_L_Busmonitor(NULL);
      if (hard) deregisterBusmonitor(acopy[i].cb);
    }
  Array < Busmonitor_Info > vcopy(vbusmonitor);
  for (i=0; i<vcopy(); i++)
    {
      vcopy[i].cb->Get_L_Busmonitor(NULL);
      if (hard) deregisterVBusmonitor(vcopy[i].cb);
    }
  Array < Broadcast_Info > bcopy(broadcast);
  for (i=0; i<bcopy(); i++)
     {
      bcopy[i].cb->Get_L_Data(NULL);
      if (hard) deregisterBroadcastCallBack(bcopy[i].cb);
    }
  Array < Group_Info > gcopy(group);
  for (i=0; i<gcopy(); i++)
    {
      gcopy[i].cb->Get_L_Data(NULL);
      if (hard) deregisterGroupCallBack(gcopy[i].cb, gcopy[i].dest);
    }
  Array < Individual_Info > icopy(individual);
  for (i=0; i<icopy(); i++)
    {
      icopy[i].cb->Get_L_Data(NULL);
      if (hard) deregisterIndividualCallBack(icopy[i].cb, icopy[i].src,icopy[i].dest);
    }

  TraceDataLockRelease(&datalock);
  return true;
}

bool
Layer3::send_L_Data (L_Data_PDU * l)
{
  int ret = 0;
  if (!TraceDataLockWait(&datalock))
    return false;
  try
    {
      TRACEPRINTF(Loggers(), 3, this, "Send %s", l->Decode ()());
      if (Connection_Lost())
        {
          if (!layer2->Open())
            {
                {
                  ret = 0;
                  throw Exception(LOOP_RETURN);
                }
            }
        }

      if (l->source == 0)
        l->source = layer2->getDefaultAddr();
      ret = layer2->Send_L_Data(l);
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;

}

bool
Layer3::deregisterBusmonitor (L_Busmonitor_CallBack * c)
{
  unsigned i;
  int ret = 0;
  if (Connection_Lost())
    return false;
  if (!TraceDataLockWait(&datalock))
      return false;
  try
    {
      for (i = 0; i < busmonitor(); i++)
        if (busmonitor[i].cb == c)
          {
            busmonitor[i] = busmonitor[busmonitor() - 1];
            busmonitor.resize(busmonitor() - 1);
            if (busmonitor() == 0)
              {
                mode = 0;
                layer2->leaveBusmonitor();
                layer2->Open();
              }
            TRACEPRINTF(Loggers(), 3, this, "deregisterBusmonitor %p = 1", c);
            ret = 1;
            throw Exception(LOOP_RETURN);
          }
      TRACEPRINTF(Loggers(), 3, this, "deregisterBusmonitor %p = 0", c);
      ret = 0;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;
}

bool
Layer3::deregisterVBusmonitor (L_Busmonitor_CallBack * c)
{
  unsigned i;
  int ret = 0;
  if (!TraceDataLockWait(&datalock))
      return false;
  try
    {
      for (i = 0; i < vbusmonitor(); i++)
        if (vbusmonitor[i].cb == c)
          {
            vbusmonitor[i] = vbusmonitor[vbusmonitor() - 1];
            vbusmonitor.resize(vbusmonitor() - 1);
            if (vbusmonitor() == 0)
              {
                layer2->closeVBusmonitor();
              }
            TRACEPRINTF(Loggers(), 3, this,
                "deregisterVBusmonitor %p = 1", c);
            ret = 1;
            throw Exception(LOOP_RETURN);
          }
      TRACEPRINTF(Loggers(), 3, this, "deregisterVBusmonitor %p = 0", c);
      ret = 0;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;
}

bool
Layer3::deregisterBroadcastCallBack (L_Data_CallBack * c)
{
  unsigned i;
  int ret = 0;
  if (!TraceDataLockWait(&datalock))
      return false;
  try
    {
      for (i = 0; i < broadcast(); i++)
        if (broadcast[i].cb == c)
          {
            broadcast[i] = broadcast[broadcast() - 1];
            broadcast.resize(broadcast() - 1);
            TRACEPRINTF(Loggers(), 3, this, "deregisterBroadcast %p = 1", c);
            ret = 1;
            throw Exception(LOOP_RETURN);
          }
      TRACEPRINTF(Loggers(), 3, this, "deregisterBroadcast %p = 0", c);
      ret = 0;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;

}

bool
Layer3::deregisterGroupCallBack (L_Data_CallBack * c, eibaddr_t addr)
{
  unsigned i;
  if (!TraceDataLockWait(&datalock))
    return false;
  int ret = 0;
  try
    {
      for (i = 0; i < group(); i++)
        if (group[i].cb == c && group[i].dest == addr)
          {
            group[i] = group[group() - 1];
            group.resize(group() - 1);
            TRACEPRINTF(Loggers(), 3, this,
                "deregisterGroupCallBack %p = 1", c);
            for (i = 0; i < group(); i++)
              {
                if (group[i].dest == addr)
                  {
                    ret = 1;
                    throw Exception(LOOP_RETURN);
                  }
              }
            if (addr)
              layer2->removeGroupAddress(addr);
            ret = 1;
            throw Exception(LOOP_RETURN);
          }
      TRACEPRINTF(Loggers(), 3, this, "deregisterGroupCallBack %p = 0", c);
      ret = 0;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;

}

bool
  Layer3::deregisterIndividualCallBack (L_Data_CallBack * c, eibaddr_t src,
					eibaddr_t dest)
{
  unsigned i;
  if (!TraceDataLockWait(&datalock))
    return false;
  int ret = 0;
  try
    {
      for (i = 0; i < individual(); i++)
        if (individual[i].cb == c && individual[i].src == src
            && individual[i].dest == dest)
          {
            individual[i] = individual[individual() - 1];
            individual.resize(individual() - 1);
            TRACEPRINTF(Loggers(), 3, this, "deregisterIndividual %p = 1", c);
            for (i = 0; i < individual(); i++)
              {
                if (individual[i].dest == dest)
                  {
                    ret = 1;
                    throw Exception(LOOP_RETURN);
                  }

              }
            if (dest)
              layer2->removeAddress(dest);
            TRACEPRINTF(Loggers(), 3, this, "deregisterIndividual %p = 0", c);
            ret = 1;
            throw Exception(LOOP_RETURN);
          }
      ret = 0;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;

}

bool
Layer3::registerBusmonitor (L_Busmonitor_CallBack * c)
{
  TRACEPRINTF(Loggers(), 3, this, "registerBusmontior %p", c);

  if (!TraceDataLockWait(&datalock))
    return false;
  int ret = 0;
  try
    {
      ret = 0;
      if (individual())
        throw Exception(LOOP_RETURN);
      if (group())
        throw Exception(LOOP_RETURN);
      if (broadcast())
        throw Exception(LOOP_RETURN);
      if (mode == 0)
        {
          layer2->Close();
          if (!layer2->enterBusmonitor())
            {
              layer2->Open();
              throw Exception(LOOP_RETURN);
            }
        }
      mode = 1;
      busmonitor.resize(busmonitor() + 1);
      busmonitor[busmonitor() - 1].cb = c;
      TRACEPRINTF(Loggers(), 3, this, "registerBusmontior %p = 1", c);
      ret = 1;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;

}

bool
Layer3::registerVBusmonitor (L_Busmonitor_CallBack * c)
{
  TRACEPRINTF(Loggers(), 3, this, "registerVBusmonitor %p", c);
  if (Connection_Lost())
    return false;
  if (!TraceDataLockWait(&datalock))
    return false;
  int ret = 0;
  try
    {
      if (!vbusmonitor() && !layer2->openVBusmonitor())
        {
          ret = 0;
          throw Exception(LOOP_RETURN);
        }
      vbusmonitor.resize(vbusmonitor() + 1);
      vbusmonitor[vbusmonitor() - 1].cb = c;
      TRACEPRINTF(Loggers(), 3, this, "registerVBusmontior %p = 1", c);
      ret = 1;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;

}

bool
Layer3::registerBroadcastCallBack (L_Data_CallBack * c)
{
  TRACEPRINTF(Loggers(), 3, this, "registerBroadcast %p", c);
  if (!TraceDataLockWait(&datalock))
    return false;
  int ret = 0;
  try
    {
      if (mode == 1)
        {
          ret = 0;
          throw Exception(LOOP_RETURN);
        }
      broadcast.resize(broadcast() + 1);
      broadcast[broadcast() - 1].cb = c;
      TRACEPRINTF(Loggers(), 3, this, "registerBroadcast %p = 1", c);
      TraceDataLockRelease(&datalock);
      ret = 1;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;

}

bool
Layer3::registerGroupCallBack (L_Data_CallBack * c, eibaddr_t addr)
{
  int ret = 0;
  if (!TraceDataLockWait(&datalock))
    return false;

  try
    {
      unsigned i;
      TRACEPRINTF(Loggers(), 3, this, "registerGroup %p", c);
      if (mode == 1)
        {
          ret = 0;
          throw Exception(LOOP_RETURN);
        }
      for (i = 0; i < group(); i++)
        {
          if (group[i].dest == addr)
            break;
        }
      if (i == group())
        if (addr)
          if (!layer2->addGroupAddress(addr))
            {
              ret = 0;
              throw Exception(LOOP_RETURN);
            }
      group.resize(group() + 1);
      group[group() - 1].cb = c;
      group[group() - 1].dest = addr;
      TRACEPRINTF(Loggers(), 3, this, "registerGroup %p = 1", c);
      ret = 1;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;

}

bool
  Layer3::registerIndividualCallBack (L_Data_CallBack * c,
				      Individual_Lock lock, eibaddr_t src,
				      eibaddr_t dest)
{

  unsigned i;

  int ret = 0;
  if (!TraceDataLockWait(&datalock))
    return false;

  try
    {
      TRACEPRINTF(Loggers(), 3, this, "registerIndividual %p %d", c, lock);
      if (mode == 1)
        {
          ret = 0;
          throw Exception(LOOP_RETURN);
        }
      for (i = 0; i < individual(); i++)
        if (lock == Individual_Lock_Connection && individual[i].src == src
            && individual[i].lock == Individual_Lock_Connection)
          {
            TRACEPRINTF(Loggers(), 3, this,
                "registerIndividual locked %04X %04X", individual[i].src, individual[i].dest);
            ret = 0;
            throw Exception(LOOP_RETURN);
          }

      for (i = 0; i < individual(); i++)
        {
          if (individual[i].dest == dest)
            break;
        }
      if (i == individual() && dest)
        if (!layer2->addAddress(dest))
          {
            ret = 0;
            throw Exception(LOOP_RETURN);
          }
      individual.resize(individual() + 1);
      individual[individual() - 1].cb = c;
      individual[individual() - 1].dest = dest;
      individual[individual() - 1].src = src;
      individual[individual() - 1].lock = lock;
      TRACEPRINTF(Loggers(), 3, this, "registerIndividual %p = 1", c);

      ret = 1;
      throw Exception(LOOP_RETURN);
    }
  catch (Exception &e)
    {
      TraceDataLockRelease(&datalock);
      return ret;
    }

  assert(0);
  return false;
}

void
Layer3::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event(PTH_EVENT_SEM, stop1);

  unsigned i;
  unsigned long lastlowerversion = unknownVersion;

  while (pth_event_status(stop) != PTH_STATUS_OCCURRED)
    {
      LPDU *l = layer2->Get_L_Data(stop);
      if (!l)
        {
          // connection upper layer lost
          if (Connection_Lost())
            {
              if (lastlowerversion != Proxy().currentVersion())
                {
                  if (droponl2loss)
                    StopAllClients(false);
                  lastlowerversion = Proxy().currentVersion();
                  bumpVersion();
                  TRACEPRINTF(Loggers(), 1, this, "transition to down state");
                  // try to reopen
                }
              layer2->Open();
            }

          pth_sleep(1); // need to give it a break and not retry too often
          continue;
        }
      else
        {
          if (!Connection_Lost() && lastlowerversion != Proxy().currentVersion())
            {
              TRACEPRINTF(Thread::Loggers(), 2, this,
                                "got lower layer connection");
              lastlowerversion = Proxy().currentVersion();
              bumpVersion();
              TRACEPRINTF(Loggers(), 1, this, "transition to up state");
            }
        }

      if (!TraceDataLockWait(&datalock))
        break;

      if (l->getType() == L_Busmonitor)
        {
          L_Busmonitor_PDU *l1, *l2;
          l1 = (L_Busmonitor_PDU *) l;

          TRACEPRINTF(Loggers(), 3, this, "Recv %s", l1->Decode ()());
          for (i = 0; i < busmonitor(); i++)
            {
              l2 = new L_Busmonitor_PDU(*l1);
              busmonitor[i].cb->Get_L_Busmonitor(l2);
            }
          for (i = 0; i < vbusmonitor(); i++)
            {
              l2 = new L_Busmonitor_PDU(*l1);
              vbusmonitor[i].cb->Get_L_Busmonitor(l2);
            }
        }
      if (l->getType() == L_Data)
        {
          L_Data_PDU *l1;
          l1 = (L_Data_PDU *) l;
          if (l1->repeated)
            {
              CArray d1 = l1->ToPacket();
              for (i = 0; i < ignore(); i++)
                if (d1 == ignore[i].data)
                  {
                    WARNLOGSHAPE(Loggers(), LOG_WARNING,
                        Logging::DUPLICATESMAX1PER10SEC, this,
                        Logging::MSGNOHASH, "Repeated discarded");
                    goto wt;
                  }
            }
          l1->repeated = 1;
          ignore.resize(ignore() + 1);
          ignore[ignore() - 1].data = l1->ToPacket();
          ignore[ignore() - 1].end = getTime() + 1000000;
          l1->repeated = 0;

          if (l1->AddrType == IndividualAddress
              && l1->dest == layer2->getDefaultAddr())
            l1->dest = 0;
          TRACEPRINTF(Loggers(), 3, this, "Recv %s", l1->Decode ()());

          if (l1->AddrType == GroupAddress && l1->dest == 0)
            {
              for (i = 0; i < broadcast(); i++)
                broadcast[i].cb->Get_L_Data(new L_Data_PDU(*l1));
            }
          if (l1->AddrType == GroupAddress && l1->dest != 0)
            {
              for (i = 0; i < group(); i++)
                if (group[i].dest == l1->dest || group[i].dest == 0)
                  group[i].cb->Get_L_Data(new L_Data_PDU(*l1));
            }
          if (l1->AddrType == IndividualAddress)
            {
              for (i = 0; i < individual(); i++)
                if (individual[i].dest == l1->dest)
                  if (individual[i].src == l1->source || individual[i].src == 0)
                    individual[i].cb->Get_L_Data(new L_Data_PDU(*l1));
            }
        }
      redel: for (i = 0; i < ignore(); i++)
        if (ignore[i].end < getTime())
          {
            ignore.deletepart(i, 1);
            goto redel;
          }
      wt: delete l;

      TraceDataLockRelease(&datalock);
    }
  pth_event_free(stop, PTH_FREE_THIS);
}
