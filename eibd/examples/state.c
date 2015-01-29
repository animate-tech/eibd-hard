/*
    EIBD client example 
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


#include "common.h"
#include <argp.h>
#include <stdarg.h>
#include <zlib.h>

/** option list */
static struct argp_option options[] = {
  {"threads", 't', 0, 0, 
   "dumps the thread state"},
  {"backends", 'b', 0, 0,
   "dump the EIBD backend states"},
  {"servers", 's', 0, 0, 
   "dump the EIBD server states"},
  {0}
};

static char doc[] = "EIBD state dump test";
static char args_doc[] = "URL";

struct arguments
{
  int op;
};

/** parses and stores an option */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
 struct arguments *arguments = (struct arguments *) state->input;
 if (key=='t' || key=='b' || key=='s') 
   {      
     arguments->op=key;
   }
 else
   return  ARGP_ERR_UNKNOWN;
 return 0;
}

/** information for the argument parser*/
static struct argp argp = { options, parse_opt, args_doc, doc };

int
main (int ac, char *ag[])
{
  unsigned char *buf = malloc(65535);
  unsigned char *decbuf = malloc(16*65535);
  int len = 65535;
  unsigned long declen = 16*65535;
  EIBConnection *con;
  int index;
  /** storage for the arguments*/
  struct arguments arg;
  int rc;
  int v;

  argp_parse (&argp, ac, ag, 0, &index, &arg);
  if (index > ac - 1)
    die ("arguments and backend URL expected, exiting");
  if (index < ac - 1)
    die ("unexpected parameter, exiting");

  con = EIBSocketURL (ag[ac-1]);
  if (!con)
    die ("Open failed");
 
  switch(arg.op) {
  case 't':
    len =  EIB_State_Threads(con, declen, decbuf);
    if (len == -1)
      die ("receiving thread states failed");
    break;
  case 'b':
    len =  EIB_State_Backends(con, len, buf);
    if (len == -1)
      die ("receiving backend states failed");

#if 0
    for (v=0; v<len; v++) { 
      printf("%03u ", (int) buf[v]);
      if (v%5==4) printf("\n");
    }
#endif

    rc =   uncompress(decbuf,&declen, buf, (unsigned long) len);
    /** decompress buffer */
    if (rc!=Z_OK)
      { 
	// printf("\nlen %d/ret %d\n",len,rc);
	die ("decompressing received buffer failed");
      }

    break;
  case 's':
    len =  EIB_State_Servers(con, len, buf);
    if (len == -1)
      die ("receiving client states failed");

    rc =   uncompress(decbuf,&declen, buf, (unsigned long) len);
    /** decompress buffer */
    if (rc!=Z_OK)
      { 
	// printf("\nlen %d/ret %d\n",len,rc);
	die ("decompressing received buffer failed");
      }
    
    break;
  default:
    die ("unknown operation"); 
    break;
  }
  printf("returned buffer len: %d\n", len);

  
  printf(decbuf);

  EIBClose (con);
  return 0;
}
