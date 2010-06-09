/* -*- Mode: C -*- */

/* gc_support.h -- */

/*
 * Copyright (c)1996, 1997, 1998 The Regents of the University of
 * California (Regents). All Rights Reserved. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research, and not-for-profit purposes,
 * without fee and without a signed licensing agreement, is hereby
 * granted, provided that the above copyright notice, this paragraph and
 * the following two paragraphs appear in all copies, modifications, and
 * distributions. 
 *
 * Contact The Office of Technology Licensing, UC Berkeley, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
 * commercial licensing opportunities. 
 *
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
 * ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION
 * TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS. 
 */

/* Support for the Boehm C Garbage Collector */

#ifndef gc_support_h
#define gc_support_h


/* The following definitions as per the instructions in the GC README file. */

#ifdef USE_GC_SUPPORT_P

/* Uncomment these lines if thread support is compiled in the
   collector.

#if defined OS_SOLARIS || defined OS_SUNOS5
#  define SOLARIS_THREADS
#endif

*/

#include <gc.h>

#define malloc(n) GC_malloc(n)
#define calloc(m,n) GC_malloc((m)*(n))

/* The following redefinition may turn out to be a performance hit
 * (according to the GC README file), but it makes the current
 * implementation a little safer and it is easy to implement.
 * The right thing to do would be to go arounf the code and conditionalize
 * with 'HAVE_GC_SUPPORT_P' all the calls to 'free'.
 *
 * Marco Antoniotti 19961207
 */
#define free(p) GC_free(p)

#endif /* USE_GC_SUPPORT_P */

#endif /* gc_support_h */

/* end of file -- gc_support_h -- */
