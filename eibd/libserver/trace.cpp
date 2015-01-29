/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>
    Copyright (C) 2007- logging rewritten by Z2 GmbH, T. Przygienda <prz@net4u.ch>

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
#include "eibclient.h"
#include "classinterfaces.h"
#include <sys/time.h>

#define TDEBUG 0

// cheapmap moved here to prevent too much compiling & code linking if
// trace is not really used

template < class K > int compar(const void * k1, const void *k2) throw ()
{
  const K *kk1 = (const K *) k1;
  const K *kk2 = (const K *) k2;
#if TDEBUG
  std::cout << "cmp " << *kk1 << " " << *kk2 << "\n"  ;
#endif
  return ( *kk1 < *kk2 ? 1 : (*kk1 > *kk2 ? -1 : 0) );
}

template < class K, class C >
CheapMap<K,C>::CheapMap()
{
  tree=NULL;
  _size = 0;
}

template < typename T >
void delnodes(void *nodep)  throw ()
{
#if TDEBUG
  std::cout << "deleting " << nodep << "\n" ;
#endif
  delete ((T *) nodep);
}

template < typename T > void printnodes(const void *nodep,
                                        const VISIT which,
                                        const int depth)  throw ()
{
  T *n = (T *) nodep;
  if (n && which == postorder)
  {
    std::cout << *n;
  }
}

template < class K, class C >
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& o,
                                      const struct _node<K,C> &n)
{
  o << n.key ;
  o << "/" ;
  o << n.content << "\n";
  return o;
}

template < class K, class C >
CheapMap<K,C>::~CheapMap()
{
  tdestroy (tree, delnodes < struct _node<K,C> >);
}

template < class K, class C >
void CheapMap<K,C>::Dump(void)
{
  twalk (tree, printnodes < struct _node<K,C> >);
  std::cout.flush();
}

template < class K, class C >
C & CheapMap<K,C>::operator [] (const K &k)
{
  void   *f = tfind ( &k, &tree, compar<K> );
  node_t *n = (node_t *) f;

  if (f == NULL)
  {
    n = new node_t;
    f = n;
    if (!f)
    {
      throw Exception(MEMORY_ALLOC);
    }
#if TDEBUG
    std::cout << "alloc " << f << "\n" ;
#endif
    n->key = k;
    if (tsearch ( (void *) n, &tree, compar<K> ) == NULL)
    {
      throw Exception(MEMORY_ALLOC);
    }
    _size++;
  }
  n = (node_t *) f;
  return n->content;
}

template < class K, class C >
C *CheapMap<K,C>::find(const K &k)
{
#if TDEBUG
  std::cout << " looking for " << k << " in " << tree << "\n";
#endif
  void   *f = tfind ( &k, &tree, compar<K> );
  node_t *n = (node_t *) f;
  return n != NULL ? &n->content : NULL;
}

template < class K, class C >
CheapMap<K,C> & CheapMap<K,C>::operator -= (const K &k)
{
  node_t n = { k };
  void   *f = tfind ( &k, &tree, compar<K> );
  if (!f)
    return *this;
  _size--;
  tdelete( &n, &tree, compar);
  return *this;
}

bool Logs::syslogOpen = false;

// Logs class, all different logging facilities

const struct Logging::_pp Logging::policyPars[Logging::POLICYMAX] =
{
  {
    false, 0, 0
  },
  { true,  0, 0 },
  { true,  1, 1 },
  { true,  1, 10 },
  { true,  63, 1 },
  { true,  31, 1 },
  { true,  31, 10 },
};



bool Logs::singleton=true;

Logs::Logs(Logging::logMethod errdest,
           Logging::logMethod warndest,
           Logging::logMethod infodest,
           Logging::logMethod tracedest,
           sysLogParameters *syslogpars) : err(errdest), warn(warndest), info(infodest), trace(tracedest)
{
  assert(singleton); // log to one single instance of things only!
  singleton=false;
  if (syslogpars)
  {
    syslogPars = *syslogpars;
  }
  else
  {
    // come up with reasonable defaults just in case
    strncpy(syslogPars.ident,"eibd",sizeof(syslogPars.ident)-1);
    syslogPars.options = 0;
    syslogPars.facility = LOG_LOCAL3;
  }
  if (!syslogOpen &&
      ( (errdest|warndest|infodest|tracedest) & Logging::SYSLOG))
  {
    openlog(syslogPars.ident, syslogPars.options, syslogPars.facility);
  }
}

Logs::~Logs()
{
  if (syslogOpen)
    closelog();
  fflush(stdout);
  fflush(stderr);
}
// TraceLog class

void
Logs::TracePacketUncond (const unsigned int layer,
                         const LoggableObjectInterface *inst,
                         const char *msg, int Len,
                         const uchar * data)
{
  int i;
  char dbuf[256] = "\0";
  static const int linelen = 16;

  trace.shapedLog(layer, inst, Logging::NOPOLICY, 0, msg, Len);

  for (i = 0; i < Len; i++)
    {
      sprintf(&dbuf[3 * (i % linelen)], "%02X ", data[i]);
      if ((i && (i % linelen) == linelen - 1) || i == Len - 1)
        {
          trace.shapedLog(layer, inst, Logging::NOPOLICY, 0, "%s", dbuf);
          dbuf[0] = '\0';
        }
    }
}

// Logging class implementation

Logging::Logging(logMethod m)
{
  logto = m;
  levels = 0xff;
  lastLogged.lastLogged = MSGNOHASH;
  lastLogged.lastLoggedTimes = 0;
  lastLogged.timeStamp = 0;
  lastLogged.lastLoggedLevel = LOG_LEVEL_0;
  pth_mutex_init(&shapeFilterMapMutex);
  pth_mutex_init(&linebufmutex);
}

void Logging::flush(void)
{
  time_t        t;
   struct tm    *tmp;
   shapeFilterMap::iterator i;

   t = time(NULL);
   tmp = localtime(&t);

   dumpRepeats(tmp,0);

   // empty the map
   for ( shapeFilterMap::const_iterator ii=filterMap.begin ();
       ii != filterMap.end(); ii++ )
     {
       dumpSupressions(tmp , 0, LOG_NOTICE , "", ii->second, 0);
     }
}

Logging::~Logging()
{
  time_t        t;
  struct tm    *tmp;
  shapeFilterMap::iterator i;

  t = time(NULL);
  tmp = localtime(&t);

  dumpRepeats(tmp,0);

  // empty the map
  while ( filterMap.begin () != filterMap.end() )
  {
    i = filterMap.begin();
#if TDEBUG
    std::cout << i->first << "/" << *i->second << "\n";
#endif
    dumpSupressions(tmp , 0, LOG_NOTICE , "", i->second, 0);
    delete i->second;
    filterMap.erase(i->first);
  }
}

const char *Logging::logLevel2Str(const int level) const
{
  char *r = "";
  // default behavior is to get the syslog level & then its string
  int l=level;
  switch (l)
  {
  case LOG_EMERG:
    r = EIBD_LOG_EMERG;
    break;
  case LOG_ALERT:
    r = EIBD_LOG_ALERT;
    break;
  case LOG_CRIT:
    r = EIBD_LOG_CRIT;
    break;
  case LOG_ERR:
    r = EIBD_LOG_ERR;
    break;
  case LOG_WARNING:
    r = EIBD_LOG_WARN;
    break;
  case LOG_NOTICE:
    r = EIBD_LOG_NOTICE;
    break;
  case LOG_INFO:
    r = EIBD_LOG_INFO;
    break;
  case LOG_DEBUG:
    r = EIBD_LOG_DEBUG;
    break;
  default:
    r = "???";
    break;
  }
  return r;
}

Logging::returnStatus Logging::vstring2dev(const struct tm *tm,
    const suseconds_t subs,
    const LoggableObjectInterface *obj,
    const unsigned int level,
    const char *msg,
    va_list ap)
{
  pth_mutex_acquire(&linebufmutex,0,NULL);
  va_list aq;
  va_copy(aq, ap);

  linebuf[0]='\0';

  if (logto & (STDOUT|STDERR))
    {
      strftime(linebuf,sizeof(linebuf),"%3b %2d %H:%M:%S", tm);
      snprintf(&linebuf[strlen(linebuf)],sizeof(linebuf)-strlen(linebuf), ".%03d", (int) subs/1000);

      if (logto & STDOUT)
	{
	  fprintf(stdout,"%8s %8s %8s ", linebuf, logLevel2Str(level), (obj ? obj->_str() : ""));
	  vfprintf(stdout, msg, ap);
	  fprintf(stdout,"\n");
	}
      /* don't write to both! ap will be invalid by then ! */
      else
	{
	  if (logto & STDERR)
	    {
	      fprintf(stderr,"%8s %8s %8s ", linebuf, logLevel2Str(level), (obj ? obj->_str() : ""));
	      vfprintf(stderr, msg, ap);
	      fprintf(stderr,"\n");
	    }
	}
    }

  if (logto & SYSLOG)
    {
      snprintf(linebuf,sizeof(linebuf)-1,"%5s: %s", logLevel2Str(level), msg);
      vsyslog(logLevel2SyslogLevel(level), linebuf, aq);
    }

  va_end(aq);
  pth_mutex_release(&linebufmutex);
  return OK;
}

Logging::returnStatus Logging::string2dev(const struct tm *tm,
    const suseconds_t subs,
    const LoggableObjectInterface *obj,
    const unsigned int level,
    const char *msg,
    ...)
{
  pth_mutex_acquire(&linebufmutex,0,NULL);
  va_list       ap;
  returnStatus  r;
  va_start (ap, msg);
  r = vstring2dev(tm, subs, obj, level, msg, ap);
  va_end(ap);
  pth_mutex_release(&linebufmutex);
  return r;
}

unsigned int Logging::cheap2nmodulo(unsigned int rn, unsigned int m)
{
  return rn & (~m);
}

Logging::returnStatus Logging::string2devShape(const struct tm *tm,
    const suseconds_t subs,
    const LoggableObjectInterface *obj,
    const unsigned int    level,
    const struct _pp    &policy,
    logMessageHash       msgHash,
    const char          *msg,
    va_list              ap)
{
  bool writeOut = true;
  returnStatus r = OK;

  struct timeval t;

  // if level suppressed, we're excused
  if (!shouldBePrinted(level))
  {
    return OK;
  }

  if (msgHash == MSGNOHASH &&
      (policy.duplicates || policy.msgpertick))
  {
    return WRONGARGS;
  }

  if (msgHash && policy.msgpertick)
  {
    shapeFilterMap::const_iterator f;
    _ShapeFilter *nf;

    gettimeofday (&t, 0);

    pth_mutex_acquire(&shapeFilterMapMutex, 0, NULL);
    f = filterMap.find(msgHash);

    if (f == filterMap.end() )
    {
      // insert new entry
      _ShapeFilter *nf = new _ShapeFilter;
#if TDEBUG
      std::cout << "alloced " << nf << "\n" ;
#endif
      if ( !nf )
      {
        pth_mutex_release(&shapeFilterMapMutex);
        return MEMORYALLOC;
      }
      nf->msgsSinceTimestamp = 0;
      nf->msgsSupressedSinceTimestamp = 0;
      nf->timeStamp = cheap2nmodulo(t.tv_sec,policy.timerressec);

      filterMap[msgHash] = nf;

      f = filterMap.find(msgHash);
    }

    nf = f->second;
    unsigned int newtick = cheap2nmodulo(t.tv_sec,policy.timerressec);

#if TDEBUG
    std::cout << "TI " << newtick << " ST " << nf->timeStamp <<
    " P.M " << policy.msgpertick << " SP " << nf->msgsSinceTimestamp << "\n";
#endif
    if (nf->timeStamp == newtick)
    {
      // shaping in the same timer tick
      if (nf->msgsSinceTimestamp + nf->msgsSupressedSinceTimestamp + 1 > policy.msgpertick)
      {
        nf->msgsSupressedSinceTimestamp ++;
        writeOut = false;
      }
      else
      {
        nf->msgsSinceTimestamp ++;
        // and write out
      }
    }
    // next second
    else
    {
      dumpSupressions(tm, subs, level, msg, nf, newtick);
    }

    pth_mutex_release(&shapeFilterMapMutex);
  }

  /** duplicates runs after shaping policy since shaping may suppress the
   *  message before it's even considered for duplicates */
  if (writeOut && msgHash && policy.duplicates &&
      (lastLogged.lastLogged == msgHash ||
       lastLogged.lastLogged == MSGNOHASH))
  {

    gettimeofday (&t, 0);

    if (lastLogged.timeStamp == cheap2nmodulo(t.tv_sec,7))
    {
      if (lastLogged.lastLoggedTimes)
      {
        writeOut = false;
      }
      lastLogged.lastLoggedTimes ++;
    }
    else
    {
      dumpRepeats(tm,subs);
      lastLogged.lastLogged=msgHash;
      lastLogged.lastLoggedLevel = level;
      lastLogged.lastLoggedTimes = 0;
      lastLogged.timeStamp = cheap2nmodulo(t.tv_sec,7);
    }
  }
  else
  {
    dumpRepeats(tm, subs);
  }

  if (writeOut)
  {
    r= vstring2dev(tm, subs, obj, level, msg, ap);
  }
  return r;
}

Logging::logMessageHash Logging::messageHash(const unsigned int level,
    const char *msg) const
{
  logMessageHash h= level;
  char *c = (char *) msg;
  int   i = 0;
  while (c && *c)
  {
    h ^= ( *c << 8*i );
    c++;
    i++;
    i &= sizeof(h)-1;
  }
#if TDEBUG
  std::cout << msg << "/" << h << "\n" ;
#endif
  return h;
}

Logging::returnStatus Logging::shapedLog(const unsigned int level,
    const LoggableObjectInterface *obj,
    shapingPolicies  policy,
    logMessageHash   msgHash,
    const char      *msg, ...)
{
  va_list       ap;
  time_t        t;
  struct tm    *tmp;
  returnStatus  r;
  const struct _pp  *p;
  struct timeval  tv;

  // if level suppressed, we're excused
  if (!shouldBePrinted(level))
  {
    return OK;
  }

  if (policy > POLICYMAX - 1)
  {
    return NOSUCHPOLICY;
  }

  p = &policyPars[policy];

  t = time(NULL);
  tmp = localtime(&t);
  gettimeofday(&tv,NULL);

  if (tmp == NULL)
  {
    return GETTIMEFAIL;
  }

  if ((p->duplicates || p->msgpertick) &&
      msgHash == MSGNOHASH)
  {
    // compute hash here
    msgHash = messageHash(level,msg);
  }

  //assert(msg[0]);

  va_start (ap, msg);
  r = string2devShape(tmp, tv.tv_usec, obj, level, *p, msgHash, msg, ap);
  va_end(ap);
  return r;
}

void Logging::dumpRepeats(const struct tm *tm,
    const suseconds_t subs)
{
  if (lastLogged.lastLoggedTimes>1)
  {
    string2dev(tm, subs, NULL, lastLogged.lastLoggedLevel,
               "last message repeated %d times", lastLogged.lastLoggedTimes -1);
  }
  lastLogged.lastLogged = MSGNOHASH;
  lastLogged.lastLoggedTimes = 0;
  lastLogged.timeStamp = 0;
}

void Logging::dumpSupressions(const struct tm *tm,
                              const suseconds_t subs,
                              const unsigned int level,
                              const char *msg,
                              class _ShapeFilter *nf,
                              const unsigned int newtick)
{
  if (nf->msgsSupressedSinceTimestamp && msg[0])
  {
    std::string subst(msg);
    while (subst.find('%')!=string::npos)
      {
        subst.replace(subst.find("%"),1,"arg:");
      }

    string2dev(tm, subs, NULL, level,
               "message '%s' suppressed %d times due to frequency ...",
               subst.c_str(), nf->msgsSupressedSinceTimestamp);
  }

  nf->timeStamp = newtick;
  nf->msgsSinceTimestamp = 0;
  nf->msgsSupressedSinceTimestamp = 0;
}

int Logging::logLevel2SyslogLevel(const unsigned int level) const
{
  return level;
}

std::basic_ostream<char>& operator <<(std::basic_ostream<char>& o,
                                      const class _ShapeFilter & f)
{
  o << "@" << f.timeStamp;
  o << " R " << f.msgsSinceTimestamp;
  o << " S " << f.msgsSupressedSinceTimestamp;
  return o;
}

int Logging::levelBitMask2BitPos(const levelMaskValues lb) const
{
  int r=lb;
  for (int i = 1; !(r&0x1) && i<16; r<<=1, i++);
  return r;
}

// error logging implemenation
int ErrorLog::logLevel2SyslogLevel(const unsigned int level) const
{
  int l = (level);
  switch (l)
  {
  case (LOG_EMERG):
  case (LOG_ALERT):
  case (LOG_CRIT):
  case (LOG_ERR):
	  break;
  // at least error in syslog
  default:
	  l= LOG_ERR;
	  break;
  }
  return l;
}

// warning logging implementation
int WarnLog::logLevel2SyslogLevel(const unsigned int level) const
{
  int l = (level);
  switch (l)
  {

  case (LOG_EMERG):
  case (LOG_ALERT):
  case (LOG_CRIT):
  case (LOG_ERR):
	  l = LOG_WARNING;
	  break;
  case (LOG_DEBUG):
  case (LOG_INFO):
	  l = LOG_NOTICE;
  break;

  default:
	  break;
  }
  return l;
}

// warning logging implementation
int InfoLog::logLevel2SyslogLevel(const unsigned int level) const
{
  int l = (level);
  switch (l)
  {
  case (LOG_EMERG):
  case (LOG_ALERT):
  case (LOG_CRIT):
  case (LOG_ERR):
  case (LOG_WARNING):
  case (LOG_NOTICE):
	  l = LOG_INFO;
    break;

  default:
    break;
  }
  return l;
}

// tracing implementation
int TraceLog::logLevel2SyslogLevel(const unsigned int level) const
{
  return LOG_DEBUG;
}

const char* TraceLog::logLevel2Str(const int level) const
{
  // special case, we return tracing level
  // we cannot print into a buffer, otherwise the thing is not thread-safe
  static const int size=12;
  const char *l[size] = {  "trace:1",
                       "trace:2",
                       "trace:3",
                       "trace:4",
                       "trace:5",
                       "trace:6",
                       "trace:7",
                       "trace:8",
                       "trace:9",
                       "trace:10",
                       "trace:11",
                       "trace:12",
                    };
  if (level>=size)
  {
    return "???";
  }
  return l[level];
}
