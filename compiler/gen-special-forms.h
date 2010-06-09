/* -*- Mode: C -*- */

/* gen-special-forms.h -- */

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

/* Code generation.
 *
 * The strategy is brain-dead: use PRINTF statements directly from the
 * IR.
 *
 * Since most of what we generate is structure-typed variable
 * declarations and initialitions, one might extend the IR to include
 * them, and construct more stuff before emitting it.  The execution
 * overhead would probably be small.  However, it's not clear that the
 * extra machinery would pay off.
 *
 * No other obvious abstractions come to mind.
 *
 * Currently, name conventions are not done nicely.  They are
 * reproduced typically in at least two places (declaration and use),
 * so they should be expressed by #DEFINEs.  But maybe not.  Why name
 * names?
 */

#ifndef GEN_SPECIAL_FORMS_H
#define GEN_SPECIAL_FORMS_H

extern void
special_form_generator(lv* sf_form);

extern void
generate_special_form_function(lv* sf_form);

#endif /* GEN_SPECIAL_FORMS_H */

/* end of file -- gen-special-forms.h -- */
