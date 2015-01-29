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

#include "eibdstate.h"
extern "C" {
#include <zlib.h>
};

  static std::string xmlstyle(
  			    "<?xml version=""1.0"" encoding=""UTF-8""?>"
  			    "<xsl:stylesheet version=""1.0"""
  			    "     xmlns:xsl=""http://www.w3.org/1999/XSL/Transform"">"
  			    ""
  			    "  <xsl:output method=""xml"""
  			    "              doctype-public=""gxl"""
  			    "            doctype-system=""http://www.gupro.de/GXL/gxl-1.0.dtd"""
  			    "            encoding=""UTF-8"""
  			    "            indent=""yes""/>"
  			    ""
  			    ""
  			    ""
  			    "  </xsl:stylesheet>"
  			    );

  void EIBDInstance::StateWrapper(
  		  XMLTree &xmltree,
  		  CArray &erg,
  		  Layer3 * l3,
  		  Logs * t,
  		  ClientConnection * c,
  		  pth_event_t &stop,
  		  int which)

  {
    CArray compressed;
    unsigned long compressedlen;

    Element * eibdxml = xmltree.GenerateEIBDStatusRoot();
    if (!eibdxml)
      {
        ERRORLOGSHAPE (t, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, NULL, Logging::MSGNOHASH,
  		     "State Report: Memory allocation failure, sending reject");
        c->sendreject(stop, EIB_PROCESSING_ERROR);
      }

    Element * stat = eibdxml->addElement(XMLSTATUSELEMENT);
    if (!stat)
      {
        ERRORLOGSHAPE (t, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, NULL, Logging::MSGNOHASH,
  		     "State Report: Memory allocation failure, sending reject");
        c->sendreject(stop, EIB_PROCESSING_ERROR);
      }

    stat->addAttribute(XMLSTATUSVERSIONATTR, VERSION);
    stat->addAttribute(XMLSTATUSSTARTTIMEATTR, this->starttime.fmtString("%c"));

    // grab the eibd instance state

    pth_mutex_acquire(&this->lock,false,NULL);

    // backend
    if (which & 1) {
      if (l3) {
        l3->_xml(stat);
      }
    }

    if (which & 2) {
      // all servers
      if (this->inetserver)
        this->inetserver->_xml(stat);
      if (this->localserver)
        this->localserver->_xml(stat);

  #ifdef HAVE_EIBNETIPSERVER
      if (this->serv)
        this->serv->_xml(stat);
  #endif
    }

    pth_mutex_release(&this->lock);

    xmltree.setCompression(3);

    const CArray & r = xmltree.writeCArray();

    // std::cout << r.len() << ":" << r.array() << "\n";

    // just in case get a little bit bigger buffer for compressed stuff
    compressed.resize(r.len()+32);

    compressedlen = compressed.len();
    if (compress( compressed.array(), &compressedlen,
  		r.array(), r.len()) != Z_OK)
      {
        ERRORLOGSHAPE (t, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, NULL, Logging::MSGNOHASH,
  		     "State Report: Buffer compression failed on state request, sending reject");
        c->sendreject(stop, EIB_PROCESSING_ERROR);
      }

    // if compress data exceeds packet size, just send processing error
    if (compressedlen>65000)
      {
        ERRORLOGSHAPE (t, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC, NULL, Logging::MSGNOHASH,
  		    "State Report: Status buffer too large to send, sending reject");
        c->sendreject(stop, EIB_PROCESSING_ERROR);
      }

    erg.resize(2+compressedlen);

    EIBSETTYPE (erg, EIB_STATE_REQ_BACKENDS);
    erg.setpart(compressed.array(), 2, compressedlen);

  #if 0
    for (int v=0; v<compressedlen; v++) {
      assert( erg.array()[v+2] == compressed.array()[v] );
      printf("%03u ", (int) erg.array()[v+2]);
      if (v%5==4) printf("\n");
    }

    printf("compressedlen %d, sending %d\n",compressedlen,erg.len());
  #endif

    c->sendmessage (erg.len(), erg.array (), stop);
  }

