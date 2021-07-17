/* -*- Mode: C -*- */

/* discrete.c -- */

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
/* -*- Mode: C -*- */

/* discrete.c -- Discrete phase. */

#ifndef DISCRETE_I
#define DISCRETE_I

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <shift_config.h>
#include "shifttypes.h"
#include "collection.h"
#include "shiftlib.h"
#include "discrete.h"


long shift_zeno_threshold = 100;
long nb_consecutive_transitions = 0;
int search_index = -1;
int bt_index = -1;
extern char Transition_Data[2000];
extern int Zeno_Warning;
extern char testing[30];
void push_sync_requirements(Component *c);


#ifndef HAVE_SHIFT_GUI_P
/* GUI_btf -- The following function is really a place holder for the graphic
 * environment entry point
 */
void
GUI_btf(char* ignore)
{
  fprintf(stderr, "SHIFT runtime: discrete.c %d:\n", __LINE__);
  fprintf(stderr, "SHIFT runtime: the graphic environment has not\n");
  fprintf(stderr, "               been installed.\n\n");
  exit(1);
}

/* GUI_btf2 -- The following function is really a place holder for the graphic
 * environment entry point
 */
void
GUI_btf2(char* ignore)
{
  fprintf(stderr, "SHIFT runtime: discrete.c %d:\n", __LINE__);
  fprintf(stderr, "SHIFT runtime: the graphic environment has not\n");
  fprintf(stderr, "               been installed.\n\n");
  exit(1);
}

#endif


/* add_to_potential_trans_set --
 * Made into a function!
 * Marco Antoniotti 19971030
 */
void
add_to_potential_trans_set(Component* c, TransitionDescriptor** ptd)
{
  vid x;

  x.cvp.c = c;
  x.cvp.p = ptd;
  add_to_set(x, potential_transitions);
}

/* Return 1 iff the component C is in the exit discrete state.
 * Alain Girault 19970201.
 */
int
in_exit_mode(Component *c)
{
  return !strcmp(c->M->name, "exit");
}


/* Remove the component C from the run-time environment.
 * Alain Girault 19970201.
 */
void
remove_component(Component *c)
{
  Component *curr;
  Component *prev;

  /* 1. Remove C from the list pointed by continuous_components */
  if (!continuous_components)
    {
      runtime_error("list of continuous components empty");
    }
  else if (continuous_components == c)
    {
      continuous_components = continuous_components->continuous_next;
      c->continuous_next = NULL;
    }
  else
    {
      prev = continuous_components;
      curr = prev->continuous_next;
      while (curr != c && curr != NULL)
	{
	  curr = curr->continuous_next;
	  prev = prev->continuous_next;
	}
      if (curr == c)
	{
	  prev->continuous_next = curr->continuous_next;
	  curr->continuous_next = NULL;
	}
      else
	{
	  runtime_error("component %d of type %s not found", 
			c->name, 
			c->desc->name);
	}
    }

  /* 2. Remove C from the list pointed by live_components */
  if (!live_components)
    {
      runtime_error("list of live components empty");
    }
  else if (live_components == c)
    {
      live_components = live_components->live_next;
      c->live_next = NULL;
    }
  else
    {
      prev = live_components;
      curr = prev->live_next;
      while (curr != c && curr != NULL)
	{
	  curr = curr->live_next;
	  prev = prev->live_next;
	}
      if (curr == c)
	{
	  prev->live_next = curr->live_next;
	  curr->live_next = NULL;
	}
      else
	{
	  runtime_error("component %d of type %s not found", 
			c->name, 
			c->desc->name);
	}
    }

  /* 3. Remove C from the list pointed by c->desc->component_first */
  if (!c->desc->component_first)
    {
      runtime_error("list of components of type %s empty", c->desc->name);
    }
  else if (c->desc->component_first == c)
    {
      c->desc->component_first = c->desc->component_first->component_next;
      c->component_next = NULL;
    }
  else
    {
      prev = c->desc->component_first;
      curr = prev->component_next;
      while (curr != c && curr != NULL)
	{
	  curr = curr->component_next;
	  prev = prev->component_next;
	}
      if (curr == c)
	{
	  prev->component_next = curr->component_next;
	  curr->component_next = NULL;
	}
      else
	{
	  runtime_error("component %d of type %s not found", 
			c->name, 
			c->desc->name);
	}
    }
}


/* Find the exported event belonging to CTD and having name EVENT_NAME.
 * Marco Antoniotti, Alain Girault 19970114.
 */
EventDescriptor*
find_exported_event(char* event_name, 
		    ComponentTypeDescriptor* ctd)
{
  EventDescriptor** event_list = ctd->events;

#ifdef INHERIT_DEBUG
  printf("\nexported event name %s\n", event_name);
#endif /* INHERIT_DEBUG */
  for (; *event_list ; event_list++)
    {
#ifdef INHERIT_DEBUG
      printf("\nchecking event %s at %x\n", (*event_list)->name, *event_list);
#endif /* INHERIT_DEBUG */
      if (!strcmp(event_name, (*event_list)->name))
	{
	  return *event_list;
	}
    }
  return NULL;
}


/* Find a possible transition in C with event E from the transition
 * array PTD.
 */
TransitionDescriptor**
find_transition(Component* c,
		EventDescriptor* e,
		TransitionDescriptor** ptd)
{
  /* Find next possible transition.
   */
  for (; *ptd; ptd++)
    {
      EventDescriptor **ep = (*ptd)->events;
      for (; *ep; ep++)
	{
	  if (*ep == e)
	    break;
	}
      if (! *ep)
	continue;
      if ((*ptd)->guard && ! ((*ptd)->guard)(c))
	{
	  /* A guard is blocking this transition: remember it. */
	  add_to_potential_trans_set(c, ptd);
	}
      else
	break;
    }
  return *ptd? ptd : 0;
}


/* Return true if transition TD in component C must (and can) be
 * synchronized to event E in component OC.
 *
 * Log:
 * Corrected the two instances of DATAITEM accesses.  Was previously 
 * using 'c' to access the linked variable C in C:E.  Now we are 
 * using 'cc' (i.e. the container class) to access the linked variable.
 *
 * Tunc Simsek 8th October, 1997
 *
 * The synchronization bug has been finally fixed. Previously the
 * following code wouldn't synchronize:
 *
 * global set(A) S := {create(A)};
 *
 * type A
 * {
 *   export alpha;
 *   discrete s1, s2;
 *   transition
 *     s1 -> s2 {alpha, S:alpha(one)};
 * }
 *
 * Marco Zandonadi 19971023
 */
int
backward_compatible_transition(TransitionDescriptor* td,
			       Component* c,
			       Component* oc,
			       EventDescriptor* e)
{
  ExternalEventDescriptor** eed = td->sync_list;
  ExternalEventDescriptor* current_ee;
  int link;
  Component* cc;
  external_event_expr ext_event_expression;

  /* No point in agonizing.
   */
  /* Instead we must agonize! It looks like when backtracking the
   * pointer is not properly reset.  Hence the backtracking fails to
   * consider the next transition.
   *
   * Marco Antoniotti 19961015
   */
  /* Weeeell maybe we need this anyway */
  if (c->searching && *c->searching != td) return 0;

  /* current_ee = find_external_event_named(e->name, */

  for (; *eed; eed++)
    {
      link = (*eed)->link_var_offset;
      ext_event_expression = (*eed)->ext_ev_expr; /* A nice function */

      /* Old code:
       *
       * if ((*eed)->event != e)
       *   continue;
       *
       * It probably needs a major rewriting, but the substitution
       * works in the context of 'backward_compatible_transitions'.
       * Marco Antoniotti, Alain Girault 19970114.
       */

      if (strcmp((*eed)->name, e->name))
	continue;

      cc = (*eed)->global? _global : c;

      if ((*eed)->connection_type == SINGLE_CONNECTION)
	{
	  if (oc == (Component*) ext_event_expression(c))
	    break;
	}
      else if ((*eed)->connection_type == SET_CONNECTION)
	{
	  if (c->searching == 0)
	    {
#ifdef HAVE_NEW_HASH_P
	      if (belongs_to(oc, (GenericSet*)ext_event_expression(c)))
#else
	      if (belongs_to(&oc, (GenericSet*)ext_event_expression(c)))
#endif /* HAVE_NEW_HASH_P */
		{
		  if ((*eed)->sync_type == SYNC_ALL)
		    break;
		  else
		    {
		      /* The previous version of the code was not
		       * discriminating among all the possible cases.
		       * In particular, when we have a SYNC_ONE
		       * transition checked in a component for the
		       * first time, we try to actually set its sync variable
		       * to the 'other component', assuming that it
		       * will be the one in the transition.
		       *
		       * This requires changes in 'push_sync_requirements'
		       * (where we need to stop the search) and in
		       * 'recompute_backpointers' (where we need to
		       * clean up this value.
		       *
		       * Marco Antoniotti, Aleks Gollu 19961010
		       */

		      /* This must be left as it is, since the
		       * 'sync_var_offset' contains the variable bound
		       * by the set synchronization.
		       *
		       * Marco Antoniotti 19970520
		       */
		      DATA_ITEM(Component*, c, (*eed)->sync_var_offset) = oc;
		      break;
		    }
		}
	    }
	  else if ((*eed)->sync_type == SYNC_ALL)
	    {
#ifdef HAVE_NEW_HASH_P
	      if (belongs_to(oc, (GenericSet*)ext_event_expression(c)))
		break;
#else
	      if (belongs_to(&oc, (GenericSet*)ext_event_expression(c)))
		break;
#endif /* HAVE_NEW_HASH_P */
	    }
	  else
	    {
	      /* I would prefer to defer this to the main search loop,
	       * but the mutual exclusion needs to be checked somewhere.
	       */

	      /* Added the oc == c test. This prevents the
	       * synchronization bug from happening.
	       * 19971023 Marco Zandonadi 
	       */
	      if (oc == DATA_ITEM(Component *, c, (*eed)->sync_var_offset)
		  || oc == c)
		break;
	    }
	}
      else
	runtime_error("unknown connection type");
    }
  return *eed? 1 : 0;
}


/* Find a possible transition in C from the transition array PTD and
 * with OC:E as an external event.  Return that transition or NIL.
 */
TransitionDescriptor**
find_backlink_transition(Component* c,
			 Component* oc,
			 EventDescriptor* e,
			 TransitionDescriptor** ptd)
{
  /* Find next possible transition.
   */
  for (; *ptd; ptd++)
    {
      if (backward_compatible_transition(*ptd, c, oc, e))
	{
	  if ((*ptd)->guard && ! ((*ptd)->guard)(c))
	    {
	      /* Monitor this guard in the future. */
	      add_to_potential_trans_set(c, ptd);
	    }
	  else
	    break;
	}
    }
  return *ptd? ptd : 0;
}


/* This function performs a search on the backtrack stack 
 * that resulted from "int search()".  Its purpose is 
 * to find a transition that contains an open event
 * and to see if there is another transition that closes
 * that open event.
 *
 * Tunc Simsek 16th December, 1997
 */
int
open_events_matched_p()
{
  int i; /* Define this integer for use with the
	  * FOR_ALL_READY_COMPONENTS macro.
	  */
  int search_result = 0;
  GenericSet *open_components = new_set(&cvp_type);
  GenericSet *closure_components = new_set(&cvp_type);
  void add_cvp_to_set(Component *c, 
		      char *e, /* Using the event name only, not
				* the entire descriptor.
				*/ 
		      GenericSet *S);


  /* One important comment is of order here:
   *
   * In the code below you will see that the sets under
   * construction are constructed of
   *       
   *     ( Component *c, char *e ) pairs.
   *
   * It is important to realize that both of these 
   * elements are in a sense "hashed" or "static".  This
   * is important because "sets" are modified hash-tables
   * and the latter in the above pair is only checked 
   * for Memory Address value.  Thus if you use something
   * that is not static or hashed you will get yourself
   * in trouble.
   */
  FOR_ALL_READY_COMPONENTS {
    EventDescriptor **e = d->events;
    ExternalEventDescriptor **ee = d->sync_list;

    for (; *e; e++)
      {
	/* First we find the (component, event) pairs such that
	 * event is a local and open event.  We store these in the 
	 * set "open_components".  Note that a set is a hashed
	 * creature so there is no duplication.
	 */
	char *name = (*e)->name;

	if ((*e)->type == OPEN_EVENT_T)
	  add_cvp_to_set(c, name, open_components);
      }

    for (; *ee; ee++)
      {
	/* Next we find all the (component, event) pairs such
	 * that event is an external and open event of component.
	 * Once again we store these in the set "closure_components".
	 * Note that a set is hashed so that there are no duplicates.
	 */
	EventDescriptor *e = (*ee)->event;
	external_event_expr ext_ev_expr = (*ee)->ext_ev_expr;
	char *name = e->name;

	if (e->type == OPEN_EVENT_T)
	  {

	    switch((*ee)->connection_type)
	      {
		Component *oc;
		
	      case SINGLE_CONNECTION:
		
		if((oc = (Component*) ext_ev_expr(c)) != c)
		  add_cvp_to_set(oc, 
				 name, 
				 closure_components); 
		break;
	      case SET_CONNECTION:
		
		switch((*ee)->sync_type)
		  {
		  case SYNC_ONE:
		    
		    if((oc = DATA_ITEM(Component *,
				       c,
				       (*ee)->sync_var_offset)) != c)
		      add_cvp_to_set(oc,
				     name, 
				     closure_components); 
		    break;
		  case SYNC_ALL:
		    
		    FOR_ALL_ELEMENTS (el, (GenericSet*) ext_ev_expr(c))
		      {
			if((oc = el.cvp.c) != c)
			  add_cvp_to_set(oc, 
					 name, 
					 closure_components); 
		      } END_FAE;
		    break;
		  }
		break;
	      }
	  }
      }
  } END_FARC;


  /* Now we take the difference of the two sets computed above
   * to see if there are any "open" local events that do not
   * have an "open" external event closing them.
   */
  if (empty_set_p((GenericSet*) set_difference(open_components, 
					       closure_components)))
    {
      search_result = 1;
    }
  else
    {
      search_result = 0;
    }

  clear_set(open_components);
  clear_set(closure_components);
  free(open_components);
  free(closure_components);
  
  return search_result;
}

void
add_cvp_to_set(Component *c, 
	       char *e, 
	       GenericSet *S)
{
  vid *v = new(vid);
  ComponentVoidp *cvp = &v->cvp;
  
  cvp->c = c;
  cvp->p = (void *) e;
  add_to_set(*v , S);
}

/* Search for a possible synchronization of a set of components.
 * (stack-based depth-first search with backtracking).
 *
 * The algorithm uses two stacks, the search stack and the backtrack
 * stack.
 *
 * The search stack contains SearchItems.  A SearchItem is a
 * condition to be satisfied for the success of the search.
 *
 * The backtrack stack contains information for resuming the search
 * when the current search tree branch fails.
 */
int
search()
{
continue_search:

  while (! search_empty())
    {
      SearchKind sk = search_top().sk;
      Component *c = search_top().c;
      EventDescriptor *e = search_top().e;
      GenericSet *s = search_top().s;
      Component *oc = search_top().oc;
      int svo = search_top().svo;
      search_pop();

      switch (sk)
	{
	case SK_SINGLE:
	  {
	    /* Begin searching c, e.
	     */

	    if (c)
	      {   /******MAK***  if not c then there is no transition (nil) */

		TransitionDescriptor **pt = e->transitions;
		if (c->searching)
		  {
		    /* The search state already includes this component.
		     * See if the transition produces this event.
		     */
		    for (; *pt; pt++)
		      if (*pt == *c->searching) break;

		    if (*pt)
		      continue;
		    else
		      {
			/* bt_push(c, e); */
			/* Just added. Marco Antoniotti */
			search_push(c, e);
			goto backtrack;
		      }
		  }
		else
		  {
		    /* Unexplored component.  Find the first possible
		     * transition, or backtrack.
		     */
		    c->searching =  find_transition(c, e, c->M->outgoing);
		    if (c->searching)
		      {
			bt_push(c, e);
			push_sync_requirements(c);
			continue;
		      }
		    else
		      search_push(c, e); /* Aleks 6pm ??? */
		      goto backtrack;
		  }
	      } /* if (c) */
	    else
	      goto backtrack;
	  }

	case SK_SET:
	  {
	    /* Begin searching a set with SYNC_ONE. Remember that the
	     * event pushed on the stack may be incorrect because of
	     * inheritance. We have to retrieve the correct exported
	     * event through the name. Note that event though the
	     * exported event may be incorrect, its name is the good one.
	     * Marco Antoniotti, Alain Girault 19970114.
	     */

	    if (!empty_set_p(s))
	      { 
		/*******MAK  if not s then there is no transition (nil) */

		SetCell *sc = s->head->next;
		Component *first = (Component *) sc->u.v;
		EventDescriptor* good_e = find_exported_event(e->name, 
							      first->desc);
		assert(good_e != NULL);
		bt_cell_push(c, good_e, s, sc, svo);

		/* Original (Marco Antoniotti 19960524)

		   DATA_ITEM(Component *, c, svo) = first;

		   */

		/* Begin Modification (Marco Antoniotti 19960524) */
		if (svo != -1)
		  {
		    /* In this case we had a set connection with SYNC_ONE
		     * and sync_var_offset == -1.
		     * This means that there is no variable which must
		     * hold the chosen component.
		     * I take that this should work.
		     */
		    DATA_ITEM(Component *, c, svo) = first;
		  }
		/* End Modification (Marco Antoniotti 19960524) */

		search_push((Component *) first, good_e);
		continue;
	      }
	    else
	      goto backtrack; /* Crying out for a search_push... */
	  }
	  
	case SK_BACKLINK:
	  {
	    if (c)
	      { /*******MAK*** if not c there is no transition */

		if (c->searching)
		  {
		    if (backward_compatible_transition(*c->searching, c, oc, e))
		      continue;
		    else
		      search_backlink_push(c, oc, e); /* Aleks 6pm */
		      goto backtrack;
		  }
		else
		  {
		    /* Begin searching a backlinked component.  Must find
		     * a transition in C that explicitly synchronizes with
		     * event E in component OC.
		     */
		    c->searching =
		      find_backlink_transition(c, oc, e, c->M->outgoing);
		    if (c->searching)
		      {
			bt_backlink_push(c, oc, e);
			push_sync_requirements(c);
			continue;
		      }
		    else
		      {
			search_backlink_push(c, oc, e);
			goto backtrack;
		      }
		  }
	      } /* if (c) */
	  }

	case SK_NO_SINGLE:
	  break;

	default:
	  assert(0);
	}
    }
  return (int) 1;

  /* Backtrack until a new choice is available.
   */
backtrack:

  while (! bt_empty())
    {
      BacktrackKind bk = bt_top().bk;
      Component *c = bt_top().c;
      Component *oc = bt_top().oc;
      EventDescriptor *e = bt_top().e;
      TransitionDescriptor **pt = bt_top().pt;
      GenericSet *s = bt_top().s;
      SetCell *cell = bt_top().cell;
      int svo = bt_top().svo;
      TransitionDescriptor** next_transitions = 0;

      switch (bk)
	{
	case BK_NEXT_ELEMENT:
	  /* Backtrack to the next set element, if possible.
	   */
	  cell = cell->next;
	  if (cell != s->head)
	    {
	      Component *next = (Component *) cell->u.v;
	      DATA_ITEM(Component *, c, svo) = next;
	      search_pop_to(bt_top().stack_position);
	      search_push(next, e);
	      bt_top().cell = cell;
	      goto continue_search;
	    }
	  else
	    {
	      /* The set is exhausted.  Place set back on search
	       * stack and continue backtracking.
	       */
	      search_set_push(c, e, s, svo);
	      /* WE SHOULD CLEAN THIS UP!
	       * Since the set is exhausted, we want to be sure to
	       * clear the Sync Var in component.
	       * DATA_ITEM(Component*, c, svo) = 0;
	       *
	       * Marco Antoniotti, Aleks Gollu 19970212
	       */
	      DATA_ITEM(Component*, c, svo) = 0;
	      bt_pop();
	      continue;
	    }

	case BK_NEXT_TRANSITION:
	  /* Before trying next transition, remove from search stack
	   * all searches which depend on this one.
	   */
	  search_pop_to(bt_top().stack_position);
	  next_transitions = c->searching + 1;
	  c->searching = 0;
	  c->searching = find_transition(c, e, next_transitions);
	  if (c->searching)
	    {
	      push_sync_requirements(c);
	      goto continue_search;
	    }
	  else
	    {
	      search_push(c, e);
	      bt_pop();
	      continue;
	    }

	case BK_NEXT_BACKLINK_TRANSITION:
	  /* We need to clean up the 'sync_var_offset' */
	  search_pop_to(bt_top().stack_position);
	  next_transitions = c->searching + 1;
	  c->searching = 0;
	  c->searching = find_backlink_transition(c, oc, e, next_transitions);
	  if (c->searching)
	    {
	      push_sync_requirements(c);
	      goto continue_search;
	    }
	  else
	    {
	      search_backlink_push(c, oc, e);
	      bt_pop();
	      continue;
	    }

	default:
	  assert(0);
	}
    }
  /* Nothing left to backtrack: failure.
   */
  /* printf("SHIFT DEBUG: 'search' about to return 0\n"); */
  return (int) 0;
}
	

/* Add C, D to the current search state.
 */
void
push_sync_requirements(Component *c)
{
  ExternalEventDescriptor** eep;
  EventDescriptor** ep;
  TransitionDescriptor* d = *c->searching;
  external_event_expr ext_event_expression;

  /* Add external events from sync list.
   */
  for (eep = d->sync_list ; *eep ; eep++)
    {
      int o = (*eep)->link_var_offset;
      int g = (*eep)->global;
      Component* cc = g ? _global : c;
      EventDescriptor* ed = (*eep)->event;
      ext_event_expression = (*eep)->ext_ev_expr; /* A nice function */

      switch ((*eep)->connection_type)
	{
	case SINGLE_CONNECTION:
	  {
	    /* We need to push the event in 'x' corresponding to the
	     * external declaration in the 'sync_list'. However, the
	     * type of 'x' varies at runtime because of inheritance.
	     * Therefore, we need to find the proper event in a
	     * dynamic way. We find it by matching the name of the
	     * exported event *EEP.
	     * Marco Antoniotti, Alain Girault 19970114.
	     */

	    Component* x = (Component*) ext_event_expression(c);
	    if (x == NULL)
	      search_push(x, NULL);
	    else
	      {
		EventDescriptor* ed = find_exported_event((*eep)->name,
							  x->desc);

		if (ed == NULL)
		  {
		    runtime_warning("push_sync_requirements:%d:", __LINE__);
		    runtime_warning("exported event %s not found in type %s",
				    (*eep)->name,
				    x->desc->name);
		  }
		else
		  search_push(x, ed);
	      }
	    break;
	  }

	case SET_CONNECTION:
	  switch ((*eep)->sync_type)
	    {
	    case SYNC_ALL:
	      /* In this case we simply push all the components in the
	       * set as if they were SK_SINGLE.
	       * Marco Antoniotti 19960718.
	       */

	      /* As for SINGLE_CONNECTION, we find the correct external
	       * event at run time by matching the name of the exported
	       * event *EEP.
	       * Marco Antoniotti, Alain Girault 19970114.
	       */
	      
	      FOR_ALL_COMPONENTS(y, (GenericSet*)ext_event_expression(c)) {
		EventDescriptor* ed = find_exported_event((*eep)->name,
							  y->desc);
		if (ed == NULL)
		  {
		    runtime_warning("push_sync_requirements:%d:", __LINE__);
		    runtime_warning("exported event %s not found in type %s",
				    (*eep)->name,
				    y->desc->name);
		  }
		else
		  search_push(y, ed);
	      } END_FAC;
	      break;

	    case SYNC_ONE:
	      {
		/* The 'search_set_push' macro is actually used only for
		 * SYNC_ONE connections.  Hence, the SK_SET search kind
		 * can be safely assumed to refer only to these types of
		 * connections.
		 * Marco Antoniotti and Alecs Gollu 19960719
		 */

		/* See also comment in 'backward_compatible_transition'
		 * Marco Antoniotti, Aleks Gollu 19961010
		 */

		/* Here we keep the old code where the event pushed is
		 * the one linked to the exported event *EEP at compile
		 * time by SHIC. We do that because we don't have the
		 * component type that exports the event we are looking
		 * for. Later, when dealing with the SK_SET case in the
		 * SEARCH function, we will retrieve the correct
		 * exported event from the "right" component type.
		 * Marco Antoniotti, Alain Girault 19970114.
		 */

		Component* x = DATA_ITEM(Component*, 
					 c, 
					 (*eep)->sync_var_offset);

		/* Push only if the not yet found a proper component */
		if (x == NULL)
		  search_set_push(c,
				  ed,
				  (GenericSet*)ext_event_expression(c),
				  (*eep)->sync_var_offset);
	      }
	      break;

	    default:
	      runtime_error("unknown sync type.");
	    }
	  break;

	default:
	  runtime_error("unknown sync type.");
	}
    }
  /* Add backpointers for local events.
   */
  for (ep = d->events ; *ep ; ep++)
    {
      GenericSet *s = DATA_ITEM(GenericSet*, c, (*ep)->backlink_var_offset);
      FOR_ALL_COMPONENTS(k, s) {
	search_backlink_push(k, c, *ep);
      } END_FAC;
    }
}


/* Find if component C can take transition D.
 */
int
transitable(Component* c, TransitionDescriptor** pd)
{
  int search_result = 0;
  TransitionDescriptor** trans_descs = pd;
  TransitionDescriptor* trans_desc;
  void clear_transitions_bindings(Component*, TransitionDescriptor*);

  for (trans_desc = *trans_descs ; *trans_descs ; trans_descs++)
    clear_transitions_bindings(c, trans_desc);

  bt_index = -1;
  search_index = -1;
  c->searching = pd;
  push_sync_requirements(c);
  search_result = search();

  /* printf("SHIFT DEBUG: 'search' returned %d.\n", s); */

  if (search_result != 0)
    {
      bt_push(c, 0); /* for taking transitions */
      /* Test whether all the 'open' events are
       * "matched" (or closed) by a transition in another instance which
       * uses them as 'external'.
       * If true, then the search succeeds, otherwise it fails.
       */
      if (open_events_matched_p())
	{
	  /* printf("SHIFT DEBUG: 'search' about to return 1\n"); */
	  search_result = 1;
	}
      else
	{
	  search_result = 0;
	  /* Will also clear the bt stack here !!!
	   */
	}
    }
  return search_result;
}


void
take_transitions()
{
  int i;

  /* Increment the number of transitions taken since the last discrete
   * step and check for zeno behavior. Note that because of the ==
   * test, we stop only the first time we hit the threshold. We then
   * give the hand to the user but no further warnings will be given.
   */
  nb_consecutive_transitions++;
  if (nb_consecutive_transitions == shift_zeno_threshold)
    {
      if (shift_gui_debug)
	{
	  Zeno_Warning = 1;	/* This flag tells serverListen to stop*/
	  serverListen();
	}
      else if (shift_debug)
	{
	  printf("WARNING: possible zeno behavior\n");
	  userInteract();
	}
    }

  /* Call the exiting function before doing anything else.
   * Marco Antoniotti 19961209
   */
  FOR_ALL_READY_COMPONENTS {
    c->M->exiting_function(c);
  } END_FARC;

  if (before_transition_function)
    {
      FOR_ALL_READY_COMPONENTS {
	before_transition_function(c, d);
      } END_FARC;
    }

  if (shift_gui_debug)
    GUI_btf2(Transition_Data);
  else if (shift_debug)
    btf();
  /* Peter Cooke 1-5-98 Database trasition output  */
    DB_btf();

  FOR_ALL_READY_COMPONENTS {
    if (d->define_f)
      d->define_f(c);
  } END_FARC;

  FOR_ALL_READY_COMPONENTS {
    if (d->f1)
      d->f1(c);
  } END_FARC;

  /* Go to new mode. */
  FOR_ALL_READY_COMPONENTS {
    d->to->entering_function(c);
  } END_FARC;

  FOR_ALL_READY_COMPONENTS {
    if (d->f2)
      d->f2(c);
  } END_FARC;

  FOR_ALL_READY_COMPONENTS {
    c->searching = 0;
  } END_FARC;

  if (after_transition_function)
    {
      FOR_ALL_READY_COMPONENTS {
	after_transition_function(c, d);
      } END_FARC;
    }

  if (shift_gui_debug)
    {
      /* Call GUI_atf() */
    }
  else if (shift_debug)
    {
      /* Call atf() */
    }
}


/* Given that the guard for transition descriptor D is satisfied in
 * component C, take the transition if it can be taken.
 */
int
attempt_transition(Component *c, TransitionDescriptor **pd)
{
  /* printf("\n[SHIFT DEBUG] attempting transitions from %s %d\n",
   *	 c->desc->name,
   *	 c->name);
   */
  if (transitable(c, pd))
    {
      take_transitions();
      return 1;
    }
  else
    {
      c->searching = 0;
      return 0;
    }
}


void
clear_transitions_bindings(Component* c, TransitionDescriptor* trans_desc)
{
  ExternalEventDescriptor** eeds = trans_desc->sync_list;
  ExternalEventDescriptor* eed;

  for (; *eeds ; eeds++)
    {
      eed = *eeds;
      if (eed->connection_type == SET_CONNECTION
	  && eed->sync_type == SYNC_ONE)
	DATA_ITEM(Component*, c, eed->sync_var_offset) = 0;
    }
}


void
recompute_backpointers()
{
  Component* c = 0;
  TransitionDescriptor** ptd = 0;
  EventDescriptor** ped = 0;
  ExternalEventDescriptor** eeds;
  ExternalEventDescriptor* eed;
  ModeDescriptor** mp;

  /* Clear backpointer sets. */
  for (c = live_components ; c ; c = c->live_next)
    {
      if (c->M)
	{
	  ptd = c->M->outgoing;
	  for ( ; *ptd ; ptd++)
	    {
	      ped = (*ptd)->events;
	      for ( ; *ped ; ped++)
		clear_set(DATA_ITEM(GenericSet*,
				    c,
				    (*ped)->backlink_var_offset));

	      /* The next cleanup operation is done because of the new
	       * treatment of 'SYNC_ONE' connections in
	       * 'push_sync_requirements' and
	       * 'backward_compatible_transition'.
	       *
	       * Marco Antoniotti and Aleks Gollu 19961010
	       */
	      eeds = (*ptd)->sync_list;
	      for ( ; *eeds ; eeds++)
		{
		  eed = *eeds;
		  if (eed->connection_type == SET_CONNECTION
		      && eed->sync_type == SYNC_ONE)
		    DATA_ITEM(Component*, c, eed->sync_var_offset) = 0;
		}
	    }
	}
    }
  /* Recompute new backpointer sets of interest. */
  for (c = live_components ; c ; c = c->live_next)
    {
      if (c->M)
	{
	  mp = c->desc->modes;

	  /* printf("SHIFT DEBUG: live component: (%s %d)\n", c->desc->name,
	     c->name); */

	  for ( ; *mp != 0; mp++)
	    {
	      ModeDescriptor* mode = *mp;
	      TransitionDescriptor** ptd = mode->outgoing;

	      for ( ; *ptd ; ptd++)
		{
		  ExternalEventDescriptor** ped = (*ptd)->sync_list;

		  for ( ; *ped ; ped++)
		    {
		      ExternalEventDescriptor* ed = *ped;
		      int ct = ed->connection_type;
		      int lvo = ed->link_var_offset;
		      int g = ed->global;
		      Component* cc = g ? _global : c;
		      EventDescriptor* ev = ed->event;
		      external_event_expr ext_event_expression
			= (*ped)->ext_ev_expr;

		      if (ct == SINGLE_CONNECTION)
			{
			  Component* x = (Component*) ext_event_expression(c);

			  if (x != NULL && x != cc)
			    /* if x == cc external_event reduces to
			       local event Aleks Jan 25 */

			    {
			      GenericSet* bps;
			      EventDescriptor* good_ev;
#ifdef INHERIT_DEBUG
			      printf("\ntype of x = %s\n", x->desc->name);
			      printf("\nold event adress is %x\n", ev);
#endif /* INHERIT_DEBUG */
                              /* The exported event linked at compile
			       * time by the SHIC compiler may be
			       * incorrect. We find the good one at run
			       * time by matching the name of the
			       * external event ED.
			       * Marco Antoniotti, Alain Girault 19970114.
			       */
                              good_ev = find_exported_event(ed->name, x->desc);
			      bps = DATA_ITEM(GenericSet*,
					      x,
					      good_ev->backlink_var_offset);
			      add_to_component_set(c, bps);
			    }
			}
		      else /* SET_CONNECTION */
			{
			  /* As for SINGLE_CONNECTION, we find the
			   * correct external event at run time by
			   * matching the name of the exported event
			   * ED.
			   * Marco Antoniotti, Alain Girault 19970114.
			   */

			  GenericSet* s
			    = (GenericSet*) ext_event_expression(c);
			  EventDescriptor* good_ev;

			  if (s != NULL && !empty_set_p(s))
			    {
			      FOR_ALL_COMPONENTS (x, s) {
				/* if x == cc external_event reduces to
				   local event Aleks Jan 25 */
				if (x != cc) {
				  GenericSet* bps;
				  EventDescriptor* good_ev;

				  good_ev = find_exported_event(ed->name, 
								x->desc);
				  bps = DATA_ITEM(GenericSet*, 
						  x,
						  good_ev->backlink_var_offset);
				  add_to_component_set(c, bps);
				}
			      } END_FAC;
			    }
			}
		    }
		}
	    }
	}
    }
}


/* check_live_components -- Check for possible transitions in all
 * components potentially capable of them.  If any transition is
 * taken, start over.
 */
void
check_live_components()
{
  Component** pc, *lc;

restart:
  /* Connections may have changed.  Discard all possible transitions.
   */
  clear_set(potential_transitions);
  recompute_backpointers();
  lc = live_components ;
  for (pc = &live_components ; *pc ; pc = &(*pc)->live_next)
    {
      TransitionDescriptor** pd;

      if(lc != live_components)
	/* Live_components has changed since we started
	 * traversing it, so a new potential transition might
	 * have been added at the beginning of the live_components
	 * list, or the creation itself might have caused a false
	 * guard to become TRUE.
	 *
	 * Tunc Simsek 3rd November, 1997
	 */
	goto restart;
      while (! (*pc)->M
	     || (! *(*pc)->M->outgoing
		 && ! (*pc)->desc->synchronizes_externally_p))
	{
	  /* Cleanup: remove dead component. */
	  *pc = (*pc)->live_next;
	  if (! *pc)
	    return;
	}
      for (pd = (*pc)->M->outgoing ; *pd ; pd++)
	{
	  if ((*pd)->guard && ! ((*pd)->guard)(*pc))
	    {
	      add_to_potential_trans_set(*pc, pd);
	      continue;
	    }
	  if (attempt_transition(*pc, pd))
	    goto restart;
	}
    }
}


/* Check the active guards, and take a transition if possible.
 */
void
discrete_step()
{
  GenericSet* trans_to_delete = new_set(&cvp_type);

  /* Time advances so we reset the number of consecutive transitions
   * taken since the last time.
   */
  nb_consecutive_transitions = 0;

  /* Check all potential transitions. */
  FOR_ALL_ELEMENTS (d, potential_transitions)
    {
      Component* c = d.cvp.c;
      TransitionDescriptor** pd = (TransitionDescriptor**) d.cvp.p;
      if (((*pd)->guard)(c))
	{
	  /* This remove was really messing things up.  As CLtL
	   * teaches us, removing an element from a list while
	   * traversing it, is evil (which is what is happening here,
	   * given the implementation of sets.) Instead we just collect it to
	   * remove it later.
	   *
	   * Marco Antoniotti 19970130
	   *
	   * remove_from_set(d, potential_transitions);
	   */
	  add_to_set(d, trans_to_delete);

	  if (attempt_transition(c, pd))
	    {
	      /* This calls clear the 'potential_transitions' set as a side
	       * effect. This is ok since we are now returning anyway.
	       */
	      check_live_components();
	      clear_set(trans_to_delete); /* Just cleanup a bit... */
	      free(trans_to_delete);
	      return;
	    }
	}
    }
  END_FAE;

  FOR_ALL_ELEMENTS (d, trans_to_delete)
    {
      remove_from_set(d, potential_transitions);
    }
  END_FAE;

  clear_set(trans_to_delete);
  free(trans_to_delete);
}


/* Warning: only called by do_link_assign (which is never called). */
static void
replace_backpointers(Component *l, Component *r, Component *self,
		     ExternalEventDescriptor **ex)
{
  vid* self_box = (vid*) safe_malloc(sizeof(vid));

  self_box->v = (void*) self;

  for ( ; *ex ; ex++)
    {
      EventDescriptor* ed = (*ex)->event;
      if (l) 
	remove_from_set(*self_box, DATA_ITEM(GenericSet *,
					     l,
					     ed->backlink_var_offset));
      add_to_set(*self_box, DATA_ITEM(GenericSet *,
				      r,
				      ed->backlink_var_offset));
    }
  /* Note that we cannot 'free' the 'self_box' because of the
   * 'add_to_set' call.
   */
}


/* Warning: only called by do_linkset_assign (which is never called). */
static void
remove_backpointers(Component* a,
		    Component* self,
		    ExternalEventDescriptor** ex)
{
  vid* self_box = (vid*) safe_malloc(sizeof(vid));

  self_box->v = (void*) self;

  for ( ; *ex ; ex++)
    {
      EventDescriptor* ed = (*ex)->event;

      remove_from_set(*self_box, DATA_ITEM(GenericSet*,
					   a,
					   ed->backlink_var_offset));
    }
  free(self_box);
}


/* Warning: only called by do_linkset_assign (which is never called). */
static void
add_backpointers(Component *a,
		 Component *self,
		 ExternalEventDescriptor **ex)
{
  vid* self_box = (vid*) safe_malloc(sizeof(vid));

  self_box->v = (void*) self;
  for (; *ex; ex++)
    {
      EventDescriptor* ed = (*ex)->event;

      add_to_set(*self_box, DATA_ITEM(GenericSet *,
				      a,
				      ed->backlink_var_offset));
    }
  /* Note that we cannot 'free' the 'self_box' because of the
   * 'add_to_set' call.
   */
}


/* Warning: never called. */
void
do_link_assign(Component** lp, Component* r, Component* self,
	       ExternalEventDescriptor** eed)
{
  if (eed)
    replace_backpointers(*lp, r, self, eed);
  *lp = r;
}


/* Warning: never called. */
void
do_linkset_assign(void** vlp, void* vr, Component* self,
		  ExternalEventDescriptor** eed)
{
  GenericSet** lp = (GenericSet**) vlp;
  GenericSet* r = (GenericSet*) vr;

  if (eed)
    {
      FOR_ALL_COMPONENTS(c, *lp) {
	remove_backpointers(c, self, eed);
      } END_FAC;
      FOR_ALL_COMPONENTS(c, r) {
	add_backpointers(c, self, eed);
      } END_FAC;
    }
  *lp = r;	/* Hopefully there is GC... */
}


void
print_search_stack()
{
  int stidx = search_index;
  extern void sdi(Component*);

  printf("[search stack]--------------------\n");
  while (stidx >= 0)
    {
      printf("\n[search stack %d %s]=============================\n",
	     stidx,
	     search_stack[stidx].sk == SK_SINGLE ? "SK_SINGLE" :
	     search_stack[stidx].sk == SK_SET ? "SK_SET" :
	     "SK_BACKLINK");
      printf("[Component]\n");
      sdi(search_stack[stidx].c);
      
      if (search_stack[stidx].sk == SK_BACKLINK)
	{
	  printf("[Other Component]\n");
	  sdi(search_stack[stidx].oc);
	}
      if (search_stack[stidx].sk == SK_SET)
	{
	  printf("[Synch variable offset]\n%d\n", search_stack[stidx].svo);
	}
      printf("[Event]\n");
      printf("%s\n",
	     search_stack[stidx].e ? (search_stack[stidx].e)->name :
	     "no event"
	     );
      stidx--;
    }
  printf("[search stack]--------------------\n");
}


void
print_bt_stack()
{
  int btidx = bt_index;
  extern void sdi(Component*);

  printf("[backtrack stack]--------------------\n");
  while (btidx >= 0)
    {
      printf("[backtrack stack %d %s]=============================\n",
	     btidx,
	     bt_stack[btidx].bk == BK_POP ? "BK_POP" :
	     bt_stack[btidx].bk == BK_NEXT_ELEMENT ? "BK_NEXT_ELEMENT" :
	     bt_stack[btidx].bk == BK_NEXT_TRANSITION ? "BK_NEXT_TRANSITION" :
	     "BK_NEXT_BACKLINK_TRANSITION");
      printf("[Component]\n");
      sdi(bt_stack[btidx].c);
      if (bt_stack[btidx].bk == BK_NEXT_BACKLINK_TRANSITION)
	{
	  printf("[Other Component]\n");
	  sdi(bt_stack[btidx].oc);
	}
      if (bt_stack[btidx].bk == BK_NEXT_ELEMENT)
	{
	  printf("[Synch variable offset]\n%d\n", bt_stack[btidx].svo);
	}
      printf("[Event]\n");
      printf("%s\n",
	     bt_stack[btidx].e ? (bt_stack[btidx].e)->name :
	     "no event"
	     );
      printf("[Transition]\n");
      printf("\n");
      btidx--;
    }
  printf("[backtrack stack]--------------------\n\n");
}

#endif

/* end of file -- discrete.c -- */
