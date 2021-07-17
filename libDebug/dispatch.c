/* -*- Mode: C -*- */

/* dispatch.c -- */

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

/* dispatch.c -- */

#ifndef DISPATCH_I
#define DISPATCH_I

#include <limits.h>
#include <stdlib.h>

#include <shift_config.h>

#include "shift_debug.h"
#include "shift_api_types.h"
#include "shift_api_info.h"
#include "shift_tools.h"
#include "shift_db_debug.h"


/* DBL_DGS - Number of precision digits in a "double".
 * This seems to be defined only in Solaris.
 */

#ifndef DBL_DIG
#define DBL_DIG 15
#endif


typedef enum {
  dbg_cmd_ok,
  dbg_cmd_failed
} dbg_command_retcode;


dbg_command_retcode setTypeTrace(char*, traceType);
dbg_command_retcode setCompTrace(char*, traceType);
extern  db_cmd* DB_dbg_List;


void
dispatchRequest(commandName command, char* rS)
/* id is one of the commands #defined above
 * rS is the arguments
 */
{
  void setFile(char*);

  switch (command)
    {
    case empty_e:
      /* Do nothing.  This is a no-op! */
      break;

    case setfile_e:
      setFile(rS); 
      break;

    case setprecision_e:
      setPrecision(rS);
      break;

    case setzeno_e:
      setZeno(rS);
      break;

    case setFS_e:
      shift_DB_setFieldSeperator(rS);
      break;

    case setFS2_e:
      shift_DB_setFieldSeperator2(rS);
      break;

    case setSOR_e:
      shift_DB_setSOR(rS);
      break;

    case setEOR_e:
      shift_DB_setEOR(rS);
      break;

    case stop_e:
      /* This will be exercised only if command is read asynchronously */
      /* Not operational in command-line version */
      Tracer->returnControl = FALSE;
      break;

    case unbreakcomp_e:
      if (setCompTrace(rS, unbreak_e) == dbg_cmd_ok)
	printf("Component %s cleared off breaking flags", rS);
      else
	printf("No component %s with any breaking flags", rS);
      break;

    case untracecomp_e:
      if (setCompTrace(rS, untrace_e) == dbg_cmd_ok)
	printf("Component %s cleared off tracing flags", rS);
      else
	printf("No component %s with any tracing flags", rS);
      break;

    case unbreaktype_e:
      if (setTypeTrace(rS, unbreak_e) == dbg_cmd_ok)
	printf("Type %s cleared off breaking flags", rS);
      else
	printf("No type %s with any breaking flags", rS);
      break;

    case untracetype_e:
      if (setTypeTrace(rS, untrace_e) == dbg_cmd_ok)
	printf("Type %s cleared off tracing flags", rS);
      else
	printf("No type %s with any tracing flags", rS);
      break;

    case cont_e:
      printf("SHIFT debugger: ");
      printf("error: this should be caught by dbgControl().\n");
      break;

    case exit_e:
    case quit_e:
      printf("SHIFT debugger: end of session.\n\n");
      exit(0);

    case help_e:
      printCmdLineHelp();
      break;

    case parent_e:
      printParentType(rS);
      break;

    case children_e:
      printChildrenType(rS);
      break;

    case discrete_e:
      printListDiscrete(rS);
      break;

    case current_e:
      printCurrentMode(rS);
      break;

    case localvar_e:
      printListLocalVars(rS);
      break;

    case globalvar_e:
      printListGlobalVars();
      break;

    case globalval_e:
      printGlobalVal(rS);
      break;

    case list_e:
      printListCompsOfType(rS);
      break;

    case listsub_e:
      printListSubCompsOfType(rS);
      break;

    case listtypes_e:
      printListTypes();
      break;

    case print_e:
      printLocalValsOfComp(rS);
      /* Old code: displayComponent(rS); */
      break;

    case ctracetype_e:
      if (setTypeTrace(rS, diff_e) == dbg_cmd_ok)
	printf("Type marked for tracing continuous variables");
      else
	printf("Type not marked");
      break;

    case db_ctracetype_e:
      /* new db command */
	
      if (setTypeTrace(rS, diff_e) == dbg_cmd_ok)
	{
	  DB_dbg_List = shift_DB_New_cmd (DB_dbg_List,"db_ctt",rS );
	  printf("Type marked for tracing continuous variables");
	}
      else
	printf("Type not marked");
      break;

    case dtracetype_e:
      if (setTypeTrace(rS, desc_e) == dbg_cmd_ok)
	printf("Type marked for tracing transitions");
      else
	printf("Type not marked");
      break;

    case db_dtracetype_e:
      /*new DB command */
      if (setTypeTrace(rS, desc_e) == dbg_cmd_ok)
	{
	  DB_dbg_List = shift_DB_New_cmd (DB_dbg_List,"db_dtt",rS);
	  printf("Type marked for tracing transitions");
	}
      else
	printf("Type not marked");
      break;

    case tracetype_e:
      if (setTypeTrace(rS, both_e) == dbg_cmd_ok)
	printf("Type marked for tracing transitions and continuous variables");
      else
	printf("Type not marked");
      break;

    case ctracecomp_e:
      if (setCompTrace(rS, diff_e) == dbg_cmd_ok)
	printf("Component marked for tracing continuous variables");
      else
	printf("Component not marked");
      break;

    case dtracecomp_e:
      if (setCompTrace(rS, desc_e) == dbg_cmd_ok)
	printf("Component marked for tracing transitions");
      else
	printf("Component not marked");
      break;

    case db_ctracecomp_e:
    /* new db command */
    if (setCompTrace(rS, diff_e) == dbg_cmd_ok)
      {
	/* need to split rs */
	DB_dbg_List = shift_DB_New_cmd (DB_dbg_List,"db_ctc",rS);
	printf("Component marked for tracing continuous variables");
      }
    else
      printf("Component not marked");
      break;

    case db_dtracecomp_e:
      /* new db command */
      if (setCompTrace(rS, desc_e) == dbg_cmd_ok)
	{
	  DB_dbg_List = shift_DB_New_cmd (DB_dbg_List,"db_dtc",rS);
	  printf("Component marked for tracing transitions");
	}
      else
	printf("Component not marked");
      break;

    case tracecomp_e:
      if (setCompTrace(rS, both_e) == dbg_cmd_ok)
	printf("Component marked for tracing transitions and continuous variables");
      else
	printf("Component not marked");
      break;

    case desct_e:
      printf("Not Supported");
      break;

    case descd_e:
      printf("Not Supported");
      break;

    case breaktype_e:
      if (setTypeTrace(rS, break_e) == dbg_cmd_ok)
	printf("Type marked for breaking at transitions");
      else
	printf("Type not marked");
      break;

    case breakcomp_e:
      if (setCompTrace(rS, break_e) == dbg_cmd_ok)
	printf("Component marked for breaking at transitions");
      else
	printf("Component not marked");
      break;

    case status_e:
      printf("Not Supported");
      break;

    case delete_e:
      printf("Not Supported");
      break;

    case printglobal_e:
      printf("%s", printGlobal());
      break;

    case set_docfile_e:
      shift_DB_Set_description_file(rS);
      break;
    
    case set_logdir_e:
      shift_DB_Set_LOGDIR(rS);
      break;

    case invalid_command_e:
    default:
      fprintf(stderr, "SHIFT debugger: invalid command");
      break;
    }
}


/* Make sure that file_name is a meaningful file name, and set the
 * output file name in the structure Tracer. 
 */
void
setFile(char* file_name)
{
  if (Tracer->outfile != NULL)
    fclose(Tracer->outfile);
  
  Tracer->outfile = fopen(file_name, "a");

  if (!Tracer->outfile)
    {
      fprintf(stderr,
	      "SHIFT: dispatch.c: %d: could not open file %s.\n",
	      __LINE__,
	      file_name);
      fprintf(stderr,
	      "       Make sure you have write permission.\n");
      exit(1);
    }
  else
    printf("Output file for tracing is now %s", file_name);
}


/* Set the global variable shift_debug_precision for printing double
 * values
 */
setPrecision(char* rS)
{
  if (!rS)
    printf("NULL string");
  else
    {
      switch (strlen(rS))
	{
	case 1:
	  if (isdigit(rS[0]))
	    {
	      int new_precision = atoi(rS);
	      if (new_precision <= DBL_DIG)
		{
		  shift_debug_precision = new_precision;
		  printf("Printing precision set to %d",
			 shift_debug_precision);
		}
	      else
		printf("The precision must be less than %d", DBL_DIG);
	    }
	  else
	    printf("Bad string: \"%s\"", rS);
	  break;

	case 2:
	  if (isdigit(rS[0]) && isdigit(rS[1]))
	    {
	      int new_precision = atoi(rS);
	      if (new_precision <= DBL_DIG)
		{
		  shift_debug_precision = new_precision;
		  printf("Printing precision set to %d",
			 shift_debug_precision);
		}
	      else
		printf("The precision must be less than %d", DBL_DIG);
	    }
	  else
	    printf("Bad string: \"%s\"", rS);
	  break;

	default:
	  printf("Bad string: \"%s\"", rS);
	  break;
	}
    }
}


/* Set the global variable shift_zeno_threshold for detecting zeno behavior
 */
setZeno(char* rS)
{
  if (!rS)
    printf("NULL string");
  else if (strlen(rS) <= 0)
    printf("NULL string");
  else
    {
      int iter = 0;
      long new_zeno = 0;

      for (iter - 0 ; iter < strlen(rS) ; iter++)
	if (!isdigit(rS[iter]))
	  {
	    printf("Bad string: \"%s\"", rS);
	    return;
	  }
      
      new_zeno = atol(rS);
      if (new_zeno < 0)
	printf("Bad string: \"%s\"", rS);
      else
	{
	  shift_zeno_threshold = new_zeno;
	  printf("Zeno detection set to %ld", shift_zeno_threshold);
	}
    }
}


/* Check that type_name exists and print the parent type (if any).
 */
printParentType(char* type_name)
{
  shift_api_user_type* user_type = shift_api_find_user_type(type_name);

  if (user_type)
    {
      if (user_type->parent)
	printf("Parent type of %s is: %s", 
	       type_name, 
	       user_type->parent->name);
      else
	printf("Type %s has no parent type", type_name);
    }
  else
    printf("There is no type \"%s\"", type_name);
}


/* Check that type_name exists and print the list of children types (if any).
 */
printChildrenType(char* type_name)
{
  shift_api_user_type* user_type = shift_api_find_user_type(type_name);

  if (user_type)
    {
      shift_api_find_children(user_type);
      if (user_type->children[0])
	{
	  int iter = 0;

	  printf("Children types of %s are:", type_name);
	  while (user_type->children[iter])
	    {
	      printf(" %s", user_type->children[iter]->name);
	      iter++;
	    }
	}
      else
	printf("Type %s has no children types", type_name);
    }
  else
    printf("There is no type \"%s\"", type_name);
}


/* Check that type_name exists and print the list of its discrete modes.
 */
printListDiscrete(char* type_name)
{
  shift_api_user_type* user_type = shift_api_find_user_type(type_name);

  if (user_type)
    {
      if (user_type->modes[0])
	{
	  int iter = 0;

	  printf("Discrete modes of %s are:", type_name);
	  while (user_type->modes[iter])
	    {
	      printf(" %s", user_type->modes[iter]->name);
	      iter++;
	    }
	}
      else
	printf("Type %s has no discrete modes", type_name);
    }
  else
    printf("There is no type \"%s\"", type_name);
}


/* Check that the component exists and print its current mode.
 * If it is the implicit mode "loop~X", then it is not printed.
 */
printCurrentMode(char* rS)
{
  int i;
  char* name = getCompId(rS, &i);

  if (name)
    {
      shift_api_user_type* user_type = shift_api_find_user_type(name);

      if (user_type)
	{
	  shift_api_instance* comp = shift_api_find_instance(user_type, i);

	  if (comp)
	    {
	      shift_api_mode* mode = shift_api_find_current_mode(comp);

	      if (mode)
		{
		  if (!strncmp(mode->name, "loop~", 5))
		    printf("Component %s has no mode", rS);
		  else
		    printf("The current mode of component %s is %s",
			   rS, mode->name);
		}
	      else
		printf("Component %s has no mode", rS);
	    }
	  else
	    printf("There is no component \"%s\"", rS);
	}
      else
	printf("There is no type \"%s\"", name);
    }
  else
    printf("Bad string: \"%s\"", rS);
}


/* Prints the list of local variables of a type
 */
printListLocalVars(char* type_name)
{
  shift_api_user_type* user_type = shift_api_find_user_type(type_name);

  if (user_type)
    {
      int iter = 0;
      shift_api_variable** vars = shift_api_find_local_variables(user_type);

      printf("List of local variables of %s:", type_name);
      while (vars[iter])
	{
	  printf("\n    %s", shift_tools_print_variable(vars[iter]));
	  iter++;
	}
    }
  else
    printf("There is no type \"%s\"", type_name);
}


/* Prints the list of global variables
 */
printListGlobalVars()
{
  int iter = 0;

  printf("List of global variables:");
  while (shift_api_global_variables[iter])
    {
      printf("\n    %s",
	     shift_tools_print_variable(shift_api_global_variables[iter]));
      iter++;
    }
}

/* Prints the value of a global variable
 */
printGlobalVal(char* name)
{
  static char d[OUTBUFSIZE];
  char* a = d;
  shift_api_value* value = shift_api_find_global_value(name);

  if (value)
    {
      shift_tools_print_value(value, a);
      printf("    %s = %s", name, a);
    }
  else
    printf("There is no global variable \"%s\"", name);
}


/* Prints the values of all the local variables of a component
 */
printLocalValsOfComp(char* rS)
{
  int i;
  char* name = getCompId(rS, &i);

  if (name)
    {
      shift_api_user_type* user_type = shift_api_find_user_type(name);

      if (user_type)
	{
	  shift_api_instance* inst = shift_api_find_instance(user_type, i);

	  if (inst)
	    {
	      printf("{%s\n", rS);
	      shift_tools_print_all_values(stdout, inst);
	      printf("}");
	    }
	  else
	    printf("There is no component \"%s\"", rS);
	}
      else
	printf("There is no type \"%s\"", name);
    }
  else
    printf("Bad string: \"%s\"", rS);
}


/* Prints the list of all types of the program.
 */
printListTypes()
{
  if (shift_api_user_types[0])
    {
      int iter = 0;

      printf("List of all the types:");
      while (shift_api_user_types[iter])
	{
	  printf(" %s", shift_api_user_types[iter]->name);
	  iter++;
	}
    }
  else
    printf("There are no types in the program");
}


/* Prints all the information about one component
 */
displayComponent(char* rS)
{
  int i;
  char* name = getCompId(rS, &i);

  if (name)
    {
      Component* c = shift_tools_find_component(name, i);

      if (c)
	{
	  printf("{%s %d \n", c->desc->name, c->name);
	  printf("%s" , displayAll(c));
	  printf("}");
	}
      else
	printf("There is no component \"%s\"", rS);
    }
  else
    printf("Bad string: \"%s\"", rS);
}


/* Prints the list of components in the program along with their type.
 */
printListComps()
{
  if (continuous_components)
    {
      Component *comp;

      printf("List of all the components:");
      for (comp = continuous_components ; comp ; comp = comp->continuous_next)
	{
	  printf("\n    %s %d", comp->desc->name, comp->name);
	}
    }
  else
    printf("There are no components in the program");
}


/* Check that type_name exists and print its list of components.
 */
printListCompsOfType(char* type_name)
{
  shift_api_user_type* user_type = shift_api_find_user_type(type_name);

  if (user_type)
    {
      shift_api_instance** comps = shift_api_find_instances(user_type);

      if (comps[0])
	{
	  int iter = 0;

	  printf("List of components of type %s:", type_name);
	  while (comps[iter])
	    {
	      printf("\n    %s %d", 
		     comps[iter]->type->name, 
		     comps[iter]->id);
	      iter++;
	    }
	}
      else
    	printf("Type %s has no active instances", type_name);
    }
  else
    printf("There is no type \"%s\"", type_name);
}


/* Prints the list of components of type parentType and recurse on all
 * its children.
 */
recurseOnSubComps(ComponentTypeDescriptor *parent_type)
{
  Component *comp;

  for (comp = parent_type->component_first ; 
       comp ; 
       comp = comp->component_next)
    printf("\n    %s %d", parent_type->name, comp->name);
  if (parent_type->children[0])
    {
      int iter = 0;

      while (parent_type->children[iter])
	{
	  recurseOnSubComps(parent_type->children[iter]);
	  iter++;
	}
    }
}


/* Check that type_name exists and print the list of components and
 * components of all children types.
 */
printListSubCompsOfType(char* type_name)
{
  ComponentTypeDescriptor *compType = shift_tools_find_type(type_name);

  if (compType)
    {
      printf("List of components and sub-components of type %s:", type_name);
      recurseOnSubComps(compType);
    }
  else
    printf("There is no type \"%s\"", type_name);
}


dbg_command_retcode
setTypeTrace(char* rS, traceType level)
{
  char* name = (char*)malloc(NAMESIZE*sizeof(char));
  char* tail = (char*)malloc(CMDBUFSIZE*sizeof(char));
  shift_api_user_type* type;
  int ack = -1;

  getToken(rS, name, tail, " ");
  type = shift_api_find_user_type(name);
  if (type)
    {
      shift_api_variable** variables =
	shift_tools_extract_variables(type, rS);
      shift_api_exported_event** events =
	shift_tools_extract_events(type, rS);

      if (events && variables)
	switch (level)
	  {
	  case unbreak_e:
	    ack = shift_api_unbreak_type(type);
	    break;

	  case untrace_e:
	    ack = shift_api_untrace_type(type);
	    break;

	  case break_e:
	    ack = shift_api_break_type(type, events);
	    break;

	  case diff_e:
	  case desc_e:
	  case both_e:
	    ack = shift_api_trace_type(type, events, variables, level);
	    break;
	  }
    }
  free(name);
  free(tail);
  if (ack == 0)
    return dbg_cmd_ok;
  else
    return dbg_cmd_failed;
}


dbg_command_retcode
setCompTrace(char* rS, traceType level)
{
  int i;
  char* name = getCompId(rS, &i);
  int ack = -1;

  if (name)
    {
      shift_api_user_type* type = shift_api_find_user_type(name);

      if (type)
	{
	  shift_api_instance* instance = shift_api_find_instance(type, i);

	  if (instance)
	    {
	      shift_api_variable** variables =
		shift_tools_extract_variables(type, rS);
	      shift_api_exported_event** events =
		shift_tools_extract_events(type, rS);

	      if (events && variables)
		switch (level)
		  {
		  case unbreak_e:
		    ack = shift_api_unbreak_instance(instance);
		    break;

		  case untrace_e:
		    ack = shift_api_untrace_instance(instance);
		    break;

		  case break_e:
		    ack = shift_api_break_instance(instance, events);
		    break;

		  case diff_e:
		  case desc_e:
		  case both_e:
		    ack = shift_api_trace_instance(instance,
						   events,
						   variables,
						   level);
		    break;
		  }
	    }
	}
    }
  if (ack == 0)
    return dbg_cmd_ok;
  else
    return dbg_cmd_failed;
}


void
printCmdLineHelp()
{
  printf("\nSHIFT Command Line Debugger Help\n");
  printf("================================\n\n");
  printf("Currently supported commands:\n");
  
  printf("\n1. General commands: \n");
  printf("\n    setFS STRING (sfs)\n\t set field seperator to STRING\n");
  printf("\n    setEOR STRING (seor)\n\t set EOR to STRING\n");
  printf("\n    setSOR STRING (ssor)\n\t set SOR to STRING\n");
  printf("\n    setfile <file> (sf) \n\tDump trace output into field \
<file>; the default is shift-trace.hst\n");
  printf("\n    setprecision <n> (sp) \n\tSet the precision to <n> \
digits\n");
  printf("\n    setzeno <n> (sz) \n\tSet the zeno detection to <n> \
consecutive transitions\n");
  printf("\n    setDocFile (sdf)\n\t Set Document file comment to \
<n>\n");
  printf("\n    setLogDir  (sld)\n\tSet Log Directory.  only before \
simulation starts\n");
  printf("\n    exit (e) \n\tExit the simulation\n");
  printf("\n    help (h) \n\tPrint this help message\n");

  printf("\n2. Breaking commands: \n");
  printf("\n    cont <n> (c) \n\tContinue the simulation for <n> \
continuous time clicks\n");
  printf("\n    breaktype <type> (bt) \n\tStop before any discrete \
transition in all components of type <type>\n");
  printf("\n    breakcomp <type id> (bc) \n\tStop before any discrete \
transition in component <type id>\n");
  printf("\n    unbreaktype <type> (ubt) \n\tClear all the break requests \
for type <type>\n");
  printf("\n    unbreakcomp <type id> (ubc) \n\tClear all the break \
requests for component <type id>\n");

  printf("\n3. Printing commands: \n");
  printf("\n    printglobal (pg) \n\tList all the global components\n");
  printf("\n    listtypes (lt) \n\tList all the types in the program\n");
  printf("\n    list <type> (l) \n\tList all the components of type \
<type>\n");
  printf("\n    print <type id> (p) \n\tPrint the internal visible state \
of component <type id>\n");
  printf("\n    typeparent <type> (tp) \n\tPrint the parent type of type \
<type>\n");
  printf("\n    listtypechildren <type> (ltc) \n\tList the children types \
of type <type>\n"); 
  printf("\n    listsub <type> (ls) \n\tList all components of type \
<type> as well as of the children \n\ttypes of type <type>\n");
  printf("\n    listdiscrete <type> (ld) \n\tList all the discrete modes \
of type <type>\n");
  printf("\n    currentmode <type id> (cm) \n\tPrint the current mode \
of component <type id>\n");
  printf("\n    listlocalvars <type> (llv) \n\tList all the local \
variables of type <type>\n");
  printf("\n    listglobalvars (lgv) \n\tList all the global variables\n");
  printf("\n    printglobalval <name> (pgv) \n\tPrint the value of the \
global variable <name>\n");

  printf("\n4. Tracing commands: \n");
  printf("\n    tracetype <type> (tt) \n\tSame as ctracetype plus \
dtracetype\n");
  printf("\n    ctracetype <type> (ctt) \n\tFor all components of type \
<type>, dump all continuousvariables \n\tevery time click; variables can \
be specified with option -v\n");
 printf("\n    db_ctracetype <type> (db_ctt) \n\tFor all components of type \
<type>,DB dump all continuousvariables \n\tevery time click; variables can \
be specified with option -v\n");
  printf("\n    dtracetype <type> (dtt) \n\tFor all components of type \
<type>, dump all transition information \n\tbefore any transition; \
synchronization events can be specified with \n\toption -e\n");
  printf("\n    db_dtracetype <type> (db_dtt) \n\tFor all components of type \
<type>, dump all transition information \n\tbefore any transition; \
synchronization events can be specified with \n\toption -e\n");
  printf("\n    tracecomp <type id> (tc) \n\tSame as ctracecomp plus \
dtracecomp\n");
  printf("\n    ctracecomp <type id> (ctc) \n\tFor component <type id>, \
dump all continuous variables every \n\ttime click; variables can be \
specified with option -v\n");
  printf("\n    db_ctracecomp <type id> (db_ctc) \n\tFor component <type id>, \
dump all continuous variables every \n\ttime click; variables can be \
specified with option -v\n");
  printf("\n    dtracecomp <type id> (dtc) \n\tFor component <type id>, \
dump all transition information before \n\tany transition; synchronization \
events can be specified \n\twith option -e\n");
  printf("\n    db_dtracecomp <type id> (db_dtc) \n\tFor component <type id>, \
dump all transition information before \n\tany transition; synchronization \
events can be specified \n\twith option -e\n");
  printf("\n    untracetype <type> (utt) \n\tClear all the trace requests \
for type <type>\n");
  printf("\n    untracecomp <type id> (utc) \n\tClear all the trace \
requests for component <type id>\n");
}


#endif /* DISPATCH_I */

/* end of file -- dispatch.c -- */
