/* -*- Mode: C -*- */

/* gen-iterators.h -- */

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

/* Code generation for aggregate iterators (sets and arrays.)
 * Marco Antoniotti 19970105
 */

#ifndef COMPILER_GEN_ITER_H
#define COMPILER_GEN_ITER_H

#include <shift_config.h>

extern void
cons_generator(lv *e);

extern void
copy_set_generator(lv *e);

extern void
copy_array_generator(lv *e);

extern void
set_former_generator(lv* e);

extern void
array_former_generator(lv* e);

extern void
index_generator(lv *e);

extern void
generate_set_former_function(lv* set_former_expr);

extern void
generate_array_former_function(lv* array_former_expr);

extern void
generate_exists_function(lv* exists_expr);

extern void
generate_minmax_function(lv* minmax_expr, int);

extern void
generate_reduce_form_function(lv* reduce_form);

#endif /* COMPILER_GEN_ITER_H */

/* end of file -- gen-iterators.h -- */
