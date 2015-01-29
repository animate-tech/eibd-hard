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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef TRACE_H
#define TRACE_H

#include <stdarg.h>
#include <syslog.h>
#include <search.h>
#include <stdio.h>
#include <iostream>
#include "pthsem.h"

template < class K > int compar(const void * k1, const void *k2) throw ();

template < typename T > void delnodes(void *nodep) throw ();
template < typename T > void printnodes(void *nodep) throw ();

/** node on tree as template */
template < class K, class C >
class _node
{
  public:
    K  key;
    C  content;
};

template < class K, class C >
std::basic_ostream<char>& operator << (std::basic_ostream<char> &,
                                       const class _node<K,C> &);

/** tsearch POSIX interface wrapper.
 *  The tree is passing out pointers to nodes
 *  so surrouding application has to protect it via mutex if necessary */
template < class K, class C > class CheapMap
{
  protected:
    void        *tree;
    unsigned int _size;

    typedef class _node<K,C> node_t;

  public:
    CheapMap();

    ~CheapMap();

    unsigned int size(void)
    {
      return _size;
    }

    /** returns the content of the key on the map. Important is
     * that it allocates the key with empty storage if key is not around!
     */
    C & operator [] (const K &k);

    /** returns the pointer into the content of the map entry that
     * corresponds to key. Does _not_ insert the key into map if not present
     * but returns NULL. */
    C *find(const K &k);

    /** rmeoves the content & key from the map. */
    CheapMap & operator -= (const K &k);

    void Dump(void);

    friend std::basic_ostream<char>& operator<< < > (std::basic_ostream<char> &,
        const class _node<K,C> &);
};

/** keeps the current second & how many times a message already
 *  was seen, suppressed within the second. Cannot be kept inside
 *  @ref Logging where it belongs since we need an << operator for it
 *  for iostream.  */
class _ShapeFilter
{
  public:
    timestamp_t   timeStamp;
    unsigned int  msgsSinceTimestamp;
    unsigned int  msgsSupressedSinceTimestamp;
  public:
    _ShapeFilter() {};
    ~_ShapeFilter() {};
};

std::basic_ostream<char>& operator<< (std::basic_ostream<char> &,
                                      const class _ShapeFilter &);

/** implements a generic logging class that accomodates errors/warning/infos/traces
 *  including a potential shaping against too many messages being generated and
 *  generic backend to a device.
 *
 *  The class allows to call logging without any frills via @refer Log method.
 *  If shapping, message repetition logging is desired, @refer shapedLog must
 *  be used.
 */
class Logging
{

public:

    /** the log levels are aligned for simplicity with syslog values
     *  so naive @ref LogLevel2SyslogLevel works */
	typedef enum
    {
      LOG_LEVEL_0  = LOG_MASK(LOG_EMERG),
      LOG_LEVEL_1  = LOG_MASK(LOG_ALERT),
      LOG_LEVEL_2  = LOG_MASK(LOG_CRIT),
      LOG_LEVEL_3  = LOG_MASK(LOG_ERR),
      LOG_LEVEL_4  = LOG_MASK(LOG_WARNING),
      LOG_LEVEL_5  = LOG_MASK(LOG_NOTICE),
      LOG_LEVEL_6  = LOG_MASK(LOG_INFO),
      LOG_LEVEL_7  = LOG_MASK(LOG_DEBUG),
    } levelMaskValues;

    /** log methods, can be OR'ed */
    typedef enum _logMethod
    {
      NONE   = 0x00,
      STDOUT = 0x01,
      STDERR = 0x02,
      SYSLOG = 0x04
    } logMethod;

    /** shaping policies as quick to get enum */
    typedef enum _shapingPolicies
    {
      NOPOLICY              = 0,
      SUMMDUPLICATESONLY    = NOPOLICY + 1,
      DUPLICATESMAX1PERSEC  = SUMMDUPLICATESONLY + 1,
      DUPLICATESMAX10PERSEC = DUPLICATESMAX1PERSEC + 1,
      DUPLICATESMAX1PERMIN  = DUPLICATESMAX10PERSEC + 1,
      DUPLICATESMAX1PER30SEC= DUPLICATESMAX1PERMIN + 1,
      DUPLICATESMAX1PER10SEC= DUPLICATESMAX1PER30SEC + 1,
      POLICYMAX             = DUPLICATESMAX1PER10SEC + 1,

    } shapingPolicies;

    /** that's how we keep messages apart */
    typedef unsigned long logMessageHash;

    /** const for no hash */
    static const logMessageHash MSGNOHASH = 0;

    /** return codes of methods */
    typedef enum
    {
      OK          = 0,
      WRONGARGS   = -1,
      GETTIMEFAIL = -2,
      NOSUCHPOLICY= -3,
      MEMORYALLOC = -4,
    } returnStatus;

  public:

  private:
    unsigned int cheap2nmodulo(unsigned int rn, unsigned int m);

    static const struct _pp
    {
      bool         duplicates;
      unsigned int timerressec; /**< modulo timer tick, please 2^n-1
                                        since modulo is implemented by using & */
      unsigned int msgpertick;
    } policyPars[POLICYMAX];

    unsigned int levels;
    logMethod    logto;

    /** max line length of the output */
    static const unsigned int   MAXLOGLINELEN = 2048;
    mutable pth_mutex_t  linebufmutex;
    char  linebuf[MAXLOGLINELEN];

    /** keeps the state of the last logged message for
     *  repetitions */
    struct
    {
      logMessageHash lastLogged;
      unsigned int   lastLoggedTimes;
      unsigned int   lastLoggedLevel;
      timestamp_t    timeStamp;
    } lastLogged;

    /** generates hash of a string and its level to use for shaping
     *  of messages
     *  @param level  level of message
     *  @param msg    message text */
    logMessageHash messageHash(const unsigned int level, const char *msg) const;

    /** type for the map of shape filter entries ordered on message hash */
    typedef std::map < logMessageHash, class _ShapeFilter * > shapeFilterMap;

    /** mutex to protect the filter map */
    pth_mutex_t     shapeFilterMapMutex;
    /** message hashes with their filters (if any) */
    shapeFilterMap filterMap;

  protected:
    /** pulls out the bit position out of level mask */
    int levelBitMask2BitPos(const levelMaskValues lb) const;

    /** if repeated message is found, repeat message is flushed */
    void dumpRepeats(const struct tm *tm,
        const suseconds_t subs);
    /** dumps number of supression the policy enforced if any.
     * @param tm      pointer to current time structure
     * @param subs    subseconds micro
     * @param level   log level, will be translated to string
     * @param msg     message format string
     * @param f       pointer to shapefilter of the message
     * @param newtick tick to set on the filter after dump */
    void dumpSupressions(const struct tm *tm,
                         const suseconds_t subs,
                         const unsigned int level,
                         const char *msg,
                         class _ShapeFilter *f,
                         const unsigned int newtick) ;

    /** translated logging level into syslog level when
     * writing to syslog. Can be overwritten by subclasses */
    virtual int logLevel2SyslogLevel(const unsigned int level) const;

    /** translates sylog! log level to output string (for stdout, stderr) */
    virtual const char *logLevel2Str(const int level) const;

    /** write without any further frills a string to devices at level
     * @param tm    pointer to current time structure
     * @param level log level, will be translated to string
     * @param msg   message format string */
    virtual returnStatus vstring2dev(const struct tm *tm,
                                     const suseconds_t subs,
                                     const class LoggableObjectInterface *obj_logging,
                                     const unsigned int level,
                                     const char *msg,
                                     va_list ap);

    /** equivalent to @ref vstring2dev, actually just converts
     *  the variable arg list into va_list and calls it. */
    returnStatus string2dev(const struct tm *tm,
                            const suseconds_t subs,
                            const class LoggableObjectInterface *obj_logging,
                            const unsigned int level,
                            const char *msg,
                            ...) ;

    /** write formatted string out to logging device
     * @param level            level of message
     * @param msgDuplicates    should message duplicates of last message be summarized
     * @param policy           ref to shaping policy
     * @param obj_logging      object logging whose ID should be stamped on message
     * @param msgHash          optional message hash, if MSGNOHASH, a message hash
     *                        will not be computed and no shaping/duplicates will
     *                        be performed.
     * @param msg              msg string
     */
    virtual returnStatus string2devShape(const struct tm *tm,
                                         const suseconds_t subs,
                                         const class LoggableObjectInterface *obj_logging,
                                         const unsigned int    level,
                                         const struct _pp    &policy,
                                         const logMessageHash msgHash,
                                         const char    *msg,
                                         va_list        ap);
  public:
    /** constructor
     * @param logMethod        where to log, can be OR'ed for multiple destinations
     * @param sysLogParameters optional syslog parameter structure
     */
    Logging(logMethod m = SYSLOG);

    virtual ~Logging();

    void flush(void);
    /** flush the log */

    /** returns whether should trace message be written based on configuration
       * @param level level of the message
       * @return bool
       */
    virtual bool shouldBePrinted (int level)
    {
      return (levels & (1 << level));
    };

    /** logs a message, optionally shaping the output & keeping track
     *  of repetitions.
     * @param level            level of message
     * @param policy           policy to shape the message (class)
     * @param msgDuplicates    should message duplicates of last message be summarized
     * @param maxMsgsPerSecond max. messages of this type to be printed per second
     * @param msgHash          optional message hash, if MSGNOHASH, one will be computed
     *                        from the msg string & level. arguments of the msg are
     *                        disregarded since the implementation would be too heavy.
     * @param msg              msg string & arguments
     */
    virtual returnStatus shapedLog(const unsigned int level,
                                   const class LoggableObjectInterface *obj_logging,
                                   shapingPolicies  policy,
                                   logMessageHash msgHash,
                                   const char *msg, ...);


    /** sets the level mask */
    void setLevels(unsigned int levelmask)
    {
      levels = levelmask;
    };
    /** returns current levels of logging */
    unsigned int Levels(void)
    {
      return levels;
    };
};

class ErrorLog : public Logging
{
  protected:
    int logLevel2SyslogLevel(const unsigned int level) const;
  public:
    ErrorLog(logMethod m = SYSLOG) : Logging(m)
    {
    };
};

class WarnLog : public Logging
{
  protected:
    int logLevel2SyslogLevel(const unsigned int level) const;
  public:
    WarnLog(logMethod m = SYSLOG) : Logging(m)
    {
    };
};

class InfoLog : public Logging
{
  protected:
    int logLevel2SyslogLevel(const unsigned int level) const;
  public:
    InfoLog(logMethod m = SYSLOG) : Logging(m)
    {
    };
};

#define TRACE_LEVEL_0 Logging::LOG_LEVEL_0
#define TRACE_LEVEL_1 Logging::LOG_LEVEL_1
#define TRACE_LEVEL_2 Logging::LOG_LEVEL_2
#define TRACE_LEVEL_3 Logging::LOG_LEVEL_3
#define TRACE_LEVEL_4 Logging::LOG_LEVEL_4
#define TRACE_LEVEL_5 Logging::LOG_LEVEL_5
#define TRACE_LEVEL_6 Logging::LOG_LEVEL_6
#define TRACE_LEVEL_7 Logging::LOG_LEVEL_7

/** tracing is a kind of special logging without shaping (since it
 * should be used for debugging only and in case of writing it to
 * syslog the level to syslog has to be munched down to syslog debug level */
class TraceLog : public Logging
{
    /** message levels to print */
    int layers;
  protected:
    int logLevel2SyslogLevel(const unsigned int level) const;
    const char *logLevel2Str(const int level) const;
  public:
    TraceLog (logMethod m= Logging::STDOUT) : Logging(m)
    {
    }

};

class Logs
{
  private:
    static bool singleton;

  private:
    /** used optionally to pass syslog parameters when
     * logging to syslog */
    struct sysLogParameters
    {
      char ident[16];
      int  options;   //< options passed to openlog
      int  facility;  //< facility that we use for logging
    };
      /** did we already open the syslog for our application ?
     *  Static since it's a class variable, not instance */
    static bool syslogOpen;

    sysLogParameters syslogPars;
  public:
      ErrorLog err;
      WarnLog  warn;
      InfoLog  info;
      TraceLog trace;
  public:
    Logs(Logging::logMethod errdest   = (Logging::logMethod) (Logging::STDERR | Logging::SYSLOG),
         Logging::logMethod warndest  = (Logging::logMethod) (Logging::STDERR | Logging::SYSLOG),
         Logging::logMethod infodest  = (Logging::logMethod) (Logging::STDERR | Logging::SYSLOG),
         Logging::logMethod tracedest =  Logging::STDOUT,
         sysLogParameters *syslogpars = NULL);

    ~Logs();

    void setTraceLevel(int level)
    {
      trace.setLevels(level);
    }
    void setWarnLevel(int level)
    {
      warn.setLevels(level);
    }
    void setInfoLevel(int level)
    {
    	info.setLevels(level);
    }
    void flush()
    {
      trace.flush();
      info.flush();
      warn.flush();
      err.flush();
    }

    /** prints a message with a hex dump unconditional
     * @param layer level of the message
     * @param inst pointer to the source
     * @param msg Message
     * @param Len length of the data
     * @param data pointer to the data
     */
    virtual void TracePacketUncond (const unsigned int layer,
                                    const class LoggableObjectInterface *inst,
                                    const char *msg,
                                    const int Len, const uchar * data);

    /** prints a message with a hex dump
     * @param layer level of the message
     * @param inst pointer to the source
     * @param msg Message
     * @param Len length of the data
     * @param data pointer to the data
     */
    void TracePacket (const unsigned int layer,
                      const class LoggableObjectInterface  *inst,
                      const char *msg, const int Len,
                      const uchar * data)
    {
      if (!trace.shouldBePrinted (layer))
        return;
      TracePacketUncond (layer, inst, msg, Len, data);
    }

    /** prints a message with a hex dump
     * @param layer level of the message
     * @param inst pointer to the source
     * @param msg Message
     * @param c array with the data
     */
    void TracePacket (const unsigned int layer,
                      const class LoggableObjectInterface  *inst,
                      const char *msg, const CArray & c)
    {
      TracePacket (layer, inst, msg, c.len() , c.array ());
    }

};

#define LOGPRINTF(logging, cmp, level, obj, args...) \
    do { if ((logging)->cmp.shouldBePrinted(level)) \
        (logging)->cmp.shapedLog(level, obj, Logging::NOPOLICY, Logging::MSGNOHASH, ##args); } while (0)

#define LOGPRINTFSHAPE(logging, cmp, level, policy, msgHash, obj, args...) \
    do { if ((logging)->cmp.shouldBePrinted(level)) \
        (logging)->cmp.shapedLog(level, obj, policy, msgHash, ##args); } while (0)

/** error logging with shaping.
 * @pars logs    pointer to instance of the Logs object
 * @pars level   use LOG_LEVEL_? or LOG_MASK(SYSLOG_LEVEL)
 * @pars policy  number of shaping policy
 * @pars obj     logging object or NULL
 * @pars msgHash optional message hash
 * @pars args    printf style message & its args
 */
#define ERRORLOGSHAPE(logs, level, policy, obj, msgHash, args...)  LOGPRINTFSHAPE(logs, err, level, policy, msgHash, obj, ##args)
/** @ref ERRORLOGSHAPE without shaping */
#define ERRORLOG(logs, level, obj, args...)  ERRORLOGSHAPE(logs, level, Logging::NOPOLICY, obj, Logging::MSGNOHASH, ##args)
/** warning logging with shaping. parameters equivalent to @ref ERRORLOGSHAPE */
#define WARNLOGSHAPE(logs, level, policy, obj, msgHash, args...)  LOGPRINTFSHAPE(logs, warn, level, policy, msgHash, obj, ##args)
#define WARNLOG(logs, level, obj, args...)  WARNLOGSHAPE(logs, level, Logging::NOPOLICY, obj, Logging::MSGNOHASH, ##args)
/** info logging with shaping. parameters equivalent to @ref ERRORLOGSHAPE */
#define INFOLOGSHAPE(logs, level, policy, obj, msgHash, args...)  LOGPRINTFSHAPE(logs, info, level, policy, msgHash, obj, ##args)
#define INFOLOG(logs, level, obj, args...)  INFOLOGSHAPE(logs, level, Logging::NOPOLICY, obj, Logging::MSGNOHASH, ##args)

#define TRACEPRINTF(logs, layer, obj, args...) LOGPRINTF(logs, trace, layer, obj, ##args)
#endif
