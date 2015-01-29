/***************************************************************************
*   Copyright (C) 2005- by A. Przygienda, Z2 Sagl, Switzerland            *
*   prz _at_ net4u.ch                                                     *
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Z2, GmbH DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
* SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS, IN NO EVENT SHALL Z2, GmbH BE LIABLE FOR ANY
* SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
* IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
* ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
* THIS SOFTWARE.
 *
 * This file loosely based on code by Vladislav Grinchenko in ASSA
 *
 */

#ifndef TIME_VAL_H
#define TIME_VAL_H

#include <sys/time.h>		// gettimeofday(3)
#include <unistd.h>		// gettimeofday(3)
#include <pthsem.h>

#include <string>
#include <cstring>
#include <cstdio>
using std::string;

#include "c_format.h"

class TimeValException : public std::exception {
private:
  std::string c;
public:
  TimeValException(const std::string &s);
  ~TimeValException() throw () {};
  const char *what() const throw();
public:
  friend std::basic_ostream<char>& operator <<(std::basic_ostream<char>& os,
					       const TimeValException &e);
};

class TimeValXMLParseErrorException : public TimeValException {
public:
  TimeValXMLParseErrorException(const std::string &s, const std::string &b);
  ~TimeValXMLParseErrorException() throw () {};
};

/** @file TimeVal.h
*
* Class TimeVal is a wrapper around UNIX timeval structure.
*
*/
class TimeVal : public timeval
{
public:
  friend std::basic_ostream<char>& operator << (std::basic_ostream<char>&,
						const TimeVal &);

  enum {
    gmt,					/**< GMT */
    loc						/**< Local Time Zone */
  };

  typedef enum {
    XML_DATETIME,
    XML_DURATION
  } parseformat;

  const static unsigned int NEVER = 0;
  const static unsigned int UNTILNOW = 0;

  const static TimeVal Never;
  const static TimeVal UntilNow;

  /** Default constructor. Sets time to 0 sec. 0 usecs. To get
      current time, use TimeVal (gettimeofday());
  */
  TimeVal ();

  /** Constructor from seconds/milliseconds pair.
   */
  TimeVal (long sec_, long msec_);

  /** Constructor from double.
   */
  TimeVal (double d_);

  /** Constructor from <TT> struct timeval</TT>.
   */
  TimeVal (const timeval& tv_);

  /** constructor parsing strings */
  TimeVal (const std::string &s, const parseformat fmt);

  /** Copy constructor.
   */
  TimeVal (const TimeVal& tv_);

  /** Destructor
   */
  ~TimeVal();

  /// Set seconds.
  void sec (long sec_) { tv_sec = sec_; }

  /// Get secons.
  long sec (void) const { return tv_sec; }

  /// Set milliseconds.
  void msec (long msec_) { tv_usec = msec_; }

  /// Get milliseconds.
  long msec (void) const { return tv_usec; }

  /// Convert tv_usec (microseconds) to milliseconds (1 / 1,000 of a second)
  long millisec () const;

  /// Set timezone.
  void tz (int tz_) { m_tz = tz_; }

  /// Get timezone.
  int tz (void) const { return m_tz; }

  TimeVal& operator= (const TimeVal& tv_);

  /// Addition.
  TimeVal& operator+= (const TimeVal& rhs_);

  /// Substraction.
  TimeVal& operator-= (const TimeVal& rhs_);

  /// Addition.
  friend TimeVal operator+(const TimeVal& lhs_, const TimeVal& rhs_);

  /// Substraction.
  friend TimeVal operator-(const TimeVal& lhs_, const TimeVal& rhs_);

  /// Comparison.
  bool operator< (const TimeVal& rhs_) const;

  /// Equality.
  bool operator==(const TimeVal& rhs_) const;

  /// is contained in a range ?
  bool between(const TimeVal &timeAxisStart,const TimeVal &timeAxisEnd) const;

  /// Comparison.
  friend bool operator> (const TimeVal& lhs_, const TimeVal& rhs_);

  /// Comparison.
  friend bool operator!=(const TimeVal& lhs_, const TimeVal& rhs_);

  /// Comparison.
  friend bool operator<=(const TimeVal& lhs_, const TimeVal& rhs_);

  /// Comparison.
  friend bool operator>=(const TimeVal& lhs_, const TimeVal& rhs_);

  /** Format timeval structure into readable format.
      Default format is CCYY/DDD HH:MM:SS.MMM which is de fasco
      for the software. To get something different, pass fmt_
      format string as specified by strftime(3). Popular format
      is "%c" which will return something like:
      "Fri Oct 1 10:54:27 1999". Note that timezone aspect of
      formatting time is controlled by tz() member function.
      @param fmt_ Format string as in strftime(3)
      @return Formatted string.
  */
  string fmtString (const char* fmt_ = NULL) const;

  /** Format timeval in unix format */
  string fmt_unix () const;

  /** Format timeval structure in readable format HH:MM:SS
   */
  string fmt_hh_mm_ss () const;

  /** Format timeval structure in readable format HH:MM:SS.MLS
   */
  string fmt_hh_mm_ss_mls () const;

  /** Format timeval structure in readable format MM:SS
   */
  string fmt_mm_ss () const;

  /** Format according to XML standard */
  string fmt_XML () const;

  /** Format according to XML standard, interpret as duration and not time */
  string fmt_XML_duration () const;

  /** Format timeval structure in readable format MM:SS.MLS
   */
  string fmt_mm_ss_mls () const;

  /** Format timeval structure in readable format SS.MLS
   */
  string fmt_ss_mls () const;

  /** Dump value of struct timeval to the log file with mask
      TRACE = DBG_APP15
  */
  void dump_to_log (const string& name_ = "") const;

  /** Static that returns zero timeval: {0,0}
   */
  static TimeVal zeroTime () { return m_zero; }

  /** Shields off underlying OS differences in getting
      current time.
      @return time of the day as timeval
  */
  static TimeVal gettimeofday ();

protected:
  /// Internal initialization common to most constructors.
  void init (long, long, int);

private:
  /// Normalization after arithmetic operation.
  void normalize ();

private:
  /// Time zone
  int m_tz;

  /// Zero time value
  static TimeVal m_zero;

  /** from here on all the stuff needed for parsing */
  //@{
  enum valueIndex
    {
      CentYear   = 0,
      Month      ,
      Day        ,
      Hour       ,
      Minute     ,
      Second     ,
      MiliSecond ,  //not to be used directly
      utc        ,
      TOTAL_SIZE
    };

  enum utcType
    {
      UTC_UNKNOWN = 0,
      UTC_STD        ,          // set in parse() or normalize()
      UTC_POS        ,          // set in parse()
      UTC_NEG                   // set in parse()
    };

   enum timezoneIndex
     {
       hh = 0,
       mm ,
       TIMEZONE_ARRAYSIZE
     };

  static struct _parser_context {
    int            fStart, fEnd;
    int            fValue[TOTAL_SIZE];
    int            fTimeZone[TIMEZONE_ARRAYSIZE];
    int            fBufferMaxLen;

    double         fMiliSecond;
    bool           fHasTime;
    char          *fBuffer;
  } parser_context;

  static const unsigned char DURATION_STARTER    ;
  static const unsigned char DURATION_Y           ;
  static const unsigned char DURATION_M           ;
  static const unsigned char DURATION_D           ;
  static const unsigned char DURATION_H           ;
  static const unsigned char DURATION_S           ;

  static const unsigned char DATE_SEPARATOR       ;
  static const unsigned char TIME_SEPARATOR       ;
  static const unsigned char TIMEZONE_SEPARATOR   ;
  static const unsigned char DATETIME_SEPARATOR   ;
  static const unsigned char MILISECOND_SEPARATOR ;

  static const unsigned char UTC_STD_CHAR         ;
  static const unsigned char UTC_POS_CHAR         ;
  static const unsigned char UTC_NEG_CHAR         ;

  static const std::string   UTC_SET;

  static const int YMD_MIN_SIZE    ;   // CCYY-MM-DD
  static const int YMONTH_MIN_SIZE ;    // CCYY_MM
  static const int TIME_MIN_SIZE   ;    // hh:mm:ss
  static const int TIMEZONE_SIZE   ;    // hh:mm
  static const int DAY_SIZE        ;    // ---DD
  // static const int MONTH_SIZE      = 6;    // --MM--
  static const int MONTHDAY_SIZE   ;    // --MM-DD
  static const int NOT_FOUND       ;

  //define static constants to be used in assigning default values for
  //all date/time excluding duration
  static const int YEAR_DEFAULT ;
  static const int MONTH_DEFAULT;
  static const int DAY_DEFAULT  ;

  inline void           setBuffer(const std::string & );
  void                  parseDateTime();       //DateTime
  void                  parseDate();           //Date
  void                  parseTime();           //Time
  void                  parseDay();            //gDay
  void                  parseMonth();          //gMonth
  void                  parseYear();           //gYear
  void                  parseMonthDay();       //gMonthDay
  void                  parseYearMonth();      //gYearMonth
  void                  parseDuration();       //duration
  inline  void          reset();
  // allow multiple parsing
  inline  void          initParser();
  inline  bool          isNormalized()               const;
  void                  getDate();
  void                  getTime();
  void                  getYearMonth();
  void                  getTimeZone(const int);
  void                  parseTimeZone();
  int                   findUTCSign(const int start);
  int                   indexOf(const int start
				, const int end
				, const unsigned char ch)     const;
  int                   parseInt(const int start, const int end)     const;
  int                   parseIntYear(const int end) const;
  double                parseMiliSecond(const int start
					, const int end) const;
  void                  validateDateTime()          const;
  void                  xmlnormalize();

public:
  static TimeVal NoTime;
};
//------------------------------------------------------------------------------
// Inlines
//------------------------------------------------------------------------------

inline void
TimeVal::
init (long s_, long ms_, int tz_)
{
  tv_sec = s_;
  tv_usec = ms_;
  m_tz = tz_;
  normalize ();
}

inline
TimeVal::
TimeVal ()
{
  init (0, 0, gmt);
}

inline
TimeVal::
TimeVal (long sec_, long msec_)
{
  init (sec_, msec_, gmt);
}

inline
TimeVal::
TimeVal (double d_)
  : m_tz (gmt)
{
  init (0, 0, gmt);
  long l = long(d_);
  tv_sec = l;
  tv_usec = (long) ((d_ - double(l))*1000000.0);
  normalize();
}

inline
TimeVal::
TimeVal (const timeval& tv_)
{
  init (tv_.tv_sec, tv_.tv_usec, gmt);
}

inline
TimeVal::
TimeVal (const TimeVal& tv_)
{
  init (tv_.tv_sec, tv_.tv_usec, tv_.m_tz);
}

inline
TimeVal::
~TimeVal(void) {
}

inline TimeVal
TimeVal::
gettimeofday ()
{
  timeval tv;
  ::gettimeofday (&tv, 0);
  return tv;
}

inline long
TimeVal::
millisec () const
{
  return (msec () % 1000000) / 1000;
}

inline string
TimeVal::
fmt_hh_mm_ss () const
{
  return fmtString ("%T");
}

inline string
TimeVal::
fmt_mm_ss () const
{
  return fmtString ("%M:%S");
}

inline string
TimeVal::
fmt_XML () const
{
  return fmtString ("%Y-%m-%dT%H:%M:%S") + c_format(".%03lu", millisec());
}

inline string
TimeVal::
fmt_XML_duration() const
{
  char buf[80];
  memset (buf, 0, 80);
  sprintf(buf, "PT%ld.%03luS", (long) tv_sec, millisec());

  return string(buf);
}


//------------------------------------------------------------------------------
// Friend functions
//------------------------------------------------------------------------------

inline TimeVal&
TimeVal::
operator=(const TimeVal& tv_)
{
  init (tv_.tv_sec, tv_.tv_usec, tv_.m_tz);
  return *this;
}

inline TimeVal
operator+(const TimeVal& lhs_, const TimeVal& rhs_)
{
  TimeVal temp(lhs_);
  temp += rhs_;
  temp.normalize ();
  return temp;
}

inline TimeVal
operator-(const TimeVal& lhs_, const TimeVal& rhs_)
{
  TimeVal temp(lhs_);
  temp -= rhs_;
  temp.normalize ();
  return temp;
}

inline bool
TimeVal::
operator<(const TimeVal& rhs_) const
{
  return (tv_sec < rhs_.tv_sec
	  || (tv_sec == rhs_.tv_sec && tv_usec < rhs_.tv_usec) ) ;
}

inline bool
TimeVal::
operator==(const TimeVal& rhs_) const
{
  return !( ((const TimeVal &) *this) < rhs_ || rhs_ < ((const TimeVal &) *this));
}

inline bool
operator> (const TimeVal& lhs_, const TimeVal& rhs_)
{
  return rhs_ < lhs_;
}

inline bool
operator!=(const TimeVal& lhs_, const TimeVal& rhs_)
{
  return !( lhs_ == rhs_ );
}

inline bool
operator<=(const TimeVal& lhs_, const TimeVal& rhs_)
{
  return !(rhs_ < lhs_);
}

inline bool
operator>=(const TimeVal& lhs_, const TimeVal& rhs_)
{
  return !(lhs_ < rhs_);
}


#endif /* TIME_VAL_H */
