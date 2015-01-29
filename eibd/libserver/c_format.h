// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-

// Copyright (c) 2001-2005 International Computer Science Institute
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software")
// to deal in the Software without restriction, subject to the conditions
// listed in the XORP LICENSE file. These conditions include: you must
// preserve this copyright notice, and you cannot mention the copyright
// holders in advertising related to the Software without their permission.
// The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
// notice is a summary of the XORP LICENSE file; the license in that file is
// legally binding.

// $XORP: xorp/libxorp/c_format.hh,v 1.6 2005/03/25 02:53:37 pavlin Exp $

/*
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
*/

#ifndef __LIBXORP_C_FORMAT_HH__
#define __LIBXORP_C_FORMAT_HH__

/** c_format is a macro that creates a string from a c-style format
    string.  It takes the same arguments as printf, but %n is illegal and 
    will cause abort to be called.
     
    Pseudo prototype:
    string c_format(const char* format, ...);
     
    In practice c_format is a nasty macro, but by doing this we can check
    the compile time arguments are sane and the run time arguments.
*/

#define c_format(format, args...)					\
  (c_format_validate(format, arg_count(args)), do_c_format(format, ## args))

//
// Template magic to allow us to count the number of varargs passed to
// the macro c_format.  We could also count the size of the var args data
// for extra protection if we were doing the formatting ourselves...
//

// Comment this out to find unnecessary calls to c_format()
inline int arg_count() {
  return 0;
}


template <class A>
inline int arg_count(A) {
  return 1;
}

template <class A, class B>
  inline int arg_count(A,B) {
  return 2;
}

template <class A, class B, class C>
  inline int arg_count(A,B,C) {
  return 3;
}

template <class A, class B, class C, class D>
  inline int arg_count(A,B,C,D) {
  return 4;
}

template <class A, class B, class C, class D, class E>
  inline int arg_count(A,B,C,D,E) {
  return 5;
}

template <class A, class B, class C,class D, class E, class F>
  inline int arg_count(A,B,C,D,E,F) {
  return 6;
}

template <class A, class B, class C, class D, class E, class F, class G>
  inline int arg_count(A,B,C,D,E,F,G) {
  return 7;
}

template <class A, class B, class C, class D, class E, class F, class G,
  class H>
  inline int arg_count(A,B,C,D,E,F,G,H) {
  return 8;
}

template <class A, class B, class C, class D, class E, class F, class G,
  class H, class I>
  inline int arg_count(A,B,C,D,E,F,G,H,I) {
  return 9;
}

template <class A, class B, class C, class D, class E, class F, class G,
  class H, class I, class J>
  inline int arg_count(A,B,C,D,E,F,G,H,I,J) {
  return 10;
}

template <class A, class B, class C, class D, class E, class F, class G,
  class H, class I, class J, class K>
  inline int arg_count(A,B,C,D,E,F,G,H,I,J,K) {
  return 11;
}

template <class A, class B, class C, class D, class E, class F, class G,
  class H, class I, class J, class K, class L>
  inline int arg_count(A,B,C,D,E,F,G,H,I,J,K,L) {
  return 12;
}

template <class A, class B, class C, class D, class E, class F, class G,
  class H, class I, class J, class K, class L, class M>
  inline int arg_count(A,B,C,D,E,F,G,H,I,J,K,L,M) {
  return 13;
}

template <class A, class B, class C, class D, class E, class F, class G,
  class H, class I, class J, class K, class L, class M, class N>
  inline int arg_count(A,B,C,D,E,F,G,H,I,J,K,L,M,N) {
  return 14;
}

void c_format_validate(const char* fmt, int n);

#if defined(__printflike)
std::string do_c_format(const char* fmt, ...) __printflike(1,2);
#elif (defined(__GNUC__))
std::string do_c_format(const char* fmt, ...)
  __attribute__((__format__(printf, 1, 2)));
#else
string do_c_format(const char* fmt, ...);
#endif

#endif // __LIBXORP_C_FORMAT_HH__
