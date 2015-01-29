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

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "usbif.h"
#include "usb.h"
#include "eibtypes.h"
#include "eibpriority.h"
#include "layer2.h"
#include "statemachine.h"

extern USBLoop *loop;

USBEndpoint
USBLowLevelDriver::parseUSBEndpoint(const char *addr)
{
  USBEndpoint e;
  e.bus = -1;
  e.device = -1;
  e.config = -1;
  e.altsetting = -1;
  e.interface = -1;
  if (!*addr)
    return e;
  if (!isdigit(*addr))
    return e;
  e.bus = atoi(addr);
  while (isdigit(*addr))
    addr++;
  if (*addr != ':')
    return e;
  addr++;
  if (!isdigit(*addr))
    return e;
  e.device = atoi(addr);
  while (isdigit(*addr))
    addr++;
  if (*addr != ':')
    return e;
  addr++;
  if (!isdigit(*addr))
    return e;
  e.config = atoi(addr);
  while (isdigit(*addr))
    addr++;
  if (*addr != ':')
    return e;
  addr++;
  if (!isdigit(*addr))
    return e;
  e.altsetting = atoi(addr);
  if (*addr != ':')
    return e;
  addr++;
  if (!isdigit(*addr))
    return e;
  e.interface = atoi(addr);
  return e;
}

bool USBLowLevelDriver::check_device(libusb_device * dev, USBEndpoint & e, USBDevice & e2)
{
  struct libusb_device_descriptor desc;
  struct libusb_config_descriptor *cfg;
  const struct libusb_interface *intf;
  const struct libusb_interface_descriptor *alts;
  const struct libusb_endpoint_descriptor *ep;
  libusb_device_handle *h;
  int j, k, l, m, r;
  int in, out;

  if (!dev)
    return false;

  if (libusb_get_bus_number(dev) != e.bus && e.bus != -1)
    return false;
  if (libusb_get_device_address(dev) != e.device && e.device != -1)
    return false;

  if (libusb_get_device_descriptor(dev, &desc))
    return false;

  cfg=NULL;
  for (j = 0; j < desc.bNumConfigurations; j++)
    {
      if (cfg)
        {
          libusb_free_config_descriptor(cfg);
          cfg=NULL;
        }
      if (libusb_get_config_descriptor(dev, j, &cfg))
        continue;
      if (cfg->bConfigurationValue != e.config && e.config != -1)
        continue;

      for (k = 0; k < cfg->bNumInterfaces; k++)
        {
          intf = &cfg->interface[k];
          for (l = 0; l < intf->num_altsetting; l++)
            {
              alts = &intf->altsetting[l];
              if (alts->bInterfaceClass != LIBUSB_CLASS_HID)
                continue;
              if (alts->bAlternateSetting != e.altsetting && e.altsetting != -1)
                continue;
              if (alts->bInterfaceNumber != e.interface && e.interface != -1)
                continue;

              in = 0;
              out = 0;
              for (m = 0; m < alts->bNumEndpoints; m++)
                {
                  ep = &alts->endpoint[m];
                  if (ep->wMaxPacketSize == 64)
                    {
                      if (ep->bEndpointAddress & LIBUSB_ENDPOINT_IN)
                        {
                          if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK)
                              == LIBUSB_TRANSFER_TYPE_INTERRUPT)
                            in = ep->bEndpointAddress;
                        }
                      else
                        {
                          if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK)
                              == LIBUSB_TRANSFER_TYPE_INTERRUPT)
                            out = ep->bEndpointAddress;
                        }
                    }
                }

              if (!in || !out)
                continue;
              r = libusb_open(dev, &h);
              if (!r)
                {
                  USBDevice e1;
                  e1.dev = dev;
                  libusb_ref_device(dev);
                  e1.config = cfg->bConfigurationValue;
                  e1.interface = alts->bInterfaceNumber;
                  e1.altsetting = alts->bAlternateSetting;
                  e1.stringdes[0] = desc.iManufacturer;
                  e1.stringdes[1] = desc.iProduct;
                  e1.stringdes[2] = desc.iSerialNumber;
                  e1.sendep = out;
                  e1.recvep = in;
                  libusb_close(h);
                  e2 = e1;
                  libusb_free_config_descriptor(cfg);
                  return true;
                }
              else
                {
                  TRACEPRINTF(Thread::Loggers(), 1, NULL,
                      "Suitable device USB cannot be opened, code %d", r);
                }
            }
        }
    }
  return false;
}

#define SHOWTRANSITION_NAME(f) TRACEPRINTF(Thread::Loggers(), 6, this, "FSM: %s", f);

USBDevice
USBLowLevelDriver::_fsm_detectUSBEndpoint(USBEndpoint &e, Logs * tr)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  libusb_device **devs;
  int i, count;
  USBDevice e2;
  e2.dev = NULL;
  libusb_context *context = loop->GrabContext(this);
  count = libusb_get_device_list(context, &devs);

  for (i = 0; i < count; i++)
    if (check_device(devs[i], e, e2))
      break;

  libusb_free_device_list(devs, 1);
  loop->ReleaseContext(this);

  return e2;
}

const pth_time_t USBLowLevelDriver::infinity_time = { 327676, 0 };

USBLowLevelDriver::USBLowLevelDriver(const char *Dev, Logs * tr, int flags,
    int inquemaxlen, int outquemaxlen, int maxpacketsoutpersecond,
    IPv4NetList &ipnetfilters) :
    LowLevelDriverInterface(tr, inquemaxlen, outquemaxlen, flags,
        maxpacketsoutpersecond, "to-USB-device", "from-USB-device"), Thread(tr,
        PTH_PRIO_STD, "USB Driver"), version(unknownVersion + 100), internemiver(
        vUnknown),
        FSM(this,true,tr,"USBLowLevelDriver FSM"),
        DeviceDefinition(Dev)
{
  TRACEPRINTF(Thread::Loggers(), 2, this, "Created & allocated @ %p proxy %p", this, & Proxy());
  resetaddrtables=flags&FLAG_B_RESET_ADDRESS_TABLE;
  Start(); // start first so it starts to process packets
}

void
USBLowLevelDriver::_fsm_stopEndPoint()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  libusb_context *context = loop->GrabContext(this);

  if (d.interface && dev)
    {
      TRACEPRINTF(Thread::Loggers(), 3, this, "releasing interface & returning kernel driver");
      libusb_release_interface(dev, d.interface);
      libusb_attach_kernel_driver(dev, d.interface);
    }
  if (dev)
    {
      TRACEPRINTF(Thread::Loggers(), 3, this, "Closing libusb");
      libusb_close(dev);
      // whatever those were, not valid anymore
      recvh = NULL;
      sendh = NULL;
      dev = NULL;
    }
  loop->ReleaseContext(this);
}

void USBLowLevelDriver::_fsm_reclaimBuffers()
{

  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);
  errorstowardsreset=0;
  pktthissec = 0;
  _fsm_set_NewState(fsm_state_reclaim_buffers);
}

void USBLowLevelDriver::_fsm_reclaimBuffers_shorttic()
{

  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);
  errorstowardsreset=0;
  pktthissec = 0;
  if (!sendh && !recvh) // all cleaned up now
    {
      _fsm_transitionToDownState();
    }
  // otherwise refresh timers and keep going
  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE,  shorttic, pth_time(1, 0));
}

void USBLowLevelDriver::_fsm_reclaimBuffers_longtic()
// even we could not reclaim buffers, go ahead
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);
  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE,  longtic,  pth_time(10, 0));
  ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT,
                Logging::DUPLICATESMAX1PER30SEC, this, Logging::MSGNOHASH,
                "USB device cannot be brought down %s, either extremee stability problems or libusb problems", InterfaceModel());
}

void USBLowLevelDriver::_fsm_transitionToDownState()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
                Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                "USB device %s going down", InterfaceModel());
  // never, ever hit us with something that is not having cleaned up buffers
  // and going down !
  assert(!recvh && !sendh);
  ConnectionStateInterface::TransitionToDownState();
  _fsm_stopEndPoint();
  _fsm_set_NewState(fsm_state_down);
  errorstowardsreset = 0;
  internemiver = vUnknown;
  pth_yield(NULL);
}

bool
USBLowLevelDriver::_fsm_startEndPoint()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  // stop processing on libusb
  libusb_context *context = loop->GrabContext(this);

  TRACEPRINTF(Thread::Loggers(), 3, this, "_fsm_startEndPoint");

  assert(FSM.GetCurrentState() == fsm_state_down);
  try
    {
      d = _fsm_detectUSBEndpoint(e, Thread::Loggers());
      if (d.dev == 0)
        {
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
              Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
              "Device open for USB Failed, no suitable device: %s", strerror(errno));
          throw std::exception();
        }
      if (libusb_open(d.dev, &dev) < 0)
        {
          ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT,
              Logging::DUPLICATESMAX1PERMIN, this, Logging::MSGNOHASH,
              "Device open for USB Failed: %s", strerror(errno));
          throw std::exception();
        }
      INFOLOG(Thread::Loggers(), LOG_INFO, this,
          "Detected USB device %d:%d:%d:%d:%d (%d:%d) %s", libusb_get_bus_number (d.dev), libusb_get_device_address (d.dev), d.config, d.altsetting, d.interface, d.sendep, d.recvep, InterfaceModel());

      if (d.dev)
        {
          libusb_unref_device(d.dev);
        }
      libusb_detach_kernel_driver(dev, d.interface);
      if (libusb_set_configuration(dev, d.config) < 0)
        {
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
              Logging::DUPLICATESMAX1PERMIN, this, Logging::MSGNOHASH,
              "Device configuration for USB failed: %s", strerror(errno));
          throw std::exception();
        }
      if (libusb_claim_interface(dev, d.interface) < 0)
        {
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
              Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
              "Device claim for USB failed: %s", strerror(errno));
          throw std::exception();
        }
      if (libusb_set_interface_alt_setting(dev, d.interface, d.altsetting) < 0)
        {
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
              Logging::DUPLICATESMAX1PERMIN, this, Logging::MSGNOHASH,
              "Device alt setting for USB failed: %s", strerror(errno));
          throw std::exception();
        }
      TRACEPRINTF(Thread::Loggers(), 1, this, "Claimed");
    }
  catch (std::exception &e)
    {
      if (dev)
        libusb_close(dev);
      dev = NULL;
      loop->ReleaseContext(this);
      return false;
    }
  loop->ReleaseContext(this);
  return true;
}

USBLowLevelDriver::~USBLowLevelDriver()
{

  TRACEPRINTF(Thread::Loggers(), 1, this, "Destructor");
  TransitionToDownState();
  Stop();
}

bool
USBLowLevelDriver::init()
{
  return true;
}

bool
USBLowLevelDriver::Connection_Lost() const
{
  return FSM.GetCurrentState() < fsm_state_up;
}

bool USBLowLevelDriver::Always_Send_Packet(CArray l)
{

  Thread::Loggers()->TracePacket(3, this, "Send_Packet Send", l);

  return Put_On_Queue_Or_Drop<CArray, CArray>(inqueue, l, &in_signal, true,
      indropmsg, &send_empty);
}

bool
USBLowLevelDriver::Send_Packet(CArray l, bool  always)
{
  CArray pdu;

  if (FSM.GetCurrentState() < fsm_state_up && ! always)
    {
      return false;
    }
  return Always_Send_Packet(l);
}

 uchar USBLowLevelDriver::_init[64] =
    { 0x01, 0x13, 0x0a, 0x00, 0x08, 0x00, 0x02, 0x0f, DeviceFeatureSet, 0x00,
        0x00, ActiveEMI, 0x01 };
 uchar USBLowLevelDriver::_ask[64] =
    { 0x01, 0x13, 0x09, 0x00, 0x08, 0x00, 0x01, 0x0f, DeviceFeatureGet,
        0x00, 0x00, supportedEMI };

bool
USBLowLevelDriver::SendReset()
{
  if (FSM.GetCurrentState() == fsm_state_up_reset_start)
    {
      // drain any pending queues first
      DrainIn();
      DrainOut();

      LowLevelDriverInterface::SendReset();
      TRACEPRINTF(Thread::Loggers(), 1, this,
          "Init Sequence & EMI Version Request");
      CArray r1, *r = 0;
      LowLevelDriverInterface *iface = NULL;
      int cnt = 0;

      // ask for EMI type
      _ask[11] = supportedEMI;
      Always_Send_Packet(CArray(_ask, sizeof(_ask)));
    }
  return true;
}

void USBLowLevelDriver::_fsm_drain_queues()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  DrainIn();
  DrainOut();
}

bool USBLowLevelDriver::_fsm_resetReceived()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  CArray r1, *r = 0;
  bool pass = false;
  uchar emiver;
  uchar verbyte;

  r = Get_Packet(stop, true); // also when link down
  if (r)
    {
      r1 = *r;
      Thread::Loggers()->TracePacket(3, this,
          "Init Sequence & EMI Version Request Recvd ", r1);
      pass = !(r1.len() != 64 || r1[0] != 01 || (r1[1] & 0x0f) != 0x03
          || r1[2] != 0x0b || r1[3] != 0x00 || r1[4] != 0x08 || r1[5] != 0x00
          || r1[6] != 0x03 || r1[7] != 0x0F || r1[8] != 0x02 || r1[11] != 0x01);
      verbyte = r1[13];
      delete r;
      r = 0;
      if (!pass)
        {
          // well, failed, let's request again
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
              Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
              "Failed to retrieve EMI version on interface");
        }
      else
        {
          Thread::Loggers()->TracePacket(3, this, "GOT Emi Response", r1);
        }
    }
  else
    {
      TRACEPRINTF(Thread::Loggers(), 2, this,
          "no packet on interface during EMI Version request ");
      return false;
    }
  TRACEPRINTF(Thread::Loggers(), 8, this, "EMI Byte 0x%d", (int) verbyte);
  if (verbyte & 0x2) // prefer 2
    emiver = 2;
  else if (verbyte & 0x1)
    emiver = 1;
  else if (verbyte & 0x4)
    emiver = 3;
  else
    emiver = 0;

  switch (emiver)
    {
  case 1: // emi1
    internemiver = vEMI1;
    break;
  case 2: // emi2
    internemiver = vEMI2;
    break;
  case 3: // vCEMI
    internemiver = vCEMI;
    break;
  default:
    ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT, Logging::DUPLICATESMAX1PERMIN,
        this, Logging::MSGNOHASH,
        "Unsupported EMI on USB %02x %02x %s", r1[12], r1[13], (r1[12]==0 && r1[13]==0) ? "(possibly BUS loss)" : "");
    internemiver = vUnknown;
    return false;
    }
  ++ErrCounters::stat_resets;
  INFOLOG(Thread::Loggers(), LOG_INFO, this,
      "%s reset successfully, set to %s", InterfaceModel(), EMI2Str(internemiver));

  return true;
}

void USBLowLevelDriver::_fsm_send_state_request()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE,  shorttic, pth_time(Time2RetransmitResets, 0));
  switch (internemiver)
    {
  case vEMI1: // emi1
    _init[12] = 1;
    break;
  case vEMI2: // emi2
    _init[12] = 2;
    break;
  case vCEMI: // vCEMI
    _init[12] = 3;
    break;
  default:
    assert(false); // how did we get here ?
    }
  pktthissec = 0;
  Always_Send_Packet(CArray(_init, sizeof(_init)));
  _ask[11] = BUSConnStatus;
  Always_Send_Packet(CArray(_ask, sizeof(_ask)));
}

EMIVer
USBLowLevelDriver::getEMIVer(void)
{
  return internemiver;
}

const char *
TransferStatus2String(libusb_transfer_status s)
{
  switch (s)
    {
  case LIBUSB_TRANSFER_COMPLETED:
    return "completed";
  case LIBUSB_TRANSFER_ERROR:
    return "transfer error";
  case LIBUSB_TRANSFER_TIMED_OUT:
    return "transfer timeout";
  case LIBUSB_TRANSFER_CANCELLED:
    return "transfer canceled";
  case LIBUSB_TRANSFER_STALL:
    return "transfer stalled";
  case LIBUSB_TRANSFER_NO_DEVICE:
    return "no device";
  case LIBUSB_TRANSFER_OVERFLOW:
    return "overflow";
    }
  return "unknown";
}

void usb_complete(struct libusb_transfer *transfer)
{
  usb_complete_t * complete = (usb_complete_t *) transfer->user_data;
  TRACEPRINTF(complete->tr, 5, NULL, "usb_complete for transfer %p cancelling %d with status %s",
      (void *) transfer,
      complete->waitingforcancellation,
      TransferStatus2String(transfer->status));
  pth_sem_inc(&complete->signal,false);
}

void USBLowLevelDriver::_fsm_startReceive(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  libusb_context *context=loop->GrabContext(this);

  try
    {
      if (!recvh)
        {
          recvh = libusb_alloc_transfer(0);
          if (!recvh)
            {
              ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT,
                  Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Error allocate receive on USB");
              ++stat_recverr;
              errorstowardsreset++;
              throw std::exception();
            }
          libusb_fill_interrupt_transfer(recvh, dev, d.recvep, recvbuf,
              sizeof(recvbuf), usb_complete, &recvc, 30000);
          recvc.waitingforcancellation = false;
          if (libusb_submit_transfer(recvh))
            {
              ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT,
                  Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Error start receive on USB %s", TransferStatus2String(recvh->status));
              ++stat_recverr;
              if (recvh->status == LIBUSB_TRANSFER_NO_DEVICE)
                {
                  FSM.PushEvent(fsm_event_too_many_errors);
                }
              else
                {
                  errorstowardsreset++;
                }

              TRACEPRINTF(Thread::Loggers(), 5, this,
                  "freeing receive transfer %p", (void *) recvh);
              libusb_free_transfer(recvh);
              recvh = 0;
              throw std::exception();
            }

          TRACEPRINTF(Thread::Loggers(), 5, this,
              "submitted receive transfer %p", (void *) recvh);
          pth_event(PTH_EVENT_SEM | PTH_MODE_REUSE | PTH_UNTIL_DECREMENT, recve,
              &recvc.signal);
        }
    }
  catch (std::exception &e)
    {
    }

  loop->ReleaseContext(this);
}

void USBLowLevelDriver::_fsm_completeReceive()
{

  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  assert(recvh);
  libusb_context *context = loop->GrabContext(this);
  if (recvh->status != LIBUSB_TRANSFER_COMPLETED)
    {
      if (recvh->status != LIBUSB_TRANSFER_TIMED_OUT )
        // this is unsolvable, VERY slow buses will generate it
        // on regular bases
        {
          ++stat_recverr;
          errorstowardsreset++;
          ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT,
              Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
              "Receive error on USB: %s", TransferStatus2String(recvh->status));
        }
    }
  else
    {
      TRACEPRINTF(Thread::Loggers(), 5, this,
          "RecvComplete %d", recvh->actual_length);
      static const uchar statusnotify[] =
        { 0x01, 0x13, 0x0A, 0x00, 0x08, 0x00, 0x02, 0x0F, DeviceFeatureInfo,
            0x00, 0x00, BUSConnStatus, 0x01 };
      static const uchar statusresponse[] =
        { 0x01, 0x13, 0x0A, 0x00, 0x08, 0x00, 0x02, 0x0F, DeviceFeatureResp,
            0x00, 0x00, BUSConnStatus, 0x01 };
      static CArray notres1(statusnotify, sizeof(statusnotify));
      static CArray notres2(statusresponse, sizeof(statusresponse));
      CArray partres(recvbuf, sizeof(statusnotify));

      // 5.3.3.1 received bus status either by request or as notification
      (partres)[12] = (partres)[12] & 0x01;

      notres1[12] = 0x01;
      notres2[12] = 0x01; // test for up ?
      /** Thread::Loggers()->TracePacket(0, this, "RecvUSB part", *patres);
       * Thread::Loggers()->TracePacket(0, this, "RecvUSB notres1", notres1);
       * Thread::Loggers()->TracePacket(0, this, "RecvUSB notres2", notres2);
       */

      if (partres == notres1 || partres == notres2)
        {
          INFOLOG(Thread::Loggers(), LOG_INFO, this,
              "%s KNX BUS & interface received operational status", InterfaceModel());
          FSM.PushEvent(fsm_event_bus_up_report);
        }
      else
        {
          notres1[12] = 0x00;
          notres2[12] = 0x00; // test for down ?
          if (partres == notres1 || partres == notres2)
            {
              ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT,
                                Logging::DUPLICATESMAX1PERMIN, this, Logging::MSGNOHASH,
                  "%s KNX BUS reported not ready, operational but down", InterfaceModel());
              FSM.PushEvent(fsm_event_bus_down_report);
            }
          else
            {
              CArray *np = new CArray(recvbuf, sizeof(recvbuf));
              // status check was here

              Thread::Loggers()->TracePacket(3, this,
                  "RecvUSB & Queuing for Consumption", *np);

              Put_On_Queue_Or_Drop<CArray *, CArray *>(outqueue, np,
                  &out_signal, true, outdropmsg);
            }
        }
    }
  if (recvh)
    {
      TRACEPRINTF(Thread::Loggers(), 5, this,
          "freeing receive transfer %p", (void *) recvh);
      libusb_free_transfer(recvh);
    }
  recvh = 0;
  loop->ReleaseContext(this);
}


void USBLowLevelDriver::_fsm_completeReceiveAndRedo()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  _fsm_completeReceive();
  _fsm_startReceive();
}

void USBLowLevelDriver::_fsm_startSend()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  libusb_context *context=loop->GrabContext(this);
  if (!sendh && !inqueue.isempty())
    {
      try
        {
          const CArray & c = inqueue.top();
          Thread::Loggers()->TracePacket(4, this, "Run Send", c);
          memset(sendbuf, 0, sizeof(sendbuf));
          memcpy(sendbuf, c.array(),
              (c() > sizeof(sendbuf) ? sizeof(sendbuf) : c()));
          sendh = libusb_alloc_transfer(0);
          sendc.waitingforcancellation = false;
          if (!sendh)
            {
              ++stat_senderr;
              errorstowardsreset++;
              ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT,
                  Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Error start send on USB %s", TransferStatus2String(sendh->status));

              throw std::exception();
            }
          libusb_fill_interrupt_transfer(sendh, dev, d.sendep, sendbuf,
              sizeof(sendbuf), usb_complete, &sendc, 1000);
          if (libusb_submit_transfer(sendh))
            {
              ++stat_senderr;
              if (sendh->status == LIBUSB_TRANSFER_NO_DEVICE)
                {
                  FSM.PushEvent(fsm_event_too_many_errors);
                }
              else
                {
                  errorstowardsreset++;
                }

              ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT,
                  Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                  "Error send on USB %s", TransferStatus2String(sendh->status));

              TRACEPRINTF(Thread::Loggers(), 5, this,
                  "freeing send transfer %p", (void *) sendh);
              libusb_free_transfer(sendh);
              sendh = 0;
              throw std::exception();
            }
          TRACEPRINTF(Thread::Loggers(), 5, this,
              "submitted send transfer %p", (void *) sendh);
          pth_event(PTH_EVENT_SEM | PTH_MODE_REUSE | PTH_UNTIL_DECREMENT, sende,
              &sendc.signal);
        }
      catch (std::exception &e)
        {

        }
    }
  loop->ReleaseContext(this);
}

void USBLowLevelDriver::_fsm_completeSend(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  assert(sendh);
  libusb_context *context = loop->GrabContext(this);
  if (sendh->status != LIBUSB_TRANSFER_COMPLETED)
    {
      ++stat_senderr;
      if (sendh->status == LIBUSB_TRANSFER_NO_DEVICE)
        {
          FSM.PushEvent(fsm_event_too_many_errors);
        }
      else
        {
          errorstowardsreset++;
        }
      ERRORLOGSHAPE(Thread::Loggers(), LOG_CRIT,
          Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
          "Send error %s on USB", TransferStatus2String(sendh->status));
    }
  else
    {
      TRACEPRINTF(Thread::Loggers(), 5, this,
          "SendComplete %d", sendh->actual_length);
      pth_sem_dec(&in_signal);
      inqueue.get();
      if (inqueue.isempty())
        pth_sem_set_value(&send_empty, 1);
      pktthissec++;
    }
  if (sendh)
    {
      TRACEPRINTF(Thread::Loggers(), 5, this,
          "freeing send transfer %p", (void *) sendh);
      libusb_free_transfer(sendh);
    }
  sendh = 0;
  loop->ReleaseContext(this);
}

void USBLowLevelDriver::_fsm_init(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  e = parseUSBEndpoint(DeviceDefinition.c_str());
  _fsm_transitionToDownState();
  FSM.PushEvent(fsm_event_spontaneous); // that will give immediately a kick to try interface
};

void USBLowLevelDriver::_fsm_try_device()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  TRACEPRINTF(Thread::Loggers(), 4, this,
        "Trying to regain USB Device");
  errorstowardsreset = 0;
  if (_fsm_startEndPoint()) // will try to bring it back
    {
      _fsm_set_NewState(fsm_state_up_reset_start);
      FSM.PushEvent(fsm_event_spontaneous);
      return;
    }
  _fsm_set_NewState(fsm_state_reclaim_buffers);
}

void USBLowLevelDriver::_fsm_send_reset_and_restart_timer(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  pktthissec = 0;
  SendReset();
  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, shorttic, pth_time(Time2RetransmitResets, 0));
}

void USBLowLevelDriver::_fsm_reset_failed(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  TransitionToDownState();
#if 0
  // let's close the USB, this does not look good
  {
    loop->RefreshContext(this);
  }
#endif
  _fsm_set_NewState(fsm_state_reclaim_buffers);
}

void USBLowLevelDriver::_fsm_reset_received(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  if (_fsm_resetReceived())
    {
      pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, longtic, pth_time(Time2WaitforBUSStatus, 0));

      _fsm_set_NewState(fsm_state_up_no_bus);
      FSM.PushEvent(fsm_event_spontaneous);
    }
  _fsm_startReceive();
}

void USBLowLevelDriver::_fsm_no_up_bus_up(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  _fsm_set_NewState(fsm_state_up);
  ConnectionStateInterface::TransitionToUpState();
  INFOLOG(Thread::Loggers(), LOG_INFO, this,
                "%s KNX BUS interface ready and up", InterfaceModel());
}

void USBLowLevelDriver::_fsm_idle()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  // designated empty, never implement anything on it
  return;
}

void USBLowLevelDriver::_fsm_spurious_event()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  // well, no hurt but why here ?
  INFOLOG(Thread::Loggers(), LOG_ALERT, this,
                  "%s spurious FSM event, to be sure shutting down interface", InterfaceModel());
  TransitionToDownState();
  _fsm_set_NewState(fsm_state_reclaim_buffers);
}

void USBLowLevelDriver::_fsm_stop_shorttick()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE,  shorttic, infinity_time);
}

void USBLowLevelDriver::_fsm_stop_longtick()
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE,  longtic, infinity_time);
}

void USBLowLevelDriver::_fsm_up_onesec_tic(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, shorttic,
               pth_time(1, 0));
  pktthissec = 0; // just clear number packets
  _fsm_startReceive();
  _fsm_startSend();
}

void USBLowLevelDriver::_fsm_bus_up_to_down(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  _fsm_set_NewState(fsm_state_up_no_bus);
  FSM.PushEvent(fsm_event_spontaneous);
}

void USBLowLevelDriver::_fsm_stopReceive(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  libusb_context *context = loop->GrabContext(this);
  if (recvh)
    {
      TRACEPRINTF(Thread::Loggers(), 4, this, "Stop USB Receive");
      if (!recvc.waitingforcancellation) {
        libusb_cancel_transfer(recvh);
        recvc.waitingforcancellation=true;
      }
    }
  loop->ReleaseContext(this);
}

void USBLowLevelDriver::_fsm_stopSend(void)
{
  SHOWTRANSITION_NAME( __PRETTY_FUNCTION__);

  libusb_context *context = loop->GrabContext(this);
  if (sendh)
    {
          TRACEPRINTF(Thread::Loggers(), 4, this, "Stop USB Send");
          if (!sendc.waitingforcancellation) {
            libusb_cancel_transfer(sendh);
            sendc.waitingforcancellation=true;
          }
    }
  loop->ReleaseContext(this);
}

// template StateMachine<USBLowLevelDriver::USBLowLevelDriverStates, USBLowLevelDriver::USBLowLevelDriverEvents, USBLowLevelDriver>;

void
USBLowLevelDriver::_fsm_set_NewState(USBLowLevelDriverStates s)
// set new state, obviously events MAY still be pending in the queue generated by the old state
// (spontaneous transfer being one of those) so
{
  const struct
  {
    USBLowLevelDriverStates state;
    pth_time_t newshorttimertick;
    pth_time_t newlongetimertick;
    bool       startreceive;
    bool       startsend;
  } state2InitialTimers[] =
    {
      { fsm_state_init,            pth_time(1, 0), pth_time(10, 0),
                                   false, false },
      { fsm_state_down,            infinity_time,  pth_time(Time2RetryDeviceOpen, 0),
                                   false, false },
      { fsm_state_up_reset_start,  pth_time(Time2RetransmitResets, 0),  pth_time(Time2ResetUsbLibContext, 0),
                                   true, true },
      { fsm_state_up_no_bus,       pth_time(Time2RetransmitResets, 0), pth_time(Time2WaitforBUSStatus, 0),
                                   true, true },
      { fsm_state_reclaim_buffers, pth_time(1, 0), pth_time(10, 0),
                                   false, false },
      { fsm_state_up,              pth_time(1, 0), infinity_time,
                                   true, true  },
      { fsm_state_invalid,         infinity_time,   infinity_time,
                                   false, false }, // overrun
    };

  assert(state2InitialTimers[s].state == s &&
      state2InitialTimers[s].state != fsm_state_invalid ); // jsut in case states change

  // stop timers
  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, longtic, state2InitialTimers[s].newlongetimertick);
  pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, shorttic, state2InitialTimers[s].newshorttimertick);

  if (state2InitialTimers[s].startreceive)
    _fsm_startReceive();
  else
    {
      _fsm_stopReceive();
      DrainIn();
    }

  if (state2InitialTimers[s].startsend)
      _fsm_startSend();
    else
      {
        _fsm_stopSend();
        DrainOut();
      }

  FSM.SetCurrentState(s);
}

void USBLowLevelDriver::InitFSM(void)
{

  FSM.SetStateCallback(fsm_state_init, fsm_event_spontaneous,
      &USBLowLevelDriver::_fsm_init);
  //
  FSM.SetStateCallback(fsm_state_down, fsm_event_spontaneous,
      &USBLowLevelDriver::_fsm_try_device);
  FSM.SetStateCallback(fsm_state_down, fsm_event_long_tic,
      &USBLowLevelDriver::_fsm_try_device);
  FSM.SetStateCallback(fsm_state_down, fsm_event_short_tic,
      &USBLowLevelDriver::_fsm_stop_shorttick);

  // may be just cancelled transfers
  FSM.SetStateCallback(fsm_state_down, fsm_event_USB_received,
      &USBLowLevelDriver::_fsm_completeReceive);
  FSM.SetStateCallback(fsm_state_down, fsm_event_USB_transmitted,
      &USBLowLevelDriver::_fsm_completeSend);

  FSM.SetStateCallback(fsm_state_down, fsm_event_input_ready,
      &USBLowLevelDriver::_fsm_drain_queues);
  FSM.SetStateCallback(fsm_state_down, fsm_event_output_generated,
      &USBLowLevelDriver::_fsm_drain_queues);

  FSM.SetStateCallback(fsm_state_down, fsm_event_too_many_errors,
      &USBLowLevelDriver::_fsm_reclaimBuffers);
  //
  // fsm_state_up_reset_start
  //
  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_spontaneous,
      &USBLowLevelDriver::_fsm_send_reset_and_restart_timer);
  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_short_tic,
      &USBLowLevelDriver::_fsm_send_reset_and_restart_timer);
  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_long_tic,
      &USBLowLevelDriver::_fsm_reset_failed);

  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_USB_received,
      &USBLowLevelDriver::_fsm_completeReceiveAndRedo);
  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_USB_transmitted,
      &USBLowLevelDriver::_fsm_completeSend);
  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_input_ready,
      &USBLowLevelDriver::_fsm_startSend);

  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_output_generated,
      &USBLowLevelDriver::_fsm_reset_received);

  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_bus_up_report,
      &USBLowLevelDriver::_fsm_idle);
  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_bus_down_report,
      &USBLowLevelDriver::_fsm_reset_failed);

  FSM.SetStateCallback(fsm_state_up_reset_start, fsm_event_too_many_errors,
      &USBLowLevelDriver::_fsm_reclaimBuffers);
  //
  // no bus yet
  //
  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_spontaneous,
      &USBLowLevelDriver::_fsm_send_state_request);
  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_short_tic,
      &USBLowLevelDriver::_fsm_send_state_request);
  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_long_tic,
      &USBLowLevelDriver::_fsm_reset_failed);

  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_bus_up_report,
      &USBLowLevelDriver::_fsm_no_up_bus_up);
  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_bus_down_report,
      &USBLowLevelDriver::_fsm_idle);

  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_USB_received,
      &USBLowLevelDriver::_fsm_completeReceiveAndRedo);
  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_USB_transmitted,
      &USBLowLevelDriver::_fsm_completeSend);

  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_input_ready,
      &USBLowLevelDriver::_fsm_startSend);
  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_output_generated,
      &USBLowLevelDriver::_fsm_idle);
  // don't consume output

  FSM.SetStateCallback(fsm_state_up_no_bus, fsm_event_too_many_errors,
      &USBLowLevelDriver::_fsm_reclaimBuffers);
  //
  // reclaim all buffers, cancel sends/received & drop into down
  //
  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_spontaneous,
      &USBLowLevelDriver::_fsm_spurious_event);
  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_short_tic,
      &USBLowLevelDriver::_fsm_reclaimBuffers_shorttic);
  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_long_tic,
      &USBLowLevelDriver::_fsm_reclaimBuffers_longtic);

  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_bus_up_report,
      &USBLowLevelDriver::_fsm_idle);
  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_bus_down_report,
      &USBLowLevelDriver::_fsm_idle);

  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_USB_received,
      &USBLowLevelDriver::_fsm_completeReceive);
  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_USB_transmitted,
      &USBLowLevelDriver::_fsm_completeSend);

  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_input_ready,
      &USBLowLevelDriver::_fsm_drain_queues);
  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_output_generated,
      &USBLowLevelDriver::_fsm_drain_queues);

  // we are reclaiming, stand by
  FSM.SetStateCallback(fsm_state_reclaim_buffers, fsm_event_too_many_errors,
      &USBLowLevelDriver::_fsm_reclaimBuffers_shorttic);
  //
  //fsm_state_up
  //
  FSM.SetStateCallback(fsm_state_up, fsm_event_short_tic,
      &USBLowLevelDriver::_fsm_up_onesec_tic);
  FSM.SetStateCallback(fsm_state_up, fsm_event_long_tic,
      &USBLowLevelDriver::_fsm_stop_longtick);
  FSM.SetStateCallback(fsm_state_up, fsm_event_USB_received,
      &USBLowLevelDriver::_fsm_completeReceiveAndRedo);
  FSM.SetStateCallback(fsm_state_up, fsm_event_USB_transmitted,
      &USBLowLevelDriver::_fsm_completeSend);

  FSM.SetStateCallback(fsm_state_up, fsm_event_input_ready,
      &USBLowLevelDriver::_fsm_startSend);
  FSM.SetStateCallback(fsm_state_up, fsm_event_output_generated,
      &USBLowLevelDriver::_fsm_idle);
  // output is for clients

  FSM.SetStateCallback(fsm_state_up, fsm_event_bus_up_report,
      &USBLowLevelDriver::_fsm_idle);
  FSM.SetStateCallback(fsm_state_up, fsm_event_bus_down_report,
      &USBLowLevelDriver::_fsm_bus_up_to_down);

  FSM.SetStateCallback(fsm_state_up, fsm_event_too_many_errors,
      &USBLowLevelDriver::_fsm_reclaimBuffers);

  FSM.SetStateCallback(fsm_state_up, fsm_event_spontaneous,
      &USBLowLevelDriver::_fsm_spurious_event);
}

void
USBLowLevelDriver::Run(pth_sem_t * stop1)
{
  stop = pth_event(PTH_EVENT_SEM, stop1);
  InitFSM();

  sendh = 0;
  recvh = 0;
  dev = NULL;
  sendh = 0;
  recvh = 0;
  pktthissec = 0;

  sendc.tr = Thread::Loggers();
  recvc.tr = Thread::Loggers();

  pth_sem_init(&sendc.signal);
  pth_sem_init(&recvc.signal);

  sende = pth_event(PTH_EVENT_SEM, &sendc.signal);
  recve = pth_event(PTH_EVENT_SEM, &recvc.signal);
  shorttic = pth_event(PTH_EVENT_RTIME, pth_time(1, 0));
  longtic = pth_event(PTH_EVENT_RTIME, pth_time(10, 0));
  input = pth_event(PTH_EVENT_SEM, &in_signal);
  output = pth_event(PTH_EVENT_SEM, &out_signal);

  _fsm_set_NewState(fsm_state_init);
  FSM.PushEvent(fsm_event_spontaneous);

  while (pth_event_status(stop) != PTH_STATUS_OCCURRED)
    {

      // only run when ALL are detached nicely
      while (FSM.EventsPending())
        {
          FSM.ConsumeEvent(); // one event
        }

      if (recvh)
        {
          pth_event_concat(stop, recve, NULL);
        }
      if (sendh)
        {
        pth_event_concat(stop, sende, NULL);
        }
      else
        {
          if (!maxpktsoutpersec || pktthissec < maxpktsoutpersec)
            pth_event_concat(stop, input, NULL);
        }

      pth_event_concat(stop, shorttic, longtic, NULL);

      if (FSM.GetTransition(FSM.GetCurrentState(), fsm_event_output_generated)
          != &USBLowLevelDriver::_fsm_idle)
        pth_event_concat(stop, output, NULL);

      pth_wait(stop);

      pth_event_isolate(stop);
      pth_event_isolate(sende);
      pth_event_isolate(recve);
      pth_event_isolate(shorttic);
      pth_event_isolate(longtic);
      pth_event_isolate(input);
      pth_event_isolate(output);

        {
          unsigned int v;
          pth_sem_get_value(&out_signal, &v);

          TRACEPRINTF(Thread::Loggers(), 4, this,
              "state: %d pth_wait recve %d recvh: %p sende %d sendh: %p stop %d inqueue: %d "
              "outqueue: %d outqueue-sem-count: %d onesec: %d tensec: %d input: %d output: %d pktsthissec: %d",
              (int) FSM.GetCurrentState(),
              (int) pth_event_status(recve), (void *) recvh, (int) pth_event_status(sende), (void *) sendh,
              (int) pth_event_status(stop), (int) inqueue.len(), (int) outqueue.len(), (int) v,
              (int) pth_event_status(shorttic), (int) pth_event_status(longtic), (int) pth_event_status(input),
              (int) pth_event_status(output), (int) pktthissec);
        }

      // first push all the receive/send things to make sure the buffers are served & freed

      if (sendh && pth_event_status(sende) == PTH_STATUS_OCCURRED)
        {
          FSM.PushEvent(fsm_event_USB_transmitted);
        }
      if (recvh && pth_event_status(recve) == PTH_STATUS_OCCURRED)
        {
          FSM.PushEvent(fsm_event_USB_received);
        }

      if (errorstowardsreset > ERR2RESETTHRESHOLD)
        {
          errorstowardsreset=0;
          FSM.PushEvent(fsm_event_too_many_errors);
        }
      else
        {
          // none of that will be processed further

          if (pth_event_status(shorttic) == PTH_STATUS_OCCURRED)
            {
              FSM.PushEvent(fsm_event_short_tic);
            }
          if (pth_event_status(longtic) == PTH_STATUS_OCCURRED)
            {
              FSM.PushEvent(fsm_event_long_tic);
            }
          if (pth_event_status(input) == PTH_STATUS_OCCURRED && !sendh
              && !(maxpktsoutpersec && pktthissec > maxpktsoutpersec)) // not sending already, no events if overrun with packets ?
            {
              FSM.PushEvent(fsm_event_input_ready);
            }
          if (pth_event_status(output) == PTH_STATUS_OCCURRED)
            {
              // as long we internally process packets attach the event
              if (FSM.GetTransition(FSM.GetCurrentState(),
                  fsm_event_output_generated) != &USBLowLevelDriver::_fsm_idle)
                FSM.PushEvent(fsm_event_output_generated);
            }

        }
    }
  _fsm_stopSend();
  _fsm_stopReceive();

  pth_event_free(stop, PTH_FREE_THIS);
  pth_event_free(sende, PTH_FREE_THIS);
  pth_event_free(recve, PTH_FREE_THIS);
  pth_event_free(shorttic, PTH_FREE_THIS);
  pth_event_free(longtic, PTH_FREE_THIS);
  pth_event_free(input, PTH_FREE_THIS);
  pth_event_free(output, PTH_FREE_THIS);

}


const char USBLowLevelDriver::indropmsg[] =
    "USBLowLevelDriver: incoming queue length exceeded, dropping packet";
const char USBLowLevelDriver::outdropmsg[] =
    "USBLowLevelDriver: outgoing queue length exceeded, dropping packet";

const char *
USBLowLevelDriver::InterfaceModel() const
{
  int i;
  static char buf[255];
  char cbuf[255] = "", *rbuf;
  buf[0] = '\0';

  if (!Connection_Lost() && dev)
    {
      for (i = 0; i < 3; i++)
        {
          // guard against 128+ which is negative index!
          if (d.stringdes[i] > 0 && d.stringdes[i] < 128)
            {
              libusb_get_string_descriptor_ascii(dev, d.stringdes[i],
                  (unsigned char *) cbuf, sizeof(cbuf));
              // printf("%d %d: %s\n",i, d.stringdes[i], cbuf);
              if (*buf)
                {
                  strncat(buf, " - ", sizeof(buf) - 1);
                }
              strncat(buf, cbuf, sizeof(buf) - 1);
            }
        }

      rbuf = buf;
      while (*rbuf && rbuf - buf < sizeof(buf) - 1)
        {
          if (!isascii(*rbuf))
            *rbuf = '?';
          rbuf++;
        }
    }
  return buf;
}

Element *
USBLowLevelDriver::_xml(Element *parent) const
{

  int i;

  Element *p = parent->addElement(XMLDRIVERELEMENT);
  p->addAttribute(XMLDRIVERELEMENTATTR, _str());
  p->addAttribute(XMLDRIVERSTATUSATTR,
      Connection_Lost() != 0 ? XMLSTATUSDOWN : XMLSTATUSUP);

  if (!Connection_Lost())
    {
      p->addAttribute(XMLDRIVERDEVICEATTR, InterfaceModel());
    }

  ErrCounters::_xml(p);
  inqueue._xml(p);
  outqueue._xml(p);
  return p;
}
