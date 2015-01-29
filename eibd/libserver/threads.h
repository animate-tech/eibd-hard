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

#ifndef THREADS_H
#define THREADS_H

extern "C" {
#include <pthsem.h>
};

#include <set>

/** interface for a class started by a thread */
class Runable
{
public:
//  typedef void (* THREADENTRY ) (pth_sem_t * stopcond);

};

/** pointer to an thread entry point in Runable
 * the thread should exit, if stopcond can be deceremented
 */
 typedef void (Runable::*THREADENTRY) (pth_sem_t * stopcond);

/** implements a Thread */
class Thread : virtual protected LoggableObjectInterface
{
public:
   typedef std::set<pth_t,std::less<pth_t> > pthset;
   static pthset &get_running_threads(void) {
     return allgoing;
   }
private:
   static pthset allgoing;

private:
  /** delete at stop */
  bool autodel;
  /** C entry point for the threads */
  static void *ThreadWrapper (void *arg);
  /** thread id */
  pth_t tid;
  /** object to run */
  Runable *obj;
  /** entry point */
  THREADENTRY entry;
  /** stop condition */
  pth_sem_t should_stop;
  /** priority */
  int prio;
  /** name */
  char tname[32];
  /** logs we're writing to */
  Logs *logs;
private:
  void RunAndCatch (pth_sem_t * stop);
protected:
  /** main function of the thread
   * @param stop if stop can be decemented, the routine should exit
   */
    virtual void Run (pth_sem_t * stop);
public:
    /** create a thread
     * if o and t are not present, Run is runned, which has to be replaced
     * @param o Object to run
     * @param t Entry point
     */
    Thread (Logs *tr, int Priority = PTH_PRIO_STD, const char *name = "unknown", Runable * o = 0, THREADENTRY t = 0);
    virtual ~ Thread ();

    /** starts the thread*/
  void Start ();
  /** stops the thread, if it is running */
  void Stop ();
  /** stops the thread and delete it asynchronous */
  void StopDelete ();
  const char *name() const { return tname; }
  /** return the current thread logger */
  Logs *Loggers() const;

  /** get a data lock or if returns False, means abandon everything, thread is stopping */
  bool CheckDataLock( pth_sem_t *lock ) const ;
  bool WaitForDataLock( pth_sem_t *lock ) const ;
  bool WaitForDataLock( pth_mutex_t *lock ) const;
  void ReleaseDataLock( pth_sem_t *lock ) const ;
  void ReleaseDataLock( pth_mutex_t *lock ) const;

};


#endif
