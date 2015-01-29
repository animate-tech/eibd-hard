/***************************************************************************
*   Copyright (C) 2005- by Z2 Sagl, Switzerland
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
 *
 * This file loosely based on code by Vladislav Grinchenko in ASSA
 *
 */

#include <time.h>		// localtime(3), gmtime(3)
#include <stdio.h>		// sprintf(3)

#include "timeval.h"
#include "c_format.h"

#include <cstdlib>
#include <iostream>

struct TimeVal::_parser_context TimeVal::parser_context;

TimeVal TimeVal::m_zero;       // zero time
TimeVal TimeVal::NoTime;
static const long ONE_SECOND = 1000000;
const TimeVal TimeVal::Never(TimeVal::NEVER);
const TimeVal TimeVal::UntilNow(TimeVal::UNTILNOW);

std::basic_ostream<char>& operator <<(std::basic_ostream<char>& os,
				      const TimeValException &e)
{
  return os << e.what();
}

TimeValException::TimeValException(const std::string &s)
{
  c = s;
};

const char *TimeValException::what() const throw()
{
  return c.c_str();
}

TimeValXMLParseErrorException::TimeValXMLParseErrorException(const std::string &s, const std::string &b) :
  TimeValException(s + std::string(" ") + b)
{
}

std::basic_ostream<char>& operator << (std::basic_ostream<char>& os,
				       const TimeVal &v)
{
  return os << v.tv_sec << "." << v.tv_usec;
}

TimeVal&
TimeVal::
operator+=(const TimeVal& rhs_)
{
  tv_sec += rhs_.tv_sec;
  tv_usec += rhs_.tv_usec;

  if (tv_usec >= ONE_SECOND) {
    tv_usec -= ONE_SECOND;
    tv_sec++;
  }
  else if (tv_sec >= 1 && tv_usec < 0) {
    tv_usec += ONE_SECOND;
    tv_sec--;
  }
  normalize ();
  return *this;
}

TimeVal&
TimeVal::
operator-=(const TimeVal& rhs_)
{
  tv_sec -= rhs_.tv_sec;
  tv_usec -= rhs_.tv_usec;

  if (tv_usec < 0) {
    tv_usec += ONE_SECOND;
    tv_sec--;
  }
  else if (tv_usec >= ONE_SECOND) {
    tv_usec -= ONE_SECOND;
    tv_sec++;
  }
  normalize ();
  return *this;
}

void
TimeVal::
normalize ()
{
  if (tv_usec >= ONE_SECOND) {
    do {
      tv_sec++;
      tv_usec -= ONE_SECOND;
    }
    while (tv_usec >= ONE_SECOND);
  }
  else if (tv_usec <= -ONE_SECOND) {
    do {
      tv_sec--;
      tv_usec += ONE_SECOND;
    }
    while (tv_usec <= -ONE_SECOND);
  }

  if (tv_sec >= 1 && tv_usec < 0) {
    tv_sec--;
    tv_usec += ONE_SECOND;
  }
  else if (tv_sec < 0 && tv_usec > 0) {
    tv_sec++;
    tv_usec -= ONE_SECOND;
  }
}

string
TimeVal::
fmtString (const char* fmt_) const
{
  struct tm ct;
  char buf[80];
  memset (buf, 0, 80);

  if (m_tz == gmt)
    ct = *( localtime ((const time_t*) &tv_sec) );
  else
    ct = *( gmtime ((const time_t*) &tv_sec) );

  if (fmt_ == NULL) {
    strftime (buf, 80, "%Y/%j %H:%M:%S", &ct);
    sprintf (buf + strlen(buf),
	     ".%03ld", (tv_usec %1000000)/1000);
  }
  else {
    strftime(buf, 80, fmt_, &ct);
  }
  return string (buf);
}

string
TimeVal::
fmt_hh_mm_ss_mls () const
{
  struct tm ct;
  char buf [80];
  memset (buf, 0, 80);

  if (m_tz == gmt)
    ct = *( localtime ((const time_t*) &tv_sec) );
  else
    ct = *( gmtime ((const time_t*) &tv_sec) );

  strftime (buf, 80, "%H:%M:%S", &ct);
  sprintf (buf + strlen(buf), ".%03ld", millisec ());

  return string (buf);
}

string
TimeVal::
fmt_mm_ss_mls () const
{
  struct tm ct;
  char buf [80];
  memset (buf, 0, 80);

  if (m_tz == gmt)
    ct = *( localtime ((const time_t*) &tv_sec) );
  else
    ct = *( gmtime ((const time_t*) &tv_sec) );

  strftime (buf, 80, "%M:%S", &ct);
  sprintf (buf + strlen(buf), ".%03ld", millisec ());

  return string (buf);
}

string
TimeVal::
fmt_unix () const
{
  return string (c_format("%ld.%ld", tv_sec, tv_usec));
}

bool TimeVal::between(const TimeVal &timeAxisStart,const TimeVal &timeAxisEnd) const
{
  if (timeAxisStart != TimeVal::Never    && timeAxisStart > *this) return false;
  if (timeAxisEnd   != TimeVal::UntilNow && timeAxisEnd   < *this) return false;
  return true;
}

// parser specifica for XML datetime/duration

TimeVal::TimeVal (const std::string &s, const parseformat fmt)
{
  struct tm conv;
  time_t    res;

  memset(&parser_context,0,sizeof(parser_context));

  reset();
  setBuffer(s);
  initParser();

  switch (fmt) {
  case XML_DATETIME:
    parseDateTime();
    break;
  case XML_DURATION:
    parseDuration();
    break;
  default:
    throw TimeValException("unknown XML TimeVal format");
    break;
  }

  xmlnormalize();
  memset(&conv,0,sizeof(conv));
  conv.tm_sec = parser_context.fValue[Second];
  conv.tm_min = parser_context.fValue[Minute];
  conv.tm_hour= parser_context.fValue[Hour];
  conv.tm_mday= parser_context.fValue[Day];
  if (parser_context.fValue[Month] > 0) {
    conv.tm_mon = parser_context.fValue[Month]-1;
  }
  conv.tm_year = parser_context.fValue[CentYear];
  if (conv.tm_year > 1900)
    conv.tm_year -= 1900;
  conv.tm_isdst = 0;

  if (fmt == XML_DURATION) {
    conv.tm_year += 70; // start of epoch added to the duration
    conv.tm_mday += 1;
    conv.tm_hour += 1;
  }

  tv_sec = mktime (&conv);
  tv_usec = parser_context.fValue[MiliSecond] * 1000;
  m_tz = gmt;

  // not give we get here, destructor does same thing to be sure
}

inline void TimeVal::setBuffer(const std::string & aString)
{
  reset();

  parser_context.fEnd = aString.size();
  parser_context.fBufferMaxLen = aString.size() + 1;
  parser_context.fBuffer = (char *) aString.c_str();
}

inline void TimeVal::reset()
{
  for ( int i=0; i < TOTAL_SIZE; i++ )
    parser_context.fValue[i] = 0;

  parser_context.fMiliSecond   = 0;
  parser_context.fHasTime      = false;
  parser_context.fTimeZone[hh] = parser_context.fTimeZone[mm] = 0;
  parser_context.fStart = parser_context.fEnd = 0;

  if (parser_context.fBuffer)
    parser_context.fBuffer = NULL;
}

inline void TimeVal::initParser()
{
  parser_context.fStart = 0;   // to ensure scan from the very first beginning
  // in case the pointer is updated accidentally by someone else.
}

inline bool TimeVal::isNormalized() const
{
  return ( parser_context.fValue[utc] == UTC_STD ? true : false );
}

 const unsigned char TimeVal::DURATION_STARTER     =  'P';
 const unsigned char TimeVal::DURATION_Y           =  'Y';
 const unsigned char TimeVal::DURATION_M           =  'M';
 const unsigned char TimeVal::DURATION_D           =  'D';
 const unsigned char TimeVal::DURATION_H           =  'H';
 const unsigned char TimeVal::DURATION_S           =  'S';

 const unsigned char TimeVal::DATE_SEPARATOR       =  '-';
 const unsigned char TimeVal::TIME_SEPARATOR       =  ':';
 const unsigned char TimeVal::TIMEZONE_SEPARATOR   =  ':';
 const unsigned char TimeVal::DATETIME_SEPARATOR   =  'T';
 const unsigned char TimeVal::MILISECOND_SEPARATOR =  '.';

 const unsigned char TimeVal::UTC_STD_CHAR         =  'Z';
 const unsigned char TimeVal::UTC_POS_CHAR         =  '+';
 const unsigned char TimeVal::UTC_NEG_CHAR         =  '-';

 const std::string   TimeVal::UTC_SET("Z+-");

 const int TimeVal::YMD_MIN_SIZE    = 10;   // CCYY-MM-DD
 const int TimeVal::YMONTH_MIN_SIZE = 7;    // CCYY_MM
 const int TimeVal::TIME_MIN_SIZE   = 8;    // hh:mm:ss
 const int TimeVal::TIMEZONE_SIZE   = 5;    // hh:mm
 const int TimeVal::DAY_SIZE        = 5;    // ---DD
// const int MONTH_SIZE      = 6;    // --MM--
 const int TimeVal::MONTHDAY_SIZE   = 7;    // --MM-DD
 const int TimeVal::NOT_FOUND       = -1;

//define constants to be used in assigning default values for
//all date/time excluding duration
 const int TimeVal::YEAR_DEFAULT  = 2000;
 const int TimeVal::MONTH_DEFAULT = 01;
 const int TimeVal::DAY_DEFAULT   = 15;

// ---------------------------------------------------------------------------
//  local methods
// ---------------------------------------------------------------------------
static inline int fQuotient(int a, int b)
{
  div_t div_result = div(a, b);
  return div_result.quot;
}

static inline int fQuotient(int temp, int low, int high)
{
  return fQuotient(temp - low, high - low);
}

static inline int mod(int a, int b, int quotient)
{
  return (a - quotient*b) ;
}

static inline int modulo (int temp, int low, int high)
{
  //modulo(a - low, high - low) + low
  int a = temp - low;
  int b = high - low;
  return (mod (a, b, fQuotient(a, b)) + low) ;
}

static inline bool isLeapYear(int year)
{
  return((year%4 == 0) && ((year%100 != 0) || (year%400 == 0)));
}

static int maxDayInMonthFor(int year, int month)
{

  if ( month == 4 || month == 6 || month == 9 || month == 11 ) {
    return 30;
  }
  else if ( month==2 ) {
    if ( isLeapYear(year) )
      return 29;
    else
      return 28;
  }
  else {
    return 31;
  }

}

// ---------------------------------------------------------------------------
//  Parsers
// ---------------------------------------------------------------------------

//
// [-]{CCYY-MM-DD}'T'{HH:MM:SS.MS}[TimeZone]
//
void TimeVal::parseDateTime()
{
  initParser();
  getDate();

  //parser_context.fStart is supposed to point to 'T'
  if (parser_context.fBuffer[parser_context.fStart++] != DATETIME_SEPARATOR)
    throw TimeValXMLParseErrorException("DateTime_dt_missingT", parser_context.fBuffer);

  getTime();
  validateDateTime();
  xmlnormalize();
}

//
// [-]{CCYY-MM-DD}[TimeZone]
//
void TimeVal::parseDate()
{
  initParser();
  getDate();
  parseTimeZone();
  validateDateTime();
  xmlnormalize();
}

void TimeVal::parseTime()
{
  initParser();

  // time initialize to default values
  parser_context.fValue[CentYear]= YEAR_DEFAULT;
  parser_context.fValue[Month]   = MONTH_DEFAULT;
  parser_context.fValue[Day]     = DAY_DEFAULT;

  getTime();

  validateDateTime();
  xmlnormalize();
}

//
// {---DD}[TimeZone]
//  01234
//
void TimeVal::parseDay()
{
  initParser();

  if (parser_context.fBuffer[0] != DATE_SEPARATOR ||
      parser_context.fBuffer[1] != DATE_SEPARATOR ||
      parser_context.fBuffer[2] != DATE_SEPARATOR  ) {
      throw TimeValXMLParseErrorException("DateTime_gDay_invalid", parser_context.fBuffer);
    }

  //initialize values
  parser_context.fValue[CentYear] = YEAR_DEFAULT;
  parser_context.fValue[Month]    = MONTH_DEFAULT;
  parser_context.fValue[Day]      = parseInt(parser_context.fStart+3, parser_context.fStart+5);

  if ( DAY_SIZE < parser_context.fEnd ) {
    int sign = findUTCSign(DAY_SIZE);
    if ( sign < 0 ) {
      throw TimeValXMLParseErrorException("DateTime_gDay_invalid",parser_context.fBuffer);
    }
    else {
      getTimeZone(sign);
    }
  }

  validateDateTime();
  xmlnormalize();
}

//
// {--MM--}[TimeZone]
// {--MM}[TimeZone]
//  012345
//
void TimeVal::parseMonth()
{
  initParser();

  if (parser_context.fBuffer[0] != DATE_SEPARATOR ||
      parser_context.fBuffer[1] != DATE_SEPARATOR  ) {
    throw TimeValXMLParseErrorException("DateTime_gMth_invalid", parser_context.fBuffer);
  }

  //set constants
  parser_context.fValue[CentYear] = YEAR_DEFAULT;
  parser_context.fValue[Day]      = DAY_DEFAULT;
  parser_context.fValue[Month]    = parseInt(2, 4);

  // REVISIT: allow both --MM and --MM-- now.
  // need to remove the following lines to disallow --MM--
  // when the errata is officially in the rec.
  parser_context.fStart = 4;
  if ( parser_context.fEnd >= parser_context.fStart+2 && parser_context.fBuffer[parser_context.fStart] == DATE_SEPARATOR && parser_context.fBuffer[parser_context.fStart+1] == DATE_SEPARATOR ) {
    parser_context.fStart += 2;
  }

  //
  // parse TimeZone if any
  //
  if ( parser_context.fStart < parser_context.fEnd ) {
    int sign = findUTCSign(parser_context.fStart);
    if ( sign < 0 )
      {
	throw TimeValXMLParseErrorException("DateTime_gMth_invalid",parser_context.fBuffer);
      }
    else {
      getTimeZone(sign);
    }
  }

  validateDateTime();
  xmlnormalize();
}

//
//[-]{CCYY}[TimeZone]
// 0  1234
//
void TimeVal::parseYear()
{
  initParser();

  // skip the first '-' and search for timezone
  //
  int sign = findUTCSign((parser_context.fBuffer[0] == '-') ? 1 : 0);

  if (sign == NOT_FOUND) {
    parser_context.fValue[CentYear] = parseIntYear(parser_context.fEnd);
  }
  else {
    parser_context.fValue[CentYear] = parseIntYear(sign);
    getTimeZone(sign);
  }

  //initialize values
  parser_context.fValue[Month] = MONTH_DEFAULT;
  parser_context.fValue[Day]   = DAY_DEFAULT;   //java is 1

  validateDateTime();
  xmlnormalize();
}

//
//{--MM-DD}[TimeZone]
// 0123456
//
void TimeVal::parseMonthDay()
{
  initParser();

  if (parser_context.fBuffer[0] != DATE_SEPARATOR ||
      parser_context.fBuffer[1] != DATE_SEPARATOR ||
      parser_context.fBuffer[4] != DATE_SEPARATOR ) {
    throw TimeValXMLParseErrorException("DateTime_gMthDay_invalid", parser_context.fBuffer);
  }

  //initialize
  parser_context.fValue[CentYear] = YEAR_DEFAULT;
  parser_context.fValue[Month]    = parseInt(2, 4);
  parser_context.fValue[Day]      = parseInt(5, 7);

  if ( MONTHDAY_SIZE < parser_context.fEnd ) {
      int sign = findUTCSign(MONTHDAY_SIZE);
      if ( sign<0 ) {
	throw TimeValXMLParseErrorException("DateTime_gMthDay_invalid",parser_context.fBuffer);
      }
      else {
	getTimeZone(sign);
      }
  }

  validateDateTime();
  xmlnormalize();
}

void TimeVal::parseYearMonth()
{
  initParser();

  // get date
  getYearMonth();
  parser_context.fValue[Day] = DAY_DEFAULT;
  parseTimeZone();

  validateDateTime();
  xmlnormalize();
}

//
//PnYn MnDTnH nMnS: -P1Y2M3DT10H30M
//
// [-]{'P'{[n'Y'][n'M'][n'D']['T'][n'H'][n'M'][n'S']}}
//
//  Note: the n above shall be >= 0
//        if no time element found, 'T' shall be absent
//
void TimeVal::parseDuration()
{
  initParser();

  // must start with '-' or 'P'
  //
  unsigned char c = parser_context.fBuffer[parser_context.fStart++];
  if ( (c != DURATION_STARTER) &&
       (c != '-')            )
    {
      throw TimeValXMLParseErrorException("DateTime_dur_Start_dashP", parser_context.fBuffer);
    }

  // 'P' must ALWAYS be present in either case
  if ( (c == '-') &&
       (parser_context.fBuffer[parser_context.fStart++]!= DURATION_STARTER ))
    {
      throw TimeValXMLParseErrorException("DateTime_dur_noP",parser_context.fBuffer);
    }

  // java code
  //date[utc]=(c=='-')?'-':0;
  //parser_context.fValue[utc] = UTC_STD;
  parser_context.fValue[utc] = (parser_context.fBuffer[0] == '-'? UTC_NEG : UTC_STD);

  int negate = ( parser_context.fBuffer[0] == '-' ? -1 : 1);

  //
  // No negative value is allowed after 'P'
  //
  // eg P-1234, invalid
  //
  if (indexOf(parser_context.fStart, parser_context.fEnd, '-') != NOT_FOUND)
    {
      throw TimeValXMLParseErrorException("DateTime_dur_DashNotFirst", parser_context.fBuffer);
    }

  //at least one number and designator must be seen after P
  bool designator = false;

  int endDate = indexOf(parser_context.fStart, parser_context.fEnd, DATETIME_SEPARATOR);
  if ( endDate == NOT_FOUND )
    {
      endDate = parser_context.fEnd;  // 'T' absent
    }

  //find 'Y'
  int end = indexOf(parser_context.fStart, endDate, DURATION_Y);
  if ( end != NOT_FOUND ) {
    //scan year
    parser_context.fValue[CentYear] = negate * parseInt(parser_context.fStart, end);
    parser_context.fStart = end+1;
    designator = true;
  }

  end = indexOf(parser_context.fStart, endDate, DURATION_M);
  if ( end != NOT_FOUND ) {
    //scan month
    parser_context.fValue[Month] = negate * parseInt(parser_context.fStart, end);
    parser_context.fStart = end+1;
    designator = true;
  }

  end = indexOf(parser_context.fStart, endDate, DURATION_D);
  if ( end != NOT_FOUND ) {
    //scan day
    parser_context.fValue[Day] = negate * parseInt(parser_context.fStart,end);
    parser_context.fStart = end+1;
    designator = true;
  }

  if ( (parser_context.fEnd == endDate) &&   // 'T' absent
       (parser_context.fStart != parser_context.fEnd)   )   // something after Day
    {
      throw TimeValXMLParseErrorException("DateTime_dur_inv_b4T", parser_context.fBuffer);
    }

  if ( parser_context.fEnd != endDate ) // 'T' present
    {
      //scan hours, minutes, seconds
      //

      // skip 'T' first
      end = indexOf(++parser_context.fStart, parser_context.fEnd, DURATION_H);
      if ( end != NOT_FOUND )
        {
	  //scan hours
	  parser_context.fValue[Hour] = negate * parseInt(parser_context.fStart, end);
	  parser_context.fStart = end+1;
	  designator = true;
        }

      end = indexOf(parser_context.fStart, parser_context.fEnd, DURATION_M);
      if ( end != NOT_FOUND )
        {
	  //scan min
	  parser_context.fValue[Minute] = negate * parseInt(parser_context.fStart, end);
	  parser_context.fStart = end+1;
	  designator = true;
        }

      end = indexOf(parser_context.fStart, parser_context.fEnd, DURATION_S);
      if ( end != NOT_FOUND )
        {
	  //scan seconds
	  int mlsec = indexOf (parser_context.fStart, end, MILISECOND_SEPARATOR);

	  /***
	   * Schema Errata: E2-23
	   * at least one digit must follow the decimal point if it appears.
	   * That is, the value of the seconds component must conform
	   * to the following pattern: [0-9]+(.[0-9]+)?
	   */
	  if ( mlsec != NOT_FOUND )
            {
	      /***
	       * make usure there is something after the '.' and before the end.
	       */
	      if ( mlsec+1 == end )
                {
		  throw TimeValXMLParseErrorException("DateTime_dur_inv_seconds",parser_context.fBuffer);
                }

	      parser_context.fValue[Second]     = negate * parseInt(parser_context.fStart, mlsec);
	      parser_context.fValue[MiliSecond] = negate * parseInt(mlsec+1, end);
            }
	  else
            {
	      parser_context.fValue[Second] = negate * parseInt(parser_context.fStart,end);
            }

	  parser_context.fStart = end+1;
	  designator = true;
        }

      // no additional data should appear after last item
      // P1Y1M1DT is illigal value as well
      if ( (parser_context.fStart != parser_context.fEnd) ||
	   parser_context.fBuffer[--parser_context.fStart] == DATETIME_SEPARATOR )
        {
	  throw TimeValXMLParseErrorException("DateTime_dur_NoTimeAfterT", parser_context.fBuffer);
        }
    }

  if ( !designator )
    {
      throw TimeValXMLParseErrorException("DateTime_dur_NoElementAtAll", parser_context.fBuffer);
    }

}

// ---------------------------------------------------------------------------
//  Scanners
// ---------------------------------------------------------------------------

//
// [-]{CCYY-MM-DD}
//
// Note: CCYY could be more than 4 digits
//       Assuming parser_context.fStart point to the beginning of the Date Section
//       parser_context.fStart updated to point to the position right AFTER the second 'D'
//       Since the lenght of CCYY might be variable, we can't check format upfront
//
void TimeVal::getDate()
{

  // Ensure enough chars in buffer
  if ( (parser_context.fStart+YMD_MIN_SIZE) > parser_context.fEnd)
    throw TimeValXMLParseErrorException("DateTime_date_incomplete", parser_context.fBuffer);

  getYearMonth();    // Scan YearMonth and
  // parser_context.fStart point to the next '-'

  if (parser_context.fBuffer[parser_context.fStart++] != DATE_SEPARATOR)
    {
      throw TimeValXMLParseErrorException("DateTime_date_invalid", parser_context.fBuffer);
      //("CCYY-MM must be followed by '-' sign");
    }

  parser_context.fValue[Day] = parseInt(parser_context.fStart, parser_context.fStart+2);
  parser_context.fStart += 2 ;  //parser_context.fStart points right after the Day

  return;
}

//
// hh:mm:ss[.msssss]['Z']
// hh:mm:ss[.msssss][['+'|'-']hh:mm]
// 012345678
//
// Note: Assuming parser_context.fStart point to the beginning of the Time Section
//       parser_context.fStart updated to point to the position right AFTER the second 's'
//                                                  or ms if any
//
void TimeVal::getTime()
{

  // Ensure enough chars in buffer
  if ( (parser_context.fStart+TIME_MIN_SIZE) > parser_context.fEnd)
    throw TimeValXMLParseErrorException("DateTime_time_incomplete", parser_context.fBuffer);
  //"Imcomplete Time Format"

  // check (fixed) format first
  if ((parser_context.fBuffer[parser_context.fStart + 2] != TIME_SEPARATOR) ||
      (parser_context.fBuffer[parser_context.fStart + 5] != TIME_SEPARATOR)  )
    {
      throw TimeValXMLParseErrorException("DateTime_time_invalid", parser_context.fBuffer);
      //("Error in parsing time" );
    }

  //
  // get hours, minute and second
  //
  parser_context.fValue[Hour]   = parseInt(parser_context.fStart + 0, parser_context.fStart + 2);
  parser_context.fValue[Minute] = parseInt(parser_context.fStart + 3, parser_context.fStart + 5);
  parser_context.fValue[Second] = parseInt(parser_context.fStart + 6, parser_context.fStart + 8);
  parser_context.fStart += 8;

  // to see if any ms and/or utc part after that
  if (parser_context.fStart >= parser_context.fEnd)
    return;

  //find UTC sign if any
  int sign = findUTCSign(parser_context.fStart);

  //parse miliseconds
  int milisec = (parser_context.fBuffer[parser_context.fStart] == MILISECOND_SEPARATOR)? parser_context.fStart : NOT_FOUND;
  if ( milisec != NOT_FOUND )
    {
      parser_context.fStart++;   // skip the '.'
      // make sure we have some thing between the '.' and parser_context.fEnd
      if (parser_context.fStart >= parser_context.fEnd)
        {
	  throw TimeValXMLParseErrorException("DateTime_ms_noDigit",parser_context.fBuffer);
	  //("ms shall be present once '.' is present" );
        }

      if ( sign == NOT_FOUND )
        {
	  parser_context.fValue[MiliSecond] = parseInt(parser_context.fStart, parser_context.fEnd);  //get ms between '.' and parser_context.fEnd
	  parser_context.fStart = parser_context.fEnd;
        }
      else
        {
	  parser_context.fValue[MiliSecond] = parseInt(parser_context.fStart, sign);  //get ms between UTC sign and parser_context.fEnd
        }
    }
  else if(sign == 0 || sign != parser_context.fStart)
    {
      // seconds has more than 2 digits
      throw TimeValXMLParseErrorException("DateTime_min_invalid", parser_context.fBuffer);
    }

  //parse UTC time zone (hh:mm)
  if ( sign > 0 ) {
    getTimeZone(sign);
  }

}

//
// [-]{CCYY-MM}
//
// Note: CCYY could be more than 4 digits
//       parser_context.fStart updated to point AFTER the second 'M' (probably meet the parser_context.fEnd)
//
void TimeVal::getYearMonth()
{

  // Ensure enough chars in buffer
  if ( (parser_context.fStart+YMONTH_MIN_SIZE) > parser_context.fEnd)
    throw TimeValXMLParseErrorException("DateTime_ym_incomplete",parser_context.fBuffer);
  //"Imcomplete YearMonth Format";

  // skip the first leading '-'
  int start = ( parser_context.fBuffer[0] == '-' ) ? parser_context.fStart + 1 : parser_context.fStart;

  //
  // search for year separator '-'
  //
  int yearSeparator = indexOf(start, parser_context.fEnd, DATE_SEPARATOR);
  if ( yearSeparator == NOT_FOUND)
    throw TimeValXMLParseErrorException("DateTime_ym_invalid",parser_context.fBuffer);
  //("Year separator is missing or misplaced");

  parser_context.fValue[CentYear] = parseIntYear(yearSeparator);
  parser_context.fStart = yearSeparator + 1;  //skip the '-' and point to the first M

  //
  //gonna check we have enough byte for month
  //
  if ((parser_context.fStart + 2) > parser_context.fEnd )
    throw TimeValXMLParseErrorException("DateTime_ym_noMonth", parser_context.fBuffer);
  //"no month in buffer"

  parser_context.fValue[Month] = parseInt(parser_context.fStart, yearSeparator + 3);
  parser_context.fStart += 2;  //parser_context.fStart points right after the MONTH

  return;
}

void TimeVal::parseTimeZone()
{
  if ( parser_context.fStart < parser_context.fEnd ) {
    int sign = findUTCSign(parser_context.fStart);
    if ( sign < 0 ) {
      throw TimeValXMLParseErrorException("DateTime_tz_noUTCsign",parser_context.fBuffer);
      //("Error in month parsing");
    }
    else {
      getTimeZone(sign);
    }
  }

  return;
}

//
// 'Z'
// ['+'|'-']hh:mm
//
// Note: Assuming parser_context.fStart points to the beginning of TimeZone section
//       parser_context.fStart updated to meet parser_context.fEnd
//
void TimeVal::getTimeZone(const int sign)
{

  if ( parser_context.fBuffer[sign] == UTC_STD_CHAR ) {
    if ((sign + 1) != parser_context.fEnd ) {
      throw TimeValXMLParseErrorException("DateTime_tz_stuffAfterZ",parser_context.fBuffer);
      //"Error in parsing time zone");
    }

    return;
  }

  //
  // otherwise, it has to be this format
  // '[+|-]'hh:mm
  //    1   23456 7
  //   sign      parser_context.fEnd
  //
  if ( ( ( sign + TIMEZONE_SIZE + 1) != parser_context.fEnd )      ||
       ( parser_context.fBuffer[sign + 3] != TIMEZONE_SEPARATOR ) ) {
    throw TimeValXMLParseErrorException("DateTime_tz_invalid", parser_context.fBuffer);
    //("Error in parsing time zone");
  }

  parser_context.fTimeZone[hh] = parseInt(sign+1, sign+3);
  parser_context.fTimeZone[mm] = parseInt(sign+4, parser_context.fEnd);

  return;
}

// ---------------------------------------------------------------------------
//  Validator and xmlnormalizer
// ---------------------------------------------------------------------------

/**
 * If timezone present - xmlnormalize dateTime  [E Adding durations to dateTimes]
 *
 * @param date   CCYY-MM-DDThh:mm:ss+03
 * @return CCYY-MM-DDThh:mm:ssZ
 */
void TimeVal::xmlnormalize()
{

  if ((parser_context.fValue[utc] == UTC_UNKNOWN) ||
      (parser_context.fValue[utc] == UTC_STD)      )
    return;

  int negate = (parser_context.fValue[utc] == UTC_POS)? -1: 1;

  // add mins
  int temp = parser_context.fValue[Minute] + negate * parser_context.fTimeZone[mm];
  int carry = fQuotient(temp, 60);
  parser_context.fValue[Minute] = mod(temp, 60, carry);

  //add hours
  temp = parser_context.fValue[Hour] + negate * parser_context.fTimeZone[hh] + carry;
  carry = fQuotient(temp, 24);
  parser_context.fValue[Hour] = mod(temp, 24, carry);

  parser_context.fValue[Day] += carry;

  while (1)
    {
      temp = maxDayInMonthFor(parser_context.fValue[CentYear], parser_context.fValue[Month]);
      if (parser_context.fValue[Day] < 1)
        {
	  parser_context.fValue[Day] += maxDayInMonthFor(parser_context.fValue[CentYear], parser_context.fValue[Month] - 1);
	  carry = -1;
        }
      else if ( parser_context.fValue[Day] > temp )
        {
	  parser_context.fValue[Day] -= temp;
	  carry = 1;
        }
      else
        {
	  break;
        }

      temp = parser_context.fValue[Month] + carry;
      parser_context.fValue[Month] = modulo(temp, 1, 13);
      parser_context.fValue[CentYear] += fQuotient(temp, 1, 13);
    }

  // set to xmlnormalized
  parser_context.fValue[utc] = UTC_STD;

  return;
}

void TimeVal::validateDateTime() const
{

  //REVISIT: should we throw an exception for not valid dates
  //          or reporting an error message should be sufficient?
  if ( parser_context.fValue[CentYear] == 0 ) {
    throw TimeValXMLParseErrorException("DateTime_year_zero",parser_context.fBuffer);
    //"The year \"0000\" is an illegal year value");
  }

  if ( parser_context.fValue[Month] < 1  ||
       parser_context.fValue[Month] > 12  )
    {
      throw TimeValXMLParseErrorException("DateTime_mth_invalid", parser_context.fBuffer);
      //"The month must have values 1 to 12");
    }

  //validate days
  if ( parser_context.fValue[Day] > maxDayInMonthFor( parser_context.fValue[CentYear], parser_context.fValue[Month]) ||
       parser_context.fValue[Day] == 0 )
    {
      throw TimeValXMLParseErrorException("DateTime_day_invalid", parser_context.fBuffer);
      //"The day must have values 1 to 31");
    }

  //validate hours
  if ((parser_context.fValue[Hour] < 0)  ||
      (parser_context.fValue[Hour] > 24) ||
      ((parser_context.fValue[Hour] == 24) && ((parser_context.fValue[Minute] !=0) ||
				(parser_context.fValue[Second] !=0) ||
				(parser_context.fValue[MiliSecond] !=0))))
    {
      throw TimeValXMLParseErrorException("DateTime_hour_invalid", parser_context.fBuffer);
      //("Hour must have values 0-23");
    }

  //validate minutes
  if ( parser_context.fValue[Minute] < 0 ||
       parser_context.fValue[Minute] > 59 )
    {
      throw TimeValXMLParseErrorException("DateTime_min_invalid", parser_context.fBuffer);
      //"Minute must have values 0-59");
    }

  //validate seconds
  if ( parser_context.fValue[Second] < 0 ||
       parser_context.fValue[Second] > 60 )
    {
      throw TimeValXMLParseErrorException("DateTime_second_invalid", parser_context.fBuffer);
      //"Second must have values 0-60");
    }

  //validate time-zone hours
  if ( (abs(parser_context.fTimeZone[hh]) > 14) ||
       ((abs(parser_context.fTimeZone[hh]) == 14) && (parser_context.fTimeZone[mm] != 0)) )
    {
      throw TimeValXMLParseErrorException("DateTime_tz_hh_invalid", parser_context.fBuffer);
      //"Time zone should have range -14..+14");
    }

  //validate time-zone minutes
  if ( abs(parser_context.fTimeZone[mm]) > 59 )
    {
      throw TimeValXMLParseErrorException("DateTime_min_invalid", parser_context.fBuffer);
      //("Minute must have values 0-59");
    }

  return;
}

// -----------------------------------------------------------------------
// locator and converter
// -----------------------------------------------------------------------
int TimeVal::indexOf(const int start, const int end, const unsigned char ch) const
{
  for ( int i = start; i < end; i++ )
    if ( parser_context.fBuffer[i] == ch )
      return i;

  return NOT_FOUND;
}

int TimeVal::findUTCSign (const int start)
{
  std::string::size_type  pos;
  for ( int index = start; index < parser_context.fEnd; index++ ) {
    pos = UTC_SET.find(parser_context.fBuffer[index]);
    if ( pos != std::string::npos) {
      parser_context.fValue[utc] = pos+1;   // refer to utcType, there is 1 diff
      return index;
    }
  }

  return NOT_FOUND;
}

//
// Note:
//    start: starting point in parser_context.fBuffer
//    end:   ending point in parser_context.fBuffer (exclusive)
//    parser_context.fStart NOT updated
//
int TimeVal::parseInt(const int start, const int end) const
{
  unsigned int retVal = 0;
  for (int i=start; i < end; i++) {

    if (parser_context.fBuffer[i] < '0' || parser_context.fBuffer[i] > '9')
      break;

    retVal = (retVal * 10) + (unsigned int) (parser_context.fBuffer[i] - '0');
  }

  return (int) retVal;;
}

//
// [-]CCYY
//
// Note: start from parser_context.fStart
//       end (exclusive)
//       parser_context.fStart NOT updated
//
int TimeVal::parseIntYear(const int end) const
{
  // skip the first leading '-'
  int start = ( parser_context.fBuffer[0] == '-' ) ? parser_context.fStart + 1 : parser_context.fStart;

  int length = end - start;
  if (length < 4)
    {
      throw TimeValXMLParseErrorException("DateTime_year_tooShort", parser_context.fBuffer);
      //"Year must have 'CCYY' format");
    }
  else if (length > 4 &&
	   parser_context.fBuffer[start] == '0')
    {
      throw TimeValXMLParseErrorException("DateTime_year_leadingZero", parser_context.fBuffer);
      //"Leading zeros are required if the year value would otherwise have fewer than four digits;
      // otherwise they are forbidden");
    }

  bool negative = (parser_context.fBuffer[0] == '-');
  int  yearVal = parseInt((negative ? 1 : 0), end);
  return ( negative ? (-1) * yearVal : yearVal );
}

