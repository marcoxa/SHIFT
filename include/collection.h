/* -*- Mode: C -*- */

/* collection.h -- */
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

/* collection.h -- */


#ifndef COLLECTION_H
#define COLLECTION_H

#include <shift_config.h>

#include <sys/types.h>
#include <gc_support.h>
#include "inlining.h"
/* #include "const_ing.h" */

/*-----------------------------------------------------------------------------
 * Sets
 */

/* The CVP structure is used in the run-time library for sets of
 * (Component *, Something Else) pairs.  Typically: Component +
 * External Event descriptor, for backpointers; and Component +
 * Transition descriptor for active guards.
 */

typedef struct ComponentVoidp {
    Component *c;
    void *p;
} ComponentVoidp, cvp;

/* A union containing all possible set element types.
 */
typedef union vid {
    double d;
    int i;
    void *v;
    ComponentVoidp cvp;
} vid;

/* Sets are hash tables (for fast BELONGS test) with the elements
 * connected in a doubly-linked list (for fast traversal). The 
 * link is circular and the first cell is always empty. Hence the
 * test ending the traversal should be first_cell == current_cell.
 */
typedef struct SetCell {
    vid u;
    struct SetCell *previous, *next, *link;
} SetCell;

typedef struct GenericSet
{
  TypeDescriptor *et;		/* Element type descriptor */
  SetCell **table, *head;
  int size, *resize_sizep, *lengthp;

#ifdef HAVE_NEW_HASH_P
  int set_hash_id;		/* Unique id assigned to this set.
				 * Used for hashing.
				 */
#endif /* HAVE_NEW_HASH_P */
} GenericSet;


#ifndef COLLECTION_I

extern GenericSet *
new_set(TypeDescriptor *);

extern int
empty_set_p(GenericSet*);

extern int
belongs_to(void *, void *);

extern void *
set_union(void *, void *);

extern void *
set_intersection(void *, void *);

extern void *
set_difference(void *, void *);

extern int
set_equal(void *, void *);

extern void *
set_cons(TypeDescriptor *, int, ...);

/* Added header for intrinsic set support.
 * i.e. those sets that are generated with
 * the DOT notation.
 *
 * Tunc Simsek 19980528
 */
extern void *
intrinsic_set_cons(TypeDescriptor *);

/* Accessors which are compatible with the 'vid' union type */

extern double
double_first_element(GenericSet *);

extern int
int_first_element(GenericSet *);

extern cvp
cvp_first_element(GenericSet *);

extern void *
voidstar_first_element(GenericSet *);


/* Set modifiers */

extern void
add_to_set(vid, GenericSet *);

extern void
remove_from_set(vid, GenericSet *);

extern void
clear_set(GenericSet *s);


/* Set membership functions */

#ifndef HAVE_NEW_HASH_P

extern int
set_membership_I(int element, void* a_set);

extern int
set_membership_D(double element, void* a_set);

extern int
set_membership_A_S_C(void* a_s_c_element, void* a_set);

extern int
set_membership_CVP(void* a_cvp, void* a_set);

#endif /* HAVE_NEW_HASH_P */

#endif /* COLLECTION_I */


/* Useful macros for set manipulation */

#define add_to_component_set(c, s) do {					     \
    vid d_;								     \
    d_.v = c;								     \
    add_to_set(d_, (GenericSet *) s);					     \
} while (0);
    

#define remove_from_component_set(c, s) do {				     \
    vid d_;								     \
    d_.v = c;								     \
    remove_from_set(d_, (GenericSet *) s);				     \
} while (0);


#define FOR_ALL_ELEMENTS(d, set) do { if (!empty_set_p(set)) {		     \
    vid d;								     \
    GenericSet *_s = (set);						     \
    SetCell *_sc = _s->head->next;					     \
    for (; _sc != _s->head; _sc = _sc->next) {				     \
	d = _sc->u;

#define END_FAE }}} while (0)


#define DO_ALL_ELEMENTS(_m_e, _m_set, _m_type, _m_field) do           \
    {                                                                 \
      if (!empty_set_p(_m_set))                                       \
        {                                                             \
          GenericSet *_s = (_m_set);			              \
          SetCell *_sc = _s->head->next;			      \
          for (; _sc != _s->head; _sc = _sc->next) {		      \
	    _m_e = _sc->u._m_field;

#define END_DAE }}} while (0)



/*-----------------------------------------------------------------------------
 * Arrays
 */

typedef struct GenericArray
{
  TypeDescriptor *et;
  int l;
  char *data;

#ifdef HAVE_NEW_HASH_P
  int array_hash_id;		/* Unique id assigned to this array.
				 * Used for hashing.
				 */
#endif /* HAVE_NEW_HASH_P */
} GenericArray;


/* define specialized index functions for different types. */

#ifndef COLLECTION_I

#define def_index_decl(type)                                                 \
extern type *                                                                \
type ## _index(void*, int)

def_index_decl(double);
def_index_decl(int);
def_index_decl(voidstar);
def_index_decl(cvp);

#undef def_index_decl

extern void *
array_cons(TypeDescriptor *, int, ...);

extern GenericArray *
new_array(TypeDescriptor *, int);

extern void
new_array_low_level_alloc(GenericArray *, int, size_t);

extern void
ensure_array_allocation(GenericArray *, int);

extern void*
array_set(void *, int, void *);

#endif /* COLLECTION_I */

#endif /* COLLECTION_H */

/* end of file -- collection.h -- */
