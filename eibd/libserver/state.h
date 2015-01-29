/*
    EIBD client library
    Copyright (C) 2007- written by Z2 GmbH, T. Przygienda <prz@net4u.ch>
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License,
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any
    restriction coming from the use of this file. (The General Public
    License restrictions do apply in other respects; for example, they
    cover modification of the file, and distribution when not linked into
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/*
 *   massive rewrite to support many features & stabilize
 *
 *    Embedded Controller software
 *    Copyright (c) 2006- Z2, GmbH, Switzerland
 *    All Rights Reserved
 *
 *    THE ACCOMPANYING PROGRAM IS PROPRIETARY SOFTWARE OF Z2, GmbH,
 *    AND CANNOT BE DISTRIBUTED, COPIED OR MODIFIED WITHOUT
 *    EXPRESS PERMISSION OF Z2, GmbH.
 *
 *    Z2, GmbH DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 *    SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 *    AND FITNESS, IN NO EVENT SHALL Z2, LLC BE LIABLE FOR ANY
 *    SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 *    IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 *    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 *    THIS SOFTWARE.
 *
 */

#ifndef STATE_H_
#define STATE_H_

#include "client.h"

class DaemonInstance {
public:
	  // l2 of the backend
	  Layer2Interface *l2;
	  //
	  Layer3 *l3;

	  bool initialized;

	  pth_mutex_t lock;

	  TimeVal starttime;

public:
	  DaemonInstance(void) :  initialized(false) , starttime(pth_timeout(0,0))
	  { pth_mutex_init (&lock);  };

	  virtual void StateWrapper(
			  XMLTree &xmltree,
			  CArray &erg,
			  Layer3 * l3,
			  Logs * t,
			  ClientConnection * c,
			  pth_event_t &stop,
			  int which) = 0;

	  /** dumps the state of threads for eibd to stdout.
	   *  @param l3 Layer 3 interface
	   *  @param t debug output
	   *  @param c client connection
	   *  @param stop if occurs, function should abort
	   * */
	  void StateThreads (Layer3 * l3, Logs * t, ClientConnection * c,
	                pth_event_t stop);

	  /** returns a buffer with the XML description of the state of the backends.
	   *  To make the communication smaller, the output is gzipped.
	   *  @param l3 Layer 3 interface
	   *  @param t debug output
	   *  @param c client connection
	   *  @param stop if occurs, function should abort
	   * */
	  void StateBackends(Layer3 * l3, Logs * t, ClientConnection * c,
	  		   pth_event_t stop);


	  /** returns a buffer with the XML description of the state of the servers.
	   *  To make the communication smaller, the output is gzipped.
	   *  @param l3 Layer 3 interface
	   *  @param t debug output
	   *  @param c client connection
	   *  @param stop if occurs, function should abort
	   * */
	  void StateServers(Layer3 * l3, Logs * t, ClientConnection * c,
	  		  pth_event_t stop);

};



#endif /*STATE_H_*/
