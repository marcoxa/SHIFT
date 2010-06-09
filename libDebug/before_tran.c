/* -*- Mode: C -*- */

/* before_tran.c -- */

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



#ifndef BEFORE_TRAN_I
#define BEFORE_TRAN_I


#include "shift_debug.h"
#include "discrete.h"
#include "shift_tools.h"
#include "shift_api_info.h"
#include "shift_debug.h"


void
btf()
{
  int i; /* Beware!!!! i is used by FOR_ALL_READY_COMPONENTS */
  int collective_traced = 1;
  int collective_broken = 1;
  static int sequence;
  shift_debug_breakpoint* bp_list = (shift_debug_breakpoint*) NULL;
  shift_debug_breakpoint* bp_prev = (shift_debug_breakpoint*) NULL;
  int something_is_broken_or_traced = 0;
  
  FOR_ALL_READY_COMPONENTS {
    int component_broken = shift_tools_is_component_broken(c, d);
    int component_traced = shift_tools_is_component_traced(c, d);
    int type_broken = shift_tools_is_type_broken(c->desc, d);
    int type_traced = shift_tools_is_type_traced(c->desc, d);
    shift_api_user_type* type = shift_api_find_user_type(c->desc->name);
    shift_api_instance* instance = shift_api_find_instance(type, c->name);

    /* 1. Check if the component must break */
    if ( (c->break_flag == break_e && component_broken)
	 || (c->desc->break_flag == break_e && type_broken) )
      {
	something_is_broken_or_traced = 1;
      }
    /* 2. Check if the component must trace */
    if ( (c->trace_flag == both_e && component_traced)
	 || (c->trace_flag == desc_e && component_traced)
	 || (c->desc->trace_flag == both_e && type_traced)
	 || (c->desc->trace_flag == desc_e && type_traced) )
      {
	something_is_broken_or_traced = 1;
      }
  } END_FARC;

  if (something_is_broken_or_traced) {
    FOR_ALL_READY_COMPONENTS {
      int component_broken = shift_tools_is_component_broken(c, d);
      int component_traced = shift_tools_is_component_traced(c, d);
      int type_broken = shift_tools_is_type_broken(c->desc, d);
      int type_traced = shift_tools_is_type_traced(c->desc, d);
      shift_api_user_type* type = shift_api_find_user_type(c->desc->name);
      shift_api_instance* instance = shift_api_find_instance(type, c->name);

      /* 1. Check if the component must break */
      if ( (c->break_flag == break_e && component_broken)
	   || (c->desc->break_flag == break_e && type_broken) )
	{
	  shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);
	
	  /* If this is the first breakpoint, store it as the list head */
	  if (!bp_list)
	    bp_list = bp_curr;

	  /* Set the fields of the breakpoint */
	  bp_curr->instance = instance;
	  bp_curr->trans = d->api_trans;
	  bp_curr->trace_flag = NOT_P;
	  if (c->break_flag == break_e && component_broken)
	    if (c->desc->break_flag == break_e && type_broken)
	      bp_curr->break_flag = BOTH_P;
	    else
	      bp_curr->break_flag = COMP_P;
	  else
	    bp_curr->break_flag = TYPE_P;
	  bp_curr->next = (shift_debug_breakpoint*) NULL;
	
	  /* Link with the previous breakpoint */
	  if (bp_prev)
	    bp_prev->next = bp_curr;
	  bp_prev = bp_curr;
	
	  /* Record the fact that the transition is broken */
	  if (collective_broken)
	    collective_broken = 0;
	}

      /* 2. Check if the component must trace */
      if ( (c->trace_flag == both_e && component_traced)
	   || (c->trace_flag == desc_e && component_traced)
	   || (c->desc->trace_flag == both_e && type_traced)
	   || (c->desc->trace_flag == desc_e && type_traced) )
	{
	  shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);

	  /* If this is the first breakpoint, store it as the list head */
	  if (!bp_list)
	    bp_list = bp_curr;

	  /* Set the fields of the breakpoint */
	  bp_curr->instance = instance;
	  bp_curr->trans = d->api_trans;
	  bp_curr->break_flag = NOT_P;
	  if ( (c->trace_flag == both_e && component_traced)
	       || (c->trace_flag == desc_e && component_traced) )
	    if ( (c->desc->trace_flag == both_e && type_traced)
		 || (c->desc->trace_flag == desc_e && type_traced) )
	      bp_curr->trace_flag = BOTH_P;
	    else
	      bp_curr->trace_flag = COMP_P;
	  else
	    bp_curr->trace_flag = TYPE_P;
	  bp_curr->next = (shift_debug_breakpoint*) NULL;

	  /* Link with the previous breakpoint */
	  if (bp_prev)
	    bp_prev->next = bp_curr;
	  bp_prev = bp_curr;

	  /* Record the fact that the transition is traced */
	  if (collective_traced)
	    collective_traced = 0;
	}
    } END_FARC;
    
    FOR_ALL_READY_COMPONENTS {
      int component_broken = shift_tools_is_component_broken(c, d);
      int component_traced = shift_tools_is_component_traced(c, d);
      int type_broken = shift_tools_is_type_broken(c->desc, d);
      int type_traced = shift_tools_is_type_traced(c->desc, d);
      shift_api_user_type* type = shift_api_find_user_type(c->desc->name);
      shift_api_instance* instance = shift_api_find_instance(type, c->name);

      /* 1. Check if the component must break */
      if (!( (c->break_flag == break_e && component_broken)
	   || (c->desc->break_flag == break_e && type_broken) ))
	if (!( (c->trace_flag == both_e && component_traced)
	   || (c->trace_flag == desc_e && component_traced)
	   || (c->desc->trace_flag == both_e && type_traced)
	   || (c->desc->trace_flag == desc_e && type_traced)) )
	  {
	    shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);
	    
	    /* If this is the first breakpoint, store it as the list head */
	    if (!bp_list)
	      bp_list = bp_curr;

	    /* Set the fields of the breakpoint */
	    bp_curr->instance = instance;
	    bp_curr->trans = d->api_trans;
	    bp_curr->break_flag = NOT_P;
	    bp_curr->trace_flag = NOT_P;
	    bp_curr->next = (shift_debug_breakpoint*) NULL;
	    
	    /* Link with the previous breakpoint */
	    if (bp_prev)
	      bp_prev->next = bp_curr;
	    bp_prev = bp_curr;
	  }
    } END_FARC;

  }  

  /* 4. If the transition is either traced or broken,
   *    call the callback function
   */
  if (!collective_broken || !collective_traced)
    shift_api_callback->function(bp_list, sequence, tclick);

  /* 5. Increment the transition counter */
  sequence++;

  /* 6. Give back the hand to the user */
  if (!collective_broken)
    userInteract();
}


void
old_btf()
{
  int i; /* Beware!!!! i is used by FOR_ALL_READY_COMPONENTS */
  int collective_traced = 1;
  int collective_broken = 1;
  static int sequence;
  shift_debug_breakpoint* bp_list = (shift_debug_breakpoint*) NULL;
  shift_debug_breakpoint* bp_prev = (shift_debug_breakpoint*) NULL;

  
  FOR_ALL_READY_COMPONENTS {
    int component_broken = shift_tools_is_component_broken(c, d);
    int component_traced = shift_tools_is_component_traced(c, d);
    int type_broken = shift_tools_is_type_broken(c->desc, d);
    int type_traced = shift_tools_is_type_traced(c->desc, d);
    shift_api_user_type* type = shift_api_find_user_type(c->desc->name);
    shift_api_instance* instance = shift_api_find_instance(type, c->name);

    /* 1. Check if the component must break */
    if ( (c->break_flag == break_e && component_broken)
	 || (c->desc->break_flag == break_e && type_broken) )
      {
	shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);
	
	/* If this is the first breakpoint, store it as the list head */
	if (!bp_list)
	  bp_list = bp_curr;

	/* Set the fields of the breakpoint */
	bp_curr->instance = instance;
	bp_curr->trans = d->api_trans;
	bp_curr->trace_flag = NOT_P;
	if (c->break_flag == break_e && component_broken)
	  if (c->desc->break_flag == break_e && type_broken)
	    bp_curr->break_flag = BOTH_P;
	  else
	    bp_curr->break_flag = COMP_P;
	else
	  bp_curr->break_flag = TYPE_P;
	bp_curr->next = (shift_debug_breakpoint*) NULL;

	/* Link with the previous breakpoint */
	if (bp_prev)
	  bp_prev->next = bp_curr;
	bp_prev = bp_curr;
	
	/* Record the fact that the transition is broken */
	if (collective_broken)
	  collective_broken = 0;
      }

    /* 2. Check if the component must trace */
    if ( (c->trace_flag == both_e && component_traced)
	 || (c->trace_flag == desc_e && component_traced)
	 || (c->desc->trace_flag == both_e && type_traced)
	 || (c->desc->trace_flag == desc_e && type_traced) )
      {
	shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);

	/* If this is the first breakpoint, store it as the list head */
	if (!bp_list)
	  bp_list = bp_curr;

	/* Set the fields of the breakpoint */
	bp_curr->instance = instance;
	bp_curr->trans = d->api_trans;
	bp_curr->break_flag = NOT_P;
	if ( (c->trace_flag == both_e && component_traced)
	     || (c->trace_flag == desc_e && component_traced) )
	  if ( (c->desc->trace_flag == both_e && type_traced)
	       || (c->desc->trace_flag == desc_e && type_traced) )
	    bp_curr->trace_flag = BOTH_P;
	  else
	    bp_curr->trace_flag = COMP_P;
	else
	  bp_curr->trace_flag = TYPE_P;
	bp_curr->next = (shift_debug_breakpoint*) NULL;

	/* Link with the previous breakpoint */
	if (bp_prev)
	  bp_prev->next = bp_curr;
	bp_prev = bp_curr;

	/* Record the fact that the transition is traced */
	if (collective_traced)
	  collective_traced = 0;
      }

    /* 3. Even if the component neither broke nor traced, it must still
     *    appear in the collective transition
     */
    if (collective_broken && collective_traced)
      {
	shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);

	/* If this is the first breakpoint, store it as the list head */
	if (!bp_list)
	  bp_list = bp_curr;

	/* Set the fields of the breakpoint */
	bp_curr->instance = instance;
	bp_curr->trans = d->api_trans;
	bp_curr->break_flag = NOT_P;
	bp_curr->trace_flag = NOT_P;
	bp_curr->next = (shift_debug_breakpoint*) NULL;

	/* Link with the previous breakpoint */
	if (bp_prev)
	  bp_prev->next = bp_curr;
	bp_prev = bp_curr;
      }
  } END_FARC;

  

  /* 4. If the transition is either traced or broken,
   *    call the callback function
   */
  if (!collective_broken || !collective_traced)
    shift_api_callback->function(bp_list, sequence, tclick);

  /* 5. Increment the transition counter */
  sequence++;

  /* 6. Give back the hand to the user */
  if (!collective_broken)
    userInteract();
}


#endif /* BEFORE_TRAN_I */


/* end of file -- before_tran.c -- */
