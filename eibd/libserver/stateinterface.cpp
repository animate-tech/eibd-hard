#include "eibtypes.h"
#include "eibpriority.h"
#include "common.h"
#include "stateinterface.h"
#include "trace.h"

Element * ErrCounters::_xml(Element *parent) const {
  if (*stat_recverr>0)
    {
      parent->addAttribute(XMLSTATRECVERRATTR, *stat_recverr);
    }
  if (*stat_senderr>0)
    {
      parent->addAttribute(XMLSTATSENDERRATTR, *stat_senderr);
    }
  if (*stat_resets>0)
    {
      parent->addAttribute(XMLSTATRESETSATTR, *stat_resets);
    }
  if (*stat_drops>0)
    {
      parent->addAttribute(XMLSTATDROPSATTR, *stat_drops);
    }
  return parent;
};

ConnectionStateInterface::ConnectionStateInterface(class Logs *tr,ConnectionStateInterface *_proxy,bool _proxyversion)
{
  proxy=_proxy;
  version = unknownVersion;
  proxyversion=_proxyversion;
  pth_mutex_init(&lock);
  pth_sem_init(&conn_lost);
  pth_sem_init(&conn_up);
  ls=tr;
  // to down
  pth_sem_set_value(&conn_lost, 1);
  pth_sem_set_value(&conn_up, 0);
  version++;
}

ConnectionStateInterface::~ConnectionStateInterface()
{
  ls=NULL;
  proxy=NULL;
}

bool
ConnectionStateInterface::Connection_Lost () const
{
  _checkproxy();
  if (proxy) {
    return proxy->Connection_Lost();
  }
  Lock();
  unsigned int v,cv;
  pth_sem_get_value(&conn_lost,&v);
  pth_sem_get_value(&conn_up  ,&cv);
  assert ( cv == ( v==1 ? 0 : 1 ) ); // make sure they're flipped
  Unlock();
  return v!=0;
}

bool ConnectionStateInterface::TransitionToUpState()
{
  _checknoproxy();
  _checkproxy();
  // avoid unnecessary ones
  Lock();
  if (ConnectionStateInterface::Connection_Lost()) {
      pth_sem_set_value(&conn_lost, 0);
      pth_sem_set_value(&conn_up  , 1);
      version++;
  }
  Unlock();
  return true;
}

bool ConnectionStateInterface::TransitionToDownState()
{
  _checknoproxy();
  _checkproxy();
  // someone may overwrite
  Lock();
  if (!ConnectionStateInterface::Connection_Lost()) {
      pth_sem_set_value(&conn_lost, 1);
      pth_sem_set_value(&conn_up  , 0);
      version++;
  }
  Unlock();
  return true;
}

pth_event_t
ConnectionStateInterface::Connection_Wait_Until_Lost()
{
  // that may follow the proxy chain down
  _checkproxy();
  if (proxy)
    return proxy->Connection_Wait_Until_Lost();
  return pth_event(PTH_EVENT_SEM | PTH_UNTIL_COUNT, &conn_lost, 1);
}

pth_event_t
ConnectionStateInterface::Connection_Wait_Until_Up()
{
  // that may follow the proxy chain down
  _checkproxy();
  if (proxy)
    return proxy->Connection_Wait_Until_Up();
  return pth_event(PTH_EVENT_SEM | PTH_UNTIL_COUNT, &conn_up, 1);
}

const unsigned long int ConnectionStateInterface::currentVersion() const
// don't proxy, each layer its own version unless forced
{
  Lock();
  int v=version;
  if (proxyversion) {
      v= proxy->currentVersion();
  }
  Unlock();
  return v;
}

bool  ConnectionStateInterface::bumpVersion()
{
  Lock();
  ConnectionStateInterface::version++;
  Unlock();
  return true;
}
