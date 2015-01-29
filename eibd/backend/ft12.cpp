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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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


#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "ft12.h"

const char FT12LowLevelDriver::indropmsg[] = "FT12LowLevelDriver: incoming queue length exceeded, dropping packet";
const char FT12LowLevelDriver::outdropmsg[] = "FT12LowLevelDriver: outgoing queue length exceeded, dropping packet";

Element * FT12LowLevelDriver::_xml(Element *parent) const
{
  Element *p = parent->addElement(XMLDRIVERELEMENT);
  p->addAttribute(XMLDRIVERELEMENTATTR, _str());
  p->addAttribute(XMLDRIVERSTATUSATTR, Connection_Lost() ? XMLSTATUSDOWN : XMLSTATUSUP);
  p->addAttribute(XMLDRIVERDEVICEATTR, devname);
  ErrCounters::_xml(p);
  inqueue._xml(p);
  outqueue._xml(p);
  return p;
}

FT12LowLevelDriver::FT12LowLevelDriver (const char *dev, Logs * tr, int flags,
		int inquemaxlen,
		int outquemaxlen,
		int maxpacketsoutpersecond ) :
  Thread(tr,PTH_PRIO_STD, "ft1.2"),
  LowLevelDriverInterface(tr, flags, inquemaxlen, outquemaxlen, maxpacketsoutpersecond),
  devname(dev)
{
  sendflag = 0;
  recvflag = 0;
  repeatcount = 0;
  TransitionToDownState();
  Start ();
  TRACEPRINTF (Thread::Loggers(), 1, this, "Opened");
}

FT12LowLevelDriver::~FT12LowLevelDriver ()
{
  TRACEPRINTF (Thread::Loggers(), 1, this, "Close & Destroy");
  Stop ();
  TransitionToDownState();
}

bool FT12LowLevelDriver::TransitionToDownState()
{
  char dn[32];
  strncpy(dn,devname.c_str(),sizeof(dn)-1);

  if (state != state_down)
    {
      state = state_down;
      ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR,
                    Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                    "Device %s down", dn);
      if (fd != INVALID_FD)
        {
          tcsetattr(fd, TCSAFLUSH, &old);
          restore_low_latency(fd, &sold);
          close (fd);
        }
      fd = INVALID_FD;
      return LowLevelDriverInterface::TransitionToDownState();
    }
  return true;
}

bool FT12LowLevelDriver::init ()
{

  struct termios t1;

  if (state < state_ifpresent)
    {
      try
        {
          TRACEPRINTF(Thread::Loggers(), 1, this, "Initing Interface");

          fd = open(devname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC);
          if (fd == INVALID_FD)
            {
              ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, this,
                  Logging::MSGNOHASH,
                  "Device %s open for FT1.2 failed (open O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC): %s",
                  devname.c_str(), strerror(errno));
              throw std::exception();
            }
          set_low_latency(fd, &sold);

          close (fd);

          fd = open(devname.c_str(), O_RDWR | O_NOCTTY);
          if (fd == INVALID_FD)
            {
              ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, this,
                  Logging::MSGNOHASH,
                  "Device %s open for FT1.2 failed (open O_RDWR | O_NOCTTY)",
                  devname.c_str());
              throw std::exception();
            }

          if (tcgetattr(fd, &old))
            {
              restore_low_latency(fd, &sold);
              close(fd);
              fd = INVALID_FD;
              ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, this,
                  Logging::MSGNOHASH,
                  "Device open for FT1.2 failed (tcgetattr)");
              throw std::exception();
            }

          if (tcgetattr(fd, &t1))
            {
              restore_low_latency(fd, &sold);
              close(fd);
              fd = INVALID_FD;
              ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, this,
                  Logging::MSGNOHASH,
                  "Device open for FT1.2 failed (tcgetattr)");
              throw std::exception();
            }
          t1.c_cflag = CS8 | PARENB | CLOCAL | CREAD;
          t1.c_iflag = IGNBRK | INPCK | ISIG;
          t1.c_oflag = 0;
          t1.c_lflag = 0;
          t1.c_cc[VTIME] = 1;
          t1.c_cc[VMIN] = 0;
          cfsetospeed(&t1, B19200);
          cfsetispeed(&t1, 0);

          if (tcsetattr(fd, TCSAFLUSH, &t1))
            {
              restore_low_latency(fd, &sold);
              close(fd);
              fd = INVALID_FD;
              ERRORLOGSHAPE(Thread::Loggers(), LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, this,
                  Logging::MSGNOHASH,
                  "Device open for FT1.2 failed (tcsetattr)");
              throw std::exception();
            }
        }
      catch (std::exception &e)
        {
          return false;
        }
    }
  state = state_ifpresent;
  return true;
}

bool
FT12LowLevelDriver::Send_Packet (CArray l, bool always)
{
  CArray pdu;
  uchar c;
  unsigned i;
  Thread::Loggers()->TracePacket (1, this, "Send", l);

  if (Connection_Lost() && !always) {
    return false;
  }
  assert (l () <= 32);
  pdu.resize (l () + 7);
  pdu[0] = 0x68;
  pdu[1] = l () + 1;
  pdu[2] = l () + 1;
  pdu[3] = 0x68;
  if (sendflag)
    pdu[4] = 0x53;
  else
    pdu[4] = 0x73;
  sendflag = !sendflag;

  pdu.setpart (l.array (), 5, l ());
  c = pdu[4];
  for (i = 0; i < l (); i++)
    c += l[i];
  pdu[pdu () - 2] = c;
  pdu[pdu () - 1] = 0x16;

  return Put_On_Queue_Or_Drop<CArray, CArray>(inqueue, pdu, &in_signal, true, indropmsg, &send_empty);

#if 0
  pth_sem_set_value (&send_empty, 0);
  inqueue.put (pdu);
  pth_sem_inc (&in_signal, TRUE);
  return true;
#endif
}

bool
FT12LowLevelDriver::SendReset ()
{

  CArray pdu;
  TRACEPRINTF (Thread::Loggers(), 1, this, "Send Interface Reset");
  pdu.resize (4);
  pdu[0] = FIXLENFRAME;
  pdu[1] = 0x40;
  pdu[2] = 0x40;
  pdu[3] = 0x16;
  sendflag = 0;
  recvflag = 0;

  LowLevelDriverInterface::SendReset();

    state=state_send_reset;

  return Put_On_Queue_Or_Drop<CArray, CArray>(inqueue, pdu, &in_signal, true, indropmsg, &send_empty);

#if 0
  pth_sem_set_value (&send_empty, 0);
  inqueue.put (pdu);
  pth_sem_inc (&in_signal, TRUE);
#endif

}

bool FT12LowLevelDriver::TransitionToUpState()
{
  state=state_up_ready_to_send;
  repeatcount = 0;
  char dn[32];
  strncpy(dn,devname.c_str(),sizeof(dn)-1);

  INFOLOGSHAPE(Thread::Loggers(), LOG_INFO,
                Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
                "Device %s operational and up", dn);
  return LowLevelDriverInterface::TransitionToUpState();
}

void
FT12LowLevelDriver::Run (pth_sem_t * stop1)
{
  CArray last;
  int i;
  uchar buf[255];

  pth_event_t stop = pth_event(PTH_EVENT_SEM, stop1);
  pth_event_t timeout = pth_event(PTH_EVENT_RTIME, pth_time(3, 0));  // retransmit or bring interface up again
  pth_event_t onesectic = pth_event(PTH_EVENT_TIME, pth_time(1, 0));
  int pktthissec = 0;

  while (pth_event_status(stop) != PTH_STATUS_OCCURRED)
    {
      pth_event_isolate(timeout);

      if (state>state_down && repeatcount >= 5)
        {
          TransitionToDownState();
          repeatcount = 0;
          ERRORLOGSHAPE(Thread::Loggers(), LOG_ALERT,
              Logging::DUPLICATESMAX1PERMIN, this, Logging::MSGNOHASH,
              "FT1.2 Device too many errors in transmission: resetting");
          timeout = pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout,
                                      pth_time(3, 0)); // give it 3 secs
        }

      pth_event_isolate(onesectic);
      if (pth_event_status(onesectic) == PTH_STATUS_OCCURRED)
        {
          pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, onesectic,
              pth_time(1, 0));
          pktthissec = 0;
        }

        {
          pth_event_t input = pth_event(PTH_EVENT_SEM, &in_signal);

          pth_event_isolate(input);

          if (state == state_up_ready_to_send && (!maxpktsoutpersec || pktthissec < maxpktsoutpersec))
            pth_event_concat(stop, input, NULL);
          else
            pth_event_concat(stop, timeout, NULL);

          pth_event_concat(stop, onesectic, NULL);

          if (fd != INVALID_FD) // that could starve the rest of the gang actually, needs looking @ if people run lots Ft1.2
            {
              i = pth_read_ev(fd, buf, sizeof(buf), stop);
              if (i > 0)
                {
                  Thread::Loggers()->TracePacket(0, this, "Recv", i, buf);
                  akt.setpart(buf, akt(), i);
                }
            }
          else
            {
              pth_wait(stop);
            }

          pth_event_isolate(timeout);
          pth_event_isolate(input);
          pth_event_isolate(onesectic);
      }

      try
        {
          while (akt.len() > 0)
            {
              switch (akt[0])
                {
              case ACKFRAME:
                {
                  if (state < state_up_ready_to_send)
                    {
                      // first ack is for reset set
                      DrainIn();
                      TransitionToUpState();
                    }
                  else if (state == waiting_for_ack)
                    {
                      // ack, get first from the queue
                      inqueue.Lock();
                      if (!inqueue.isempty())
                        {
                          pth_sem_dec(&in_signal);
                          inqueue.get();
                          if (inqueue.isempty())
                            {
                              pth_sem_set_value(&send_empty, 1);
                            }
                          pktthissec++;
                          repeatcount = 0;
                        }
                      inqueue.Unlock();
                      state = state_up_ready_to_send; // transition
                    }
                  akt.deletepart(0, 1);
                }
                break;
              case FIXLENFRAME:
                {
                  if (akt() < 4)
                    {
                      throw std::exception();
                    }
                  if (akt[1] == akt[2] && akt[3] == 0x16)
                    {
                      // valid frame
                      uchar c1 = ACKFRAME;
                      Thread::Loggers()->TracePacket(0, this, "Send Ack", 1,
                          &c1);
                      write(fd, &c1, 1);

                      if ((akt[1] == 0xF3 && !recvflag)
                          || (akt[1] == 0xD3 && recvflag))
                        {
                          // SEND_UDAT
                          //right sequence number
                          recvflag = !recvflag;
                        }
                      if ((akt[1] & 0x0f) == 0) // a reset request
                        {
                          const uchar reset[1] =
                            { 0xA0 }; // LM_Reset.ind_message
                          CArray *c = new CArray(reset, sizeof(reset));
                          Thread::Loggers()->TracePacket(0, this, "RecvReset",
                              *c);

                          Put_On_Queue_Or_Drop<CArray *, CArray *>(outqueue, c,
                              &out_signal, true, outdropmsg);

#if 0
                          outqueue.put (c);
                          pth_sem_inc (&out_signal, TRUE);
#endif
                        }
                      // ignore 0x09 which is a status link request
                    }
                  akt.deletepart(0, 4);
                }
                break;
              case VARLENFRAME:
                {
                  int len;
                  uchar c1;
                  if (akt() < 7)
                    {
                      throw std::exception();
                    }
                  if (akt[1] != akt[2] || akt[3] != 0x68)
                    {
                      ++stat_recverr;
                      //receive error, try to resume
                      akt.deletepart(0, 1);
                      continue;
                    }
                  if (akt() < akt[1] + 6)
                    {
                      throw std::exception();
                    }

                  c1 = 0;
                  for (i = 4; i < akt[1] + 4; i++)
                    c1 += akt[i];
                  if (akt[akt[1] + 4] != c1 || akt[akt[1] + 5] != 0x16)
                    {
                      ++stat_recverr;
                      len = akt[1] + 6;
                      //Forget wrong short frame
                      akt.deletepart(0, len);
                      continue;
                    }

                  c1 = ACKFRAME;
                  Thread::Loggers()->TracePacket(0, this, "Send Ack", 1, &c1);
                  i = write(fd, &c1, 1);

                  if ((akt[4] == 0xF3 && recvflag)
                      || (akt[4] == 0xD3 && !recvflag))
                    {
                      if (CArray(akt.array() + 5, akt[1] - 1) != last)
                        {
                          WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                              Logging::DUPLICATESMAX1PER10SEC, this,
                              Logging::MSGNOHASH, "Sequence jump for FT1.2");
                          recvflag = !recvflag;
                        }
                      else
                        {
                          ++stat_recverr;
                          WARNLOGSHAPE(Thread::Loggers(), LOG_NOTICE,
                              Logging::DUPLICATESMAX1PER10SEC, this,
                              Logging::MSGNOHASH, "Wrong sequence for FT1.2");
                        }
                    }

                  if ((akt[4] == 0xF3 && !recvflag)
                      || (akt[4] == 0xD3 && recvflag))
                    {
                      recvflag = !recvflag;
                      CArray *c = new CArray;
                      len = akt[1] + 6;
                      c->setpart(akt.array() + 5, 0, len - 7);
                      last = *c;

                      Put_On_Queue_Or_Drop<CArray *, CArray *>(outqueue, c,
                          &out_signal, true, outdropmsg);

#if 0
                      outqueue.put (c);
                      pth_sem_inc (&out_signal, TRUE);
#endif
                    }
                  akt.deletepart(0, len);
                }
                break;
              default:
                {
                  ++stat_recverr;
                  //Forget unknown byte
                  akt.deletepart(0, 1);
                }
                break;
                }
            }
      }
      catch (std::exception &e)
      {
          // incomplete frame
      }

#if 0
      TRACEPRINTF (Thread::Loggers(), 1, this, "before send fd:%d empty:%d state:%d p/s: %d timeout: %d maxpktsoutpersec: %d",
          (int) fd, inqueue.isempty(),
          (int) state,
          pktthissec,
          pth_event_status(timeout) == PTH_STATUS_OCCURRED,
          (int) maxpktsoutpersec);
#endif

      if (!inqueue.isempty() &&
          (state==state_send_reset ||
              state==state_up_ready_to_send
              || (state == waiting_for_ack && pth_event_status(timeout) == PTH_STATUS_OCCURRED))
              && (!maxpktsoutpersec || pktthissec < maxpktsoutpersec))
        {
          const CArray & c = inqueue.top();
          if (inqueue.isempty()) {
              pth_sem_set_value(&send_empty, 1);
          }
          Thread::Loggers()->TracePacket(0, this, "Send", c);
          i = pth_write_ev(fd, c.array(), c(), stop);
          if (i <= 0)
            {
              pth_sleep(0.5); // give 0.5 sec to stabilize
            }
          else
            {
              if (state==state_up_ready_to_send) {
                state = waiting_for_ack;
              } // otherwise we remain in whatever it is, some down state before operational
              timeout = pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout,
                                          pth_time(0,50000)); // ca. 500 bits @ 9600 ?  = 5% = 50 msec
            }
          repeatcount++; // all count for packets/sec on ft1.2 to be secure

        }

      // try periodically to send a reset out if we're down, last thing since it invalidates some events
      if (state<state_up_ready_to_send && pth_event_status(timeout) == PTH_STATUS_OCCURRED)
        {
          if (init()) { // trying to bring it up
              SendReset();
          }
          timeout = pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout,
                            pth_time(3, 0)); // as next restart
        }
    }
  pth_event_free(stop, PTH_FREE_THIS);
  pth_event_free(timeout, PTH_FREE_THIS);
  pth_event_free(onesectic, PTH_FREE_THIS);
}

EMIVer FT12LowLevelDriver::getEMIVer ()
{
  return vEMI2;
}
