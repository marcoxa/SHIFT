/* -*- Mode: C -*- */

/* control.c -- */

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


#ifndef CONTROL_I
#define CONTROL_I

#include <shift_config.h>

#include <stdlib.h>
#include "shift_debug.h"
#include "shift_db_debug.h"


char* shift_debug_prompt_string = "SHIFT debug > ";
char* trace_file_name = "shift-trace.hst";
TraceStatus* Tracer = (TraceStatus*) NULL;
commandName getCommand(char*);
shift_debug_function* shift_api_callback = (shift_debug_function*)
     NULL;
 db_cmd* DB_dbg_List;


void
shift_debug_prompt()
{
  fprintf(stdout, "%s", shift_debug_prompt_string);
}


void
initDebug()
{
  char* name;
  FILE* commands;
  char* cmdLine = (char*) malloc(80 * sizeof(char));
  char* cmd = (char*) malloc(80 * sizeof(char));
  char* tail = (char*) malloc(80 * sizeof(char));

  /* Initialize the types and global variables for the api */
  shift_api_initialize_user_types();
  shift_api_initialize_global_variables();
 
  /* Initialize the Tracer */
  Tracer = (TraceStatus*) malloc(sizeof(TraceStatus));
  if (Tracer == 0)
    runtime_error("out of heap memory");

  Tracer->returnControl = TRUE;
  Tracer->lastLength = 0;
  Tracer->stopInN = 0;
  Tracer->timeTraced = (ComponentList*) NULL;

 

  /* Initialize the callback function structure */
  shift_api_callback = (shift_debug_function*)
    malloc(sizeof(shift_debug_function));
  if (Tracer == 0)
    runtime_error("out of heap memory");

  name = getenv("SHIFT_OUTPUT_FILE");
  if (name != 0)
    {
      Tracer->outfile = fopen(name, "a");
    }
  else
    {
      Tracer->outfile = fopen(trace_file_name, "a");
      name = trace_file_name;
    }

  if (Tracer->outfile == NULL)
    {
      fprintf(stderr,
	      "SHIFT debugger: unable to open ouput file %s.\n", name);
      fprintf(stderr,
	      "                Make sure you have write access\n");
      exit(1);
    }

  fprintf(Tracer->outfile, " Tracer outfile initialized\n");
  /* Are we running in batch mode? */
  name = getenv("SHIFT_DEBUG_SCRIPT");
  if (name)
    {
      commandName z;
      commands = fopen(name, "r");
      cmdLine = fgets(cmdLine, 80, commands);
      while (cmdLine)
	{
	  getToken(cmdLine, cmd, tail, " "); 
	  z = getCommand(cmd);
	  /* printf("Head %s, Tail %s, Id %d \n", cmd, tail, z ); */
	  if (z == cont_e)
	    break;
	  dispatchRequest( z, tail ); 
	  printf("\n");
	  cmdLine = fgets(cmdLine, 80, commands);
	}  
      Tracer->stopInN = atoi(tail);
      Tracer->lastLength = Tracer->stopInN;
      Tracer->returnControl = TRUE;
    }
}


int
getCommandLine(char* cmdLine)
{
  char* s;

  s =  fgets(cmdLine, CMDBUFSIZE, stdin);
  return 0;
}


commandName
getCommand(char* cmd)
{
  int z = strlen(cmd);
  
  if (z == 0)
    return empty_e;
  if ( !strcmp(cmd, "breakcomp")          || !strcmp(cmd, "bc") )
    return breakcomp_e;
  else if ( !strcmp(cmd, "breaktype")     || !strcmp(cmd, "bt") )
    return breaktype_e;
  else if ( !strcmp(cmd, "unbreakcomp")   || !strcmp(cmd, "ubc") )
    return unbreakcomp_e;
  else if ( !strcmp(cmd, "untracecomp")   || !strcmp(cmd, "utc") )
    return untracecomp_e;
  else if ( !strcmp(cmd, "unbreaktype")   || !strcmp(cmd, "ubt") )
    return unbreaktype_e;
  else if ( !strcmp(cmd, "untracetype")   || !strcmp(cmd, "utt") )
    return untracetype_e;
  else if ( !strcmp(cmd, "cont")          || !strcmp(cmd, "c") )
    return cont_e;
  else if ( !strcmp(cmd, "ctracecomp")    || !strcmp(cmd, "ctc") )
    return ctracecomp_e;
  else if ( !strcmp(cmd, "db_ctracecomp") || !strcmp(cmd, "db_ctc") )
    return db_ctracecomp_e;
  else if ( !strcmp(cmd, "ctracetype")    || !strcmp(cmd, "ctt") )
    return ctracetype_e;
  else if ( !strcmp(cmd, "db_ctracetype") || !strcmp(cmd, "db_ctt") )
    return db_ctracetype_e;
  else if ( !strcmp(cmd, "delete")        || !strcmp(cmd, "d") )
    return delete_e;
  else if ( !strcmp(cmd, "descd")         || !strcmp(cmd, "dd") )
    return descd_e;
  else if ( !strcmp(cmd, "desct")         || !strcmp(cmd, "dt") )
    return desct_e;
  else if ( !strcmp(cmd, "dtracecomp")    || !strcmp(cmd, "dtc") )
    return dtracecomp_e;
  else if ( !strcmp(cmd, "db_dtracecomp") || !strcmp(cmd, "db_dtc") )
    return db_dtracecomp_e;
  else if ( !strcmp(cmd, "dtracetype")    || !strcmp(cmd, "dtt") )
    return dtracetype_e;
  else if ( !strcmp(cmd, "db_dtracetype") || !strcmp(cmd, "db_dtt") )
    return db_dtracetype_e;
  else if ( !strcmp(cmd, "exit")          || !strcmp(cmd, "e") )
    return exit_e;
  else if ( !strcmp(cmd, "quit")          || !strcmp(cmd, "q") )
    return exit_e;
  else if ( !strcmp(cmd, "help")          || !strcmp(cmd, "h") )
    return help_e;
  else if ( !strcmp(cmd, "typeparent")    || !strcmp(cmd, "tp") )
    return parent_e;
  else if ( !strcmp(cmd, "listtypechildren") || !strcmp(cmd, "ltc") )
    return children_e;
  else if ( !strcmp(cmd, "listdiscrete")  || !strcmp(cmd, "ld") )
    return discrete_e;
  else if ( !strcmp(cmd, "currentmode")   || !strcmp(cmd, "cm") )
    return current_e;
  else if ( !strcmp(cmd, "listlocalvars") || !strcmp(cmd, "llv") )
    return localvar_e;
  else if ( !strcmp(cmd, "listglobalvars") || !strcmp(cmd, "lgv") )
    return globalvar_e;
  else if ( !strcmp(cmd, "printglobalval") || !strcmp(cmd, "pgv") )
    return globalval_e;
  else if ( !strcmp(cmd, "list")         || !strcmp(cmd, "l") )
    return list_e;
  else if ( !strcmp(cmd, "listsub")      || !strcmp(cmd, "ls") )
    return listsub_e;
  else if ( !strcmp(cmd, "listtypes")    || !strcmp(cmd, "lt") )
    return listtypes_e;
  else if ( !strcmp(cmd, "print")        || !strcmp(cmd, "p") )
    return print_e;
  else if ( !strcmp(cmd, "printglobal")  || !strcmp(cmd, "pg") )
    return printglobal_e;
  else if ( !strcmp(cmd, "setFS")        || !strcmp(cmd, "sfs") )
    return setFS_e;
  else if ( !strcmp(cmd, "setEOR")       || !strcmp(cmd, "seor") )
    return setEOR_e;
  else if ( !strcmp(cmd, "setFS2")       || !strcmp(cmd, "sfs2") )
    return setFS2_e;
   else if ( !strcmp(cmd, "setSOR")      || !strcmp(cmd, "ssor") )
    return setSOR_e;
  else if ( !strcmp(cmd, "setfile")      || !strcmp(cmd, "sf") )
    return setfile_e;
  else if ( !strcmp(cmd, "setprecision") || !strcmp(cmd, "sp") )
    return setprecision_e;
  else if ( !strcmp(cmd, "setzeno")      || !strcmp(cmd, "sz") )
    return setzeno_e;
  else if ( !strcmp(cmd, "status")       || !strcmp(cmd, "st") )
    return status_e;
  else if ( !strcmp(cmd, "stop")         || !strcmp(cmd, "s") )
    return stop_e;
  else if ( !strcmp(cmd, "tracetype")    || !strcmp(cmd, "tt") )
    return tracetype_e;
  else if ( !strcmp(cmd, "tracecomp")    || !strcmp(cmd, "tc") )
    return tracecomp_e;
  else if ( !strcmp(cmd, "setDocFile")   || !strcmp(cmd,"sdf") )
    return set_docfile_e;
  else if ( !strcmp(cmd, "setLogDir")    || !strcmp(cmd,"sld") )
    return set_logdir_e;
  else
    return invalid_command_e;
}


void
dbgControl()
{
  ComponentList* CompEl = Tracer->timeTraced;
  Component* Comp;
  int iter = 0;
  int dots_per_line = 50;
  static int dots_output = 0;

  fprintf(Tracer->outfile, "{TimeStamp %ld}\n", tclick);

  /* Dump state of all components in time Traced */
  while (CompEl != NULL)
    {
      Comp = CompEl->thisComp;
      fprintf(Tracer->outfile, "%s\n", display(Comp));
      CompEl = CompEl->nextComp;
    }

  /* Iterate through the type list and dump state of all components of
   * any type that is traced */
  while (component_type_list[iter])
    {
      ComponentTypeDescriptor* ctd = component_type_list[iter];
      if (ctd->trace_flag == diff_e
	  || ctd->trace_flag == both_e)
	{
	  Comp = ctd->component_first;
	  while (Comp)
	    {
	      fprintf(Tracer->outfile, "%s\n", display(Comp));
	      Comp = Comp->component_next;
	    }
	}
      iter++;
    }
  if ( DB_dbg_List != NULL )
    {
      /*   shift_DB_Traverse_CmdList (DB_dbg_List); */
    }
  if (tclick > 0)
    {
      if (dots_output % dots_per_line == 0)
	{
	  printf("\n[%ld]", tclick);
	  dots_output = 0;
	}
      dots_output++;
      putchar('.');
      fflush(stdout);
    }
   
  if (Tracer->stopInN > 1 && Tracer->returnControl == TRUE)
    {
      (Tracer->stopInN)--;
      return;
    }
  else
    {
      /* Time to interact with the user */
      printf("\nTime is %ld.\n\n", tclick);
      userInteract();
    }
  return;
}


void
userInteract()
{
  char* cmdLine = (char *)malloc(CMDBUFSIZE*sizeof(char));
  char* cmd =  (char *)malloc(NAMESIZE*sizeof(char));
  char* tail  = (char *)malloc(CMDBUFSIZE*sizeof(char));
  commandName z; int j;

  shift_debug_prompt();
  getCommandLine(cmdLine);
  getToken(cmdLine, cmd, tail, " "); 
  z = getCommand(cmd);
  /* printf("Head %s, Tail %s, Id %d \n", cmd, tail, z ); */
    
  while (z != cont_e)
    {
      dispatchRequest( z, tail );
      printf("\n\n");
      shift_debug_prompt();
      getCommandLine(cmdLine);
      getToken(cmdLine, cmd, tail, " ");
      z = getCommand(cmd);
      /* printf("Head %s, Tail %s, Id %d \n", cmd, tail, z); */
    }

  /* We must have hit cont_e */
  j = atoi(tail);
  if (j == 0)
    {
      printf("Please specify number of time-clicks larger than 0\n");
      userInteract();
    }
  else
    {
      Tracer->stopInN = atoi(tail);
      Tracer->lastLength = Tracer->stopInN;
      Tracer->returnControl = TRUE;
    }
}


#endif /* CONTROL_I */

/* end of file -- control.c -- */
