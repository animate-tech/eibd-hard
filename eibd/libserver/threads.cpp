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

#include "common.h"
#include <string.h>

void *
Thread::ThreadWrapper (void *arg)
{
 // #TRACEPRINTF (Thread::Loggers(), 4, this, "Thread Run And Catch: %s ", this->tname);
 ((Thread *) arg)->RunAndCatch (&((Thread *) arg)->should_stop);
  if (((Thread *) arg)->autodel)
    delete ((Thread *) arg);
  pth_exit (0);
  return 0;
}

Thread::pthset Thread::allgoing;

bool Thread::CheckDataLock( pth_sem_t *lock ) const
{
  unsigned val;
  bool r=pth_sem_get_value(lock,&val);
  r &= val>0;
  return r;
}

bool Thread::WaitForDataLock( pth_sem_t *lock ) const
{
  pth_event_t stop = pth_event(PTH_EVENT_SEM,  &should_stop);
  pth_event_t dlock= pth_event(PTH_EVENT_SEM | PTH_UNTIL_COUNT, lock, 0);

  pth_event_concat(stop, dlock, NULL);
  pth_wait(stop);

  bool r= (pth_event_status(stop) == PTH_STATUS_OCCURRED);
  if (!r) r=pth_sem_inc(lock,TRUE);

  pth_event_isolate(stop);
  pth_event_isolate(dlock);
  pth_event_free(dlock, PTH_FREE_THIS);
  pth_event_free(stop, PTH_FREE_THIS);
  return r;
}

bool Thread::WaitForDataLock( pth_mutex_t *lock ) const
{
  pth_event_t stop = pth_event(PTH_EVENT_SEM,  &should_stop);
  pth_mutex_acquire(lock,true,stop);

  bool r= !(pth_event_status(stop) == PTH_STATUS_OCCURRED);

  pth_event_isolate(stop);
  pth_event_free(stop, PTH_FREE_THIS);
  return r;
}

void Thread::ReleaseDataLock( pth_sem_t *lock ) const
{
  assert(CheckDataLock(lock)); // if not taken, crash
  pth_sem_dec(lock);
}

void Thread::ReleaseDataLock( pth_mutex_t *lock ) const
{
  pth_mutex_release(lock);
}
Thread::Thread (Logs *tr, int Priority, const char *name, Runable * o, THREADENTRY t)
{
  autodel = false;
  obj = o;
  entry = t;
  pth_sem_init (&should_stop);
  prio = Priority;
  tid = 0;
  strncpy(tname, name, sizeof(tname)-1);
  logs=tr;
  TRACEPRINTF (Loggers(), 4, this, "Thread Created: %s ", this->tname);
}

Thread::~Thread ()
{
  Stop ();
  allgoing.erase(tid);
}

void
Thread::Stop ()
{
  if (!tid)
    return;
  TRACEPRINTF (Loggers(), 4, this, "Thread stopping: %s using semaphore x%lx", this->tname, (unsigned long) &should_stop);

  pth_sem_inc (&should_stop, TRUE);
  pth_yield(NULL); // give a chance to grab it even if all other threads blocked on conditions already/CPU

  if (pth_join (tid, 0))
    tid = 0;
}

void
Thread::StopDelete ()
{
  autodel = true;
  pth_sem_inc (&should_stop, FALSE);
  if (!tid)
    return;
  pth_attr_t at = pth_attr_of (tid);
  pth_attr_set (at, PTH_ATTR_JOINABLE, FALSE);
  pth_attr_destroy (at);
}

void
Thread::Start ()
{
  if (tid)
    {
      pth_attr_t a = pth_attr_of (tid);
      int state;
      pth_attr_get (a, PTH_ATTR_STATE, &state);
      pth_attr_destroy (a);
      if (state != PTH_STATE_DEAD)
    	  return;
      Stop ();
    }
  pth_attr_t attr = pth_attr_new ();
  pth_attr_set (attr, PTH_ATTR_PRIO, prio);
  pth_attr_set (attr, PTH_ATTR_NAME, tname);
  TRACEPRINTF (Loggers(), 4, this, "Thread Spawned: %s as %x ", this->tname, (void *) this);
  tid = pth_spawn (attr, &ThreadWrapper, this);
  allgoing.insert(tid);
  pth_attr_destroy (attr);
}

void
Thread::RunAndCatch (pth_sem_t * stop)
{
        TRACEPRINTF (Loggers(), 4, this, "Thread Started RunAndCatch: %s ", this->tname);
	try {
		Run(stop);
	}
	catch (...)
	{
		ERRORLOGSHAPE (Thread::Loggers(), LOG_CRIT, Logging::DUPLICATESMAX1PER10SEC, this, Logging::MSGNOHASH,
				"thread %s threw exception", name());
	}
	TRACEPRINTF (Loggers(), 4, this, "Thread Exited: %s ", name());
}

void
Thread::Run (pth_sem_t * stop)
{
	TRACEPRINTF (Loggers(), 4, this, "Thread Started Running: %s ", this->tname);
	(obj->*entry) (stop);
}

Logs *Thread::Loggers() const
{
	return logs;
}
