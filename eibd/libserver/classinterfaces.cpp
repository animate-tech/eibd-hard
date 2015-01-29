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

#include "common.h"

#if 0
/* explicit instantiation */
template <>
bool DroppableQueueInterface::Put_On_Queue_Or_Drop<CArray, CArray>(Queue < CArray > &queue,
								   CArray l2,
								   const char *dropmsg,
								   pth_sem_t  *sem2inc,
								   pth_sem_t  *emptysem2reset);

template <>
bool DroppableQueueInterface::Put_On_Queue_Or_Drop< LPDU *,  LPDU *>(Queue <  LPDU * > &queue,
								     LPDU * l2,
								     const char *dropmsg,
								     pth_sem_t  *sem2inc,
								     pth_sem_t  *emptysem2reset);

template <>
bool DroppableQueueInterface::Put_On_Queue_Or_Drop< LPDU *,  LPDU *>(Queue <  LPDU * > &queue,
								     LPDU * l2,
								     const char *dropmsg,
								     pth_sem_t  *sem2inc,
								     pth_sem_t  *emptysem2reset);

#endif

template <>
  bool DroppableQueueInterface::Put_On_Queue_Or_Drop(Queue < CArray > &queue,
						     CArray l2,
						     pth_sem_t  *sem2inc,
						     bool yield,
						     const char *dropmsg,
						     pth_sem_t  *emptysem2reset,
						     int yieldvalue)
{
  if (queue.put(l2)<0)
    {
      const char *dm = dropmsg ? dropmsg : "Unspecified driver: queue length exceeded, dropping packet";
      ERRORLOGSHAPE (l, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC,
    		  	  	  NULL, Logging::MSGNOHASH,
    		  	  	  dm);
    }
  else
    {
      pth_sem_inc(sem2inc, yield);
      if (emptysem2reset) {
    	  pth_sem_set_value (emptysem2reset, yieldvalue);
      }
      return true;
    }
  return false;
}

template <>
  bool DroppableQueueInterface::Put_On_Queue_Or_Drop(Queue < CArray* > &queue,
						     CArray* l2,
						     pth_sem_t  *sem2inc,
						     bool yield,
						     const char *dropmsg,
						     pth_sem_t  *emptysem2reset,
						     int yieldvalue)
{
  if (queue.put(l2)<0)
    {
      const char *dm = dropmsg ? dropmsg : "Unspecified driver: queue length exceeded, dropping packet";
      ERRORLOGSHAPE (l, LOG_ERR, Logging::DUPLICATESMAX1PER10SEC,
    		  	  	  NULL, Logging::MSGNOHASH,
    		  	  	  dm);
    }
  else
    {
      pth_sem_inc(sem2inc, yield);
      if (emptysem2reset) {
    	  pth_sem_set_value (emptysem2reset, yieldvalue);
      }
      return true;
    }
  return false;
}
