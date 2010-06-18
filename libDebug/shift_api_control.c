/* -*- Mode: C -*- */


/* shift_api_control.c -- */

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

#include <stdlib.h>
#include <shift_config.h>
#include <ctype.h>

#include "shifttypes.h"
#include "shift_api_types.h"
#include "shift_api_internal.h"
#include "shift_api_info.h"
#include "shift_api_control.h"
#include "shift_debug.h"
#include "shift_tools.h"


/* Clears a type of all breakpoints */
int
shift_api_unbreak_type(shift_api_user_type* type)
{
  ComponentTypeDescriptor* ctd = shift_tools_find_type(type->name);
  traceType old_break_flag;

  /* 1. Check that type exists */
  if (!ctd)
    return -1;

  /* 2. Set the break_flag and trace_flag fields */
  old_break_flag = ctd->break_flag;
  ctd->break_flag = none_e;

  /* 3. If the type was previously traced, clear all the exported events */
  if (old_break_flag == break_e)
    {
      int iter = 0;

      ctd->allEventsBroken = 0;
      while (ctd->events[iter])
	{
	  ctd->events[iter]->broken = 0;
	  iter++;
	}
    }

  /* 4. Everything went ok */
  return 0;
}


/* Clears a type of all tracepoints */
int
shift_api_untrace_type(shift_api_user_type* type)
{
  ComponentTypeDescriptor* ctd = shift_tools_find_type(type->name);
  traceType old_trace_flag;

  /* 1. Check that type exists */
  if (!ctd)
    return -1;

  /* 2. Set the break_flag and trace_flag fields */
  old_trace_flag = ctd->trace_flag;
  ctd->trace_flag = none_e;

  /* 3. Clear the printVars field */
  if (ctd->printVars)
    {
      free(ctd->printVars);
      ctd->printVars = NULL;
    }

  /* 4. If the type was previously traced, clear all the exported events */
  if (old_trace_flag == desc_e
      || old_trace_flag == both_e)
    {
      int iter = 0;

      ctd->allEventsTraced = 0;
      while (ctd->events[iter])
	{
	  ctd->events[iter]->traced = 0;
	  iter++;
	}
    }

  /* 5. Everything went ok */
  return 0;
}


/* Sets a breakpoint on a user type */
int
shift_api_break_type(shift_api_user_type* type,
		     shift_api_exported_event** events)
{
  ComponentTypeDescriptor* ctd = shift_tools_find_type(type->name);
  traceType old_break_flag;

  /* 1. Check that type and events exist */
  if (!ctd || !events)
    return -1;

  /* 2. Set the break_flag field */
  old_break_flag = ctd->break_flag;
  ctd->break_flag = break_e;

  /* 3. Mark the events */
  if (events[0])
    {
      int iter = 0;

      while (events[iter])
	{
	  shift_tools_break_event(events[iter]->name, ctd);
	  iter++;
	}
      ctd->allEventsBroken = 0;
    }
  else
    ctd->allEventsBroken = 1;

  /* 4. Everything went ok */
  return 0;
}


/* Sets several trace information on a user type */
int
shift_api_trace_type(shift_api_user_type* type,
		     shift_api_exported_event** events,
		     shift_api_variable** variables,
		     traceType level)
{
  ComponentTypeDescriptor* ctd = shift_tools_find_type(type->name);
  traceType old_trace_flag;

  /* 1. Check that type, events, and variables exist */
  if (!ctd || !events || !variables)
    return -1;

  /* 2. Set the trace_flag field */
  old_trace_flag = ctd->trace_flag;
  if ( (old_trace_flag == desc_e && level == diff_e)
       || (old_trace_flag == diff_e && level == desc_e) )
    ctd->trace_flag = both_e;
  else
    ctd->trace_flag = level;

  /* 3. Mark the events */
  if (level == desc_e || level == both_e)
    if (events[0])
      {
	int iter = 0;

	while (events[iter])
	  {
	    shift_tools_trace_event(events[iter]->name, ctd);
	    iter++;
	  }
	ctd->allEventsTraced = 0;
      }
    else
      ctd->allEventsTraced = 1;

  /* 4. Compute the new list of marked variables */
  if (level == diff_e || level == both_e)
    if (variables[0])
      {
	int nb_traced = 0;
	int nb_new = 0;
	int iter = 0;

	/* Count the variables already traced */
	if (ctd->printVars)
	  while (ctd->printVars[nb_traced])
	    nb_traced++;

	/* Count the additional variables that are not already traced */
	while (variables[iter])
	  {
	    if (!shift_tools_is_var_in_list(variables[iter]->name,
					    ctd->printVars))
	      nb_new++;
	    iter++;
	  }

	if (nb_new > 0)
	  {
	    int nb_var = 0;
	    VariableDescriptor** new_vars;

	    /* Allocate the memory: one more element for the NULL */
	    new_vars = (VariableDescriptor**)
	      malloc(sizeof(VariableDescriptor*) * (nb_traced + nb_new + 1));

	    /* Fill in the list of traced variables */
	    if (ctd->printVars)
	      while (ctd->printVars[nb_var])
		{
		  new_vars[nb_var] = ctd->printVars[nb_var];
		  nb_var++;
		}
	    iter = 0;
	    while (variables[iter])
	      {
		if (!shift_tools_is_var_in_list(variables[iter]->name,
						ctd->printVars))
		  {
		    new_vars[nb_var] = 
		      shift_tools_find_variable_descriptor(variables[iter]->name,
							   ctd);
		    if (!new_vars[nb_var])
		      /* Something wrong happened */
		      return -1;
		    nb_var++;
		  }
		iter++;
	      }
	    new_vars[nb_var] = (VariableDescriptor*) NULL;
	    if (ctd->printVars)
	      free(ctd->printVars);
	    ctd->printVars = new_vars;
	  }
      }
    else
      {
	int nb_var = 0;
	int iter = 0;

	/* Count all the local variables */
	while (ctd->variables[iter].kind != NO_KIND)
	  {
	    if (ctd->variables[iter].kind == INPUT_KIND
		|| ctd->variables[iter].kind == OUTPUT_KIND
		|| ctd->variables[iter].kind == STATE_KIND)
	      nb_var++;
	    iter++;
	  }

	/* Allocate the memory: one more element for the NULL */
	if (ctd->printVars)
	  free(ctd->printVars);
	ctd->printVars = (VariableDescriptor**)
	  malloc(sizeof(VariableDescriptor*) * (nb_var + 1));

	/* Fill in the list of traced variables */
	iter = 0;
	nb_var = 0;
	while (ctd->variables[iter].kind != NO_KIND)
	  {
	    if (ctd->variables[iter].kind == INPUT_KIND
		|| ctd->variables[iter].kind == OUTPUT_KIND
		|| ctd->variables[iter].kind == STATE_KIND)
	      {
		ctd->printVars[nb_var] = &(ctd->variables[iter]);
		nb_var++;
	      }
	    iter++;
	  }
	ctd->printVars[nb_var] = (VariableDescriptor*) NULL;
      }

  /* 5. Everything went ok */
  return 0;
}


/* Clears an instance of all breakpoints */
int
shift_api_unbreak_instance(shift_api_instance* instance)
{
  Component* c = shift_tools_find_component(instance->type->name, instance->id);
  traceType old_break_flag;

  /* 1. Check that instance exists */
  if (!c)
    return -1;

  /* 2. Set the break_flag and trace_flag fields */
  old_break_flag = c->break_flag;
  c->break_flag = none_e;

  /* 3. Clear the allEventsTraced and allEventsBroken flags */
  c->allEventsBroken = 0;

  /* 4. Clear the brokenEvents field */
  if (c->brokenEvents)
    {
      free(c->brokenEvents);
      c->brokenEvents = NULL;
    }

  /* 5. Everything went ok */
  return 0;
}


/* Clears an instance of all tracepoints */
int
shift_api_untrace_instance(shift_api_instance* instance)
{
  Component* c = shift_tools_find_component(instance->type->name, instance->id);
  traceType old_trace_flag;

  /* 1. Check that instance exists */
  if (!c)
    return -1;

  /* 2. Set the break_flag and trace_flag fields */
  old_trace_flag = c->trace_flag;
  c->trace_flag = none_e;

  /* 3. Clear the allEventsTraced and allEventsBroken flags */
  c->allEventsTraced = 0;

  /* 4. Clear the printVars field */
  if (c->printVars)
    {
      free(c->printVars);
      c->printVars = NULL;
    }

  /* 5. Clear the tracedEvents field */
  if (c->tracedEvents)
    {
      free(c->tracedEvents);
      c->tracedEvents = NULL;
    }

  /* 6. If the instance was previously traced, remove it from 
   *    the list of traced components
   */
  if (old_trace_flag == diff_e || old_trace_flag == both_e)
    {
      ComponentList* comps = Tracer->timeTraced;

      while (comps != NULL)
	{
	  if (comps->thisComp == c)
	    {
	      /* The instance has been found in the list */
	      if (comps == Tracer->timeTraced)
		/* It is the first element of the list */
		Tracer->timeTraced = comps->nextComp;
	      else
		{
		  /* It is a middle element of the list */
		  comps->prevComp->nextComp = comps->nextComp;
		  if (comps->nextComp != NULL) 
		    comps->nextComp->prevComp = comps->prevComp;
		}
	      break;
	    }
	  else
	    comps = comps->nextComp;
	}
      if (!comps)
	/* The instance was not found in the list */
	return -1;
    }

  /* 7. Everything went ok */
  return 0;
}


/* Sets a breakpoint on an instance */
int
shift_api_break_instance(shift_api_instance* instance,
			 shift_api_exported_event** events)
{
  Component* c = shift_tools_find_component(instance->type->name,
					    instance->id);
  traceType old_break_flag;

  /* 1. Check that instance and events exist */
  if (!c || !events)
    return -1;

  /* 2. Set the break_flag field */
  old_break_flag = c->break_flag;
  c->break_flag = break_e;

  /* 3. If all events are already broken, return now */
  if (c->allEventsBroken)
    return 0;

  /* 4. Else compute the new list of broken events */
  if (events[0])
    {
      int nb_marked = 0;
      int nb_new = 0;
      int iter = 0;

      /* Count the events already broken */
      if (c->brokenEvents)
	while (c->brokenEvents[nb_marked])
	  nb_marked++;

      /* Count the additional events that are not already broken */
      while (events[iter])
	{
	  if (!shift_tools_is_event_in_list(events[iter]->name,
					    c->brokenEvents))
	    nb_new++;
	  iter++;
	}

      if (nb_new > 0)
	{
	  int nb_event = 0;
	  char** new_events;

	  /* Allocate the memory: one more element for the NULL */
	  new_events = (char**)
	    malloc(sizeof(char*) * (nb_marked + nb_new + 1));

	  /* Fill in the list of broken events */
	  if (c->brokenEvents)
	    while (c->brokenEvents[nb_event])
	      {
		new_events[nb_event] = (char*)
		  malloc(strlen(c->brokenEvents[nb_event]));
		strcpy(new_events[nb_event], c->brokenEvents[nb_event]);
		nb_event++;
	      }
	  iter = 0;
	  while (events[iter])
	    {
	      if (!shift_tools_is_event_in_list(events[iter]->name,
						c->brokenEvents))
		{
		  new_events[nb_event] = (char*)
		    malloc(strlen(events[iter]->name));
		  strcpy(new_events[nb_event], events[iter]->name);
		  nb_event++;
		}
	      iter++;
	    }
	  new_events[nb_event] = (char*) NULL;
	  if (c->brokenEvents)
	    free(c->brokenEvents);
	  c->brokenEvents = new_events;
	}
    }
  else
    {
      int nb_event = 0;

      /* Set the allEventsBroken flag */
      c->allEventsBroken = 1;

      /* Count all the exported events */
      if (c->desc->events)
	while (c->desc->events[nb_event])
	  nb_event++;

      /* Allocate the memory: one more element for the NULL */
      if (c->brokenEvents)
	free(c->brokenEvents);
      c->brokenEvents = (char**)
	malloc(sizeof(char*) * (nb_event + 1));

      /* Fill in the list of broken events */
      nb_event = 0;
      if (c->desc->events)
	while (c->desc->events[nb_event])
	  {
	    c->brokenEvents[nb_event] = (char*)
	      malloc(strlen(c->desc->events[nb_event]->name));
	    strcpy(c->brokenEvents[nb_event],
		   c->desc->events[nb_event]->name);
	    nb_event++;
	  }
    }

  /* 5. Everything went ok */
  return 0;
}


/* Sets several trace information on an instance */
int
shift_api_trace_instance(shift_api_instance* instance,
			 shift_api_exported_event** events,
			 shift_api_variable** variables,
			 traceType level)
{
  Component* c = shift_tools_find_component(instance->type->name,
					    instance->id);
  traceType old_trace_flag;

  /* 1. Check that instance, events, and variables exist */
  if (!c || !events || !variables)
    return -1;

  /* 2. Set the trace_flag field */
  old_trace_flag = c->trace_flag;
  if ( (old_trace_flag == desc_e && level == diff_e)
       || (old_trace_flag == diff_e && level == desc_e) )
    c->trace_flag = both_e;
  else
    c->trace_flag = level;

  /* 3. If all events are not already traced, compute the new list of
   *    traced events
   */
  if ( !c->allEventsTraced && (level == desc_e || level == both_e) )
    if (events[0])
      {
	int nb_marked = 0;
	int nb_new = 0;
	int iter = 0;

	/* Count the events already traced */
	if (c->tracedEvents)
	  while (c->tracedEvents[nb_marked])
	    nb_marked++;

	/* Count the additional events that are not already traced */
	while (events[iter])
	  {
	    if (!shift_tools_is_event_in_list(events[iter]->name,
					      c->tracedEvents))
	      nb_new++;
	    iter++;
	  }

	if (nb_new > 0)
	  {
	    int nb_event = 0;
	    char** new_events;

	    /* Allocate the memory: one more element for the NULL */
	    new_events = (char**)
	      malloc(sizeof(char*) * (nb_marked + nb_new + 1));

	    /* Fill in the list of traced events */
	    if (c->tracedEvents)
	      while (c->tracedEvents[nb_event])
		{
		  new_events[nb_event] = (char*)
		    malloc(strlen(c->tracedEvents[nb_event]));
		  strcpy(new_events[nb_event], c->tracedEvents[nb_event]);
		  nb_event++;
		}
	    iter = 0;
	    while (events[iter])
	      {
		if (!shift_tools_is_event_in_list(events[iter]->name,
						  c->tracedEvents))
		  {
		    new_events[nb_event] = (char*)
		      malloc(strlen(events[iter]->name));
		    strcpy(new_events[nb_event], events[iter]->name);
		    nb_event++;
		  }
		iter++;
	      }
	    new_events[nb_event] = (char*) NULL;
	    if (c->tracedEvents)
	      free(c->tracedEvents);
	    c->tracedEvents = new_events;
	  }
      }
    else
      {
	int nb_event = 0;

	/* Set the allEventsTraced flag */
	c->allEventsTraced = 1;

	/* Count all the exported events */
	if (c->desc->events)
	  while (c->desc->events[nb_event])
	    nb_event++;

	/* Allocate the memory: one more element for the NULL */
	if (c->tracedEvents)
	  free(c->tracedEvents);
	c->tracedEvents = (char**)
	  malloc(sizeof(char*) * (nb_event + 1));

	/* Fill in the list of traced events */
	nb_event = 0;
	if (c->desc->events)
	  while (c->desc->events[nb_event])
	    {
	      c->tracedEvents[nb_event] = (char*)
		malloc(strlen(c->desc->events[nb_event]->name));
	      strcpy(c->tracedEvents[nb_event],
		     c->desc->events[nb_event]->name);
	      nb_event++;
	    }
      }

  /* 4. Compute the new list of marked variables */
  if (level == diff_e || level == both_e)
    if (variables[0])
      {
	int nb_traced = 0;
	int nb_new = 0;
	int iter = 0;

	/* Count the variables already traced */
	if (c->printVars)
	  while (c->printVars[nb_traced])
	    nb_traced++;

	/* Count the additional variables that are not already traced */
	while (variables[iter])
	  {
	    if (!shift_tools_is_var_in_list(variables[iter]->name,
					    c->printVars))
	      nb_new++;
	    iter++;
	  }

	if (nb_new > 0)
	  {
	    int nb_var = 0;
	    VariableDescriptor** new_vars;

	    /* Allocate the memory: one more element for the NULL */
	    new_vars = (VariableDescriptor**)
	      malloc(sizeof(VariableDescriptor*) * (nb_traced + nb_new + 1));

	    /* Fill in the list of traced variables */
	    if (c->printVars)
	      while (c->printVars[nb_var])
		{
		  new_vars[nb_var] = c->printVars[nb_var];
		  nb_var++;
		}
	    iter = 0;
	    while (variables[iter])
	      {
		if (!shift_tools_is_var_in_list(variables[iter]->name,
						c->printVars))
		  {
		    new_vars[nb_var] = 
		      shift_tools_find_variable_descriptor(variables[iter]->name,
							   c->desc);
		    if (!new_vars[nb_var])
		      /* Something wrong happened */
		      return -1;
		    nb_var++;
		  }
		iter++;
	      }
	    new_vars[nb_var] = (VariableDescriptor*) NULL;
	    if (c->printVars)
	      free(c->printVars);
	    c->printVars = new_vars;
	  }
      }
    else
      {
	int nb_var = 0;
	int iter = 0;

	/* Count all the local variables */
	while (c->desc->variables[iter].kind != NO_KIND)
	  {
	    if (c->desc->variables[iter].kind == INPUT_KIND
		|| c->desc->variables[iter].kind == OUTPUT_KIND
		|| c->desc->variables[iter].kind == STATE_KIND)
	      nb_var++;
	    iter++;
	  }

	/* Allocate the memory: one more element for the NULL */
	if (c->printVars)
	  free(c->printVars);
	c->printVars = (VariableDescriptor**)
	  malloc(sizeof(VariableDescriptor*) * (nb_var + 1));

	/* Fill in the list of traced variables */
	iter = 0;
	nb_var = 0;
	while (c->desc->variables[iter].kind != NO_KIND)
	  {
	    if (c->desc->variables[iter].kind == INPUT_KIND
		|| c->desc->variables[iter].kind == OUTPUT_KIND
		|| c->desc->variables[iter].kind == STATE_KIND)
	      {
		c->printVars[nb_var] = &(c->desc->variables[iter]);
		nb_var++;
	      }
	    iter++;
	  }
	c->printVars[nb_var] = (VariableDescriptor*) NULL;
      }

  /* 5. Add instance to the list of traced instances only if it was
   *    not already in it
   */
  if ( (level == diff_e || level == both_e)
       && (old_trace_flag == desc_e || old_trace_flag == none_e) )
    {
      ComponentList* new_comp = (ComponentList*)
	malloc(sizeof(ComponentList));

      new_comp->prevComp = NULL;
      new_comp->thisComp = c;
      new_comp->nextComp = Tracer->timeTraced;
      if (Tracer->timeTraced)
	Tracer->timeTraced->prevComp = new_comp;
      Tracer->timeTraced = new_comp;
    }

  /* 6. Everything went ok */
  return 0;
}


/* Registers a callback function for breakpoints */
int
shift_api_register_breakpoint_callback(void (*f)(shift_debug_breakpoint*,
						 int,
						 int))
{
  if (shift_api_callback)
    {
      shift_api_callback->function = f;
      return 0;
    }
  else
    return -1;
}


/* end of file SHIFT_API_CONTROL_C */
