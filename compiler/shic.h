/* -*- Mode: C -*- */

/* shic.h -- */

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

/* Original version by Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */

extern lv *Program;

void init_scanner();

extern lv *number_type, *discrete_number_type,
    *symbol_type, *logical_type, *event_type, *open_event_type,
    *error_type, *void_type, *state_type, *stateset_type,
    *differential_state_type, *mode_type, *flow_function_type,
    *integer_type, *component_type;

extern lv *all_entity, *nil_entity, *false_entity, *true_entity;

/* Constructors.
 */
lv *identifier(lv *name);

/* Utility routines.
 */
lv *find_leaf(lv *);
void internal_error(char *);
void user_error(lv *, char *, ...);
extern int n_errors;

/* Type routines.
 */
int equal_type(lv *, lv *);
int set_type_p(lv *);
int type_type_p(lv *);
int descendant(lv *, lv *);
int super_type_p(lv *, lv *);
lv* find_types_ancestor(lv*, lv*);
lv *type_of(lv *);
lv *meaning(lv *);

/* Type expressions.
 */
extern lv *number_type;

/* Static semantics.
 */
void initialize_check();
void patch_sync();
void one_roof();
void insert_dummy_state();
void check();
void patch_differential_variables(lv*);
void verbose_list_data_fields(lv*);
lv *rewrite_expression(lv *expr, lv *env);
lv *ns_find(lv *name, lv *name_space);

/* Transformations.
 */
void initialize_transform();
void transform(lv *);

/* Code Generation.
 */
void initialize_generate();
void generate(lv *program);
lv *fold(lv *);
extern FILE *cfile, *hfile;
extern char *cfilename, *hfilename;
extern int LHS;

#define AS_LHS(_lhs_form_) { LHS = 1; _lhs_form_; LHS = 0; }

/* Iterators.
 */
#define do_all_expressions(e, td, code) {				     \
    lv *setup = attr(@setup, td);					     \
    dolist (_m, attr(@discrete, td)) {					     \
	lv *e = attr(@invariant, _m);					     \
	if (e) code;							     \
    } tsilod;								     \
    if (setup) {							     \
	dolist (e, attr(@define, setup)) {				     \
	    code;							     \
	} tsilod;							     \
	dolist (e, attr(@do, setup)) {					     \
	    code;							     \
	} tsilod;							     \
	dolist (e, attr(@connections, setup)) {				     \
	    code;							     \
	} tsilod;							     \
    }									     \
    dolist (_m, attr(@flow,td)) {					     \
	dolist (e, args(_m)) {						     \
	    code;							     \
	} tsilod;							     \
    } tsilod;								     \
    dolist (_d, attr(@input, td)) {					     \
	lv *_t = tl(args(_d));						     \
	if (_t) {							     \
	    lv *e = hd(_t);						     \
	    code;							     \
	}								     \
    } tsilod;								     \
    dolist (_d, attr(@output, td)) {					     \
	lv *_t = tl(args(_d));						     \
	if (_t) {							     \
	    lv *e = hd(_t);						     \
	    code;							     \
	}								     \
    } tsilod;								     \
    dolist (_d, attr(@state, td)) {					     \
	lv *_t = tl(args(_d));						     \
	if (_t) {							     \
	    lv *e = hd(_t);						     \
	    code;							     \
	}								     \
    } tsilod;								     \
    dolist (_d, attr(@global, td)) {					     \
	lv *_t = tl(args(_d));						     \
	if (_t) {							     \
	    lv *e = hd(_t);						     \
	    code;							     \
	}								     \
    } tsilod;								     \
    dolist (_trans, attr(@transition, td)) {				     \
	lv *e = attr(@guard, _trans);					     \
	if (e) code;							     \
	dolist (e, attr(@define, _trans)) {				     \
	    code;							     \
	} tsilod;							     \
	dolist (e, attr(@do, _trans)) {					     \
	    code;							     \
	} tsilod;							     \
	dolist (e, attr(@connect, _trans)) {				     \
	    code;							     \
	} tsilod;							     \
    } tsilod;								     \
}
				       
/* end of file -- shic.h -- */
