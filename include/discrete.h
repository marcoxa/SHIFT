/* -*- Mode: C -*- */

/* discrete.h -- */

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

#ifndef DISCRETE_H
#define DISCRETE_H

#include <shift_config.h>
#include <gc_support.h>
#include "run-time.h"

/* For marking components.  Instead of unmarking, the MARK_NUMBER
 * global variable is increased before a new mark phase.
 */
static int mark_number = 0;
#define MARK(c) c->mark = mark_number
#define MARKED(c) (c->mark == mark_number)
#define NEW_MARK_PHASE() mark_number++

/* Transition space search machinery.
 * Each of the elements in the enumerations is a key to the
 * search/backtracking machinery that denotes a given action to be taken.
 */
typedef enum {
  SK_SINGLE,			/* Test of a given component-event couple */
  SK_SET,			/* Start pushing elements form a set
				 * on the search stack
				 */
  SK_BACKLINK,			/* Test whether a transition is
				 * 'backward-compatible'.
				 */
  SK_NO_SINGLE			/* Negative test for a single component-event
				 * couple.
				 */
} SearchKind;

typedef enum {
  BK_POP,
  BK_NEXT_ELEMENT,
  BK_NEXT_TRANSITION,
  BK_NEXT_BACKLINK_TRANSITION
} BacktrackKind;

/* Some space could be saved by using unions inside these structures.
 */
typedef struct SearchItem {
  SearchKind sk;		/* what to search */
  Component *c, *oc;		/* component and `other component' */
  char* e_name;			/* event name */
  EventDescriptor *e;		/* which event in component */
  GenericSet *s;		/* set search */
  int svo;			/* offset of sync var for set sync */
} SearchItem;

typedef struct BacktrackItem {
    BacktrackKind bk;
    Component *c, *oc;
    EventDescriptor *e;
    TransitionDescriptor **pt;
    GenericSet *s;
    SetCell *cell;
    int svo;
    int stack_position;
} BacktrackItem;

#define MAXC 1000
SearchItem search_stack[MAXC];
BacktrackItem bt_stack[MAXC];

extern int search_index;
extern int bt_index;

#define safe_inc(x) x++; assert(x < MAXC)
#define safe_dec(x) assert(x >= 0); x--

    
#define search_push(c_, e_) do {					     \
    safe_inc(search_index);						     \
    search_top().sk = SK_SINGLE;					     \
    search_top().c = c_;						     \
    search_top().e = e_;						     \
} while (0)

#define search_top() (search_stack[search_index])
#define search_pop() do { safe_dec(search_index); } while (0)
#define search_empty() (search_index == -1)
#define search_pop_to(x) search_index = x

#define search_set_push(c_, e_, s_, svo_) do {				     \
    safe_inc(search_index);						     \
    search_top().sk = SK_SET;						     \
    search_top().c = c_;						     \
    search_top().e = e_;						     \
    search_top().s = s_;						     \
    search_top().svo = svo_;						     \
} while (0)
    
#define search_backlink_push(c_, oc_, e_) do {				     \
    safe_inc(search_index);						     \
    search_top().sk = SK_BACKLINK;					     \
    search_top().c = c_;						     \
    search_top().oc = oc_;						     \
    search_top().e = e_;						     \
} while (0)

#define bt_pop() do { safe_dec(bt_index); } while (0)
#define bt_empty() (bt_index == -1)
#define bt_top() (bt_stack[bt_index])

#define bt_push(c_, e_) do {						     \
    safe_inc(bt_index);							     \
    bt_top().bk = BK_NEXT_TRANSITION;					     \
    bt_top().stack_position = search_index;				     \
    bt_top().c = c_;							     \
    bt_top().e = e_;							     \
} while (0)

#define bt_cell_push(c_, e_, s_, sc_, svo_) do {			     \
    safe_inc(bt_index);							     \
    bt_top().bk = BK_NEXT_ELEMENT;					     \
    bt_top().stack_position = search_index;				     \
    bt_top().c = c_;							     \
    bt_top().e = e_;							     \
    bt_top().s = s_;							     \
    bt_top().cell = sc_;						     \
    bt_top().svo = svo_;						     \
} while (0)

#define bt_backlink_push(c_, oc_, e_) do {				     \
    safe_inc(bt_index);							     \
    bt_top().bk = BK_NEXT_BACKLINK_TRANSITION;				     \
    bt_top().stack_position = search_index;				     \
    bt_top().c = c_;							     \
    bt_top().e = e_;							     \
    bt_top().oc_ = oc_;							     \
} while (0)

#define add_to_transition_set(c_, ptd_) do {				     \
    vid x;								     \
    x.cvp.c = c_;							     \
    x.cvp.p = ptd_;							     \
    add_to_set(x, potential_transitions);				     \
} while (0)


/* add_to_potential_trans_set --
 * This is just a renaming of the previous macro!
 * I did it because the name is less confusing.
 * Marco Antoniotti 19961010
 */
/*
#define add_to_potential_trans_set(c_, ptd_) do {			     \
    vid x;								     \
    x.cvp.c = c_;							     \
    x.cvp.p = ptd_;							     \
    add_to_set(x, potential_transitions);				     \
} while (0)
*/

/*
  BEWARE the following macro assumed you have declared "int i" 
*/

#define FOR_ALL_READY_COMPONENTS					     \
    for (i = 0; i <= bt_index; i++) {					     \
	if (bt_stack[i].bk == BK_NEXT_TRANSITION ||			     \
	    bt_stack[i].bk == BK_NEXT_BACKLINK_TRANSITION) {		     \
	    Component *c = bt_stack[i].c;				     \
	    TransitionDescriptor *d = *c->searching;
#define END_FARC    }}



#endif /* DISCRETE_H */

/* end of file -- discrete.h -- */
