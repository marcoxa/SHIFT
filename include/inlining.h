/* -*- Mode: C -*- */

/* inlining.h -- */

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

/* -*- Mode: C -*- */

/* inlining.h -- Macro tricks to use the GCC inline facilities with
 * other ANSI compilers.
 */

#ifndef INLINING_H
#define INLINING_H

/* If this is neither GNU C nor GNU C++ */
#if !defined(__GNUC__) || !defined(__GNUG__)

/* Is this an ANSI compiler? */
#ifdef __STDC__

/* Is this the SGI cc compiler? */
#ifndef __sgi
#define inline __inline__
#endif

#else  /* __STDC__ */
#define inline
#endif /* __STDC__ */

/* Is this GNU C but not GNU C++ and the '-ansi' flag was set? */
#elif defined(__GNUC__) && !defined(__GNUG__) && defined(__STRICT_ANSI__)
#define inline __inline__
#endif /* !defined __GNUC__ || !defined __GNUG__ */


#endif /* INLINING_H */

/* end of file -- inlining.h */
