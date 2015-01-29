/* libxmlccwrap - a C++ wrapper around libxml2
 * Copyright (C) 2002-2003 Juergen Rinas.
 * modified & extended 2005-2008 Z2 GmbH, Switzerland.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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

#ifndef _xmlccwrap_h
#define _xmlccwrap_h

extern "C" {
#include <libxml/tree.h>
};

#include <list>
#include <map>
#include <string>

#include <types.h>

class XMLTree;

class Element;
typedef std::list<Element *> ElementList;
typedef ElementList::iterator ElementIterator;
typedef ElementList::const_iterator ElementConstIterator;

class Attribute;
typedef std::list<Attribute *> AttributeList;
typedef AttributeList::iterator AttributeIterator;
typedef AttributeList::const_iterator AttributeConstIterator;

typedef std::multimap<std::string, std::string> ParameterMap;
typedef ParameterMap::value_type ValuePair;


class XSLTTree
{
 private:
  void * xsltP; // it is realy a    xsltStylesheetPtr xsltP;
  // but here encapsulated in a void* to
  // avoid inclusion of libxslt headers

 public:

  XSLTTree();
  XSLTTree(const std::string &fn);
  ~XSLTTree();

  bool read(const std::string &fn);
  const void * exportxsltStylesheetPtr() const;
};


class XMLTree
{
 private:
  std::string _filename;
  std::string dtdfilename;
  Element *_root;
  std::string _encoding;
  int _compression;
  std::string errorString;

 public:
  XMLTree();
  XMLTree(const XMLTree & xmlTree);
  ~XMLTree();

  Element *getRoot() const;
  Element *setRoot(const Element & element); // create a copy of element and insert it
  Element *setRoot(Element * elementP);      // add the pointer elementP -- libxmlccwrap will delete the object if necessary

  /** generates RTI header and sets as root. next time the user calls getRoot, he gets the RTI element */
  Element *GenerateEIBDStatusRoot();
  /** generate GraphML header and sets as root. */
  Element *GenerateGraphMLRoot();

  const std::string & getFilename() const;
  const std::string & setFilename(const std::string &fn);

  const std::string & getEncoding() const;
  const std::string & setEncoding(const std::string &);

  int getCompression() const;
  int setCompression(int);


  bool read();
  bool read(const std::string &fn);
  bool readWithDTD(const std::string &fn, const std::string &_dtdfilename);
  bool readHTML();
  bool readHTML(const std::string &fn);
  bool readBuffer(const std::string &);

  bool write() const;
  bool write(const std::string &fn);
  const std::string & writeBuffer() const;
  const CArray & writeCArray() const;

  bool xslt(const XSLTTree & xsltTree, const std::string & outputfile, ParameterMap & parameterMap);
  bool xslt(const XSLTTree & xsltTree, const std::string & outputfile);

  const std::string & getErrorString();
};

class Element
{
 private:
  typedef std::map<std::string, Attribute *, std::less<std::string> > AttributeMap;

  std::string _name;
  bool _is_content;
  std::string _content;
  int _line;
  ElementList _children;
  AttributeList _proplist;
  AttributeMap _prophash;


 public:
  Element(const std::string &);
  Element(const std::string &, int line);
  Element(const std::string &, const std::string &);
  Element(const Element & from); // copy constructor
  ~Element();

  Element & operator= (const Element & from);

  const std::string & name() const;
  void setName(const std::string & n);
  int line();

  // Element lists / addressing
  const ElementList & getElementList(const std::string & = std::string()) const;
  ElementList & getElementList(const std::string & = std::string());
  const Element *getElement(const std::string &n) const;
  Element *getElement(const std::string &);

  // add/remove Elements
  Element *addElement(const std::string &);
  Element *add
    (const Element & element); // create a copy of element and insert it
  Element *add
    (Element * elementP);      // add the pointer elementP -- libxmlccwrap will delete the object if necessary
  void removeElement(Element *);

  // content in Elements
  bool isContent() const;
  const std::string & getContent() const;
  const std::string & setContent(const std::string &);
  Element *addContent(const std::string & = std::string());
  Element *addContent(const int);

  // Attribute lists / addressing
  const AttributeList & getAttributeList() const;
  const Attribute *getAttribute(const std::string &n) const;
  Attribute *getAttribute(const std::string &);
  const Attribute *operator [](const std::string &) const;

  // add/remove Attributes
  Attribute *addAttribute(const std::string &, const std::string & = std::string());
  Attribute *addAttribute(const std::string &n, const int v);
  Attribute *add
    (const Attribute & attribute); // create a copy of attribute and insert it
  Attribute *add
    (Attribute * attributeP);      // add the pointer attributeP -- libxmlccwrap will delete the object if necessary
  void removeAttribute(const std::string &);
};


class Attribute
{
 private:
  std::string _name;
  std::string _value;
 public:
  Attribute(const std::string &n, const std::string &v = std::string())
    : _name(n), _value(v)
    { }
    ;
    Attribute(const std::string &n, const int v)
      : _name(n)
      {
	char buf[16];
	snprintf(buf,sizeof(buf),"%d", v);
	_value = buf;
      }
      ;
      const std::string & name() const;
      const std::string & value() const;
      const std::string & setValue(const std::string &v);
};


std::string isolat1ToUTF8(const std::string & inString);
std::string UTF8Toisolat1(const std::string & inString);

xmlDocPtr exportXMLDocPtr(const XMLTree & xmlTree);
// you have to call
//    xmlFreeDoc(doc);
// yourselve!

#endif
