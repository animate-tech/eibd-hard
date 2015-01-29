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

#ifndef EIB_EMI_H
#define EIB_EMI_H

#include "lpdu.h"

/** convert L_Data_PDU to CEMI frame */
CArray L_Data_ToCEMI (uchar code, const L_Data_PDU & p);
/** create L_Data_PDU out of a CEMI frame */
L_Data_PDU *CEMI_to_L_Data (const CArray & data);

L_Busmonitor_PDU *CEMI_to_Busmonitor (const CArray & data);
CArray Busmonitor_to_CEMI (uchar code, const L_Busmonitor_PDU & p, int no);

/** convert L_Data_PDU to EMI1/2 frame */
CArray L_Data_ToEMI (uchar code, const L_Data_PDU & p);
/** create L_Data_PDU out of a EMI1/2 frame */
L_Data_PDU *EMI_to_L_Data (const CArray & data);


#include "layer2.h"
#include "lowlevel.h"

/** EMI'fied interface layer 2
 * @note: albeit a thread does NOT need protection by semaphore, we only modify queues in the callbacks & those are already protected */
class EMILayer2Interface :  virtual public Layer2Interface,
    protected Thread
{
protected:
  LowLevelDriverInterface *iface;
  virtual bool
  Wait_Until_Lost_Or_Sent(void);

  /** state */
  int mode;
  /** default address */
  eibaddr_t def;
  /** vbusmonitor */
  int vmode;
  /** semaphore for outqueue */
  pth_sem_t out_signal;
  /** output queue */
  Queue<LPDU *> outqueue;

  const static char outdropmsg[], indropmsg[];

  void
  Run(pth_sem_t * stop);
public:
  EMILayer2Interface(LowLevelDriverInterface * i, int flags, Logs * tr, int inquemaxlen,
      int outquemaxlen);
  virtual
  ~EMILayer2Interface();
  bool
  init();

  bool
  Send_L_Data(LPDU * l);
  LPDU *
  Get_L_Data(pth_event_t stop);

  bool
  addAddress(eibaddr_t addr);
  bool
  addGroupAddress(eibaddr_t addr);
  bool
  removeAddress(eibaddr_t addr);
  bool
  removeGroupAddress(eibaddr_t addr);

  bool
  enterBusmonitor();
  bool
  leaveBusmonitor();
  bool
  openVBusmonitor();
  bool
  closeVBusmonitor();

  bool
  Open();
  bool
  Close();
  eibaddr_t
  getDefaultAddr();
  bool
  Send_Queue_Empty();

  const char *
  _str(void) const
  {
    static char buf[32];
    snprintf(buf, sizeof(buf) - 1, "L2 %s",
        iface ? EMI2Str(iface->getEMIVer()) : "???");
    return buf;
  }

  Element *
  _xml(Element *parent) const;
  virtual void logtic(void)  { if (iface) iface->logtic(); }
  virtual LowLevelDriverInterface *
  LowLevelDriver()
  {
    return iface;
  }
  bool SendReset();
};


#endif
