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

#ifndef STATEINTERF_H
#define STATEINTERF_H

#include "xmlccwrap.h"
#include "trace.h"

class LoggableObjectInterface;

/** every object having this interface must implement the output of its state as
      XML element and track the connection state as well of the underlying connection. */
class StateInterface  {
 public:
  StateInterface() {};
  virtual ~StateInterface() {};
  /** @brief output the XML state of the element and all the elements it includes that are
   *  necessary. It generates the XML of the element state's, returns any element it created under parent
   *  or parent itself otherwise */
  virtual Element * _xml(Element *parent) const { return parent; };
};

class ConnectionStateInterface
{
protected:
  Logs  *ls;
  ConnectionStateInterface *proxy;
  void _checkproxy(void) const  { assert (!(proxy==this)); }
  void _checknoproxy(void) { assert (!proxy); }
public:
  ConnectionStateInterface(class Logs *tr, class ConnectionStateInterface *_proxy,bool _proxyversion=false);
  virtual  ~ConnectionStateInterface();
  /** indicate, if connections works, default is value of semaphore, can be overridden */
   virtual bool Connection_Lost() const;
   /** give me an event to wait on for this object
    * @note: whoever took it MUST free it as well. */
   pth_event_t Connection_Wait_Until_Lost();
   /** give me an event to wait on for this object
       * @note: whoever took it MUST free it as well. */
   pth_event_t Connection_Wait_Until_Up();
   /** some way to reset the interface trying to bring it up again */
   virtual bool SendReset () = 0;
   /** bring it into state */
   bool TransitionToUpState();
   bool TransitionToDownState();
   const ConnectionStateInterface &Proxy() const { return *proxy; }
   bool SetProxy(class ConnectionStateInterface *_proxy,
       bool _proxyversion=false) { proxy=_proxy; proxyversion=_proxyversion; _checkproxy(); return true; };
protected:
  unsigned long version;
public:
  static const unsigned long unknownVersion = -1;
  /** prz: Sept 12
      this gives a unique identifier of this version of the
      interface running that is updated after very reset so upper layers can check
      if necessary.
      Not an ideal design but unfortunately we are piggy-backing onto an architecture
      that never meant to do that */
  const unsigned long int currentVersion() const;
  bool bumpVersion();
private:
  /* if lower connection is being lost this signals up */
  mutable pth_sem_t conn_lost;
  /* if lower connection is being brought this is the signal */
  mutable pth_sem_t conn_up;
  mutable pth_mutex_t lock;
  /* should our version also be determined by our proxies' version, normally no */
  bool proxyversion;
  void Lock(void) const { pth_mutex_acquire(&lock,TRUE,NULL); };
  void Unlock(void) const { pth_mutex_release(&lock); };
};

class ErrCounters: public StateInterface
{
 public:
  UIntStatisticsCounter stat_resets;
  UIntStatisticsCounter stat_senderr;
  UIntStatisticsCounter stat_recverr;
  UIntStatisticsCounter stat_drops;

  ErrCounters(void) :
    stat_resets(0), stat_senderr(0), stat_recverr(0), stat_drops(0) { };

    Element * _xml(Element *parent) const;

};


#endif
