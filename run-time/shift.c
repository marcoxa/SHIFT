/* -*- Mode: C -*- */

/* shift.c -- */

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

#ifndef run_time_shift_i
#define run_time_shift_i

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
#include <gc_support.h>
 
#include "shifttypes.h"
#include "run-time.h"
#include "collection.h"
#include "shift_debug.h"
#include "names.h"
#include "information.h"

#ifdef TIMETESTING
#include "watch.h"
#endif
 
extern API_function_information * API_callbackList;
extern API_function_information * Debug_callbackList;
extern int shift_debug;
extern int shift_gui_debug;
GenericSet *new_components;
sim_argument *simulation_args[TOTAL_ARGS];

Component *_global; /* This must be initialized by createGlobal itself */

int _recur_depth = 0;

char *dump_continuous_state;


long tclick;

int ISVARSTEP = 0;

name_list * NameList;
name_list * GraphList;
Canvas_Data * Canvas_Data_List;
char Transition_Data[20000];
char Trace_Data[20000];
char testing[30];
int Transition_Info;
int Trace_Info;
int Zeno_Warning;
double h;			/* integration time step to be read
				 * from ISTEP env.var.
				 */
long nsteps;			/* total number of simulation steps */


char *
safe_malloc(int size)
{
  char* x = (char*)malloc(size);

  if (x == 0)
    {
      fprintf(stderr, "SHIFT runtime: error: out of heap memory.\n");
      fflush(stderr);
      exit(1);
    }
  memset(x, 0, size);
  return x;
}


/* Printing and errors */

char* TypeKindString[] = {
  "NUMBER_T", "SYMBOL_T", "LOGICAL_T",
  "ARRAY_T", "SET_T", "COMPONENT_T",
  "CVP_T",
  "UNKNOWN_T"
};


void
runtime_error(char *string, ...)
{
  va_list args;

  va_start(args, string);
  fprintf(stderr, "SHIFT runtime: error: ");
  vfprintf(stderr, string, args);
  putc('\n', stderr);
  va_end(args);
  fflush(stderr);
  exit(1);
}


void
runtime_warning(char *string, ...)
{
  va_list args;

  va_start(args, string);
  fprintf(stderr, "SHIFT runtime: warning: ");
  vfprintf(stderr, string, args);
  putc('\n', stderr);
  va_end(args);
  fflush(stderr);
}


TypeDescriptor generic_component_type = {COMPONENT_T, 0, 0};
TypeDescriptor cvp_type = {CVP_T, 0, 0};

Component *live_components = 0;
Component *continuous_components = 0;

GenericSet *potential_transitions;
GenericSet *differential_components;


#ifndef HAVE_SHIFT_GUI_P
/* initGuiDebug -- The following function is really a place holder for
 * the graphic environment entry point
 */
void
initGuiDebug()
{
  fprintf(stderr, "SHIFT runtime: discrete.c %d:\n", __LINE__);
  fprintf(stderr, "SHIFT runtime: the graphic environment has not\n");
  fprintf(stderr, "               been installed.\n\n");
  exit(1);
}

/* serverListen -- The following function is really a place holder for
 * the graphic environment entry point
 */
void
serverListen(int timeclick, int STOP_FLAG)
{
  fprintf(stderr, "SHIFT runtime: discrete.c %d:\n", __LINE__);
  fprintf(stderr, "SHIFT runtime: the graphic environment has not\n");
  fprintf(stderr, "               been installed.\n\n");
  exit(1);
}

#endif


void
main(int argc, char** argv)
{
#ifdef TIMETESTING
  Watch watch;
#endif /* TIMETESTING */
  char *estr;
  int arg_status;
  int click, ii;
  int smartpath_step;
  API_callbackList= NULL;
  Debug_callbackList= NULL;
  NameList  = NULL;
  GraphList = NULL;
  Canvas_Data_List = NULL;
  Transition_Data[0] = '\0';
  Transition_Info    = 0;
  Trace_Data[0] = '\0';
  Trace_Info    = 0;
  Zeno_Warning = 0;
  testing[0] = '\0';

  /* Initialize the random generators.
   * Unfortunately, for historical reasons we have to use pretty much
   * all of them.
   *
   * Marco Antoniotti 19970702
   */

#if defined (HAVE_DRAND48)

  srand48((long) time(NULL));	/* for drand48 */

#endif /* HAVE_DRAND48 */

  srand((unsigned int) time(NULL));
  srandom((unsigned int) time(NULL));


  /* Process more cmd line arguments */
  
  arg_status = processArguments(argc, argv);

  
  if (arg_status == 0) {
    fprintf(stderr, "Illegal arguments found. Exiting...\n");
    fprintf(stderr, "For more command-line option information, use the");
    fprintf(stderr, " '--help' option.\n");
    exit(1);
  }
  if (arg_status == 2) {
    exit(1);
  }
    /*
      for (ii = 0; ii < argc; ii++)
      if (!strcmp(argv[ii], "--varstep"))
      ISVARSTEP = 1;
      */
    /*
      if (ISVARSTEP)
      fprintf(stderr, "Variable step integration\n");
    */

  new_components = 0;
  potential_transitions = new_set(&cvp_type);
  dump_continuous_state = getenv("SHIFT_DUMP_CONTINUOUS");
 
  (void) createGlobal();

  /* *** MAK ***   reading the ISTEP env. var to define an integration step */
  
  if (simulation_args[ISTEP_ARG]->value == 1) {
    h = simulation_args[ISTEP_ARG]->double_arg;
    if (h < Epsilon || h > 1.0)
      {
	h = DEFAULT_ISTEP_ARG;
	runtime_warning("SHIFT_ISTEP environment variable out of tolerance;");
	runtime_warning("using the default value \n");
      }
  }
  else if (simulation_args[VARSTEP_ARG]->value == 1) {
    ISVARSTEP = 1;
    if (ISVARSTEP)
      fprintf(stderr, "Variable step integration\n");
  }
  else {
    fprintf(stderr, "ERROR - run-time system args: should not be here ");
  }



  if (simulation_args[NSTEPS_ARG]->value == 1)    {
    nsteps = simulation_args[NSTEPS_ARG]->long_arg;
    if (nsteps <= 1 || nsteps >= 32000000)
      {
	nsteps = def_nsteps;
	runtime_warning("SHIFT_NSTEPS environment variable too large;");
	runtime_warning("using the default value.");
      }
  }
  else
    nsteps = DEFAULT_NSTEPS_ARG;

  tclick = 0;

  information_initialize();

  information_functions();	/* This call is necessary to make sure
				 * that the TTY debugger gets a chance
				 * to be called before any transition
				 * takes place
				 *
				 * Marco Antoniotti 19970123
				 */

  check_live_components();
 
#ifdef TIMETESTING
  watch.loop = 0;
  fprintf(stderr,"              *** Time Test ***\n\n\n");
  fprintf(stderr,"Loop          Time for                Time for\n");
  fprintf(stderr,"              Discrete Step           Continuous Step\n");
  fprintf(stderr,"-------------------------------------------------------\n");
#endif /* TIMETESTING */

  for (;;)
    {
      information_functions();

#ifdef TIMETESTING
      increase_loop(&watch);
      start_timer(&watch);
#endif /* TIMETESTING */

      discrete_step();

#ifdef TIMETESTING
      end_timer(&watch);
      get_time_diff(&watch);
      fprintf(stderr,
	      "%d                %d",
	      watch.loop,
	      (watch.diff_time).tv_usec);
      start_timer(&watch);
#endif /* TIMETESTING */

      continuous_step();

#ifdef TIMETESTING
      end_timer(&watch);
      get_time_diff(&watch);
      fprintf(stderr,"                  %d\n", watch.diff_time.tv_usec);
#endif /* TIMETESTING */
    }
}

#endif /* ifdef run_time_shift_i */

/* end of file -- shift.c -- */


