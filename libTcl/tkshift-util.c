/* -*- Mode: C -*- */

/* tkshift-util.c -- */

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

#ifndef TKSHIFT_UTIL_I
#define TKSHIFT_UTIL_I

#include <shift_config.h>

#include <tcl.h>
#include "tkshift-util.h"

char *
tkshift_safe_malloc(int size)
{
  char * result = (char*) Tcl_Alloc(size);
  
  if (result == 0)
    {
      fputs("TkShift: error: cannot allocate memory.\n", stderr);
      fflush(stderr);
      exit(1);
    }
  memset(result, 0, size);
  return result;
}

void
tkshift_free(char* ptr)
{
  Tcl_Free((void*) ptr);
}


#endif /* TKSHIFT_UTIL_I */

/* end of file -- tkshift-util.c -- */
