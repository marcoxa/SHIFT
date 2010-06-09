/* -*- Mode: C -*- */


/* sim_info.c -- */


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


#include <shift_config.h>

#include <ctype.h>
#include <shift_debug.h>
#include "sim_info.h"
#include "shift_tools.h"


/* This function returns the values of all of a component's variables. If
 * the "print_names" flag is 0, the function will not print the names of
 * the variables, just their values. Likewise, if "print_discrete" is
 * 0, the the discrete state of the component will not be printed.
 */
char* 
get_component_values(char* type_name, int id,
		     int print_names, int print_discrete)
{
  Component* comp;
  char* variable_str;

  if (!shift_tools_find_type(type_name))
    {
      fprintf(stderr, "Type %s does not exist\n", type_name);
      return NULL;
    }
  comp = shift_tools_find_component(type_name, id);
  if (!comp)
    {
      fprintf(stderr, "Component %s %d not found\n", type_name, id);
      return NULL;
    } 

  variable_str = display_all_variables(comp, print_names, print_discrete);
  return variable_str;
}


char*
display_all_variables(Component* c, int print_names, int print_discrete)
{
  ComponentTypeDescriptor* myT = c->desc;
  TypeDescriptor* tD;
  ModeDescriptor* myM = c->M;
  VariableDescriptor* myV = myT->variables;
  static char d[OUTBUFSIZE];
  char* a = d;

  for (; myV->offset != -1; myV++)
    {
      if (myV->kind == INPUT_KIND
	  || myV->kind == OUTPUT_KIND 
	  || myV->kind == STATE_KIND)
	{ 

	  sprintf(a, "{");
	  a = a + strlen(a);
	  if (print_names)
	    {
	      sprintf(a, "%s ", myV->name); 
	      a = a + strlen(a);
	    }
	  a = printVariable(c, myV, a);
	  sprintf(a, "} ");
	  a = a + strlen(a);
	}
    }
  if (print_discrete)
    {
      if (c->M)
	{
	  sprintf(a, "{");
	  a = a + strlen(a);
	  if (print_names)
	    {
	      sprintf(a, "discrete ", c->M->name);	
	      a =  a + strlen(a);
	    }
	  sprintf(a, "%s}", c->M->name);
	  a =  a + strlen(a);
	}
    }    
  return d;
}


/* This function returns the value of a single variable. It prints the
 * result into a string and returns the address of the string. This is
 * the structure of the string: 
 */
char* 
get_variable_values(char* type_name, int id, 
		    char* var_name, int print_names)
{
  VariableDescriptor *vd;
  static char d[OUTBUFSIZE];
  char* a = d;
  Component *comp;
  ComponentTypeDescriptor* ctd = shift_tools_find_type(type_name);
  
  if (!ctd)
    {
      fprintf(stderr, "Type %s does not exist\n", type_name);
      d[0]='0';
      d[1]='\0';
      return d;
    }
  vd = shift_tools_find_variable_descriptor(var_name, ctd);
  if (!vd)
    {
      fprintf(stderr, "Variable %s does not exist in type %s\n",
	      var_name, type_name);
      d[0]='0';
      d[1]='\0';
      return d;
    }
  comp = shift_tools_find_component(type_name, id);
  if (!comp)
    {
      fprintf(stderr, "Component %s %d not found\n", type_name, id);
      d[0]='0';
      d[1]='\0';
      return d;
    } 
  if (print_names)
    {
      sprintf(a, "{");
      a = a + strlen(a);
      sprintf(a, "%s ", vd->name); 
      a = a + strlen(a);
    }
  a = printVariable(comp, vd, a);
  if (print_names)
    {
      sprintf(a, "}");
      a = a + strlen(a);
    }
  return d;
}


/* end of file - sim_info.c */
