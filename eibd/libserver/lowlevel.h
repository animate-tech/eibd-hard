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

#ifndef LOWLEVEL_H
#define LOWLEVEL_H

#include "common.h"
#include "classinterfaces.h"

typedef enum { vUnknown, vEMI1, vEMI2, vCEMI, vRaw } EMIVer;

extern const char *EMI2Str(const EMIVer v);

/** implements a Driver to send packets for the EMI1/2 driver */
class LowLevelDriverInterface : public DroppableQueueInterface, public ErrCounters,
                                public ConnectionStateInterface, virtual protected LoggableObjectInterface
{
public:

  int maxpktsoutpersec;

  LowLevelDriverInterface (Logs *t,
                           int flags,
			   int inquemaxlen,
			   int outquemaxlen,
			   int maxpacketsoutpersecond,
			   char *inqueuename = "to-device",
			   char *outqueuename = "from-device");

  virtual ~ LowLevelDriverInterface ();
  virtual bool init () = 0;

  /** sends a EMI frame asynchronous, can be forced even on link down */
  virtual bool Send_Packet (CArray l, bool always=false) = 0;
  /** all frames sent ? */
  virtual bool Send_Queue_Empty ();
  /** returns semaphore, which becomes 1, if all frames are sent */
  virtual pth_sem_t *Send_Queue_Empty_Cond();
  /** waits for the next EMI frame
   * @param stop return NULL, if stop occurs
   * @return returns EMI frame or NULL
   */
  virtual CArray *Get_Packet (pth_event_t stop,bool readwhenstatusdown=false);
  /** waits for packet or link loss/up/down transition.
   *  can force immediate return if link not up
   *  @param readwhenstatusdown: return packet as well when link is declared down, can be used to initialize interface before officially up
   *  @return: empty packet if state change, otherwise packet */

  /** resets the connection */
  virtual bool SendReset () { ++stat_resets; return true; }
  bool TransitionToDownState();
  void DrainOut();
  void DrainIn();

  virtual EMIVer getEMIVer () = 0;

  int readAddrTabSize ( pth_event_t stop, uchar & result);
  int writeAddrTabSize ( pth_event_t stop, uchar size, bool reread=false);

  int readEMIMem ( pth_event_t stop, memaddr_t addr, uchar len,
  		CArray & result);
  int writeEMIMem ( pth_event_t stop, memaddr_t addr,
  		 CArray data, bool reread=false);

private:
  int readEMI1Mem ( pth_event_t stop, memaddr_t addr, uchar len,
  	     CArray & result);
  int writeEMI1Mem ( pth_event_t stop, memaddr_t addr, CArray data, bool reread=false);
  int readEMI2Mem ( pth_event_t stop, memaddr_t addr, uchar len,
		     CArray & result);
  int writeEMI2Mem ( pth_event_t stop, memaddr_t addr, CArray data, bool reread=false);

protected:
  /** input queue */
  Queue<CArray> inqueue;
  /** output queue */
  Queue<CArray *> outqueue;
  /** semaphore for inqueue */
  pth_sem_t in_signal;
  /** semaphore for outqueue */
  pth_sem_t out_signal;
  /** semaphore to signal empty sendqueue */
  pth_sem_t send_empty;

public:
  virtual const char *_str(void) const { return "LowLevelDriverInterface"; }
  virtual void logtic();
};

/** pointer to a functions, which creates a Low Level interface
 * @exception Exception in the case of an error
 * @param conf string, which contain configuration
 * @param t trace output
 * @return new LowLevel interface
 */
typedef LowLevelDriverInterface *(*LowLevel_Create_Func) (const char *conf,
							  Logs * t,
							  int flags,
							  int inquemaxlen, int outquemaxlen,
							  int maxpacketsoutpersecond);

#endif
