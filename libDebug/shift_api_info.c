/* -*- Mode: C -*- */


/* shift_api_info.c -- */


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


#ifndef SHIFT_API_INFO_I
#define SHIFT_API_INFO_I

#include <shift_config.h>
#include <gc_support.h>

#include <ctype.h>
#include <shift_debug.h>

#include "run-time.h"
#include "shifttypes.h"
#include "shift_api_types.h"
#include "shift_api_internal.h"
#include "shift_api_info.h"
#include "shift_tools.h"


/* Reports an error and exits */
void
shift_api_error(char* msg)
{
  fprintf(stderr, "SHIFT api: error: %s\n", msg);
  fflush(stderr);
  exit(1);
}


/* Reports a warning */
void
shift_api_warning(char* msg)
{
  fprintf(stderr, "SHIFT api: warning: %s\n", msg);
  fflush(stderr);
}


/* Creates a mode from a ModeDescriptor */
shift_api_mode*
shift_api_create_mode(ModeDescriptor* md, shift_api_user_type* type)
{
  shift_api_mode* res = new(shift_api_mode);
  int iter = 0;

  /* 1. Copy the name field */
  res->name = safe_malloc(strlen(md->name));
  strcpy(res->name, md->name);

  /* 2. Set the type field */
  res->type = type;

  /* 3. Copy the guarged field */
  res->guarded = md->has_guards;

  /* 4. Count the number of outgoing transitions of the mode: the list is
   *    NULL terminated
   */
  while (md->outgoing[iter])
    iter++;

  /* 5. Allocate the memory: we need one more element for the NULL */
  res->outs = (shift_api_transition**)
    safe_malloc(sizeof(shift_api_transition*) * (iter+1));

  /* 6. Fill the list of outgoing transitions */
  iter = 0;
  while (md->outgoing[iter])
    {
      res->outs[iter] = shift_api_create_transition(md->outgoing[iter], type);
      res->outs[iter]->source = res;
      iter++;
    }
  res->outs[iter] = (shift_api_transition*) NULL;

  /* 7. The list of ingoing transitions can not be computed until all the
   *    lists of outgoing transitions have been computed
   */
  res->ins = (shift_api_transition**) NULL;

  /* 8. Return the result */
  return res;
}


/* Retrieves a particular mode of a user type */
shift_api_mode*
shift_api_find_mode(char* name, shift_api_user_type* type)
{
  int iter = 0;

  /* 1. Scan the list of modes of the type: the list is NULL terminated */
  while (type->modes[iter])
    {
      if (!strcmp(name, type->modes[iter]->name))
	return type->modes[iter];
      iter++;
    }

  /* 2. No matching modes were found */
  return (shift_api_mode*) NULL;
}


/* Creates an exported event from an EventDescriptor */
shift_api_exported_event*
shift_api_create_exported_event(EventDescriptor* ed)
{
  shift_api_exported_event* res = new(shift_api_exported_event);

  /* 1. Copy the name */
  res->name = safe_malloc(strlen(ed->name));
  strcpy(res->name, ed->name);
  res->backlink_var_offset = ed->backlink_var_offset;
  /* 2. The list of exporting transitions will be computed once all the 
   *    exported events and all the tramsitions have been created
   */
  res->trans = (shift_api_transition**) NULL;

  /* 3. Return the result */
  return res;
}


/* Creates an external event from an ExternalEventDescriptor */
shift_api_external_event*
shift_api_create_external_event(ExternalEventDescriptor* eed)
{
  shift_api_external_event* res = new(shift_api_external_event);

  /* 1. Copy the name */
  res->name = safe_malloc(strlen(eed->event->name));
  strcpy(res->name, eed->event->name);

  /* 2. Set the global field */
  res->global = eed->global;

  /* 3. Set the kind field */
  if (eed->connection_type == SINGLE_CONNECTION)
    res->kind = SINGLE_C;
  else if (eed->connection_type == SET_CONNECTION)
    {
      if (eed->sync_type == SYNC_ONE)
	res->kind = SET_ONE_C;
      else if (eed->sync_type == SYNC_ALL)
	res->kind = SET_ALL_C;
      else
	res->kind = UNKNOWN_C;
    }
  else
    res->kind = UNKNOWN_C;

  /* 4. Set the var field */
  res->var_name = safe_malloc(strlen(eed->var_name));
  strcpy(res->var_name, eed->var_name);
  res->sync_var_offset = eed->sync_var_offset;

  /* 5. Return the result */
  return res;
}


/* Creates a transition from a TransitionDescriptor */
shift_api_transition*
shift_api_create_transition(TransitionDescriptor* td, 
			    shift_api_user_type* type)
{
  shift_api_transition* res = new(shift_api_transition);
  int iter = 0;

  /* 1. The source field must be set by the calling function */
  res->source = (shift_api_mode*) NULL;

  /* 2. At this point, all the modes are not created so we cannot
   *    retrieve the destination mode of the transition. In order
   *    to retrieve it correctly later (in shift_api_create_user_type),
   *    we create a dummy mode with just the name. The correct structure
   *    will be retrieved through this name
   */
  res->dest = new(shift_api_mode);
  res->dest->name = safe_malloc(strlen(td->to->name));
  strcpy(res->dest->name, td->to->name);

  /* 3. Count the number of exported events: the list is null terminated */
  while (td->events[iter])
    iter++;

  /* 4. Allocate the memory: we need one more element for the NULL */
  res->exports = (shift_api_exported_event**)
    safe_malloc(sizeof(shift_api_exported_event*) * (iter+1));

  /* 5. Fill the list of exported events by retrieving them from type */
  iter = 0;
  while (td->events[iter])
    {
      res->exports[iter] = 
	shift_api_find_type_exported_event(td->events[iter]->name, type);
      iter++;
    }
  res->exports[iter] = (shift_api_exported_event*) NULL;

  /* 6. Count the number of external events: the list is NULL terminated */
  iter = 0;
  while (td->sync_list[iter])
    iter++;

  /* 7. Allocate the memory: we need one more element for the NULL */
  res->externs = (shift_api_external_event**)
    safe_malloc(sizeof(shift_api_external_event*) * (iter+1));

  /* 8. Fill the list of external events */
  iter = 0;
  while (td->sync_list[iter])
    {
      res->externs[iter] =
	shift_api_create_external_event(td->sync_list[iter]);
      iter++;
    }
  res->externs[iter] = (shift_api_external_event*) NULL;

  /* 9. Fill the field api_trans of the TransitionDescriptor */
  td->api_trans = res;

  /* 10. Return the result */
  return res;
}


/* Creates a variable from a VariableDescriptor */
shift_api_variable*
shift_api_create_variable(VariableDescriptor* vd)
{
  shift_api_variable* res = new(shift_api_variable);

  /* 1. Copy the name */
  res->name = safe_malloc(strlen(vd->name));
  strcpy(res->name, vd->name);

  /* 2. Set the kind field */
  switch (vd->kind)
    {
    case GLOBAL_KIND:
      res->kind = GLOBAL_V;
      break;

    case INPUT_KIND:
      res->kind = INPUT_V;
      break;

    case OUTPUT_KIND:
      res->kind = OUTPUT_V;
      break;

    case STATE_KIND:
      res->kind = STATE_V;
      break;

    default:
      shift_api_error("variable of unknown kind");
      break;
    }

  /* 3. Set the possible_modes field */
  switch (vd->possible_modes)
    {
    case CONSTANT_MODE:
      res->possible = CONSTANT_M;
      break;

    case ALGEBRAIC_MODE:
      res->possible = ALGEBRAIC_M;
      break;

    case DIFFERENTIAL_MODE:
      res->possible = DIFFERENTIAL_M;
      break;

    default:
      shift_api_error("variable of unknown mode");
      break;
    }

  /* 4. Set the type field */
  res->type = shift_api_create_type(vd->type);

  /* 5. Return the result */
  return res;
}


/* Creates a user type from a ComponentTypeDescriptor */
shift_api_user_type*
shift_api_create_user_type(ComponentTypeDescriptor* ctd)
{
  shift_api_user_type* res = new(shift_api_user_type);
  shift_api_transition** trans;
  int iter = 0;
  int nb_trans = 0;

  /* 1. Copy the name field */
  res->name = safe_malloc(strlen(ctd->name));
  strcpy(res->name, ctd->name);

  /* 2. Count the number of exported events */
  while (ctd->events[iter])
    iter++;

  /* 3. Allocate the memory: we need one more element for the NULL */
  res->events = (shift_api_exported_event**)
    safe_malloc(sizeof(shift_api_exported_event*) * (iter+1));

  /* 4. Fill the list of exported events */
  iter = 0;
  while (ctd->events[iter])
    {
      res->events[iter] =
	shift_api_create_exported_event(ctd->events[iter]);
      iter++;
    }
  res->events[iter] = (shift_api_exported_event*) NULL;

  if (  (!strncmp(ctd->modes[0]->name, "loop~", 5)) && 0) /*** MAK ***/
    {
      /* 5. If the first mode is "loop~X", create an empty list of modes */
      res->modes = (shift_api_mode**) safe_malloc(sizeof(shift_api_mode*));
      res->modes[0] = (shift_api_mode*) NULL;

      /* 6. Create an empty list of all transitions */
      trans
	= (shift_api_transition**) safe_malloc(sizeof(shift_api_transition*));
      trans[0] = (shift_api_transition*) NULL;
    }
  else
    {
      /* 7. Else count the number of modes: the list is NULL terminated */
      iter = 0;
      while (ctd->modes[iter])
	iter++;

      /* 8. Allocate the memory: we need one more element for the NULL */
      res->modes = (shift_api_mode**)
	malloc(sizeof(shift_api_mode*) * (iter+1));

      /* 9. Fill the list of modes */
      iter = 0;
      while (ctd->modes[iter])
	{
	  res->modes[iter] = shift_api_create_mode(ctd->modes[iter], res);
	  /* Note: shift_api_create_mode also creates the list of outgoing
	   * transitions of the mode
	   */
	  iter++;
	}
      res->modes[iter] = (shift_api_mode*) NULL;

      /* 10. Count the total number of transitions of all the modes: each
       *     list is NULL terminated
       */
      iter = 0;
      while (res->modes[iter])
	{
	  int iter2 = 0;

	  while (res->modes[iter]->outs[iter2])
	    {
	      nb_trans++;
	      iter2++;
	    }
	  iter++;
	}

      /* 11. Allocate the memory: we need one more element for the NULL */
      trans = (shift_api_transition**)
	malloc(sizeof(shift_api_transition*) * (nb_trans+1));

      /* 12. Fill the list of transitions */
      iter = 0;
      nb_trans = 0;
      while (res->modes[iter])
	{
	  int iter2 = 0;

	  while(res->modes[iter]->outs[iter2])
	    {
	      trans[nb_trans] = res->modes[iter]->outs[iter2];

	      /* 13. Update the dest field of each transition */
	      trans[nb_trans]->dest =
		shift_api_find_mode(trans[nb_trans]->dest->name, res);

	      nb_trans++;
	      iter2++;
	    }
	  iter++;
	}
      trans[nb_trans] = (shift_api_transition*) NULL;

      /* 14. For each mode, create the list of ingoing transitions */
      iter = 0;
      while (res->modes[iter])
	{
	  shift_api_find_ingoing_transitions(res->modes[iter]);
	  iter++;
	}
    }

  /* 15. For each exported event, create the list of exporting transitions */
  iter = 0;
  while (res->events[iter])
    {
      int iter2 = 0;
      int nb_trans = 0;

      /* 16. Count the number of exporting transitions by scanning the
       *     list of all transitions: the list is NULL terminated
       */
      while (trans[iter2])
	{
	  if (shift_api_is_event_exported(res->events[iter], trans[iter2]))
	    nb_trans++;
	  iter2++;
	}

      /* 17. Allocate the memory: we need one more element for the NULL */
      res->events[iter]->trans = (shift_api_transition**)
	malloc(sizeof(shift_api_transition*) * (nb_trans+1));

      /* 18. Fill the list of exporting transitions */
      iter2 = 0;
      nb_trans = 0;
      while (trans[iter2])
	{
	  if (shift_api_is_event_exported(res->events[iter], trans[iter2]))
	    {
	      res->events[iter]->trans[nb_trans] = trans[iter2];
	      nb_trans++;
	    }
	  iter2++;
	}
      res->events[iter]->trans[nb_trans] = (shift_api_transition*) NULL;
      iter++;
    }

  /* 19. Initialize the list of children to NULL */
  res->children = (shift_api_user_type**) NULL;

  /* 20. Initialize the list of local variables to NULL */
  res->variables = (shift_api_variable**) NULL;

  /* 21. Return the result */
  return res;
}


/* Creates a builtin type from a TypeDescriptor */
shift_api_builtin_type*
shift_api_create_builtin_type(TypeDescriptor* td)
{
  shift_api_builtin_type* res = new(shift_api_builtin_type);

  /* 1. Set the kind and the element_type fields: except for a set or an
   *    array, the element_type field is empty
   */
  switch (td->kind)
    {
    case NUMBER_T:
      res->kind = NUMBER_B;
      res->element_type = (shift_api_type*) NULL;
      break;

    case SYMBOL_T:
      res->kind = SYMBOL_B;
      res->element_type = (shift_api_type*) NULL;
      break;

    case LOGICAL_T:
      res->kind = LOGICAL_B;
      res->element_type = (shift_api_type*) NULL;
      break;

    case ARRAY_T:
      res->kind = ARRAY_B;
      res->element_type = shift_api_create_type(td->s);
      break;

    case SET_T:
      res->kind = SET_B;
      res->element_type = shift_api_create_type(td->s);
      break;

    default:
      res->kind = UNKNOWN_B;
      res->element_type = (shift_api_type*) NULL;
      break;
    }

  /* 2. Return the result */
  return res;
}


/* Creates a type from a TypeDescriptor */
shift_api_type*
shift_api_create_type(TypeDescriptor* td)
{
  shift_api_type* res = new(shift_api_type);

  /* 1. Set the kind field and depending on the kind, set the builtin_t
   *    field or the user_t field
   */
  switch (td->kind)
    {
    case NUMBER_T:
    case SYMBOL_T:
    case LOGICAL_T:
    case ARRAY_T:
    case SET_T:
      res->kind = BUILTIN_K;
      /* Create a new builtin type */
      res->t.builtin_t = shift_api_create_builtin_type(td);
      break;

    case COMPONENT_T:
      res->kind = USER_K;
      /* The user type already exists: retrieve it */
      res->t.user_t = shift_api_find_user_type(td->ct->name);
      break;

    case FOREIGN_T:
      res->kind = FOREIGN_K;
      break;

    default:
      res->kind = UNKNOWN_K;
      break;
    }

  /* 2. Return the result */
  return res;
}


/* Tests if an event is exported by a transition */
int
shift_api_is_event_exported(shift_api_exported_event* event,
			    shift_api_transition* trans)
{
  int iter = 0;

  if (trans->exports)
    while (trans->exports[iter])
      {
	if (!strcmp(event->name, trans->exports[iter]->name))
	  return 1;
	iter++;
      }
  return 0;
}


/* Initializes the list of all user types */
void
shift_api_initialize_user_types()
{
  int iter = 1;

  /* 1. Count the number of user types: the list is NULL terminated.
   *    The list begins with the "global" user type that we do not
   *	want to count: hence we begin with iter = 1
   */
  while (component_type_list[iter])
    iter++;

  /* 2. Allocate the memory: we need one more element for the NULL.
   *    Since the first user type is "global", iter is the correct count
   */
  shift_api_user_types = (shift_api_user_type**)
    safe_malloc(sizeof(shift_api_user_type*) * iter);

  /* 3. Fill the list of user types */
  iter = 1;
  while (component_type_list[iter])
    {
      shift_api_user_types[iter-1] =
	shift_api_create_user_type(component_type_list[iter]);
      iter++;
    }
  shift_api_user_types[iter-1] = (shift_api_user_type*) NULL;

  /* 4. Fill the parent field of each user type */
  iter = 1;
  while (component_type_list[iter])
    {
      if (component_type_list[iter]->parent)
	shift_api_user_types[iter-1]->parent =
	  shift_api_find_user_type(component_type_list[iter]->parent->name);
      else
	shift_api_user_types[iter-1]->parent = (shift_api_user_type*) NULL;
      iter++;
    }
}


/* Initializes the list of global variables */
void
shift_api_initialize_global_variables()
{
  ComponentTypeDescriptor* ctd = shift_tools_find_type("global");
  int iter = 0;
  int nb_var = 0;
  int nb_type = 0;

  /* 1. Check that the global type exists */
  if (ctd)
    {
      /* 2. Count the number of global variables: the list ends with a
       *    NO_KIND variable and only the variables of kind GLOBAL_KIND,
       *    must be created
       */
      while (ctd->variables[iter].kind != NO_KIND)
	{
	  if (ctd->variables[iter].kind == GLOBAL_KIND)
	    nb_var++;
	  iter++;
	}

      /* 3. Allocate the memory: we need one more element for the NULL */
      shift_api_global_variables = (shift_api_variable**)
	malloc(sizeof(shift_api_variable*) * (nb_var+1));

      /* 4. Fill the list of global variables */
      iter = 0;
      nb_var = 0;
      while (ctd->variables[iter].kind != NO_KIND)
	{
	  if (ctd->variables[iter].kind == GLOBAL_KIND)
	    {
	      shift_api_global_variables[iter] =
		shift_api_create_variable(&(ctd->variables[iter]));
	      nb_var++;
	    }
	  iter++;
	}
      shift_api_global_variables[nb_var] = (shift_api_variable*) NULL;
    }
  else
    {
      /* 5. Else create an empty list */
      shift_api_global_variables = (shift_api_variable**)
	malloc(sizeof(shift_api_variable*));
      shift_api_global_variables[0] = (shift_api_variable*) NULL;
    }
}


/* Retrieves the list of all user types */
shift_api_user_type**
shift_api_find_user_types()
{
  /* The list of user types has been created by
   * shift_api_initialize_user_types
   */
  return shift_api_user_types;
}


/* Retrieves a particular user type */
shift_api_user_type*
shift_api_find_user_type(char* name)
{
  int iter = 0;

  /* 1. Scan the list of user types: the list is NULL terminated */
  while (shift_api_user_types[iter])
    {
      if (!strcmp(shift_api_user_types[iter]->name, name))
	return shift_api_user_types[iter];
      iter++;
    }

  /* 2. No matching user types were found */
  return (shift_api_user_type*) NULL;
}


/* Retrieves the current mode of an instance of a user type */
shift_api_mode*
shift_api_find_current_mode(shift_api_instance* instance)
{
  shift_api_user_type* user_type = instance->type;
  Component* c = shift_tools_find_component(user_type->name, instance->id);

  /* 1. Check that the component exists */
  if (!c)
    return (shift_api_mode*) NULL;

  /* 2. Retrieve the current mode in the list of modes of the type */
  return shift_api_find_mode(c->M->name, user_type);
}


/* Retrieves the list of outgoing transitions of a mode */
shift_api_transition**
shift_api_find_outgoing_transitions(shift_api_mode* mode)
{
  /* The list of outgoing transitions has been created by
   * shift_api_create_mode
   */
  return mode->outs;
}


/* Retrieves the list of ingoing transitions of a mode */
shift_api_transition**
shift_api_find_ingoing_transitions(shift_api_mode* mode)
{
  /* 1. Check whether the list of ingoing transitions already exists */
  if (mode->ins)
    return mode->ins;

  /* 2. Otherwise gather all the ingoing transitions and create the list */
  if (!strncmp(mode->name, "loop~", 5))
    {
      /* 3. The implicit mode "loop~X" has no ingoing transitions: create
       *    and return and empty list of transitions
       */
      mode->ins = (shift_api_transition**)
	malloc(sizeof(shift_api_transition*));
      mode->ins[0] = (shift_api_transition*) NULL;
      return mode->ins;
    }
  else
    {
      shift_api_user_type* type = mode->type;
      shift_api_mode** modes = type->modes;
      int iter1 = 0;
      int nb_in = 0;

      /* 4. Count the number of ingoing transitions by scaning the lists
       *    of outgoing transitions: each list is NULL terminated
       */
      while (modes[iter1])
	{
	  int iter2 = 0;

	  while (modes[iter1]->outs[iter2])
	    {
	      if (!strcmp(modes[iter1]->outs[iter2]->dest->name, mode->name))
		nb_in++;
	      iter2++;
	    }
	  iter1++;
	}

      /* 5. Allocate the memory: we need one more element for the NULL */
      mode->ins = (shift_api_transition**)
	safe_malloc(sizeof(shift_api_transition*) * (nb_in + 1));
      

      /* 6. Fill the new list of pointers */
      iter1 = 0;
      nb_in = 0;
      while (modes[iter1])
	{
	  int iter2 = 0;

	  while (modes[iter1]->outs[iter2])
	    {
	      if (!strcmp(modes[iter1]->outs[iter2]->dest->name, mode->name))
		{
		  mode->ins[nb_in] = modes[iter1]->outs[iter2];
		  nb_in++;
		}
	      iter2++;
	    }
	  iter1++;
	}
      mode->ins[nb_in] = (shift_api_transition*) NULL;
      
      /* 7. Return the result */
      return mode->ins;
    }
}


/* Retrieves the list of exporting transitions of an exported event */
shift_api_transition**
shift_api_find_exporting_transitions(shift_api_exported_event* event)
{
  /* The list of exporting transitions has been created by
   * shift_api_create_user_type
   */
  return event->trans;
}


/* Retrieves the list of exported events of a transition */
shift_api_exported_event**
shift_api_find_transition_exported_events(shift_api_transition* trans)
{
  /* The list of exported events has been created by
   * shift_api_create_user_type
   */
  return trans->exports;
}


/* Retrieves the list of external events of a transition */
shift_api_external_event**
shift_api_find_external_events(shift_api_transition* trans)
{
  /* The list of external events has been created by
   * shift_api_create_transition
   */
  return trans->externs;
}


/* Retrieves the list of exported events of a user type */
shift_api_exported_event**
shift_api_find_type_exported_events(shift_api_user_type* type)
{
  /* The list of exported events has been created by
   * shift_api_create_user_type
   */
  return type->events;
}


/* Retrieves a particular exported event of a user type */
shift_api_exported_event*
shift_api_find_type_exported_event(char* name, shift_api_user_type* type)
{
  int iter = 0;

  /* 1. Scan the list of exported events of the type: the list is 
   *    NULL terminated
   */
  while (type->events[iter])
    {
      if (!strcmp(name, type->events[iter]->name))
	return type->events[iter];
      iter++;
    }

  /* 2. No matching exported events were found */
  return (shift_api_exported_event*) NULL;
}


/* Retrieves the list of modes of a user type */
shift_api_mode**
shift_api_find_modes(shift_api_user_type* type)
{
  /* The list of modes has been created by shift_api_create_user_type */
  return type->modes;
}


/* Retrieves the list of children types of a user type */
shift_api_user_type**
shift_api_find_children(shift_api_user_type* type)
{
  ComponentTypeDescriptor* ctd;
  int iter = 0;

  /* 1. Check whether the list of children already exists */
  if (type->children)
    return type->children;

  /* 2. Check that type exists */
  ctd = shift_tools_find_type(type->name);
  if (!ctd)
    return (shift_api_user_type**) NULL;

  /* 3. Count the number of children: the list is NULL terminated */
  while (ctd->children[iter])
    iter++;

  /* 4. Allocate the memory: we need one more element for the NULL */
  type->children = (shift_api_user_type**)
    safe_malloc(sizeof(shift_api_user_type*) * (iter+1));

  /* 5. Fill the list of children */
  iter = 0;
  while (ctd->children[iter])
    {
      /* All the user types already exist, so instead of creating a
       * new one we look for the existing one in shift_api_user_types
       */
      type->children[iter] =
	shift_api_find_user_type(ctd->children[iter]->name);
      iter++;
    }
  type->children[iter] = (shift_api_user_type*) NULL;

  /* 6. Return the result */
  return type->children;
}


/* Retrieves the list of local variables of a user type */
shift_api_variable**
shift_api_find_local_variables(shift_api_user_type* type)
{
  ComponentTypeDescriptor* ctd;
  int iter = 0;
  int nb_var = 0;

  /* 1. Check whether the list of local variables already exists */
  if (type->variables)
    return type->variables;

  /* 2. Check that type exists */
  ctd = shift_tools_find_type(type->name);
  if (!ctd)
    return (shift_api_variable**) NULL;

  /* 3. Count the number of local variables: the list ends with a 
   *    NO_KIND variable and only the variables of kind INPUT_KIND,
   *    OUTPUT_KIND, and STATE_KIND must be created
   */
  while (ctd->variables[iter].kind != NO_KIND)
    {
      if (ctd->variables[iter].kind == INPUT_KIND
	  || ctd->variables[iter].kind == OUTPUT_KIND
	  || ctd->variables[iter].kind == STATE_KIND)
	nb_var++;
      iter++;
    }

  /* 4. Allocate the memory: we need one more element for the NULL */
  type->variables = (shift_api_variable**)
    safe_malloc(sizeof(shift_api_variable*) * (nb_var+1));

  /* 5. Fill the list of local variables */
  iter = 0;
  nb_var = 0;
  while (ctd->variables[iter].kind != NO_KIND)
    {
      if (ctd->variables[iter].kind == INPUT_KIND
	  || ctd->variables[iter].kind == OUTPUT_KIND
	  || ctd->variables[iter].kind == STATE_KIND)
	{
	  type->variables[nb_var] =
	    shift_api_create_variable(&(ctd->variables[iter]));
	  nb_var++;
	}
      iter++;
    }
  type->variables[nb_var] = (shift_api_variable*) NULL;

  /* 6. Return the result */
  return type->variables;
}



/* Retrieves a particular local variable of a user type */
shift_api_variable*
shift_api_find_local_variable(char* name, shift_api_user_type* type)
{
  int iter = 0;

  /* 1. Retrieve the list of local variables if it does not already exist */
  if (!type->variables)
    shift_api_find_local_variables(type);

  /* 2. Scan the list of local variables: the list is NULL terminated */
  while (type->variables[iter])
    {
      if (!strcmp(name, type->variables[iter]->name))
	return type->variables[iter];
      iter++;
    }

  /* 3. No matching local variables were found */
  return (shift_api_variable*) NULL;
}


/* Retrieves the list of global variables */
shift_api_variable**
shift_api_find_global_variables()
{
  /* The list of global variables has been initialized by 
   * shift_api_initialize_global_variables
   */
  return shift_api_global_variables;
}


/* Retrieves a particular global variable */
shift_api_variable*
shift_api_find_global_variable(char* name)
{
  int iter = 0;

  /* 1. Retrieve the list of global variables if it does not already exist */
  if (!shift_api_global_variables)
    shift_api_find_global_variables();

  /* 2. Scan the list of global variables: the list is NULL terminated */
  while (shift_api_global_variables[iter])
    {
      if (!strcmp(name, shift_api_global_variables[iter]->name))
	return shift_api_global_variables[iter];
      iter++;
    }

  /* 3. No matching global variables were found */
  return (shift_api_variable*) NULL;
}


/* Retrieves the list of instances of a user type */
shift_api_instance**
shift_api_find_instances(shift_api_user_type* type)
{
  ComponentTypeDescriptor* ctd = shift_tools_find_type(type->name);
  Component* component_list;
  shift_api_instance** res;
  int iter = 0;

  /* 1. Check that type exists */
  if (!ctd)
    return (shift_api_instance**) NULL;

  /* 2. Count the number of instances: the list is NULL terminated */
  component_list = ctd->component_first;
  while (component_list)
    {
      component_list = component_list->component_next;
      iter++;
    }

  /* 3. Allocate the memory: we need one more element for the NULL */
  res = (shift_api_instance**)
    safe_malloc(sizeof(shift_api_instance*) * (iter+1));

  /* 4. Fill the list of instances */
  iter = 0;
  component_list = ctd->component_first;
  while (component_list)
    {
      res[iter] = new(shift_api_instance);
      res[iter]->type = type;
      res[iter]->id = component_list->name;
      iter++;
      component_list = component_list->component_next;
    }
  res[iter] = (shift_api_instance*) NULL;

  /* 5. Return the result */
  return res; 
}


/* Counts the number of instances of a user type */
int
shift_api_count_instances(shift_api_user_type* type)
{
  shift_api_instance** instances = shift_api_find_instances(type);
  int iter = 0;

  if (instances)
    while (instances[iter])
      iter++;
  return iter;
}


/* Retrieves a particular instance of a user type */
shift_api_instance*
shift_api_find_instance(shift_api_user_type* type, int id)
{
  Component* c = shift_tools_find_component(type->name, id);
  shift_api_instance* res;

  /* 1. Check that the instance exists */
  if (!c)
    return (shift_api_instance*) NULL;

  /* 2. Allocate the memory */
  res = new(shift_api_instance);

  /* 3. Fill the fields of the new structure */
  res->type = type;
  res->id = id;

  /* 4. Return the result */
  return res;
}

/* Retrieves the value of a global variable */
shift_api_value*
shift_api_find_global_value(char* name)
{
  shift_api_value* val;
  shift_api_type* val_type;
  Component* c = shift_tools_find_component("global", 0);
  VariableDescriptor* vd;
  double temp_num;
  Component* temp_comp;
  GenericArray* temp_array;
  GenericSet* temp_set;
  Component **comp;

  /* 1. Check that the component exists */
  if (!c)
    return (shift_api_value*) NULL;

  /* 2. Check that the variable exists */
  vd = shift_tools_find_variable_descriptor(name, c->desc);
  
  if (!vd)
    return (shift_api_value*) NULL;

  /* 3. Create the type of the value */

  /* Modification: With inheritance, sometimes if a variable is of
     type A but the instance is a SubA instance, the printing routines
     would print A. This here fixes this erroneous behaviour 
     DW 10/20/97 */
  comp = (Component**)DATA_ADDRESS(void, c, vd->offset);

  if ( vd->type->kind == COMPONENT_T && 
       (*comp)  )
    {
      TypeDescriptor *real_type;
      real_type = new(TypeDescriptor);
      real_type->s = 0;
      real_type->kind = COMPONENT_T;
      real_type->ct = (*comp)->desc;
      val_type = shift_api_create_type(real_type);
    } else {
      val_type = shift_api_create_type(vd->type);
    }

  if (!val_type)
    return (shift_api_value*) NULL;

  /* 4. Access the value depending on the mode of the variable */
  switch (vd->possible_modes)
    {
    case CONSTANT_MODE:
      val = shift_api_make_value(DATA_ADDRESS(void, c, vd->offset), val_type);
      return val;
      break;

    case ALGEBRAIC_MODE:
      switch (vd->type->kind)
	{
	case NUMBER_T:
	  temp_num = _AAT_(c,
			   vd->mode_offset,
			   vd->offset,
			   double,
			   vd->ff_offset,
			   DoubleFlowFunction,
			   vd->context_offset,
			   0);
	  val = shift_api_make_value((void*) &temp_num, val_type);
	  return val;
	  break;

	case COMPONENT_T:
	  temp_comp = _AAT_(c,
			    vd->mode_offset,
			    vd->offset,
			    Component*,
			    vd->ff_offset,
			    ComponentFlowFunction,
			    vd->context_offset,
			    0);
	  val = shift_api_make_value((void*) &temp_comp, val_type);
	  return val;
	  break;

	case ARRAY_T:
	  temp_array = _AAT_(c,
			     vd->mode_offset,
			     vd->offset,
			     GenericArray*,
			     vd->ff_offset,
			     ArrayFlowFunction,
			     vd->context_offset,
			     0);
	  val = shift_api_make_value((void*) &temp_array, val_type);
	  return val;
	  break;

	case SET_T:
	  temp_set = _AAT_(c,
			   vd->mode_offset,
			   vd->offset,
			   GenericSet*,
			   vd->ff_offset,
			   SetFlowFunction,
			   vd->context_offset,
			   0);
	  val = shift_api_make_value((void*) &temp_set, val_type);
	  return val;
	  break;

	case SYMBOL_T:
	  shift_api_error("accessing algebraic variable of type symbol");
	  break;

	case LOGICAL_T:
	  shift_api_error("accessing algebraic variable of type logical");
	  break;

	default:
	  shift_api_error("accessing algebraic variable of unknown type");
	  break;
	}
      break;

    case DIFFERENTIAL_MODE:
      if (vd->type->kind == NUMBER_T)
	{
	  temp_num = _DA_(c,
			  vd->mode_offset,
			  vd->state_offset,
			  vd->ff_offset,
			  vd->context_offset,
			  0);
	  val = shift_api_make_value((void*) &temp_num, val_type);
	  return val;
	}
      else
	shift_api_error("accessing differential variable not number");
      break;

    default:
      shift_api_error("unknown variable mode");
      break;
    }

  /* 5. We should never reach this point */
  shift_api_error("wrong control path");
}


/* Retrieves the value of a local variable of an instance of a user type */
shift_api_value*
shift_api_find_local_value(char* name, shift_api_instance* instance)
{
  shift_api_value* val;
  shift_api_type* val_type;
  Component* c;
  VariableDescriptor* vd;
  double temp_num;
  Component* temp_comp;
  GenericArray* temp_array;
  GenericSet* temp_set;
  Component **comp;
  /* 1. Check that the component exists */
  c = shift_tools_find_component(instance->type->name, instance->id);
  if (!c)
    return (shift_api_value*) NULL;

  /* 2. Check that the variable exists */
  vd = shift_tools_find_variable_descriptor(name, c->desc);
  if (!vd)
    return (shift_api_value*) NULL;

  /* 3. Create the type of the value. */

  /* Fixing the debugger info. mechanism
   * so that the instance of a variable (and not
   * its declared type) will be acquired 
   * by the debugger.
   *
   * Dan & Tunc Tag Team Effort
   * 19971023
   */

  /* When assigning the instance we have 
   * to distinguish btwn. CONSTANT & ALGEBRAIC mode.
   */
  if (vd->possible_modes == CONSTANT_MODE)
    {
      comp = (Component**) DATA_ADDRESS(void, c, vd->offset);
    }
  else if (vd->possible_modes == ALGEBRAIC_MODE)
    {
      if (vd->type->kind == COMPONENT_T)
	{
	  Component *comp_aat,
	    **p;
	  
	  comp_aat =  _AAT_(c,
			    vd->mode_offset,
			    vd->offset,
			    Component*,
			    vd->ff_offset,
			    ComponentFlowFunction,
			    vd->context_offset,
			    0);
	  comp = &comp_aat ;
	}
    }

	  
  if ( vd->type->kind == COMPONENT_T && *comp != 0 )
    {
      TypeDescriptor *real_type;

      real_type = new(TypeDescriptor);
      real_type->s = 0;
      real_type->kind = COMPONENT_T;
      real_type->ct = (*comp)->desc;
      val_type = shift_api_create_type(real_type);
    } 
  else 
    {
      val_type = shift_api_create_type(vd->type);
    }
  
  if (!val_type)
    return (shift_api_value*) NULL;

  /* 4. Access the value depending on the mode of the variable */
  switch (vd->possible_modes)
    {
    case CONSTANT_MODE:
      val = shift_api_make_value(DATA_ADDRESS(void, c, vd->offset), val_type);
      return val;
      break;

    case ALGEBRAIC_MODE:
      switch (vd->type->kind)
	{
	case NUMBER_T:
	  temp_num = _AAT_(c,
			   vd->mode_offset,
			   vd->offset,
			   double,
			   vd->ff_offset,
			   DoubleFlowFunction,
			   vd->context_offset,
			   0);
	  val = shift_api_make_value((void*) &temp_num, val_type);
	  return val;
	  break;

	case COMPONENT_T:
	  temp_comp = _AAT_(c,
			    vd->mode_offset,
			    vd->offset,
			    Component*,
			    vd->ff_offset,
			    ComponentFlowFunction,
			    vd->context_offset,
			    0);
	  val = shift_api_make_value((void*) &temp_comp, val_type);
	  return val;
	  break;

	case ARRAY_T:
	  temp_array = _AAT_(c,
			     vd->mode_offset,
			     vd->offset,
			     GenericArray*,
			     vd->ff_offset,
			     ArrayFlowFunction,
			     vd->context_offset,
			     0);
	  val = shift_api_make_value((void*) &temp_array, val_type);
	  return val;
	  break;

	case SET_T:
	  temp_set = _AAT_(c,
			   vd->mode_offset,
			   vd->offset,
			   GenericSet*,
			   vd->ff_offset,
			   SetFlowFunction,
			   vd->context_offset,
			   0);
	  val = shift_api_make_value((void*) &temp_set, val_type);
	  return val;
	  break;

	case SYMBOL_T:
	  shift_api_error("accessing algebraic variable of type symbol");
	  break;

	case LOGICAL_T:
	  shift_api_error("accessing algebraic variable of type logical");
	  break;

	default:
	  shift_api_error("accessing algebraic variable of unknown type");
	  break;
	}
      break;

    case DIFFERENTIAL_MODE:
      if (vd->type->kind == NUMBER_T)
	{
	  temp_num = _DA_(c,
			  vd->mode_offset,
			  vd->state_offset,
			  vd->ff_offset,
			  vd->context_offset,
			  0);
	  val = shift_api_make_value((void*) &temp_num, val_type);
	  return val;
	}
      else
	shift_api_error("accessing differential variable not number");
      break;

    default:
      shift_api_error("unknown variable mode");
      break;
    }

  /* 5. We should never reach this point */
  shift_api_error("wrong control path");
}


/* Makes a value */
shift_api_value*
shift_api_make_value(void* x, shift_api_type* type)
{
  shift_api_value* val = new(shift_api_value);


 switch (type->kind)
    {
    case BUILTIN_K:
      val->kind = BUILTIN_K;
      val->v.value_v = shift_api_make_builtin_value(x, type->t.builtin_t);
      return val;
      break;

    case USER_K:
      val->kind = USER_K;
      val->v.instance_v = shift_api_make_user_value((Component**) x,
						    type->t.user_t);
      return val;
      break;

      /* Modifying:
       *
       * Adding these case so that it will not exit with
       * an error when an unknown value encountered.
       *
       * Tunc Simsek 19980603
       */
    case FOREIGN_K:
      val->kind = FOREIGN_K;
      val->v.foreign_v =  *((void **) x);
      /* Note to self:
       *
       * the correct way to dereference this 
       * may also be ((vid *) x)->v
       */      
      return val;
      break;

    case UNKNOWN_K:
      val->kind = UNKNOWN_K;
      val->v.unknown_v = *((void **) x);
      /* Note to self:
       *
       * the correct way to dereference this 
       * may also be ((vid *) x)->v
       */
      return val;
      break;

    default:
      shift_api_error("unknown type kind");
      break;
    }
}


/* Makes a builtin type value value */
shift_api_builtin_value*
shift_api_make_builtin_value(void* x, shift_api_builtin_type* type)
{
  shift_api_builtin_value* val = new(shift_api_builtin_value);

  switch (type->kind)
    {
    case NUMBER_B:
      val->kind = NUMBER_B;
      val->element_type = (shift_api_type*) NULL;
      val->v.number_v = shift_api_make_number_value((double*) x);
      return val;
      break;

    case SYMBOL_B:
      val->kind = SYMBOL_B;
      val->element_type = (shift_api_type*) NULL;
      val->v.symbol_v = shift_api_make_symbol_value((int*) x);
      return val;
      break;

    case LOGICAL_B:
      val->kind = LOGICAL_B;
      val->element_type = (shift_api_type*) NULL;
      val->v.logical_v = shift_api_make_logical_value((int*) x);
      return val;
      break;

    case SET_B:
      val->kind = SET_B;
      val->element_type = type->element_type;
      val->v.set_v = shift_api_make_set_value((GenericSet**) x, 
					      type->element_type);
      return val;
      break;

    case ARRAY_B:
      val->kind = ARRAY_B;
      val->element_type = type->element_type;
      val->v.array_v = shift_api_make_array_value((GenericArray**) x,
						  type->element_type);
      return val;
      break;

    default:
      shift_api_error("unknown variable kind");
      break;
    }
}


/* Makes a number value */
double
shift_api_make_number_value(double* num)
{
  return *num;
}


/* Makes a symbol value */
char*
shift_api_make_symbol_value(int* sym)
{
  char* val;

  if (*sym < first_shift_symbol || *sym >= last_shift_symbol)
    {
      val = safe_malloc(13);
      strcpy(val, "#<UNDEFINED>");
      return val;
    }
  else
    {
      val = safe_malloc(strlen(shift_symbol_table[(*sym) - first_shift_symbol]));
      strcpy(val, shift_symbol_table[(*sym) - first_shift_symbol]);
      return val;
    }
}


/* Makes a logical value */
int
shift_api_make_logical_value(int* log)
{
  return *log;
}


/* Makes a set value */
shift_api_set_value*
shift_api_make_set_value(GenericSet** gsp, shift_api_type* type)
{
  shift_api_set_value* val;
  GenericSet* gs = *gsp;

  /* 1. Check that the generic set is not empty */
  if (!gs)
    /* An empty generic set means that it has not been initialized */
    return (shift_api_set_value*) NULL;

  /* 2. Allocate the memory */
  val = new(shift_api_set_value);

  /* 3. Set the size field */
  val->size = gs->size;

  /* 4. Set the kind field, allocate the memory (with one more element for
   *    the the NULL), and fill in the list of values
   */
  switch (gs->et->kind)
    {
    case NUMBER_T:
    case SYMBOL_T:
    case LOGICAL_T:
    case SET_T:
    case ARRAY_T:
      val->kind = BUILTIN_K;
      if (val->size == 0)
	{
	  val->e.values_e = (shift_api_builtin_value**)
	    safe_malloc(sizeof(shift_api_builtin_value*));
	  val->e.values_e[0] = (shift_api_builtin_value*) NULL;
	}
      else
	{
	  int iter = 0;

	  val->e.values_e = (shift_api_builtin_value**)
	    safe_malloc(sizeof(shift_api_builtin_value*) * (val->size + 1));
	  FOR_ALL_ELEMENTS(d, gs)
	    {
	      val->e.values_e[iter] =
		shift_api_make_builtin_value(&d, type->t.builtin_t);
	      iter++;
	    } END_FAE;
	  val->e.values_e[iter] = (shift_api_builtin_value*) NULL;
	}
      return val;
      break;

    case COMPONENT_T:
      val->kind = USER_K;
      if (val->size == 0)
	{
	  val->e.instances_e = (shift_api_instance**)
	    safe_malloc(sizeof(shift_api_instance*));
	  val->e.instances_e[0] = (shift_api_instance*) NULL;
	}
      else
	{
	  int iter = 0;

	  val->e.instances_e = (shift_api_instance**)
	    safe_malloc(sizeof(shift_api_instance*) * (val->size + 1));
	  FOR_ALL_ELEMENTS(d, gs)
	    {
	      
	      
	      shift_api_user_type *instance_type = 
		shift_api_find_user_type(((Component *)d.v)->desc->name);
	      /* Changed 7/24/97 to make the value in the set to be
		 of the correct type when printed. */
	      val->e.instances_e[iter] = 
		shift_api_make_user_value((Component**) &d, instance_type);
	      iter++;
	    } END_FAE;
	  val->e.instances_e[iter] = (shift_api_instance*) NULL;
	}
      return val;
      break;

      /* Adding the case for foreign types.
       *
       * Tunc Simsek 19980605
       */
    case FOREIGN_T:
      val->kind = FOREIGN_K;
      if (val->size == 0)
	{
	  val->e.foreign_v = (void **)
	    safe_malloc(sizeof(void*));
	  val->e.foreign_v[0] = (void *) NULL;
	}
      else
	{
	  int iter = 0;

	  val->e.foreign_v = (void **)
	    safe_malloc(sizeof(void*) * (val->size + 1));
	  FOR_ALL_ELEMENTS(d, gs)
	    {
	      val->e.foreign_v[iter] = (void *) d.v;
	      iter++;
	    } END_FAE;
	  val->e.foreign_v[iter] = (void *) NULL;
	}
      return val;
      break;

	  
    default:
      shift_api_error("unknown variable kind");
      break;
    }
}


/* Makes an array value */
shift_api_array_value*
shift_api_make_array_value(GenericArray** gap, shift_api_type* type)
{
  shift_api_array_value* val;
  GenericArray* ga = *gap;

  /* 1. Check that the generic array is not empty */
  if (!ga)
    /* An empty generic array means that it has not been initialized */
    return (shift_api_array_value*) NULL;

  /* 2. Allocate the memory */
  val = new(shift_api_array_value);

  /* 3. Set the size field */
  val->size = ga->l;

  /* 4. Set the kind field, allocate the memory (with one more element for
   *    the the NULL), and fill in the list of values
   */
  switch (ga->et->kind)
    {
    case NUMBER_T:
      val->kind = BUILTIN_K;
      if (val->size == 0)
	{
	  val->e.values_e = (shift_api_builtin_value**)
	    safe_malloc(sizeof(shift_api_builtin_value*));
	  val->e.values_e[0] = (shift_api_builtin_value*) NULL;
	}
      else
	{
	  int iter = 0;

	  val->e.values_e = (shift_api_builtin_value**)
	    safe_malloc(sizeof(shift_api_builtin_value*) * (val->size + 1));
	  for (iter = 0 ; iter < val->size ; iter++)
	    val->e.values_e[iter] = 
	      shift_api_make_builtin_value(double_index(ga, iter),
					   type->t.builtin_t);
	  val->e.values_e[iter] = (shift_api_builtin_value*) NULL;
	}
      return val;
      break;

    case SYMBOL_T:
    case LOGICAL_T:
      val->kind = BUILTIN_K;
      if (val->size == 0)
	{
	  val->e.values_e = (shift_api_builtin_value**)
	    safe_malloc(sizeof(shift_api_builtin_value*));
	  val->e.values_e[0] = (shift_api_builtin_value*) NULL;
	}
      else
	{
	  int iter = 0;

	  val->e.values_e = (shift_api_builtin_value**)
	    safe_malloc(sizeof(shift_api_builtin_value*) * (val->size + 1));
	  for (iter = 0 ; iter < val->size ; iter++)
	    val->e.values_e[iter] = 
	      shift_api_make_builtin_value(int_index(ga, iter),
					   type->t.builtin_t);
	  val->e.values_e[iter] = (shift_api_builtin_value*) NULL;
	}
      return val;
      break;

      /* Adding case for foreign types.
       *
       * Tunc Simsek 19980605
       */
    case FOREIGN_T:
      val->kind = FOREIGN_K;
      if (val->size == 0)
	{
	  val->e.foreign_v = (void **)
	    safe_malloc(sizeof(void *));
	  val->e.foreign_v[0] = (void *) NULL;
	}
      else
	{
	  int iter = 0;

	  val->e.foreign_v = (void **)
	    safe_malloc(sizeof(void *) * (val->size + 1));
	  for (iter = 0 ; iter < val->size ; iter++)
	    {
	      void **v = voidstar_index(ga, iter);
	      
	      val->e.foreign_v[iter] = *v;
	    }
	  val->e.values_e[iter] = (void*) NULL;
	}
      return val;
      break;

    case SET_T:
    case ARRAY_T:
      val->kind = BUILTIN_K;
      if (val->size == 0)
	{
	  val->e.values_e = (shift_api_builtin_value**)
	    safe_malloc(sizeof(shift_api_builtin_value*));
	  val->e.values_e[0] = (shift_api_builtin_value*) NULL;
	}
      else
	{
	  int iter = 0;

	  val->e.values_e = (shift_api_builtin_value**)
	    safe_malloc(sizeof(shift_api_builtin_value*) * (val->size + 1));
	  for (iter = 0 ; iter < val->size ; iter++)
	    val->e.values_e[iter] = 
	      shift_api_make_builtin_value(voidstar_index(ga, iter),
					   type->t.builtin_t);
	  val->e.values_e[iter] = (shift_api_builtin_value*) NULL;
	}
      return val;
      break;

    case COMPONENT_T:
      val->kind = USER_K;
      if (val->size == 0)
	{
	  val->e.instances_e = (shift_api_instance**)
	    safe_malloc(sizeof(shift_api_instance*));
	  val->e.instances_e[0] = (shift_api_instance*) NULL;
	}
      else
	{
	  int iter = 0;
	  shift_api_user_type *instance_type;
	  
	  val->e.instances_e = (shift_api_instance**)
	    safe_malloc(sizeof(shift_api_instance*) * (val->size + 1));
	  for (iter = 0 ; iter < val->size ; iter++)
	    {
	      shift_api_user_type *instance_type =
		shift_api_find_user_type((*((Component**)voidstar_index(ga,
									iter)))
					 ->desc->name);

	      val->e.instances_e[iter] = 
		shift_api_make_user_value((Component**) voidstar_index(ga,
								       iter),
					  instance_type);
	    }
	  val->e.instances_e[iter] = (shift_api_instance*) NULL;
	}
      return val;
      break;

    default:
      shift_api_error("unknown kind");
      break;
    }
}


/* ahift_api_make_user_value --
 * Makes a user type value.
 *
 * Note:  I had to add the 'else' branch, Otherwise, on some
 * platforms, the 'new' statement would leave garbage in the
 * 'val->type' field.
 * I do not know whether this is satisfactory, but I assume it is
 * (Alain should have a second look at this).
 *
 * Marco Antoniotti 19970715
 */
shift_api_instance*
shift_api_make_user_value(Component** c, shift_api_user_type* type)
{
  shift_api_instance* val = new(shift_api_instance);
  /* shift_api_type * extra_type;*/

  if (*c)
    {
      val->type = type; 
      val->id   = (*c)->name;
    }
  else
    {
      val->type = (shift_api_user_type*) 0;
      val->id   = 0;
    }
  return val;
}


shift_api_instance *
shift_api_find_single_item_in_transition(shift_api_instance * instance,
					 shift_api_external_event *
					 event)  
{
  int stupidMacroShortcoming;
  Component * found_comp, *c;
  shift_api_instance * found_instance;

  c = shift_tools_find_component(instance->type->name, instance->id);
  stupidMacroShortcoming = event->sync_var_offset;
  found_comp = *DATA_ADDRESS(Component*,
			     c,
			     stupidMacroShortcoming);
  found_instance= (shift_api_instance*)safe_malloc(sizeof(shift_api_instance));
  found_instance =
    shift_api_find_instance(shift_api_find_user_type(found_comp->desc->name), 
			   found_comp->name);
  return found_instance;
}

/* DW 6/24/97 
 * This function takes a string and a shift_api_type as arguments.
 * The string contains the name of a variable, the type is a
 * user-defined shift type. This function returns a list of
 * shift_api_values from every component in the simulation of that
 * type.
 */

shift_api_component_value**
shift_api_find_all_variable_values_of_a_type(char ** vars,
					     shift_api_user_type* type) 
{
   
   
  shift_api_instance **instances, *instance_curr;
  shift_api_value * value, **values_list;
  int total_instances, instance_index, variable_index, number_of_variables; 
  shift_api_component_value *component_value_curr,**component_values_list;
  char ** var_ptr;

  /* Get the list of instances of type 'type'. */
  instances = shift_api_find_instances(type);
  if (instances == NULL) {
    fprintf(stderr,"Type %s does not exist...\n");
    return NULL;
  }
  
  /* Count the number of instances and allocate the appropriate list */
  total_instances = shift_api_count_instances(type);
  component_values_list = (shift_api_component_value**)
    malloc(sizeof(shift_api_component_value*)*(total_instances+1));
  
  /* Count the number of variables in the vars argument */
  number_of_variables = 0;
  var_ptr = vars;
  while (var_ptr[number_of_variables] != NULL)
    number_of_variables++;

  /* Assemble the list */
  instance_index = 0;
  variable_index = 0;
  while (instances[instance_index] != NULL) {
    /* Initialize a new component element and a new instance elemet */
    
    component_value_curr = new(shift_api_component_value);
    instance_curr = new(shift_api_instance);
    values_list = (shift_api_value**)
      malloc(sizeof(shift_api_value*)*(number_of_variables+1));
    
    instance_curr = instances[instance_index];
    component_value_curr->instance = instance_curr;
    component_value_curr->values = values_list;
    variable_index = 0;
    while (vars[variable_index] != NULL) {
      /* Get value of the variable and store in list */
      value = (shift_api_value*)malloc(sizeof(shift_api_value*));
      value = shift_api_find_local_value(vars[variable_index],
					 instances[instance_index]);
      component_value_curr->values[variable_index] = value;
      
      variable_index++;
    }
    component_value_curr->values[variable_index] = NULL;
    component_values_list[instance_index] = component_value_curr;
    instance_index++;
  }
  component_values_list[instance_index] = NULL;
  
  return component_values_list;
}




char **
shift_api_parse_variables_string(char * string) 
{

  int l, index, total_vars, begin, var_index;
  char ** variable_list_ptr, **variable_list, *current_var;
  char current_variable_count;
  l = strlen(string);
  index = 0;
  total_vars = 0;

  /* Count the number of variables */

  while (index < l) {
    while (isspace(string[index]))
      index++;
    while ( (!isspace(string[index])) && (! string[index] == '\0') )
      index++;
    total_vars++;
  }

  variable_list = (char **)malloc((sizeof(char *))*(total_vars+1));

  /* Advance to the first non-space character */
  current_variable_count = 0;
  index = 0;
  while (current_variable_count < total_vars) {
    
    while (isspace(string[index]))
      index++;
    
    /* Get size of the variable and allocate memory */
    begin = index;
    while  ( (!isspace(string[index])) && (! string[index] == '\0') )
      index++;
    current_var = (char *)malloc(index-begin+1);

    /* Copy variable name into current_var; */

    var_index = begin;
    while (var_index < index) {
      current_var[var_index - begin] = string[var_index];
      var_index++;
    }
    current_var[var_index-begin] = '\0';
    
    /* Assign to variable list */
    variable_list[current_variable_count] = current_var;
    current_variable_count++;
  }

  variable_list[total_vars] = NULL;
  return variable_list;
}
      
      
#endif /* SHIFT_API_INFO_I */

/* end of file -- shift_api_info.c -- */
