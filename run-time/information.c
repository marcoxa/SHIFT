/* -*- Mode: C -*- */

/* information.c -- */

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

/*
 * Initialize and start simulation.
 */

#include <shift_config.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>

#include "shifttypes.h"
#include "collection.h"
#include "shift_debug.h"
#include "shift_tools.h"
#include "names.h"
#include "run-time.h"
#include "information.h"
#include "tkShift_data.h"
#include "shift_db_debug.h"

#ifdef TIMETESTING
#include "watch.h"
#endif


int shift_debug;
int shift_gui_debug;
extern sim_argument *simulation_args[];

#ifndef HAVE_SHIFT_GUI_P
void
GUI_print_breakpoints(shift_debug_breakpoint* ignore1,
			      int ignore2,
			      int ignore3)
{
  fprintf(stderr, "SHIFT runtime: discrete.c %d:\n", __LINE__);
  fprintf(stderr, "SHIFT runtime: the graphic environment has not\n");
  fprintf(stderr, "               been installed.\n\n");
  exit(1);
}
#endif /* HAVE_SHIFT_GUI_P */


void
information_initialize()
{
  initialize_debugger_functions(); /* This function initializes data
				    * structures that are needed for
				    * a debugger
				    */

  register_user_functions();	/* This function processes user requests
				 * for runtime control and information.
				 * It can be customized by a user in
				 * "user_functions.c"
				 */
}
  

void
initialize_debugger_functions()
{
  shift_debug     = simulation_args[DEBUG_ARG]->value;
  shift_gui_debug = simulation_args[GUI_DEBUG_ARG]->value;

  if (shift_gui_debug)
    {
      initGuiDebug();
      registerDebugCallback(serverListen, 1, 0, 0);
      shift_api_register_breakpoint_callback(GUI_print_breakpoints);
      /*      shift_api_register_breakpoint_callback(something);*/
      registerCallback(shift_DB_Traverse_CmdList,1,0,0);
    }
  else if (shift_debug)
    {
      initDebug();
      registerDebugCallback(dbgControl, 1, 0, 0);
      shift_api_register_breakpoint_callback(shift_tools_print_breakpoints);
      registerCallback(shift_DB_Traverse_CmdList,1,0,0);
    }
}


void
information_functions()
{
  execute_debugger_functions();   /* This executes the functions that
				   * give simulation information to a
				   * debugger
				   */

  execute_user_functions();       /* This is the call that executes
				   * the functions that a user may add
				   * to the simulation to custumize it
				   * for a precise need
				   */
}


void 
execute_debugger_functions()
{
  API_function_information* n = Debug_callbackList;

  while (n != NULL)
    {
      if (n->clicks_before_call == 0
	  && tclick >= n->first_call
	  && n->once != 2)
	{
	  (n->function_to_call)();
	  n->clicks_before_call = (n->call_every_n_clicks - 1);
      	  if (n->once == 1)
	    n->once = 2;
	}
      else
	{
	  n->clicks_before_call--;
	}
      n = n->next;
    }
}


void 
execute_user_functions()
{
  API_function_information* n = API_callbackList;

  while (n != NULL)
    {
      if (n->clicks_before_call == 0
	  && tclick >= n->first_call
	  && n->once != 2)
	{
	  (n->function_to_call)();
	  n->clicks_before_call = n->call_every_n_clicks - 1;
	  if (n->once == 1)
	    n->once = 2;
	}
      else
	{
	  n->clicks_before_call--;
	}
      n = n->next;
    }
}


int
registerCallback(void (*funct)(), int freq, int once, long first_call)
{
  API_function_information * n;
  API_function_information * m;

  n = (API_function_information*) safe_malloc(sizeof(API_function_information));
  n->call_every_n_clicks = freq;
  n->once = once;
  if (once)
    n->clicks_before_call = first_call - 1;
  else
    n->clicks_before_call = freq - 1;
  n->function_to_call = funct;
  /*  n->arg = arg;*/ 
  n->breakID = 15; /* TEMOPRARY */
  n->next = API_callbackList;
  n->previous = NULL;
  if (API_callbackList != NULL)
    API_callbackList->previous = n;   
  API_callbackList = n;
}


/* This function is temporary, as there is no need to have two
 * functions that do almost exactly the same thing. However, there
 * should be no need for a user to know what data structure they are
 * registering their funtions in.
 */
int
registerDebugCallback(void (*funct)(), int freq, int once, long first_call)
{
  API_function_information* n;
  API_function_information* m;

  n = (API_function_information*) safe_malloc(sizeof(API_function_information));
  n->call_every_n_clicks = freq;
  n->clicks_before_call = freq - 1;
  n->once = once;
  n->function_to_call = funct;
  n->breakID = 15; /* TEMOPRARY */
  n->next = Debug_callbackList;
  n->previous  =NULL;
  if (Debug_callbackList != NULL)
    Debug_callbackList->previous = n;   
  Debug_callbackList = n;
}


/* This function takes a breakID as an argument, and removes the
 * callback function with that BreakID in the API_callbackList. The
 * function returns 1 if the breakpoint removal is successfull, 0
 * otherwise.
 */
int  
unregisterCallback(int breakID)
{
  API_function_information* n;
  int found = 0;

  n = API_callbackList;
  while (n != NULL)
    {
      if (n->breakID == breakID)
	{
	  /* Remove item */
	  if (n == API_callbackList) /* First Element */
	    API_callbackList = n->next;
	  else /* Middle Element */
	    {
	      n->previous->next = n->next;
	      if (n->next != NULL) 
		n->next->previous = n->previous;
	    }
	  break;
	}
      else
	n = n->next;
    }
  if (n == NULL)
    {
      fprintf(stderr, "Error: Breakpoint %d could not be found\n", breakID);
      return 0;
    }
  else
    {
      return 1;
    }
}


/* end of file information.c */
