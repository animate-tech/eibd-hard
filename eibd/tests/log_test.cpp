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

#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
 
/** structure to store the arguments */
struct arguments
{
  /** trace level */
  int tracelevel;
  /** warning level */
  int warnlevel;
};
/** storage for the arguments*/
struct arguments arg = { 255, 255 };

/** parses and stores an option */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = (struct arguments *) state->input;
  switch (key)
    {
    case 't':
      arguments->tracelevel = (arg ? atoi (arg) : 0);
      break;
    case 'w':
      arguments->warnlevel = (arg ? atoi (arg) : 0);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/** aborts program with a printf like message */
void
die (const char *msg, ...)
{
  va_list ap;
  va_start (ap, msg);
  vprintf (msg, ap);
  printf ("\n");
  va_end (ap);

  exit (1);
}

static char doc[] = "logging subsystem unit test";

static char args_doc[] = "URL";

/** option list */
static struct argp_option options[] = {

  {"trace", 't', "LEVEL", 0, "set trace level"},
  {"warning", 'w', "LEVEL", 0, "set warning level"},
  {0}
};

/** information for the argument parser*/
static struct argp argp = { options, parse_opt, args_doc, doc };

int
main (int ac, char *ag[])
{
  int index;
  memset (&arg, 0, sizeof (arg));

  argp_parse (&argp, ac, ag, 0, &index, &arg);
 
  if (index < ac - 1)
    die ("unexpected parameter");
    
  struct timespec ts = { 0, 299999999 }; // around 1/3 sec or so
    
  Logs t;
  t.setTraceLevel (arg.tracelevel);
  
  pth_init ();
  
  // simple test for duplicates 
  for (int i=0; i<3; i++) {
      ERRORLOGSHAPE(&t, 1, Logging::DUPLICATESMAX10PERSEC, NULL,
                    Logging::MSGNOHASH, "error %d", 1);     
  }
  
  // now, let's see whether the maxpersec filter kicks in
  for (int i=0; i<7; i++) {
      ERRORLOGSHAPE(&t, 1, Logging::DUPLICATESMAX1PERSEC, NULL,
                    Logging::MSGNOHASH, "error-1 %d", 1); 
      ERRORLOGSHAPE(&t, 2, Logging::DUPLICATESMAX10PERSEC, NULL,
                    Logging::MSGNOHASH, "error-2 %d", 2);                                    
      ERRORLOGSHAPE(&t, 3, Logging::DUPLICATESMAX1PERSEC, NULL,
                    Logging::MSGNOHASH, "error-3 %d", 3);    
      nanosleep(&ts, NULL);                                                                  
  }
  
  // now, get the warn/trace tested as well
  for (int i=0; i<10; i++) {
    WARNLOGSHAPE(&t, 6, Logging::DUPLICATESMAX1PER10SEC, NULL,
                    Logging::MSGNOHASH, "warn-1 %d", 1);
    TRACEPRINTF(&t, 6, NULL, "trace-1");
    nanosleep(&ts, NULL);      
  }
  
  exit(0);
}
