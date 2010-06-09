/* -*- Mode: C -*- */

/* shift_tools.c -- */

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

/* shift_tools.c --
 * Debug aux functions
 */

#ifndef SHIFT_TOOLS_I
#define SHIFT_TOOLS_I


#include <shift_config.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "run-time.h"
#include "shifttypes.h"
#include "collection.h"
#include "shift_api_types.h"
#include "shift_api_internal.h"
#include "shift_api_info.h"
#include "shift_debug.h"
#include "shift_tools.h"
#include "discrete.h"


extern int first_shift_symbol;
extern int last_shift_symbol;
extern char *shift_symbol_table[];
extern int _step;
int shift_debug_precision = 3;


/* Forward declaration */


char* printVariable(Component*, VariableDescriptor*, char*);


/* Routine definitions */


void
shift_tools_error(char * errmsg)
{
  fprintf(stderr, "SHIFT internal tools: error: %s\n", errmsg);
  fflush(stderr);
  
  /* SIGNAL error to be caught */
  /* if not signalling ... */
  exit(1);
}


void
shift_tools_warning(char *errmsg)
{
  fprintf(stderr, "SHIFT internal tools: warning: %s\n", errmsg);
  fflush(stderr);
}


/* Prints the informations relative to a variable */
char*
shift_tools_print_variable(shift_api_variable* var)
{
  static char d[CMDBUFSIZE];
  char* a = d;

  /* 1. Check that the structure exists */
  if (!var)
    shift_api_error("pointer to a NULL variable");

  /* 2. Print the kind */
  switch (var->kind)
    {
    case GLOBAL_V:
      sprintf(a, "GLOBAL ");
      a = a + strlen(a);
      break;

    case INPUT_V:
      sprintf(a, "INPUT ");
      a = a + strlen(a);
      break;

    case OUTPUT_V:
      sprintf(a, "OUTPUT ");
      a = a + strlen(a);
      break;

    case STATE_V:
      sprintf(a, "STATE ");
      a = a + strlen(a);
      break;

    default:
      shift_api_error("unknown variable kind");
      break;
    }

  /* 3. Print the name and the type */
  sprintf(a, "%s %s", var->name, shift_tools_print_type(var->type));
  a = a + strlen(a);

  /* 4. Return the result string */
  return d;
}


/* Prints the informations relative to a type */
char*
shift_tools_print_type(shift_api_type* type)
{
  /* 1. Check that the structure exists */
  if (!type)
    shift_api_error("pointer to a NULL type");

  /* 2. Depending on the kind, calls the appropriate printing function */
  switch (type->kind)
    {
    case BUILTIN_K:
      return shift_tools_print_builtin_type(type->t.builtin_t);
      break;

    case USER_K:
      return shift_tools_print_user_type(type->t.user_t);
      break;

    default:
      shift_api_error("unknown type kind");
      break;
    }
}


/* Prints the informations relative to a user type */
char*
shift_tools_print_user_type(shift_api_user_type* user_type)
{
  /* 1. Check that the structure exists */
  if (!user_type)
    shift_api_error("pointer to a NULL user type");

  /* 2. Return the name of the user type */
  return user_type->name;
}


/* Prints the informations relative to a builtin type */
char*
shift_tools_print_builtin_type(shift_api_builtin_type* builtin_type)
{
  char* a;

  /* 1. Check that the structure exists */
  if (!builtin_type)
    shift_api_error("pointer to a NULL builtin type");

  /* 2. Print the information or call the printing function on the 
   *    element type in case of an array or a set 
   */
  switch (builtin_type->kind)
    {
    case NUMBER_B:
      return "NUMBER";
      break;

    case SYMBOL_B:
      return "SYMBOL";
      break;

     case LOGICAL_B:
      return "LOGICAL";
      break;

    case ARRAY_B:
      a = safe_malloc(CMDBUFSIZE);
      sprintf(a,
	      "ARRAY[%s]",
	      shift_tools_print_type(builtin_type->element_type));
      return a;
      break;

    case SET_B:
      a = safe_malloc(CMDBUFSIZE);
      sprintf(a,
	      "SET{%s}",
	      shift_tools_print_type(builtin_type->element_type));
      return a;
      break;

    default:
      shift_api_error("unknown builtin type kind");
      break;
   }
}

/* Adding these 2 functions for printing values
 * of foreign and unknown data types.
 *
 * Tunc Simsek 19980605
 */
char *
shift_tools_print_foreign_value(void *v, char *a)
{
  sprintf(a,
	  "%s %p (%s %d)",
	  "Foreign data @",
	  v,
	  "Int val:",
	  *((int *) v));
  a = a + strlen(a);

  return a;
}

char *
shift_tools_print_unknown_value(void *v, char *a)
{
  sprintf(a, 
	  "%s %p", 
	  "Unknown data @",
	  v);
  a = a + strlen(a);

  return a;
}


/* Prints a value */
char*
shift_tools_print_value(shift_api_value* value, char* a)
{
  /* 1. Check that the structure exists */
  if (!value)
    shift_api_error("pointer to a NULL value");

  /* 2. Depending on the kind, calls the appropriate printing function */
  switch (value->kind)
    {
    case BUILTIN_K:
      a = shift_tools_print_builtin_value(value->v.value_v, a);
      break;

    case USER_K:
      a = shift_tools_print_instance(value->v.instance_v, a);
      break;

      /* Modifying:
       *
       * Adding these cases so that it will not exit with an
       * error when an unknown kind is encountered.
       *
       * Tunc Simsek 19980603
       */
    case FOREIGN_K:
      a = shift_tools_print_foreign_value(value->v.foreign_v,
					  a);

      break;

    case UNKNOWN_K:
      a = shift_tools_print_unknown_value(value->v.unknown_v,
					  a);
      break;

    default:
      shift_api_error("unknown type kind");
      break;
    }

  /* 3. Return the result string */
  a = a + strlen(a);
  return a;
}


/* Prints a user type value (i.e., an instance) */
char*
shift_tools_print_instance(shift_api_instance* instance, char* a)
{
  /* 1. Check that the structure exists */
  if (!instance)
    shift_api_error("pointer to a NULL instance");

  if (instance->type)
    /* 2. If the type field exists, print the name of the type of 
     *    the instance and its id number
     */
    sprintf(a, "(%s %d)", instance->type->name, instance->id);
  else
    /* 3. Else it is a reference to an undefined instance */
    sprintf(a, "#<UNDEFINED>");

  /* 4. Return the result string */
  a = a + strlen(a);
  return a;
}


/* Prints a builtin type value */
char*
shift_tools_print_builtin_value(shift_api_builtin_value* value, char* a)
{
  /* 1. Check that the structure exists */
  if (!value)
    shift_api_error("pointer to a NULL builtin value");

  /* 2. Print the information or call the printing function on the 
   *    element type in case of an array or a set 
   */
  switch (value->kind)
    {
    case NUMBER_B:
      sprintf(a, "%.*f", shift_debug_precision, value->v.number_v);
      break;

    case SYMBOL_B:
      sprintf(a, "%s", value->v.symbol_v);
      break;

     case LOGICAL_B:
      sprintf(a, "%d", value->v.logical_v);
      break;

    case ARRAY_B:
      a = shift_tools_print_array(value->v.array_v, a);
      break;

    case SET_B:
      a = shift_tools_print_set(value->v.set_v, a);
      break;

    default:
      shift_api_error("unknown builtin type kind");
      break;
   }

  /* 3. Return the result string */
  a = a + strlen(a);
  return a;
}


/* Prints a set */
char*
shift_tools_print_set(shift_api_set_value* set, char* a)
{
  int iter;

  /* 1. Check that the structure exists */
  if (!set)
    {
      sprintf(a, "{UNINITIALIZED_SET}");
      a = a + strlen(a);
      return a;
    }

  /* 2. Depending on the kind, loop and call the appropriate printing
   *    function
   */
  sprintf(a, "{");
  a = a + strlen(a);
  switch (set->kind)
    {
    case BUILTIN_K:
      for (iter = 0 ; iter < set->size ; iter++)
	{
	  if (iter)
	    {
	      sprintf(a, ", ");
	      a = a + strlen(a);
	    }
	  a = shift_tools_print_builtin_value(set->e.values_e[iter], a);
	  a = a + strlen(a);
	}
      break;

    case USER_K:
      for (iter = 0 ; iter < set->size ; iter++)
	{
	  if (iter)
	    {
	      sprintf(a, ", ");
	      a = a + strlen(a);
	    }
	  a = shift_tools_print_instance(set->e.instances_e[iter], a);
	  a = a + strlen(a);
	}
      break;

      /* Adding the case for foreign types.
       *
       * Tunc Simsek 19980605
       */
    case FOREIGN_K:
      for (iter = 0; iter < set->size ; iter++)
	{
	  if (iter)
	    {
	      sprintf(a, ", ");
	      a = a + strlen(a);
	    }

	  a = 
	    shift_tools_print_foreign_value(set->e.foreign_v[iter],
					    a);
	}
      break;

    default:
      shift_api_error("unknown type kind");
      break;
    }
  sprintf(a, "}");
  a = a + strlen(a);
  return a;
}


/* Prints an array */
char*
shift_tools_print_array(shift_api_array_value* array, char* a)
{
  int iter;

  /* 1. Check that the structure exists */
  if (!array)
    {
      sprintf(a, "[UNINITIALIZED_ARRAY]");
      a = a + strlen(a);
      return a;
    }

  /* 2. Depending on the kind, loop and call the appropriate printing
   *    function
   */
  sprintf(a, "[");
  a = a + strlen(a);
  switch (array->kind)
    {
    case BUILTIN_K:
      for (iter = 0 ; iter < array->size ; iter++)
	{
	  if (iter)
	    {
	      sprintf(a, ", ");
	      a = a + strlen(a);
	    }
	  a = shift_tools_print_builtin_value(array->e.values_e[iter], a);
	  a = a + strlen(a);
	}
      break;

    case USER_K:
      for (iter = 0 ; iter < array->size ; iter++)
	{
	  if (iter)
	    {
	      sprintf(a, ", ");
	      a = a + strlen(a);
	    }
	  a = shift_tools_print_instance(array->e.instances_e[iter], a);
	  a = a + strlen(a);
	}
      break;

      /* Adding case for foreign types.
       *
       * Tunc Simsek 19980605
       */
    case FOREIGN_K:
      for (iter = 0 ; iter < array->size ; iter++)
	{
	  if (iter)
	    {
	      sprintf(a, ", ");
	      a = a + strlen(a);
	    }
	  a =
	    shift_tools_print_foreign_value(array->e.foreign_v[iter], 
					    a);
	  a = a + strlen(a);
	}
      break;

    default:
      shift_api_error("unknown type kind");
      break;
    }
  sprintf(a, "]");
  a = a + strlen(a);
  return a;
}


/* Prints the values of all the local variables and the current mode 
 * of an instance 
 */
void
shift_tools_print_all_values(FILE* outfile, shift_api_instance* inst)
{
  shift_api_variable** vars = shift_api_find_local_variables(inst->type);
  shift_api_mode* mode = shift_api_find_current_mode(inst);
  int iter = 0;

  while (vars[iter])
    {
      char d[OUTBUFSIZE];
      char* a = d;
      shift_api_value* val =
	shift_api_find_local_value(vars[iter]->name, inst);

      fprintf(outfile, "    {%s ", vars[iter]->name);
      shift_tools_print_value(val, a);
      fprintf(outfile, "%s}\n", d);
      iter++;
    }
  fprintf(outfile, "    {discrete %s}\n", mode ? mode->name : "NONE");
}


/* Prints a list of breakpoints */
void
shift_tools_print_breakpoints(shift_debug_breakpoint* bp_list,
			      int tnb,
			      int tclick)
{
  int collective_broken = 0;
  int collective_traced = 0;
  shift_debug_breakpoint* bp_curr = bp_list;

  /* 1. Check if the collective transition must be traced and/or printed */
  while (bp_curr)
    {
      if (bp_curr->break_flag == TYPE_P
	  || bp_curr->break_flag == COMP_P
	  || bp_curr->break_flag == BOTH_P)
	collective_broken = 1;
      if (bp_curr->trace_flag == TYPE_P
	  || bp_curr->trace_flag == COMP_P
	  || bp_curr->trace_flag == BOTH_P)
	collective_traced = 1;
      bp_curr = bp_curr->next;
    }

  /* 2. Print and trace the collective transition */
  if (collective_broken)
    printf("\nTime is %d.\n\n{collective transition %d", tclick, tnb);
  if (collective_traced)
    fprintf(Tracer->outfile, "{collective transition %d", tnb);
  if (collective_traced || collective_broken)
    {
      bp_curr = bp_list;
      while (bp_curr)
	{
	  if (collective_broken)
	    printf(" (%s %d)",
		   bp_curr->instance->type->name,
		   bp_curr->instance->id);
	  if (collective_traced)
	    fprintf(Tracer->outfile,
		    " (%s %d)",
		    bp_curr->instance->type->name,
		    bp_curr->instance->id);
	  bp_curr = bp_curr->next;
	}
    }
  if (!collective_traced)
    printf("}\n");
  if (!collective_broken)
    fprintf(Tracer->outfile, "}\n");

  /* 3. Print and trace each broken and traced component */
  bp_curr = bp_list;
  while (bp_curr)
    {
      if (bp_curr->break_flag == TYPE_P
	  || bp_curr->break_flag == COMP_P
	  || bp_curr->break_flag == BOTH_P)
	shift_tools_print_breakpoint(stdout, bp_curr, tnb);
      if (bp_curr->trace_flag == TYPE_P
	  || bp_curr->trace_flag == COMP_P
	  || bp_curr->trace_flag == BOTH_P)
	shift_tools_print_breakpoint(Tracer->outfile, bp_curr, tnb);
      bp_curr = bp_curr->next;
    }
}


/* Prints one breakpoint to a file */
void
shift_tools_print_breakpoint(FILE* outfile,
			     shift_debug_breakpoint* bp,
			     int tnb)
{
  int iter = 0;
  shift_api_instance* instance;
  shift_api_value* value;
  shift_api_variable** variables =
    shift_api_find_local_variables(bp->instance->type);

  fprintf(outfile, "{%s %d\n",
	  bp->instance->type->name,
	  bp->instance->id);
  fprintf(outfile, "    {before collective transition %d}\n", tnb);
  if (bp->trans != 0)
    {
      fprintf(outfile,
	      "    {transition %s -> %s}\n",
	      bp->trans->source->name,
	      bp->trans->dest->name);
      fprintf(outfile, "    {exported");
      while (bp->trans->exports[iter])
	{
	  int coma = 0;

	  if (coma)
	    fprintf(outfile, ",");
	  else
	    coma = 1;
	  fprintf(outfile, " %s", bp->trans->exports[iter]->name);
	  iter++;
	}
      fprintf(outfile, "}\n");
      fprintf(outfile, "    {external");
      iter = 0;
      while (bp->trans->externs[iter])
	{
	  int coma = 0;

	  if (coma)
	    fprintf(outfile, ",");
	  else
	    coma = 1;
	  fprintf(outfile,
		  " %s:%s",
		  bp->trans->externs[iter]->var_name,
		  bp->trans->externs[iter]->name);
	  switch (bp->trans->externs[iter]->kind)
	    {
	    case SINGLE_C:
	      break;

	    case SET_ONE_C:
	      instance =
		shift_api_find_single_item_in_transition(bp->instance,
							 bp->trans->externs[iter]);
	      if (instance)
		fprintf(outfile,
			"(one):%s %d",
			instance->type->name,
			instance->id);
	      break;

	    case SET_ALL_C:
	      if (bp->trans->externs[iter]->global == 0)
		{
		  value =
		    shift_api_find_local_value(bp->trans->externs[iter]->var_name,
					       bp->instance);
		}
	      else
		{
		  value =
		    shift_api_find_global_value(bp->trans->externs[iter]->var_name);
		}
	      if (value != NULL)
		{
		  char d[OUTBUFSIZE];
		  char* a = d;

		  shift_tools_print_value(value, a);
		  fprintf(outfile,"(all):%s", a);
		}
	      break;

	    default:
	      shift_api_error("unknown connection kind");
	      break;
	    }
	  iter++;
	}
      fprintf(outfile, "}\n");
    }

  shift_tools_print_all_values(outfile, bp->instance);
  fprintf(outfile, "}\n");
}


/* Marks an event as tracing variables */
void
shift_tools_trace_event(char* event, ComponentTypeDescriptor* ctd)
{
  int iter = 0;

  while (ctd->events[iter])
    {
      if (!strcmp(ctd->events[iter]->name, event))
	{
	  ctd->events[iter]->traced = 1;
	  return;
	}
      iter++;
    }
}


/* Marks an event as causing a break point */
void
shift_tools_break_event(char* event, ComponentTypeDescriptor* ctd)
{
  int iter = 0;

  while (ctd->events[iter])
    {
      if (!strcmp(ctd->events[iter]->name, event))
	{
	  ctd->events[iter]->broken = 1;
	  return;
	}
      iter++;
    }
}


/* Tests if a variable belongs to a list */
int
shift_tools_is_var_in_list(char* var, VariableDescriptor** variables)
{
  int iter = 0;

  if (variables)
    while (variables[iter])
      {
	if (!strcmp(var, variables[iter]->name))
	  return 1;
	iter++;
      }
  return 0;
}


/* Tests if an exported event belongs to a list */
int
shift_tools_is_event_in_list(char* event, char** events)
{
  int iter = 0;

  if (events)
    while (events[iter])
      {
	if (!strcmp(event, events[iter]))
	  return 1;
	iter++;
      }
  return 0;
}


/* Retrives a ComponentTypeDescriptor */
ComponentTypeDescriptor*
shift_tools_find_type(char* type_name)
{
  int iter = 0;

  while (component_type_list[iter])
    {
      if (!strcmp(type_name, component_type_list[iter]->name))
	return component_type_list[iter];
      else
	iter++;
    }
  return (ComponentTypeDescriptor*) NULL;
}


/* Retrieves a Component */
Component*
shift_tools_find_component(char* type_name, int id)
{
  Component* comp = (Component*) 0;
  Component* res = (Component*) 0;
  ComponentTypeDescriptor* ctd = shift_tools_find_type(type_name);

  if (ctd)
    for (comp = ctd->component_first ; comp ; comp = comp->component_next)
      if (id == comp->name)
	{
	  res = comp;
	  return comp;
	}
  return (Component*) 0;
}


/* Retrieves a VariableDescriptor in a ComponentTypeDescriptor */
VariableDescriptor*
shift_tools_find_variable_descriptor(char* var, ComponentTypeDescriptor* ctd)
{
  int iter = 0;

  /* 1. Scan the variable list: the list ends with a NO_KIND variable */
  if (ctd->variables)
    while (ctd->variables[iter].kind != NO_KIND)
      {
	/* The VariableDescriptor must match the name and be of kind
	 * GLOBAL_KIND, INPUT_KIND, OUTPUT_KIND, or STATE_KIND
	 */
	if (!strcmp(var, ctd->variables[iter].name))
	  if (ctd->variables[iter].kind == GLOBAL_KIND
	      || ctd->variables[iter].kind == INPUT_KIND
	      || ctd->variables[iter].kind == OUTPUT_KIND
	      || ctd->variables[iter].kind == STATE_KIND)
	    return &(ctd->variables[iter]);
	iter++;
      }

  /* 2. No matching variables were found */
  return (VariableDescriptor*) NULL;
}


/* Retrieves an EventDescriptor in a ComponentTypeDescriptor */
EventDescriptor*
shift_tools_find_event_descriptor(char* event, ComponentTypeDescriptor* ctd)
{
  int iter = 0;

  /* 1. Scan the event list: the list ends with NULL */
  if (ctd->events)
    while (ctd->events[iter])
      {
	if (!strcmp(event, ctd->events[iter]->name))
	  return ctd->events[iter];
	iter++;
      }

  /* 2. No matching events were found */
  return (EventDescriptor*) NULL;
}


/* Extracts a list of variables from a space separated string */
shift_api_variable**
shift_tools_extract_variables(shift_api_user_type* type, char* rS)
{
  char* tail = safe_malloc(CMDBUFSIZE * sizeof(char));
  char* name = safe_malloc(NAMESIZE * sizeof(char));
  int k = 0;
  int m = strlen(rS);
  int found = 0;

  /* Find the '-v': it marks the beginning of the variables */
  while (k < m)
    {
      if (rS[k] == '-' && rS[k+1] == 'v')
	{
	  found = 1;
	  break;
	}
      k++;
    }

  if (found)
    {
      int i = 0;
      int j = k + 2;
      shift_api_variable** variables;

      /* Count the number of variables */
      while (j < m)
	{
	  j = getToken(rS+j, name, tail, " ") + j;
	  if (name[0] == '-')
	    break;
	  i++;
	}

      /* Allocate the memory: one more element for the NULL */
      variables = (shift_api_variable**)
	safe_malloc(sizeof(shift_api_variable*) * (i + 1));

      /* Fill in the list of variables */
      i = 0;
      j = k + 2;
      while (j < m)
	{
	  j = getToken(rS+j, name, tail, " ") + j;
	  if (name[0] == '-')
	    break;
	  variables[i] = shift_api_find_local_variable(name, type);
	  if (!variables[i])
	    {
	      /* name was not found in the list of local variables
	       * so we return an empty list
	       */
	      free(tail);
	      free(name);
	      free(variables);
	      return (shift_api_variable**) NULL;
	    }
	  i++;
	}

      /* Set the last element to NULL and return the list */
      variables[i] = (shift_api_variable*) NULL;
      free(tail);
      free(name);
      return variables;
    }
  else
    {
      /* Allocate the memory only for the NULL element */
      shift_api_variable** variables = (shift_api_variable**)
	safe_malloc(sizeof(shift_api_variable*) * 1);

      variables[0] = (shift_api_variable*) NULL;
      free(tail);
      free(name);
      return variables;
    }
}


/* Extracts a list of exported events from a space separated string */
shift_api_exported_event**
shift_tools_extract_events(shift_api_user_type* type, char* rS)
{
  char* tail = safe_malloc(CMDBUFSIZE * sizeof(char));
  char* name = safe_malloc(NAMESIZE * sizeof(char));
  int k = 0;
  int m = strlen(rS);
  int found = 0;

  /* Find the '-e': it marks the beginning of the variables */
  while (k < m)
    {
      if (rS[k] == '-' && rS[k+1] == 'e')
	{
	  found = 1;
	  break;
	}
      k++;
    }

  if (found)
    {
      int i = 0;
      int j = k + 2;
      shift_api_exported_event** events;

      /* Count the number of events */
      while (j < m)
	{
	  j = getToken(rS+j, name, tail, " ") + j; 
	  if (name[0] == '-')
	    break;
	  i++;
	}

      /* Allocate the memory: one more element for the NULL */
      events = (shift_api_exported_event**)
	safe_malloc(sizeof(shift_api_exported_event*) * (i + 1));

      /* Fill in the list of events */
      i = 0;
      j = k + 2;
      while (j < m)
	{
	  j = getToken(rS+j, name, tail, " ") + j; 
	  if (name[0] == '-')
	    break;
	  events[i] = shift_api_find_type_exported_event(name, type);
	  if (!events[i])
	    {
	      /* name was not found in the list of exported events
	       * so we return an empty list
	       */
	      free(tail);
	      free(name);
	      free(events);
	      return (shift_api_exported_event**) NULL;
	    }
	  i++;
	}

      /* Set the last element to NULL and return the list */
      events[i] = (shift_api_exported_event*) NULL;
      free(tail);
      free(name);
      return events;
    }
  else
    {
      /* Allocate the memory only for the NULL element */
      shift_api_exported_event** events = (shift_api_exported_event**)
	safe_malloc(sizeof(shift_api_exported_event*) * 1);

      events[0] = (shift_api_exported_event*) NULL;
      free(tail);
      free(name);
      return events;
    }
}


/* Tests if the type ctd must trace on the transition td */
int
shift_tools_is_type_traced(ComponentTypeDescriptor* ctd,
			   TransitionDescriptor* td)
{
  if (ctd->allEventsTraced)
    /* All events are traced */
    return 1;
  else
    {
      int iter = 0;

      while (td->events[iter])
	{
	  if (td->events[iter]->traced)
	    /* At least one event in the transition is traced */
	    return 1;
	  iter++;
	}
    }
  /* Otherwise */
  return 0;
}


/* Tests if the type ctd must break on the transition td */
int
shift_tools_is_type_broken(ComponentTypeDescriptor* ctd,
			   TransitionDescriptor* td)
{
  if (ctd->allEventsBroken)
    /* All events are broken */
    return 1;
  else
    {
      int iter = 0;

      while (td->events[iter])
	{
	  if (td->events[iter]->broken)
	    /* At least one event in the transition is broken */
	    return 1;
	  iter++;
	}
    }
  /* Otherwise */
  return 0;
}


/* Tests if the component c must trace on the transition td */
int
shift_tools_is_component_traced(Component* c, TransitionDescriptor* td)
{
  /* If the flag allEventsTraced is on then the component must trace */
  if (c->allEventsTraced)
    return 1;

  /* Else scan the list of exported events */
  if (c->tracedEvents)
    {
      int iter1 = 0;

      while (c->tracedEvents[iter1])
	{
	  int iter2 = 0;

	  while (td->events[iter2])
	    {
	      if (!strcmp(c->tracedEvents[iter1], td->events[iter2]->name))
		/* At least one event in the transition is traced */
		return 1;
	      iter2++;
	    }
	  iter1++;
	}
    }

  /* Otherwise the component must not trace */
  return 0;
}


/* Tests if the component c must break on the transition td */
int
shift_tools_is_component_broken(Component* c, TransitionDescriptor* td)
{
  /* If the flag allEventsBroken is on then the component must break */
  if (c->allEventsBroken)
    return 1;

  /* Else scan the list of exported events */
  if (c->brokenEvents)
    {
      int iter1 = 0;

      while (c->brokenEvents[iter1])
	{
	  int iter2 = 0;

	  while (td->events[iter2])
	    {
	      if (!strcmp(c->brokenEvents[iter1], td->events[iter2]->name))
		/* At least one event in the transition is broken */
		return 1;
	      iter2++;
	    }
	  iter1++;
	}
    }

  /* Otherwise the component must not break */
  return 0;
}


int
getToken(char* source, char* head, char* tail, char* sep)
{
  /* Splits source into head and tail. 
   * Head is text up to first encounter of sep.
   * It eliminates leading spaces before parsing. 
   * It also eliminates \n and \r in the tail
   */

  int z, j, i, k, m;

  /* Eliminate leading spaces */
  for (i = 0; isspace(source[i]); i++);

  source = source + i;
  m = i;

  /* Source may or may not be \n terminated */
  j = z = strlen(source);
  for (i = 0; i < z; i++)
    {
      if ((*(source+i) == *sep)
	  || (*(source+i) == '\n')
	  || (*(source+i) == '\r')) {
	j = i;
	break;
      }
    }

  for (i = 0; i < j; i++)
    *(head+i) = *(source+i);
  *(head+i) = 0;

  /* eliminate spaces */
  while (isspace(*(source+i)))
    i++;

  for (k = i ; k < z; k++)
    *(tail + (k - i)) = *(source + k);

  j = strlen(tail);
  if (j > 0)
    {
      /* j = k-i-1 */
      if ((*(tail + (k - i - 1)) == '\r')
	  || (*(tail + (k - i - 1)) == '\n' ))
	*(tail+(k-i-1)) = 0; /* This eliminates Carriage return */
    }
  return i + m; /* index into new position */
}


char*
display(Component* c)
{
  static char d[OUTBUFSIZE];
  char* a = d;

  sprintf(a, "{%s %d \n", c->desc->name, c->name);
  a = a + strlen(a);

  if (c->printVars)
    a = printAttrs(c, c->printVars, a);
  else if (c->desc->printVars)
    a = printAttrs(c, c->desc->printVars, a);
  else
    {
      sprintf(a, "%s", displayAll(c));
      a = a + strlen(a);
    }
  
  sprintf(a, "}");
  return d;
}


char*
displayAll(Component* c)
{
  VariableDescriptor* vd = c->desc->variables;
  static char d[OUTBUFSIZE];
  char* a = d;

  for ( ; vd->offset != -1 ; vd++)
    {
      if (vd->kind == INPUT_KIND
	  || vd->kind == OUTPUT_KIND 
	  || vd->kind == STATE_KIND
	  || (c == _global && vd->kind == GLOBAL_KIND))
	{ 
      
	  sprintf(a, "     {%s ", vd->name);
	  a = a + strlen(a);

	  /*
	  if (vd->type->kind == NUMBER_T)
	    {
	      a = printNumberVariable(c, vd, a);
	    }
	  else
	    {
	      a = printObject(DATA_ADDRESS(void, c, vd->offset),
			      vd->type,
			      a);
	    }
	    */

	  a = printVariable(c, vd, a);
	  sprintf(a, "}\n");
	  a = a + strlen(a);
	}
    }

  if (c->M)
    {
      sprintf(a, "     {discrete %s}\n", c->M->name);
      a =  a + strlen(a);
    }

  return d;
}


int
debugger_display(Component* c)
{
  VariableDescriptor* vd = c->desc->variables;
  static char d[2000];
  char* a = d;

  char* display_var_info(Component*, VariableDescriptor*, char*);

  sprintf(a, "{%s %d", c->desc->name, c->name);
  a = a + strlen(a);

  if (c->M)
    {
      sprintf(a, " {discrete %s}\n", c->M->name);
      a =  a + strlen(a);
    }
  else
    {
      sprintf(a, "\n");
      a = a + strlen(a);
    }


  for ( ; vd->offset != -1 ; vd++)
    {
      sprintf(a, "     {");
      a = a + strlen(a);
      a = display_var_info(c, vd, a);
      a = printVariable(c, vd, a);
      sprintf(a, "}\n");
      a = a + strlen(a);
    }

  sprintf(a, "}");

  printf("%s\n", d);
  return 1;
}


/* display_var_kind --
 * Prints the kind of the variable.
 */
char*
display_var_kind(VariableDescriptor* vd, char* a)
{
  switch (vd->kind)
    {
    case INPUT_KIND:
      sprintf(a, "input: ");
      break;

    case OUTPUT_KIND:
      sprintf(a, "output:");
      break;

    case STATE_KIND:
      sprintf(a, "state: ");
      break;

    case GLOBAL_KIND:
      sprintf(a, "global: ");
      break;

    case DIFFERENTIAL_KIND:
      sprintf(a, "differential: ");
      break;

    case TEMPORARY_KIND:
      sprintf(a, "temp: ");
      break;

    case MODE_KIND:
      sprintf(a, "mode: ");
      break;

    case CONTEXT_KIND:
      sprintf(a, "context: ");
      break;

    case FLOW_FUNCTION_KIND:
      sprintf(a, "ff: ");
      break;
 
    case BP_SET_KIND:
      sprintf(a, "backptr: ");
      break;

    case SYNC_VAR_KIND:
      sprintf(a, "sync: ");
      break;

    case LOCAL_KIND:
      sprintf(a, "local: ");
      break;

    case SELF_KIND:
      sprintf(a, "self: ");
      break;

    case TYPE_KIND:
      sprintf(a, "type: ");
      break;

    case NO_KIND:
      sprintf(a, "no kind: ");
      break;

    default:
      sprintf(a, "other: ");
      break;
    }
  a = a + strlen(a);
  return a;
}


/* display_var_info --
 * Prints the information for the variable.
 */
char*
display_var_info(Component* c, VariableDescriptor* vd, char* a)
{
  char* variable_mode_string(Component*, VariableDescriptor*);

  a = display_var_kind(vd, a);
  sprintf(a, " @%d %s ", vd->offset, vd->name);
  a = a + strlen(a);

  /* Print a mode if reasonable.
   * Careful! There is quite an assumption here! Check the
   * definition of DIFFERENTIAL_MODE et al. to change this condition.
   * Marco Antoniotti 19961209
   */
  if (vd->possible_modes >= DIFFERENTIAL_MODE
      && vd-> possible_modes <= CONSTANT_MODE)
    {
      sprintf(a, " [%s] ", variable_mode_string(c, vd));
      a = a + strlen(a);
    }
  return a;
}


/* sdi -- SHIFT Debugger Display.  Displays a component completely.
 * Function used in gdb.
 */
void
sdi(Component* c)
{
  if (c != 0)
    (void) debugger_display(c);
  else
    fprintf(stderr, "SHIFT Debug: null 'Component*' pointer.\n");
}


/* debugger_trans_display -- displays a transition in readable form
 */
void
debugger_trans_display(TransitionDescriptor *td)
{
  static char d[2000];
  char* a = d;
  EventDescriptor** local_evs;
  ExternalEventDescriptor** external_evs;

  char* print_external_event(ExternalEventDescriptor* ext_ev, char* a);
  
  sprintf(a, "(transition to %s\n", td->to->name);
  a = a + strlen(a);

  /* Printing local events */
  sprintf(a, "   local (");
  a = a + strlen(a);
  for (local_evs = td->events ; *local_evs ; local_evs++)
    {
      sprintf(a, "%s%s",
	      (*local_evs)->name,
	      *(local_evs + 1) == 0 ? "" : " ");
      a = a + strlen(a);
    }
  sprintf(a, ")\n");
  a = a + strlen(a);

  /* Printing external events */
  sprintf(a, "   external (");
  a = a + strlen(a);

  for (external_evs = td->sync_list; *external_evs; external_evs++)
    {
      a = print_external_event(*external_evs, a);
      if (*(external_evs + 1) == 0)
	sprintf(a, ")\n");
      else
	sprintf(a, "\n             ");
      a = a + strlen(a);
    }
  sprintf(a, ")\n");
  a = a + strlen(a);

  fprintf(stdout, "%s", d);
  fflush(stdout);
}


void
sdi_t(TransitionDescriptor* td)
{
  if (td != 0)
    debugger_trans_display(td);
}


char*
print_external_event(ExternalEventDescriptor* eed, char* a)
{
  sprintf(a, "(%s %s connected_at_offset %d",
	  eed->name,
	  eed->connection_type == SINGLE_CONNECTION ? "single" :
	  eed->sync_type == SYNC_ONE ? "one" :
	  "set",
	  eed->link_var_offset);
  a = a + strlen(a);

  if (eed->connection_type == SET_CONNECTION
      && eed->sync_type == SYNC_ONE)
    sprintf(a, " sync_variable_at_offset %d)", eed->sync_var_offset);
  else
    sprintf(a, ")");
  a = a + strlen(a);
  return a;
}


char *
printObject(void *x, TypeDescriptor *td, char *a)
{
  switch (td->kind)
    {
    case NUMBER_T:
      a = printNumber((double *) x, a);
      break;
    case SYMBOL_T:
      a = printSymbol((int *) x, a);
      break;
    case COMPONENT_T:
      a = printComponent((Component **) x, a);
      break;
    case SET_T:
      a = printSet((GenericSet **) x, a);
      break;
    case ARRAY_T:
      a = printArray((GenericArray **) x, a);
      break;
    }
  return a;
}


/* Luigi, Marco, Michael 19960613
 * Changed calls to _AA_ and _DA_ macros
 */
char*
printNumberVariable(Component* c, VariableDescriptor* vd, char* a )
{
  double x;
  switch (vd->possible_modes)
    {
    case CONSTANT_MODE: 
      x = _VA_(c, double, vd->offset); 
      break;
    case ALGEBRAIC_MODE: 
      x = _AA_(c, vd->mode_offset, vd->offset, vd->ff_offset,
	       vd->context_offset, 0); 
      break;
    case DIFFERENTIAL_MODE:
      x = _DA_(c, vd->mode_offset, vd->state_offset, vd->ff_offset,
	       vd->context_offset, 0); 
      break;
    default:
      fprintf(stderr,
	      "SHIFT Debugger: printNumerVariable: unknown variable %d\n",
	      vd->possible_modes);
      exit(1);
      break;
    }
  sprintf(a, "%.*f", shift_debug_precision, x);
  a = a + strlen(a);
  return a;
}


char* 
printComponent(Component** pc, char* a)
{
  Component* c = *pc;

  if (c != NULL)
    {
      /* Old code:
	 sprintf(a, "(%s %d @%x)", c->desc->name, c->name, c);
	 */
      sprintf(a, "(%s %d)", c->desc->name, c->name);
      a = a + strlen(a);
    }
  else
    {
      sprintf(a, "#<UNDEFINED>");
      a = a + strlen(a);
    }
  
  return a;
}

       
char* 
printNumber(double* c, char* a)
{
  sprintf(a, "%.*f", shift_debug_precision, *c);
  a = a + strlen(a);
  return a;
}
       

char*
printSet(GenericSet** gcp, char* a)
{
  GenericSet* gs = *gcp;

  sprintf(a, "{");
  a = a + strlen(a);
  if ((gs != NULL) && (gs->size != 0))
    {
      TypeDescriptor *et = gs->et;
    
      FOR_ALL_ELEMENTS(d, gs) {
	a = printObject(&d, et, a);
	sprintf(a, ", ");
	a = a + strlen(a);
      } END_FAE;
      a = a - 2;
    }
  else if (gs == NULL)
    {
      sprintf(a, "UNINITIALIZED_SET");
      a = a + strlen(a);
    }
  sprintf(a, "}");
  a = a + strlen(a);
  return a;
}


void
debugger_print_set(GenericSet *s)
{
  char buffer_[1024];
  char* buffer = buffer_;

  sprintf(buffer, "Set == {");
  buffer += strlen(buffer);
  if ((s != NULL) && (s->size != 0))
    {
      TypeDescriptor *et = s->et;
    
      FOR_ALL_ELEMENTS(d, s) {
	switch(et->kind)
	  {
	    /* case SET_T:
	    buffer = printObject(d.v, et, buffer);
	    break; */

	  default:
	    buffer = printObject((void*) &d, et, buffer);
	    break;
	  }
	sprintf(buffer, ", ");
	buffer += strlen(buffer);
      } END_FAE;
      buffer -= 2;
    }
  else if (s == NULL)
    {
      sprintf(buffer, "UNINITIALIZED_SET");
      buffer += strlen(buffer);
    }
  sprintf(buffer, "}");
  buffer += strlen(buffer);
  printf("%s\n", buffer_);
  fflush(stdout);
}


char*
printArray(GenericArray** gcp, char* a)
{
  GenericArray* gs = *gcp;

  sprintf(a, "[");
  a = a + strlen(a);
  if ((gs != NULL) && (gs->l != 0))
    {
      int i, j;
      TypeKind typ;
      TypeDescriptor *et = gs->et;
      i = gs->l;

      typ = gs->et->kind;

      for (j = 0; j < i; j++)
	{
	  switch (typ)
	    {
	    case NUMBER_T:
	      a = printObject(double_index(gs, j), et, a);
	      break;
	    case SYMBOL_T:
	    case LOGICAL_T:
	      a = printObject(int_index(gs, j), et, a);
	      break;
	    default:
	      a = printObject(voidstar_index(gs, j), et, a);
	      break;
	    }
	  sprintf(a, ", ");
	  a = a + strlen(a);
	} 
      a = a - 2;
    }
  else if (gs == NULL)
    {
      sprintf(a,"UNINITIALIZED_ARRAY");
      a = a + strlen(a);
    }
  sprintf(a, "]");
  a = a + strlen(a);

  return a;
}


char*
printCollective(int sequence)
{
  int i; /* Beware!!!! i is used by FOR_ALL_READY_COMPONENTS */
  static char d[OUTBUFSIZE];
  char* a = d;

  sprintf(a, "{collective transition %d", sequence);
  a = a + strlen(a);
  FOR_ALL_READY_COMPONENTS {
    sprintf(a, " ");
    a = a + strlen(a);
    a = printComponent(&c, a);
    a = a + strlen(a);
  } END_FARC;
  sprintf(a, "}\n");
  a = a + strlen(a);
      
  return d;
}


char*
printTran(Component* c, TransitionDescriptor* td, int id, char* text)
{
  /* id identifies the collective transition */
  ComponentTypeDescriptor* ctd = c->desc;
  ModeDescriptor* md = c->M;
  EventDescriptor* ed = td->events[0];
  ExternalEventDescriptor* eed = td->sync_list[0];
  static char d[OUTBUFSIZE];
  char* a = d;
  int i = 0;
  int stupidMacroShortcoming;
  int coma = 0;

  /* 1. Print the header of the transition */
  sprintf(a, "{%s %d \n     ", ctd->name, c->name);
  a = a + strlen(a);
  sprintf(a, "{%s collective transition %d}\n     ", text, id);
  a = a + strlen(a);
  sprintf(a, "{transition %s -> %s}\n     ", md->name, td->to->name);
  a = a + strlen(a);
  sprintf(a, "{exported");
  a = a + strlen(a);

  /* 2. Print the exported events of the transition */
  coma = 0;
  while (ed)
    {
      if (coma)
	{
	  sprintf(a, ",");
	  a = a + strlen(a);
	}
      else
	coma = 1;
      sprintf(a, " %s", ed->name);
      a = a + strlen(a);
      ed = td->events[++i];
    }
  sprintf(a, "}\n     {external");
  a = a + strlen(a);

  /* 3. Print the external events of the transition */
  i = 0;
  coma = 0;
  while (eed)
    {
      /* Old code:
	 Component* tC = eed->global? _global : c;
	 */

      if (coma)
	{
	  sprintf(a, ",");
	  a = a + strlen(a);
	}
      else
	coma = 1;

      if (eed->connection_type == SINGLE_CONNECTION)
	{
	  /* Old code:
	     Component* ec = DATA_ITEM(Component*, tC, eed->link_var_offset);
	     if (ec)
	     {
	     sprintf(a, " {%s single ", eed->event->name);
	     a = a + strlen(a);
	     a = printComponent(&ec, a);
	     sprintf(a, "}");
	     a = a + strlen(a);
	     }
	     else
	     {
	     sprintf(a, "{%s 0}", eed->event->name); 
	     a = a + strlen(a);
	     }
	     */
	  sprintf(a, " %s:%s", eed->var_name, eed->event->name);
	  a = a + strlen(a);
	}
      else if (eed->sync_type == SYNC_ALL)
	{
	  /* Old code:
	     sprintf(a, "{%s set ", eed->event->name);
	     a = a + strlen(a);
	     stupidMacroShortcoming = eed->link_var_offset;
	     a = printSet(DATA_ADDRESS(GenericSet*, tC, stupidMacroShortcoming),
	     a);
	     sprintf(a, "}");
	     a = a + strlen(a);
	     */
	  sprintf(a, " %s:%s(one)", eed->var_name, eed->event->name);
	  a = a + strlen(a);
	}
      else if ((eed->sync_type == SYNC_ONE) && (eed->sync_var_offset != -1))
	{
	  /* Old code:
	     sprintf(a, "{%s one ", eed->event->name);
	     a = a + strlen(a);
	     stupidMacroShortcoming = eed->sync_var_offset;
	     a = printComponent(DATA_ADDRESS(Component*,
	     c,
	     stupidMacroShortcoming),
	     a);
	     sprintf(a, "}");
	     a = a + strlen(a);
	     */
	  sprintf(a, " %s:%s(all)", eed->var_name, eed->event->name);
	  a = a + strlen(a);
	}

      eed = td->sync_list[++i];
    }
  sprintf(a, "}\n");
  a = a + strlen(a);

  /* 4. Print the traced variables */
  if (c->printVars)
    a = printAttrs(c, c->printVars, a);
  else if (c->desc->printVars)
    a = printAttrs(c, c->desc->printVars, a);
  else
    {
      sprintf(a, "%s", displayAll(c));
      a = a + strlen(a);
    }

  sprintf(a, "}\n");
  return d;
}


VariableDescriptor**
constructAttrList(ComponentTypeDescriptor* ctd, char* rS)
{
  char* tail = safe_malloc(CMDBUFSIZE * sizeof(char));
  char* name = safe_malloc(NAMESIZE * sizeof(char));
  int i = 0; int j, m;
  int searching = 1;

  VariableDescriptor* vd = ctd->variables;
  VariableDescriptor** pV;
  
  if (rS[0] == '-' && rS[1] == 'v')
    {
      /* We have an attribute list, let us count them*/
      j = 2;
      m = strlen(rS);
      while (j < m)
	{
	  j = getToken(rS+j, name, tail, " ") + j;
	  if (name[0] == '-')
	    break; /* Hit next list */
	  i++;
	}
      /* We are printing i attributes */
      pV = (VariableDescriptor**)safe_malloc((i + 1)
					     * sizeof(VariableDescriptor*));
    
      i = 0;
      j = 2;
    
      while (j < m)
	{
	  j = getToken(rS+j, name, tail, " ") + j;
	  if ( name[0] == '-' )
	    break; /* Hit next list */
	  vd = ctd->variables;
	  searching = 1;
	  for (; vd->offset != -1 && searching ; vd++)
	    {
	      if (vd->kind == INPUT_KIND
		  || vd->kind == OUTPUT_KIND 
		  || vd->kind == STATE_KIND)
		{ 
		  if (!strcmp(name,  vd->name))
		    {
		      pV[i++] = vd;
		      searching = 0;
		    }
		}
	    }
	  if (searching)
	    {
	      printf("Variable %s not found\n", name);
	    }
	}
      pV[i] = 0;
    }
  else
    {
      /* We are printing all attributes */
      i = 0;
      for (; vd->offset != -1; vd++)
	{
	  if (vd->kind == INPUT_KIND
	      || vd->kind == OUTPUT_KIND 
	      || vd->kind == STATE_KIND)
	    { 
	      i++;
	    }
	}

      vd = ctd->variables;
      pV = (VariableDescriptor**)safe_malloc((i + 1)
					     * sizeof(VariableDescriptor*));
      i = 0; 
      for (; vd->offset != -1; vd++)
	{
	  if (vd->kind == INPUT_KIND
	      || vd->kind == OUTPUT_KIND 
	      || vd->kind == STATE_KIND)
	    { 
	      pV[i++] = vd;
	    }
	}
      pV[i] = 0;
    }
  return pV;
}
   

char*
printGlobal()
{
  Component* c = _global;
  VariableDescriptor* vd = c->desc->variables;
  static char d[OUTBUFSIZE];
  char* a = d;

  sprintf(a, "{Global\n");
  a = a + strlen(a);
  for (; vd->offset != -1; vd++)
    {
      if (vd->kind == GLOBAL_KIND)
	{ 
      	  sprintf(a, "     {%s ", vd->name);
	  a = a + strlen(a);

	  /*
	  if (vd->type->kind == NUMBER_T)
	    a = printNumberVariable(c, vd, a);
	  else
	    a = printObject(DATA_ADDRESS(void, c, vd->offset), vd->type, a);
	    */

	  a = printVariable(c, vd, a);
	  sprintf(a, "}\n");
	  a = a + strlen(a);
	}
    }
  sprintf(a, " }\n");

  return d;
}


char*
printSymbol(int* num, char* a)
{
  /* Check that *num is within the bounds of the symbol table. */
  if (*num < first_shift_symbol || *num >= last_shift_symbol)
    {
      strcpy(a, "#<UNDEFINED>");
      a = a + strlen(a);
      return a;
    }
  else
    {
      /* Get the entry corresponding to the index and return it. */
      strcpy(a, shift_symbol_table[(*num) - first_shift_symbol]);
      a = a + strlen(a);
      return a;
    }
}


char*
getCompId(char* rS, int* index)
{
  char* name = safe_malloc(NAMESIZE * sizeof(char));
  char* idName = safe_malloc(IDSIZE * sizeof(char));
  char* tail = safe_malloc(CMDBUFSIZE * sizeof(char));
  int id, z;

  z = getToken(rS, name, tail, " ");
  id = getToken(rS + z, idName, tail, " ") + z;

  *index = atoi(idName);
  if ((idName[0] == '\0') || (*index == 0 && idName[0] != '0'))
    /* rS must be a bad string */
    return (char*) NULL;
  else
    return name;

  /* Old code:
     char* name = (char*)malloc(NAMESIZE*sizeof(char));
     char* idName = (char*)malloc(IDSIZE*sizeof(char));
     char* tail = (char*)malloc(CMDBUFSIZE*sizeof(char));
     int id, z;

     z = getToken(rS, name, tail, " ");
     *index = getToken(rS + z, idName, tail, " ") + z;

     id = atoi(idName);
     if ((idName[0] == '\0') || (id == 0 && idName[0] != '0'))
     return (char*) NULL;
     else
     return shift_tools_find_component(name, id);
     */     
}


char *
printAttrs(Component* c, VariableDescriptor** myPV, char* a)
{
  int i = 0;
  VariableDescriptor* vd;

  while ((vd = myPV[i++]) != 0)
    {
      sprintf(a, "     {%s ", vd->name);
      a = a + strlen(a);

      /*
      if (vd->type->kind == NUMBER_T)
	a = printNumberVariable(c, vd, a);
      else
	a = printObject(DATA_ADDRESS(void, c, vd->offset), vd->type, a);
      */

      a = printVariable(c, vd, a);
      sprintf(a, "}\n");
      a = a + strlen(a);
    }

  return a;
}


/* variable_mode_string --
 * This function and the static support variable depend on the
 * definition of the DIFFERENTIAL, ALGEBRAIC and CONSTANT MODE macros
 * in 'shifttypes.h.
 * Any change must be reflected both ways.
 *
 * Marco Antoniotti 19961209
 */
static char* mode_string_table[4] = {"UNKNOWN_MODE",
				     "DIFFERENTIAL_MODE",
				     "ALGEBRAIC_MODE",
				     "CONSTANT_MODE"
};


char*
variable_mode_string(Component* c, VariableDescriptor* var_desc)
{
  int var_current_mode = DATA_ITEM(int, c, var_desc->mode_offset);
  
  if (var_current_mode < 0 || var_current_mode > 3)
    {
      /*
      shift_tools_warning("trying to print a variable mode");
      shift_tools_warning("improperly set;");
      shift_tools_warning("this might be a problem with the runtime;");
      shift_tools_warning("typing UNKNOWN");
      */
      var_current_mode = 0;
    }
  return mode_string_table[var_current_mode];
}


/* printVariable -- Prints a variable from a component. */
char*
printVariable(Component* c, VariableDescriptor* vd, char* a)
{
  double temp_number;
  Component* temp_component = 0;
  GenericArray* temp_array  = 0;
  GenericSet* temp_set      = 0;

  switch (vd->possible_modes)
    {
    case CONSTANT_MODE:
      a = printObject(DATA_ADDRESS(void, c, vd->offset), vd->type, a);
      break;

    case ALGEBRAIC_MODE:
      /* if (!ensure_flow_function(c, vd->ff)) {} */
      switch (vd->type->kind)
	{
	case NUMBER_T:
	  temp_number = _AAT_(c,
			      vd->mode_offset,
			      vd->offset,
			      double,
			      vd->ff_offset,
			      DoubleFlowFunction,
			      vd->context_offset,
			      0);
	  a = printObject((void*) &temp_number, vd->type, a);
	  break;

	case COMPONENT_T:
	  temp_component = _AAT_(c,
				 vd->mode_offset,
				 vd->offset,
				 Component*,
				 vd->ff_offset,
				 ComponentFlowFunction,
				 vd->context_offset, 0);
	  a = printObject((void*) &temp_component, vd->type, a);
	  break;

	case ARRAY_T:
	  temp_array = _AAT_(c,
			     vd->mode_offset,
			     vd->offset,
			     GenericArray*,
			     vd->ff_offset,
			     ArrayFlowFunction,
			     vd->context_offset, 0);
	  a = printObject((void*) &temp_array, vd->type, a);
	  break;

	case SET_T:
	  temp_set = _AAT_(c,
			   vd->mode_offset,
			   vd->offset,
			   GenericSet*,
			   vd->ff_offset,
			   SetFlowFunction,
			   vd->context_offset, 0);
	  a = printObject((void*) &temp_set, vd->type, a);
	  break;

	case SYMBOL_T:
	  shift_tools_error("accessing algebraic symbol variable.");
	  break;

	default:
	  shift_tools_error("accessing algebraical variable of unknown type.");
	  break;
	}
      break;

    case DIFFERENTIAL_MODE:
      if (vd->type->kind == NUMBER_T)
	{
	  temp_number = _DA_(c,
			     vd->mode_offset,
			     vd->state_offset,
			     vd->ff_offset,
			     vd->context_offset,
			     0); 
	  sprintf(a, "%.*f", shift_debug_precision, temp_number);
	  a = a + strlen(a);
	}
      else
	{
	  shift_tools_error("accessing a differentially defined non number");
	}
      break;

    default:
      shift_tools_error("unknown variable mode");
      break;
    }

  return a;
}


#endif /* SHIFT_TOOLS_I */

/* end of file -- tools.c -- */
