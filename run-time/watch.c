/* -*- Mode: C -*- */

/* watch.c -- */

/* 
 * Copyright (c)1996, 1997, 1998 The Regents of the University of
 * California (Regents). All Rights Reserved. 
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for educational, research, and not-for-profit
 * purposes, without fee and without a signed licensing agreement, is
 * hereby granted, provided that the above copyright notice, this
 * paragraph and the following two paragraphs appear in all copies,
 * modifications, and distributions.
 * 
 * Contact The Office of Technology Licensing, UC Berkeley, 2150
 * Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510)
 * 643-7201, for commercial licensing opportunities. 
 * 
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE. 
 *   
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
 * DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
 * REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS. 
 */

#ifndef SHIFT_RT_WATCH_I
#define SHIFT_RT_WATCH_I

#include <stdio.h>
#include <sys/time.h>
#include "watch.h"


struct timezone tz;

void
start_timer(Watch *watch)
{
  gettimeofday(&(watch->start_time), &tz);
}


void
increase_loop(Watch *watch)
{
  watch->loop++;
}


void
end_timer(Watch *watch)
{
  gettimeofday(&(watch->end_time), &tz);
}


void
get_time_diff(Watch *watch)
{
  watch->diff_time.tv_sec = watch->end_time.tv_sec - watch->start_time.tv_sec;
  watch->diff_time.tv_usec = watch->end_time.tv_usec - watch->start_time.tv_usec;

  if (watch->start_time.tv_usec > watch->end_time.tv_usec)
    {
      watch->diff_time.tv_usec += 1000000;
      watch->diff_time.tv_sec--;
    }
  watch->total_time
    = ((double)watch->diff_time.tv_sec + ((double)watch->diff_time.tv_usec / 1000000.));

}


void
reset_watch(Watch *watch)
{
  watch->total_time = 0;
}


#endif /* SHIFT_RT_WATCH_I */

/* end of file -- watch.c */
