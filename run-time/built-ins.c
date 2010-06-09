/* -*- Mode: C -*- */

/* built-ins.c -- */

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

/* Built in functions (mostly math)
 */

#ifndef BUILT_INS_I
#define BUILT_INS_I

#include <shift_config.h>

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include "shifttypes.h"
#include "collection.h"


void *
narrow(void *vct, void *vc)
{
  ComponentTypeDescriptor *ct1 = (ComponentTypeDescriptor *)vct;
  Component *c = (Component *)vc;
  ComponentTypeDescriptor *ct2 = c->desc;
  ComponentTypeDescriptor *pct = ct2;
  int found = 0;

  /* Check that CT1 is a parent type (or equal) of CT2. */
  if (pct != ct1)
    {
      while ( (pct->parent != NULL) && !found )
	{
	  found = pct->parent == ct1;
	  pct = pct->parent;
	}
      if (!found)
	runtime_error("incompatible types in narrow operator");
    }

  /* Return VC as a result. */
  return vc;
}


double 
max(double x, double y)
{
  return (x > y) ? x : y;
}


double 
min(double x, double y)
{
  return (x > y) ? y : x;
}

double
signum(double x)
{
  if (x == 0.0)
    return 0.0;
  else
    return (x < 0.0) ? -1.0 : 1.0;
}

double
frac(double x)
{
  return (x - (long) x);	/* Are we sure?!? Marco Antoniotti 19970102 */
}


/* Support for random generators */

#if defined (HAVE_DRAND48)

double
s_rand()
{
  return drand48();
}

#else

#ifdef OS_SUNOS4

/* SunOS 4.1.x does not have RAND_MAX in stdlib.h (AFAIK)
 * Marco Antoniotti 19971023
 */
static long rand_divisor = (32767 * 32767 * 2) + 1; /* 2^31 -1 */

#else

static long rand_divisor = RAND_MAX;

#endif /* OS_SUNOS4 */

double
s_rand()
{
  double i = (double)random();
  return i / rand_divisor;
}

#endif /* HAVE_DRAND48 */


#endif /* BUILT_INS_I */


/* end of file -- built-ins.c -- */
