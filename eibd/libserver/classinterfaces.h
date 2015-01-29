/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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

#ifndef __CLASSINTERFACES__
#define __CLASSINTERFACES__

#include <cstdio>

/** interface class that any object desired to be logged as part
 *  of the log message (e.g. protocol stack instance) must inherit
 *  to be able to pass its pointer to the logging message. */
class LoggableObjectInterface
{
 public:
  /** delivers some kind of short string ID of the object under logo.
   * Default is start of the object pointer */
  virtual const char *_str(void) const
  {
    static char b[16];
    snprintf(b, sizeof(b), "[%4p]", this);
    return b;
  }

  LoggableObjectInterface() {};

  /** implicit pointer cast from an object to allow for objects to be
   * logged instead of just their pointers.
   */
  operator const LoggableObjectInterface *() { return this; };


  virtual ~LoggableObjectInterface() { }
};

/** @brief Protocol to support operations on queues that can
    drop on overload and log when dropping. */
class DroppableQueueInterface
{
 private:
  class Logs  *l;
 protected:
  static const char outdropmsg[], indropmsg[];
 public:
  DroppableQueueInterface(class Logs *log2)
    { l = log2; }
  Logs *Loggers() { return l; }

  ~ DroppableQueueInterface() { };

  /** put some kind of loggable object
   *  on queue and increment a semaphore if successful, otherwise log a message and return false */
  template <class QUET, class PDUT>
  bool Put_On_Queue_Or_Drop(Queue < QUET > &queue,
		  PDUT l2,
		  pth_sem_t *sem2inc,
		  bool yield = true,
		  const char *dropmsg = NULL,
		  pth_sem_t *emptysem2reset = NULL,
		  int yieldvalue = 0)
  {
    if (queue.put(l2)<0)
      {
        const char *dm = dropmsg ? dropmsg : "Unspecified driver: queue length exceeded, dropping packet";
        ERRORLOGSHAPE (l, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC,
      		  	  	  l2, Logging::MSGNOHASH,
      		  	  	  dm);
      }
    else
      {
        pth_sem_inc(sem2inc, yield);
        if (emptysem2reset) {
      	  pth_sem_set_value (emptysem2reset, yieldvalue);
        }
        return true;
      }
    return false;
  }

};


/** specialized template, can't log CArray simple byte sequences */
template <>
  bool DroppableQueueInterface::Put_On_Queue_Or_Drop(Queue < CArray > &queue,
						     CArray l2,
						     pth_sem_t  *sem2inc,
						     bool yield,
						     const char *dropmsg,
						     pth_sem_t  *emptysem2reset,
						     int yieldvalue);


/** specialized template, can't log CArray simple byte sequences */
template <>
  bool DroppableQueueInterface::Put_On_Queue_Or_Drop(Queue < CArray *> &queue,
						     CArray *l2,
						     pth_sem_t  *sem2inc,
						     bool yield,
						     const char *dropmsg,
						     pth_sem_t  *emptysem2reset,
						     int yieldvalue);




#endif
