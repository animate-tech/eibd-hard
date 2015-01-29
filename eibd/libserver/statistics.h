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

#ifndef STATS_H
#define STATS_H

/** simple template for all kind of integer/char length counters */
template < typename T, const T nullelement > class StatisticsCounter 
{
 protected:
  T     _val;
  bool  overrun;
 public:
  StatisticsCounter() { _val = nullelement; }
  StatisticsCounter(T &t) { _val = t; }
  StatisticsCounter(T t) { _val = t; }
  StatisticsCounter &operator ++(void) { ++_val; if (_val==nullelement) { overrun=true; } return *this; }
  StatisticsCounter &operator +=(const T &v) { _val += v; return *this; }  
  StatisticsCounter &operator +=(const StatisticsCounter<T, nullelement> &c) { _val += *c; return *this; }
  StatisticsCounter &operator --(void) { --_val; if (_val<nullelement)  { overrun=true; } return *this; }
  StatisticsCounter &reset(void) { _val=nullelement; return *this; }
  T operator *(void) const { return _val; }
  T value(void) const { return _val; }
  StatisticsCounter &operator=(T &t) { _val=t; return *this; }
};

typedef StatisticsCounter<int,0> IntStatisticsCounter;
typedef StatisticsCounter<unsigned int,0> UIntStatisticsCounter;
typedef StatisticsCounter<long,0> LongStatisticsCounter;

#endif
