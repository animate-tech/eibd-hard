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

#include <stdlib.h>
#include "usb.h"

USBLoop::USBLoop( Logs * tr) :
		Thread(tr, PTH_PRIO_STD, "usb-loop")
{
  TRACEPRINTF(Loggers(), 2, this, "USBLoop Runner");
  pth_mutex_init(&contextmutex);
  int r = libusb_init(&_context);
  if (r)
    {
      ERRORLOGSHAPE(Loggers(), LOG_EMERG, Logging::DUPLICATESMAX1PER10SEC, NULL,
          Logging::MSGNOHASH, "USB library init failed, cannot continue");
      throw;
    }
  Start();
  // libusb_set_debug(c,3);
}

USBLoop::~USBLoop()
{
  TRACEPRINTF(Loggers(), 2, this, "USBLoop stopping");
  Stop();
  libusb_context *context = GrabContext();
  if (context)
    libusb_exit(context);
}

void USBLoop::RefreshContext(LoggableObjectInterface *who)
{
  libusb_context *context=GrabContext();
  if (context)
    libusb_exit(context);
  if (libusb_init(&_context))
    {
      ERRORLOGSHAPE(Loggers(), LOG_EMERG, Logging::DUPLICATESMAX1PER10SEC, NULL,
          Logging::MSGNOHASH, "USB library init failed, cannot continue");
      throw;
    }
  TRACEPRINTF(Loggers(), 3, who ? who: this, "context refresh");
  ReleaseContext();
}

libusb_context * USBLoop::GrabContext(LoggableObjectInterface *who)
{
  TRACEPRINTF(Loggers(), 11, who ? who : this, "LIBUSB mutex take");
  pth_mutex_acquire(&contextmutex,false,NULL);
  return _context;
}

void USBLoop::ReleaseContext(LoggableObjectInterface *who)
{
  TRACEPRINTF(Loggers(), 11, who ? who: this, "LIBUSB mutex release");
  pth_mutex_release(&contextmutex);
}

void USBLoop::Run(pth_sem_t * stop1)
{
  fd_set r, w, e;
  int rc, fds, i;
  struct timeval tv, tv1;
  const struct libusb_pollfd **usbfd, **usbfd_orig;
  pth_event_t stop = pth_event(PTH_EVENT_SEM, stop1);
  pth_event_t event = pth_event(PTH_EVENT_SEM, stop1);
  pth_event_t timeout = pth_event(PTH_EVENT_SEM, stop1);
  pth_event_t tic = pth_event(PTH_EVENT_RTIME, pth_time(5, 0));

  tv1.tv_sec = tv1.tv_usec = 0;
  TRACEPRINTF(Loggers(), 2, this, "USB Loop LoopStart");
  while (pth_event_status(stop) != PTH_STATUS_OCCURRED)
    {
      TRACEPRINTF(Loggers(), 11, this, "LoopBegin");
      FD_ZERO(&r);
      FD_ZERO(&w);
      FD_ZERO(&e);
      fds = 0;
      rc = 0;

        {
          libusb_context *context = GrabContext();
          usbfd = libusb_get_pollfds(context);
          usbfd_orig = usbfd;
          if (usbfd)
            while (*usbfd)
              {
                if ((*usbfd)->fd > fds)
                  fds = (*usbfd)->fd;
                if ((*usbfd)->events & POLLIN)
                  FD_SET((*usbfd)->fd, &r);
                if ((*usbfd)->events & POLLOUT)
                  FD_SET((*usbfd)->fd, &w);
                FD_SET((*usbfd)->fd, &e);
                usbfd++;
              }
          free(usbfd_orig);

          i = libusb_get_next_timeout(context, &tv);
          ReleaseContext();
        }

      if (i < 0)
        break;
      if (i > 0)
        {
          pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, timeout,
              pth_time(tv.tv_sec, tv.tv_usec));
          pth_event_concat(stop, timeout, NULL);
        }
      pth_event(PTH_EVENT_SELECT | PTH_MODE_REUSE, event, &rc, fds + 1, &r, &w,
          &e);
      pth_event(PTH_EVENT_RTIME | PTH_MODE_REUSE, tic, pth_time(1, 0));
      // necessary if descriptors come and go, instead of writing the whole notify we poll ever so often
      pth_event_concat(stop, event, tic, NULL);

      TRACEPRINTF(Loggers(), 11, this, "LoopWait");

      pth_wait(stop);
#if 0
      // here someone may have unplugged the thing

      GrabContext();
      if (!libusb_event_handling_ok(context))
        {
          ReleaseContext();
          break;
        }
      ReleaseContext();
#endif

      TRACEPRINTF(Loggers(), 9, this, "LoopProcess");

      pth_event_isolate(stop);
      pth_event_isolate(event);
      pth_event_isolate(timeout);
      pth_event_isolate(tic);

      if ((pth_event_status(event) == PTH_STATUS_OCCURRED
          || pth_event_status(timeout) == PTH_STATUS_OCCURRED)
          && pth_event_status(stop) != PTH_STATUS_OCCURRED)
        {
          libusb_context *context = GrabContext();
          if (libusb_handle_events_timeout(context, &tv1))
            {
              ReleaseContext();
              break;
            }
          ReleaseContext();
        }
      TRACEPRINTF(Loggers(), 11, this, "LoopEnd");
    }
  TRACEPRINTF(Loggers(), 2, this, "LoopStop");

  pth_event_free(tic, PTH_FREE_THIS);
  pth_event_free(timeout, PTH_FREE_THIS);
  pth_event_free(event, PTH_FREE_THIS);
  pth_event_free(stop, PTH_FREE_THIS);
}

USBLoop *loop = 0;

bool
USBInit(Logs * tr)
{
  int r;
  TRACEPRINTF(tr, 4, NULL, "libusb initialized");
  loop = new USBLoop( tr);
  return true;
}

void
USBEnd()
{
  if (loop)
    {
      delete loop;
    }
}
