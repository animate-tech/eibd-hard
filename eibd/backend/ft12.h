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

#ifndef EIB_FT12_H
#define EIB_FT12_H

#include <termios.h>

#include "trace.h"
#include "threads.h"
#include "lowlevel.h"
#include "lowlatency.h"

/** FT1.2 lowlevel driver*/
class FT12LowLevelDriver:public LowLevelDriverInterface, protected Thread

{
  /** old serial config */
  low_latency_save sold;
  /** needed for xml status */
  string devname;
  /** file descriptor */
  int fd;
  /** saved termios */
  struct termios old;
  /** send state */
  int sendflag;
  /** recevie state */
  int recvflag;
    /** frame in receiving */
  CArray akt;
  /** repeatcount of the transmitting frame */
  int repeatcount;

  typedef enum {
    state_down = 0,
    state_send_reset,
    state_ifpresent,
    state_up_ready_to_send,
    waiting_for_ack
  } FT12LowLevelDriverStates;
  static const int INVALID_FD = -1;

  FT12LowLevelDriverStates state;

  static const int VARLENFRAME = 0x68;
  static const int ACKFRAME=0xE5;
  static const int FIXLENFRAME=0x10;

  const static char outdropmsg[], indropmsg[];

  void Run (pth_sem_t * stop);
public:
    FT12LowLevelDriver (const char *device, Logs * tr, int flags,
    		int inquemaxlen,
    		int outquemaxlen,
    		int maxpacketsoutpersecond );
   ~FT12LowLevelDriver ();
  bool init ();

  bool Send_Packet (CArray l, bool always=false);
  bool SendReset ();
  EMIVer getEMIVer ();
  bool TransitionToDownState();
  bool TransitionToUpState();
  const char *_str(void) const
  {
    return "FT1.2 Driver";
  }
  Element * _xml(Element *parent) const;
};

#endif
