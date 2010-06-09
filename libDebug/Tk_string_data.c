/* -*- Mode: C -*- */

/* Gui_info.c -- */

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

#ifndef TKSHIFT_DATA_I
#define TKSHIFT_DATA_I

#include <stdlib.h>
#include <shift_debug.h>
#include "names.h"
#include "shift_api_info.h"
#include "shift_api_types.h"
#include "shift_api_internal.h"
#include "tkShift_data.h" 
extern char Transition_Data[20000];
extern char Trace_Data[20000];
extern int Transition_Info;
extern int Trace_Info;
/* 
 * This function prints the list of components (instances) of type
 * typeName into the string 'buffer'
 *
 * I am sending the name of the type right now, but that may not be
 * necessary. Can probably remove that in the future
 *
 */

char*
GUI_listComps(char* typeName,char * buffer)
{
  shift_api_user_type * type;
  shift_api_instance ** instances;
  int loop;
  char temp[50];

  type = shift_api_find_user_type(typeName);
  instances = shift_api_find_instances(type);
  loop = 0;

  while (instances[loop] != NULL)
    {
      sprintf(temp,"{%s %d}", typeName, instances[loop]->id);
      strcat(buffer, temp);

      loop++;
    }
  /*sprintf(temp,"%d:%s%d;", Comp, typeName, Comp->name);*/

  return buffer;
}


int
GUI_get_component_values(char * typeName, int id, char * buffer) 
{
  shift_api_user_type * user_type;
  shift_api_instance * instance;
  shift_api_variable ** variables;
  shift_api_value * value;
  shift_api_mode * mode;
  int loop;

  loop = 0;
  user_type = shift_api_find_user_type(typeName);
  instance = shift_api_find_instance(user_type, id);
  if (instance != NULL)
    {
      variables = shift_api_find_local_variables(user_type); 

      /* Need to add the printing of the mode here */
      strcat(buffer, " {");
      mode = shift_api_find_current_mode(instance);
      if (mode != NULL)
	{
	  strcat(buffer, "Discrete-state ");
	  strcat(buffer,mode->name);
	}
      else
	{
	  strcat(buffer, "Discrete-state NULL");
	}
      strcat(buffer, "}");
      while (variables[loop] != NULL)
	{
	  value = shift_api_find_local_value(variables[loop]->name, instance);
	  strcat(buffer, " {");
	  strcat(buffer, variables[loop]->name);
	  strcat(buffer, " ");
	  GUI_print_variable_value(value, buffer);
	  strcat(buffer, "}");
	  loop++;
	}
    } 
  else 
    {
      strcat(buffer, " {Non-Existing Component}");
    }

  return 1;
}


void GUI_printForeign(void *, char*);
void GUI_printUnknown(void *, char*);

int
GUI_print_variable_value(shift_api_value * value, char* buffer)
{
  switch (value->kind)
    {
    case BUILTIN_K:
      GUI_printBuiltin(value->v.value_v, buffer);
      break;

      /* Adding the foreign and unknown cases.
       *
       * Tunc Simsek 19980603
       */
    case FOREIGN_K:
      GUI_printForeign(value->v.foreign_v, buffer);
      break;

    case UNKNOWN_K:
      GUI_printUnknown(value->v.unknown_v, buffer);
      break;

    case USER_K:
      GUI_printUser(value->v.instance_v, buffer);
      break;
    }
}

/* Note to self:  why do all the functions in
 * this file return a value of '1'.
 *
 * In fact, most of these functions have an
 * exact counterpart in shift_tools.c.
 *
 * Tunc 
 */
void  
GUI_printForeign(void *fv, char *b)
{
  char s[128];

  sprintf(s,
	  "%s %p (%s %d)",
	  "Foreign data @",
	  fv,
	  "Int val:",
	  *((int *) fv));
  strcat(b, s);

  return;
}

void  
GUI_printUnknown(void *fv, char *b)
{
  char s[128];
  sprintf(s,
	  "%s %p",
	  "Unknown data @",
	  fv);
  strcat(b, s);

  return;
}


int 
GUI_printVariableType(shift_api_variable * variable, char * buffer) 
{
  char temp[70];

  switch (variable->type->kind)
    {
    case BUILTIN_K:
      switch (variable->type->t.builtin_t->kind)
	{
	case NUMBER_B:
	  sprintf(temp, "0");
	  strcat(buffer,temp);
	  break;
	case SYMBOL_B:
	  sprintf(temp, "1");
	  strcat(buffer,temp);
	  break;
	case LOGICAL_B:
	  sprintf(temp, "2");
	  strcat(buffer,temp);
	  break;
	case ARRAY_B:
	  sprintf(temp, "3");
	  strcat(buffer,temp);
	  break;
	case SET_B:
	  sprintf(temp, "4");
	  strcat(buffer,temp);
	  break;
	case UNKNOWN_B:
	  sprintf(temp, "5");
	  strcat(buffer,temp);
	  break;
	}
      break;
    case USER_K:
      if (variable->type->t.user_t == NULL)
	{
	  sprintf(temp, "#<UNDEFINED>");
	  strcat(buffer,temp);
	} 
      else 
	{
	  sprintf(temp, "%s", variable->type->t.user_t->name);
	  strcat(buffer,temp);
	}
      break;
    }
}
  
 
int
GUI_printBuiltin(shift_api_builtin_value * builtin_value, char * buffer)
{
  int a;

  switch (builtin_value->kind)
    {
    case NUMBER_B:
      a = GUI_printNumber(builtin_value->v.number_v, buffer);
      break;

    case SYMBOL_B:
      a = GUI_printSymbol(builtin_value->v.symbol_v, buffer);
      break;

    case LOGICAL_B:
      a = GUI_printLogical(builtin_value->v.logical_v, buffer);
      break;

    case ARRAY_B:
      if (builtin_value->v.array_v != NULL)
	{
	  a = GUI_printArray(builtin_value->v.array_v, buffer);
	}
      else 
	strcat(buffer, "[UNINITIALIZED_ARRAY]");
      break;

    case SET_B:
      if (builtin_value->v.set_v != NULL)
	{
	  a = GUI_printSet(builtin_value->v.set_v, buffer);
	}
      else 
	strcat(buffer, "{UNINITIALIZED_SET}");
      break;

    case UNKNOWN_B:
      fprintf(stderr, "Error, I do not know this kind\n");
      break;
    }
  return a;
}


int
GUI_printUser(shift_api_instance * instance, char * buffer)
{
  char temp[35];

  temp[0] = '\0';
  if (instance->type == NULL) 
    {
      sprintf(temp, "#<UNDEFINED>");
    } 
  else 
    {
      sprintf(temp, "(%s %d)", instance->type->name, instance->id);
    }
  strcat(buffer, temp);

  return 1;
}


int
GUI_printNumber(double number, char * buffer)
{
  char temp[15];
  sprintf(temp, "%f", number);
  /*sprintf(temp, "%f", number);*/
  strcat(buffer, temp);

  return 1;
}


int
GUI_printSymbol(char * symbol, char * buffer)
{
  char temp[50];

  sprintf(temp, "%s", symbol);
  strcat(buffer, temp);
  return 1;
}


int
GUI_printLogical(int logical, char * buffer)
{
  char temp[3];

  sprintf(temp, "%d", logical);
  strcat(buffer, temp);
  return 1;
}

void  GUI_printForeignArrayValues(void **, char *);


int
GUI_printArray(shift_api_array_value * array, char * buffer)
{
  switch (array->kind)
    {
    case BUILTIN_K:
      strcat(buffer,"[");
      if (array != NULL)
	{
	  GUI_printArrayValues(array->e.values_e, buffer);
	}
      strcat(buffer,"]");
      break;

    case USER_K:
      strcat(buffer,"[");
      if (array != NULL)
	{
	  GUI_printArrayInstances(array->e.instances_e, buffer);
	}
      strcat(buffer,"]");
      break;

      /* Adding case for foreign types.
       *
       * Tunc Simsek 19980605
       */
    case FOREIGN_K:
      strcat(buffer,"[");
      if (array != NULL)
	{
	  GUI_printForeignArrayValues(array->e.foreign_v, buffer);
	}
      strcat(buffer,"]");
      break;
    }
  
  return 1;
}


void  GUI_printForeignSetValues(void **, char *);

int
GUI_printSet(shift_api_set_value * set, char * buffer)
{
  shift_api_builtin_value** values;
  
  switch (set->kind)
    {
    case BUILTIN_K:
      strcat(buffer,"{");      
      GUI_printSetValues(set->e.values_e, buffer);
      strcat(buffer,"}");      
      break;

    case USER_K:
      strcat(buffer,"{");
      GUI_printSetInstances(set->e.instances_e, buffer);
      strcat(buffer,"}");
      break;

      /* Adding case for foreign types.
       *
       * Tunc Simsek 19980605
       */
    case FOREIGN_K:
      strcat(buffer, "{");
      GUI_printForeignSetValues(set->e.foreign_v, buffer);
      strcat(buffer, "}");
      break;
    }
  
  return 1;
}

int 
GUI_printArrayValues(shift_api_builtin_value ** values, char * buffer)
{
  int loop = 0;

  while (values[loop] != NULL)
    {
      if (loop > 0)
	strcat(buffer,",");
      GUI_printBuiltin(values[loop], buffer);
      loop++;
    }
}

int
GUI_printArrayInstances(shift_api_instance ** instances, char * buffer)
{
  int loop = 0;
  
  while (instances[loop] != NULL)
    {
      if (loop > 0)
	strcat(buffer,",");
      GUI_printUser(instances[loop], buffer);
      loop++;
    }
}

int
GUI_printSetValues(shift_api_builtin_value ** values, char * buffer)
{
  int loop = 0;

  while (values[loop] != NULL)
    {
      if (loop > 0)
	strcat(buffer,",");
      GUI_printBuiltin(values[loop], buffer);
      loop++;
    }

}

/* Adding function for printing sets
 * of foregin types.
 *
 * Tunc Simsek 19980605
 */
void
GUI_printForeignSetValues(void **v, char *b)
{
  int loop = 0;

  while (v[loop] != NULL)
    {
      if (loop > 0)
	strcat(b,",");
      GUI_printForeign(v[loop], b);
      loop++;
    }
}

/* Adding function for printing arrays
 * of foreign types.
 *
 * Tunc Simsek 19980605
 */
void
GUI_printForeignArrayValues(void **v, char *b)
{
  int loop = 0;

  while (v[loop] != NULL)
    {
      if (loop > 0)
	strcat(b,",");
      GUI_printForeign(v[loop], b);
      loop++;
    }
}


int
GUI_printSetInstances(shift_api_instance ** instances, char * buffer)
{
  int loop = 0;
  
  while (instances[loop] != NULL)
    {
      if (loop > 0)
	strcat(buffer,",");
      GUI_printUser(instances[loop], buffer);
      loop++;
    }
}


int 
GUI_get_variable_value(char *typeName,
		       int id,
		       char *variableName,
		       char *buffer)
{
  shift_api_user_type * user_type;
  shift_api_instance * instance;
  shift_api_variable ** variables;
  shift_api_value * value;
  int loop;

  user_type = shift_api_find_user_type(typeName);
  instance = shift_api_find_instance(user_type, id);

  if (instance != NULL)
    {
      value = shift_api_find_local_value(variableName, instance);
      strcat(buffer, " {");
      strcat(buffer, variableName);
      strcat(buffer, " ");
      GUI_print_variable_value(value, buffer);
      strcat(buffer, "}");
    } 
  else 
    {
      strcat(buffer, " {}");
    }
}


int 
GUI_get_global_variable_value(char *typeName,
			      int id, 
			      char *variableName,
			      char *buffer)
{
  shift_api_user_type * user_type;
  shift_api_instance * instance;
  shift_api_variable ** variables;
  shift_api_value * value;
  int loop;

  value = shift_api_find_global_value(variableName);
  strcat(buffer, " {");
  strcat(buffer, variableName);
  strcat(buffer, " ");
  GUI_print_variable_value(value, buffer);
  strcat(buffer, "}");
}


char*
GUI_printGlobal(char * buffer)
{
  int loop, var_loop;
  shift_api_variable ** variables;
  shift_api_value * value;

  variables = shift_api_find_global_variables();
  var_loop = 0;
  strcat(buffer, " {");
  while (variables[var_loop] != NULL) {
    strcat(buffer, " {");
    strcat(buffer, variables[var_loop]->name);
    strcat(buffer, " ");
    value = shift_api_find_global_value(variables[var_loop]->name);
    GUI_print_variable_value(value, buffer);
    strcat(buffer, "} ");
    var_loop++;
  }

  strcat(buffer, " }");
}

 
int 
getNumberOfEvents(char * str)
{

  int i, number_of_events;

  i = 0;
  number_of_events = 0;
  while (str[i]!= '\0')
    {
      if (str[i] == ' ')
	{
	  number_of_events++;
	}
      i++;
    }
  /* printf("Number of events for string %s is %d.\n", str,
     number_of_events); */
  return number_of_events;
}   

int
getEventName(char * eventName, char * str, int pos) {
  
  int i,j;
  
  i = pos;
  while (str[i] == ' ')
    i++;
	
  j = 0;
  while ((str[i]!= ' ') && (str[i] != '\0'))
    {
      eventName[j] = str[i];
      i++;
      j++;
    }
  eventName[j] = '\0';
  return i;
}

/* The #ifdef is needed because there's another function with the same
 * name being used when the GUI is switched off. I guess the print
 * functions would need some rework.
 * 19980310 Marco Zandonadi
 */
#ifdef HAVE_SHIFT_GUI_P

/* Prints a list of breakpoints */
void
GUI_print_breakpoints(shift_debug_breakpoint* bp_list,
		      int tnb,
		      int tclick)
{
  int collective_broken = 0;
  int collective_traced = 0;
  char temp_buffer[1000];
  shift_debug_breakpoint* bp_curr = bp_list;
  shift_debug_breakpoint* this_bp_curr;
  int break_printed;

  break_printed = 0;
  /* 1. Check if the collective transition must be traced and/or printed */
  while (bp_curr)
    {
      int collective_broken = 0;
      int collective_traced = 0;

      if (bp_curr->break_flag == TYPE_P
	  || bp_curr->break_flag == COMP_P
	  || bp_curr->break_flag == BOTH_P)
	collective_broken = 1;

      if (bp_curr->trace_flag == TYPE_P
	  || bp_curr->trace_flag == COMP_P
	  || bp_curr->trace_flag == BOTH_P)
	collective_traced = 1;

      if (collective_traced || 
	  (collective_broken && (break_printed == 0)))
	{
	  /* 2. Print and trace the collective transition */
	  if (collective_broken)
	    {
	      sprintf(temp_buffer,
		      " { {Time is %d} {collective transition %d {",
		      tclick,
		      tnb);
	      break_printed = 1;
	      strcat(Transition_Data, temp_buffer);
	    }
	  if (collective_traced)
	    {
	      sprintf(temp_buffer,
		      " { {Time is %d} {collective transition %d {",
		      tclick,
		      tnb);
	      strcat(Trace_Data, temp_buffer);
	      /*fprintf(Tracer->outfile, "{Time is %d} {collective transition %d",
		tnb);*/
	    }
	  this_bp_curr = bp_list;
	  if (collective_traced || collective_broken)
	    while (this_bp_curr)
	      {
		if (collective_broken)
		  {
		    if (!(this_bp_curr->trace_flag == TYPE_P
			  || this_bp_curr->trace_flag == COMP_P
			  || this_bp_curr->trace_flag == BOTH_P)) {
		      sprintf(temp_buffer," {%s %d}",
			      this_bp_curr->instance->type->name,
			      this_bp_curr->instance->id);
		      strcat(Transition_Data,temp_buffer);
		    }
		  }
		
		if (collective_traced)
		  {
		    if (!(this_bp_curr->break_flag == TYPE_P
			  || this_bp_curr->break_flag == COMP_P
			  || this_bp_curr->break_flag == BOTH_P)) 
		      {
			sprintf(temp_buffer," {%s %d}",
				this_bp_curr->instance->type->name,
				this_bp_curr->instance->id);
			strcat(Trace_Data, temp_buffer);
			/*fprintf(Tracer->outfile,
			  " {%s %d}",
			  bp_curr->instance->type->name,
			  bp_curr->instance->id);*/
		      }

		  }
		this_bp_curr = this_bp_curr->next;
	      }

	  if (collective_traced)
	    strcat(Trace_Data, " } } { ");
	  /*fprintf(Tracer->outfile, "}\n");*/

	  if (collective_broken) {
	    strcat(Transition_Data," } } {");
	  }

	  /* 2. Print and trace each broken and traced component */
	  this_bp_curr = bp_list;
	  while (this_bp_curr)
	    {
	      if (collective_broken)
		if (this_bp_curr->break_flag == TYPE_P
		    || this_bp_curr->break_flag == COMP_P
		    || this_bp_curr->break_flag == BOTH_P
		    ||  this_bp_curr->break_flag == NOT_P)
		  {
		    GUI_print_breakpoint(Transition_Data, this_bp_curr, tnb);
		  }
	      if (collective_traced)
		if (this_bp_curr->trace_flag == TYPE_P
		    || this_bp_curr->trace_flag == COMP_P
		    || this_bp_curr->trace_flag == BOTH_P
		    ||  this_bp_curr->trace_flag == NOT_P) 
		  GUI_print_breakpoint(Trace_Data, this_bp_curr, tnb); 
	      this_bp_curr = this_bp_curr->next;
	    }
	
	  if (collective_traced)
	    {
	      strcat(Trace_Data, " } }");
	      /*fprintf(Tracer->outfile, "}\n");*/
	    }
	  
	  if (collective_broken)
	    {
	      strcat(Transition_Data," } }");
	    }
	}
      bp_curr = bp_curr->next;
    }
}
#endif

/* Prints one breakpoint to a file */
void
GUI_print_breakpoint(char * buffer,
		     shift_debug_breakpoint* bp,
		     int tnb)
{
  shift_api_value * value;
  shift_api_instance * instance;
  int iter = 0;
  shift_api_mode* mode = shift_api_find_current_mode(bp->instance);
  shift_api_variable** variables =
    shift_api_find_local_variables(bp->instance->type);
  char temp_buffer[5000];
  
  sprintf(temp_buffer,
	  " { {%s %d}",
	  bp->instance->type->name,
	  bp->instance->id);
  strcat(buffer, temp_buffer);
  sprintf(temp_buffer, "    {before collective transition %d}",
	  tnb);
  strcat(buffer, temp_buffer);
  if (bp->trans != 0)
    {
      sprintf(temp_buffer,
	      "    {transition %s -> %s}",
	      bp->trans->source->name,
	      bp->trans->dest->name);
      strcat(buffer, temp_buffer);
      sprintf(temp_buffer, "    {exported");
      strcat(buffer, temp_buffer);
      while (bp->trans->exports[iter])
	{
	  int comma = 0;

	  if (comma)
	    {
	      strcat(buffer, ",");
	    }
	  else
	    comma = 1;
	  sprintf(temp_buffer, " %s", bp->trans->exports[iter]->name);
	  strcat(buffer, temp_buffer);
	  iter++;
	}
      sprintf(temp_buffer, "}");
      strcat(buffer, temp_buffer);
      sprintf(temp_buffer, "    {external");
      strcat(buffer, temp_buffer);
      iter = 0;
      while (bp->trans->externs[iter])
	{
	  int comma = 0;

	  if (comma)
	    {
	      strcat(buffer, ",");
	    }
	  else
	    comma = 1;
	  sprintf(temp_buffer,
		  " {%s %s",
		  bp->trans->externs[iter]->var_name,
		  bp->trans->externs[iter]->name);
	  strcat(buffer, temp_buffer);
	  switch (bp->trans->externs[iter]->kind)
	    {
	    case SINGLE_C:
	      if (bp->trans->externs[iter]->global == 0)
		{
		  value =
		    shift_api_find_local_value(bp->trans->externs[iter]->var_name,bp->instance);
		}
	      else
		{
		  value =
		    shift_api_find_global_value(bp->trans->externs[iter]->var_name);
		}

	      if (value != NULL)
		{
		  sprintf(temp_buffer," %s %d}",
			  value->v.instance_v->type->name, 
			  value->v.instance_v->id);
		  strcat(buffer, temp_buffer);
		}
	      break;

	    case SET_ONE_C:
	      instance =
		shift_api_find_single_item_in_transition(bp->instance,bp->trans->externs[iter]);
	  
	      if (instance != NULL)
		{
		  sprintf(temp_buffer," %s %d (one)}",
			  instance->type->name, 
			  instance->id);
		  strcat(buffer, temp_buffer);
		}
	  
	      /*sprintf(temp_buffer, "(one)");
		strcat(buffer, temp_buffer);*/

	      break;

	    case SET_ALL_C:

	      if (bp->trans->externs[iter]->global == 0)
		{
		  value =
		    shift_api_find_local_value(bp->trans->externs[iter]->var_name,bp->instance);
		}
	      else
		{
		  value =
		    shift_api_find_global_value(bp->trans->externs[iter]->var_name);
		}

	      temp_buffer[0] = '\0';
	      if (value != NULL)
		{
		  strcat(temp_buffer," {");
		  GUI_print_variable_value(value, temp_buffer);

		  strcat(temp_buffer,"} {}");
		  strcat(buffer, temp_buffer);
		  /*printf("Temp buffer is:%s.\n", temp_buffer);*/
		}
	      sprintf(temp_buffer, " (all)}");
	      strcat(buffer, temp_buffer);

	      break;

	    default:
	      shift_api_error("unknown connection kind");
	      break;
	    }
	  iter++;
	}
      sprintf(temp_buffer, "}");
      strcat(buffer, temp_buffer);
    }
  /*
    iter = 0;
    while (variables[iter])
    {
    char d[OUTBUFSIZE];
    char* a = d;
    shift_api_value* value =
    shift_api_find_local_value(variables[iter]->name, bp->instance);

    sprintf(temp_buffer,
    "    {%s %s}",
    variables[iter]->name,
    shift_tools_print_value(value, a));
    strcat(buffer, temp_buffer);
    iter++;
    }
    */
  sprintf(temp_buffer, "    {discrete %s} } ",
	  mode != 0 ? mode->name : "NONE");
  strcat(buffer, temp_buffer);
}


char *
GUI_print_all_variables_of_all_instances(char *typeName, 
					 char *variable_string,
					 char *buffer)
{
  char** variables;
  char temp_buffer[50];		/* Why 50? */
  shift_api_component_value **value_list;
  shift_api_user_type *type;
  int instance_index, variable_index;

  char** shift_api_parse_variables_string(char *);


  type = shift_api_find_user_type(typeName);

  /* Use API calls to create the list of values */
  variables  = shift_api_parse_variables_string(variable_string);
  value_list = shift_api_find_all_variable_values_of_a_type(variables,
							    type);

  /*printf("We have processed the vars and are printing out \n");*/
  /* Print the list of values to the buffer */
  instance_index = 0;
  while (value_list[instance_index] != NULL)
    {
      sprintf(temp_buffer,
	      " {%d { ",
	      value_list[instance_index]->instance->id);
      strcat(buffer, temp_buffer);
      variable_index = 0;
      while (value_list[instance_index]->values[variable_index] != NULL)
	{
	  strcat(buffer, " { " );
	  GUI_print_variable_value(value_list[instance_index]->values[variable_index], buffer);
	  strcat(buffer, " } " );
	  variable_index++;
	}
      strcat(buffer, "} } " );
      instance_index++;
    }
}

#endif /* TKSHIFT_DATA_I */

/* end of file -- Gui_info.c -- */

