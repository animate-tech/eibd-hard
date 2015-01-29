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

#ifndef EIB_USB_H
#define EIB_USB_H

#include "lowlevel.h"
#include "libusb.h"
#include "threads.h"
#include "ip/ipv4net.h"
#include "statemachine.h"

typedef struct
{
  int bus;
  int device;
  int config;
  int altsetting;
  int interface;
} USBEndpoint;

typedef struct
{
  libusb_device *dev;
  int config;
  int altsetting;
  int interface;
  int stringdes[3];
  int sendep;
  int recvep;
} USBDevice;

typedef struct
{
  pth_sem_t signal;
  bool waitingforcancellation; // already canceled bu not completed
  Logs *tr;
} usb_complete_t;

class USBLowLevelDriver:public LowLevelDriverInterface, protected Thread
{
  typedef enum
  {
    supportedEMI  = 0x01,
    BUSConnStatus = 0x03,
    ActiveEMI     = 0x05,
  } DeviceFeatures;
  typedef enum
  {
    DeviceFeatureGet = 0x01,
    DeviceFeatureResp= 0x02,
    DeviceFeatureSet = 0x03,
    DeviceFeatureInfo= 0x04
  } ServiceId;

  const static char outdropmsg[], indropmsg[];

  void
  Run(pth_sem_t * stop);

public:
  USBLowLevelDriver(const char *device, Logs * tr, int flags, int inquemaxlen,
      int outquemaxlen, int maxpacketsoutpersecond,
      IPv4NetList &ipnetfilters);
  ~USBLowLevelDriver();

  bool init();

  bool
  Send_Packet(CArray l, bool always=false);
  bool
  SendReset();
  bool
  Connection_Lost() const;
  EMIVer
  getEMIVer(void);

  const char *
  _str(void) const
  {
    return "USB Driver";
  }

  Element *
  _xml(Element *parent) const;

private:
  bool Always_Send_Packet(CArray l);
  const char *
  InterfaceModel() const;
  unsigned long version;
  EMIVer internemiver;
  int errorstowardsreset;
  static const int ERR2RESETTHRESHOLD = 10;
  bool resetaddrtables;

  // FSM variables
  typedef enum
  {
    fsm_state_invalid = -1,
    fsm_state_init,
    fsm_state_down,
    fsm_state_up_reset_start, // 2 reset going, sends periodically, expects EMI
    fsm_state_up_no_bus, // reset & ready to roar but no BUS present yet
    fsm_state_reclaim_buffers, // 4 reclaiming buffers for libusb
    fsm_state_up, // 5 & we see the BUS being present as well
  } USBLowLevelDriverStates;

  typedef enum
  {
    fsm_event_short_tic,
    fsm_event_long_tic,
    fsm_event_USB_received, // 2
    fsm_event_USB_transmitted,

    fsm_event_too_many_errors,

    fsm_event_bus_down_report, // 5
    fsm_event_bus_up_report,

    fsm_event_input_ready,
    fsm_event_output_generated, // 8

    fsm_event_spontaneous, // internal event to trigger stuff immediately
  } USBLowLevelDriverEvents;

  StateMachine<USBLowLevelDriverStates, USBLowLevelDriverEvents, USBLowLevelDriver> FSM;
  void _fsm_init(void);
  void _fsm_try_device();
  void _fsm_send_reset_and_restart_timer(void);
  void _fsm_reset_failed(void);
  void _fsm_reset_received(void);
  void _fsm_idle();
  void _fsm_spurious_event();
  void _fsm_up_USB_receive();
  void _fsm_input_received();
  void _fsm_send_state_request();
  void _fsm_drain_queues();

  void _fsm_startReceive(void);
  void _fsm_stopReceive(void);
  void _fsm_completeReceive(void);
  void _fsm_completeReceiveAndRedo(void);
  void _fsm_startSend(void);
  void _fsm_stopSend(void);
  void _fsm_completeSend(void);

  void _fsm_up_onesec_tic(void);
  void _fsm_no_up_bus_up(void);
  void _fsm_bus_up_to_down(void);

  bool _fsm_resetReceived();

  bool _fsm_startEndPoint();
  void _fsm_stopEndPoint();
  void _fsm_transitionToDownState();

  void _fsm_stop_shorttick(void);
  void _fsm_stop_longtick(void);

  void _fsm_reclaimBuffers();
  void _fsm_reclaimBuffers_shorttic();
  void _fsm_reclaimBuffers_longtic();

  void _fsm_set_NewState(USBLowLevelDriverStates s);

  USBEndpoint _fsm_parseUSBEndpoint (const char *addr);
  USBDevice   _fsm_detectUSBEndpoint (USBEndpoint &e,Logs * tr);

  bool check_device(libusb_device * dev, USBEndpoint & e, USBDevice & e2);

  USBEndpoint parseUSBEndpoint(const char *addr);
  void InitFSM();

  USBEndpoint e;
  pth_event_t stop ;
  uchar recvbuf[64];
  uchar sendbuf[64];
  struct libusb_transfer *sendh;
  struct libusb_transfer *recvh;
  usb_complete_t sendc, recvc;
  pth_event_t sende, recve;
  pth_event_t shorttic, longtic,input,output;
  libusb_device_handle *dev;
  USBDevice d;
  std::string DeviceDefinition;
  int pktthissec;

  // 5.3.3.1 set EMI type to EMI type
  static uchar _init[64] ;
  // 5.3 Application Note Bus Access Server Feature Device Feature Get
  static uchar _ask[64];
  const static int Time2ResetUsbLibContext=30;
  const static int Time2RetransmitResets=3;
  const static int Time2RetryDeviceOpen=10;
  const static int Time2WaitforBUSStatus=10;

  static const pth_time_t infinity_time ;

};

#endif
