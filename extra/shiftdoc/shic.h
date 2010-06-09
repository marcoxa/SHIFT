/* -*- Mode: C -*- */

/* shic.h -- */

/*
 * Copyright (c) 1996, 1997 Regents of the University of California.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and the California PATH Program.
 * 4. Neither the name of the University nor of the California PATH
 *    Program may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* Original version by Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */

extern lv *Program;

void init_scanner();

extern lv *number_type, *discrete_number_type,
    *symbol_type, *logical_type,
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
