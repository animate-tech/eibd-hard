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
 
#ident "$XORP: xorp/libxorp/c_format.cc,v 1.7 2005/06/20 21:31:43 pavlin Exp $"

extern "C" { 
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
};

#include <vector>
#include <string>
#include <iostream>
#include <sstream> 
#include <exception>
#include "c_format.h"

using namespace std;

void c_format_validate(const char* fmt, int exp_count)
{
  const char *p = fmt;
  int state = 0;
  int count = 0;
    
  while(*p != 0) {
    if (state == 0) {
      if (*p == '%') {
	count++;
	state = 1;
      }
    } else {
      switch (*p) {
      case 'd':
      case 'i':
      case 'o':
      case 'u':
      case 'x':
      case 'X':
      case 'D':
      case 'O':
      case 'U':
      case 'e':
      case 'E':
      case 'f':
      case 'g':
      case 'G':
      case 'c':
      case 's':
      case 'p':
	//parameter type specifiers
	state = 0;
	break;
      case '%':
	//escaped percent
	state = 0;
	count--;
	break;
      case 'n':
	//we don't permit %n
	fprintf(stderr, "%%n detected in c_format()\n");
	abort();
      case '*':
	//field width or precision also needs a parameter
	count++;
	break;
      }
    }
    p++;
  }
  if (exp_count != count) {
    abort();
  }
}

string do_c_format(const char* fmt, ...)
{
  size_t buf_size = 4096;             // Default buffer size
  vector<char> b(buf_size);
  va_list ap;

  do {
    va_start(ap, fmt);
    int ret = vsnprintf(&b[0], buf_size, fmt, ap);
    if ((size_t)ret < buf_size) {
      string r = string(&b[0]);	// Buffer size is OK
      va_end(ap);
      return r;
    }
    buf_size = ret + 1;		// Add space for the extra '\0'
    b.resize(buf_size);
  } while (true);

  return "";
}
