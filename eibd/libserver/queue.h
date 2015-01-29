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

#ifndef QUEUE_H
#define QUEUE_H

#include "config.h"
#include "statistics.h"
#include "timeval.h"
#include "assert.h"
#include "stateinterface.h"

#define QUEUE_UNLIMITED_LEN   (0)

/** implement a generic FIFO queue with thread safety */
template < class T > class Queue : public StateInterface
{
protected:
  /** element in the queue */
  typedef struct _Entry
  {
    /** value */
    T entry;
    /** next element */
    struct _Entry *Next;
#if HAVE_QUEUESTATS
    /** timestamp of insertion */
    TimeVal  timestamp;
#endif
  } Entry;

  /** list head */
  Entry *akt;
  /** pointer where to store the pointer to the next element */
  Entry **head;

  /** elements in the queue */
  int _len;

  /** protection */
  mutable pth_mutex_t lock;

public:
#if HAVE_QUEUESTATS
  /** maximum ever reached queue length */
  IntStatisticsCounter  stat_maxlen;
  /** how many inserts into queue */
  IntStatisticsCounter  stat_inserts;
  /** how many times queue dropped */
  IntStatisticsCounter  stat_drops;

  /** longest delay for an element */
  TimeVal stat_longest_delay;
  /** sum of all delays, divided by inserts-_len-drops (i.e. elements that came & went) gives you mean */
  TimeVal stat_delay_sum;
#endif

protected:
  /** maxlen of the queue before dropping, 0 means ignore */
  int maxlen;
  char _name[32];

public:

  /** initialize queue
   * @param maxlen   maxlen of queue before dropping, 0 for no restriction
   *
   */
  Queue (char *name = "unknown", int maxlen=0)
#if HAVE_QUEUESTATS
    : stat_maxlen(0), stat_inserts(0), stat_drops(0),
    stat_longest_delay(0,0), stat_delay_sum(0,0)
#endif
  {
    pth_mutex_init(&lock);
    strncpy(_name,name,sizeof(_name)-1);
    akt = 0;
    head = &akt;
    _len = 0;
    this->maxlen = maxlen;
  }

void Lock(void) const { pth_mutex_acquire(&lock,TRUE,NULL); };
void Unlock(void) const { pth_mutex_release(&lock); };

  /** copy constructor
   * @param maxlen   maxlen of queue before dropping
  */
    Queue (const Queue < T > &c, int maxlen=0)
#if HAVE_QUEUESTATS
      : stat_maxlen(0), stat_inserts(0), stat_drops(0),
      stat_longest_delay(0,0), stat_delay_sum(0,0)
#endif
  {
      Lock();
    Entry *a = c.akt;
    akt = 0;
    head = &akt;
    _len = 0;
    this->maxlen = maxlen;
    while (a)
      {
	put (a->entry);
	a = a->Next;
      }
    Unlock();
  }

  /** destructor */
  virtual ~ Queue ()
  {
    Lock();
    while (akt)
      get ();
    Unlock();
  }

  /** assignment operator */
  const Queue < T > &operator = (const Queue < T > &c)
  {
    Lock();
    while (akt)
      get ();
    Entry *a = c.akt;
    while (a)
      {
	put (a->entry);
	a = a->Next;
      }
    Unlock();
    return *this;
  }

  /** @brief adds a element to the queue end
   *
   * @param el      element to add
   * @return length of queue, <0 if element has been dropped due to queue overrun
   */
  int put (const T & el, const int _maxlen=0)
  {
    Lock();
    Entry *elem = new Entry;
    int l= _maxlen ? _maxlen : this->maxlen;
    if (l!=0 && _len>l) {
#if HAVE_QUEUESTATS
      ++stat_drops;
#endif
      Unlock();
      return -1;
    }

#if HAVE_QUEUESTATS
    elem->timestamp = TimeVal(pth_timeout(0,0));
    ++stat_inserts;
#endif
    elem->Next = 0;
    elem->entry = el;
    *head = elem;
    head = &elem->Next;
    _len++;
#if HAVE_QUEUESTATS
    stat_maxlen = *stat_maxlen < _len ? _len : *stat_maxlen;
#endif
    Unlock();
    return _len;
  }

  /** remove the element from the queue head and returns it */
  T get ()
  {
    Lock();
    assert (akt != 0);
    Entry *e = akt;

#if HAVE_QUEUESTATS
    TimeVal diff = TimeVal(pth_timeout(0,0)) - e->timestamp;

    // std::cout << diff << "\n";
    // std::cout << e->timestamp << "\n";
    // std::cout << "---\n";

    if (diff > stat_longest_delay) stat_longest_delay = diff;
    stat_delay_sum += diff;
#endif

    T a = akt->entry;
    akt = akt->Next;
    delete e;
    if (!akt)
      head = &akt;
    _len--;
    Unlock();
    return a;
  }

  /** returns the element from the queue head */
  const T & top () const
  {
    Lock();
    assert (akt != 0);
    Unlock();
    return akt->entry;
  }

  /** return true, if the queue is empty */
  int isempty () const
  {
    return akt == 0;
  }

  int len() const
  {
    return _len;
  }

  /** assign queue name by assigning char */
  void operator=(const char *n)
  {
    Lock();
    if (n)
      strncpy(this->_name,n,sizeof(_name));
    Unlock();
  }

  const char *name(void) const
    {
      return _name;
    }

  /** assign maximum queue length by assigning an integer */
  void operator=(const unsigned val)
    {
       Lock();
       this->maxlen = val;
       Unlock();
    }


  Element * _xml(Element *parent) const;

};

#endif
