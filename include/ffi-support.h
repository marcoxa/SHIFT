/* -*- Mode: C -*- */

/* ffi_support.h -- */

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

/* Support for the Foreign Function Interface.
 * Mostly copying routines.
 */

#ifndef ffi_support_h
#define ffi_support_h

#include<shift_config.h>

#include<collection.h>

extern void
shift_ffi_signal_filter(int sig);

extern double*
shift_ffi_cvt_D_array_1D_C(Array*);

extern int*
shift_ffi_cvt_I_array_1I_C(Array*);

extern double**
shift_ffi_cvt_D_array_2D_C(Array*);

extern int**
shift_ffi_cvt_I_array_2I_C(Array*);

extern void
shift_ffi_cvt_D_array_1D_shift(double*, Array**);

extern void
shift_ffi_cvt_I_array_1I_shift(int*, Array**);

extern void
shift_ffi_cvt_D_array_2D_shift(double**, Array**);

extern void
shift_ffi_cvt_I_array_2I_shift(int**, Array**);

#endif /* ffi_support_h */

/* end of file -- ffi-support.h -- */
