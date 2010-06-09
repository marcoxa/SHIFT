/* -*- Mode: C -*- */

/* run-time.h -- */

/*
 * Copyright (c)1996, 1997, 1998 The Regents of the University of
 * California (Regents). All Rights Reserved. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research, and not-for-profit purposes,
 * without fee and without a signed licensing agreement, is hereby
 * granted, provided that the above copyright notice, this paragraph and
 * the following two paragraphs appear in all copies, modifications, and
 * distributions. 
 *
 * Contact The Office of Technology Licensing, UC Berkeley, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
 * commercial licensing opportunities. 
 *
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
 * ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION
 * TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS. 
 */

/* run-time.h -- Common definitions used in the run-time. */


#ifndef RUN_TIME_H
#define RUN_TIME_H

#include <shift_config.h>
#include <string.h>

#define def_istep 0.05
#define Epsilon 0.000001
#define def_nsteps 1000

/* Runtime related declarations
 */
#ifndef run_time_shift_i

extern char* safe_malloc(int);

extern void runtime_error(char *string, ...);
extern void runtime_warning(char *string, ...);

#endif /* run_time_shift_i */


#define MAX_RECURSION_ALGEBRAIC_DEPTH 1000

#ifndef run_time_shift_i
extern int _recur_depth;
#endif /* run_time_shift_i */



/* 
   These are the definitions for the arguments that are can be used
   with possible a simulation. To find out if any of these arguments
   are set, access the global variable simulation_args[INDEX]. to add
   filed to the array, just define them here, and allocate more space
   to the simulation_args array in shift.c
   Daniel Wiesmann 10/10/97
*/

/* Simulation length arguments */
#define NSTEPS_ARG 0     /* Number of steps in the simulation */
#define NTIME_ARG 1      /* Total time to run simulation for */

/* Simulation integration step arguments */
#define VARSTEP_ARG 2    /* Variable step runge-kutta */
#define ISTEP_ARG 3   


/* Simulation debug mode arguments */
#define DEBUG_ARG 4          /* default, the command line debugger */
#define GUI_DEBUG_ARG 5       /* Run with TkShift */
#define DBG_SCRIPT_ARG 6   /* Run with debug script */
#define DB_DUMP_ARG 7      /* Run with dump-to-database option */
#define NO_DEBUG_ARG 8
#define DUMP_SMARTPATH_ARG 9


#define TOTAL_ARGS 10  /* Important: Keep this updated */

typedef struct sim_argument {
  int type;
  int value;
  char * string_arg; /* To hold a filename */
  double double_arg;
  long long_arg;
  int int_arg;
} sim_argument;
  

/* 
   The following list contains the default values for the argumets
   of a simulation 
   */

#define DEFAULT_NSTEPS_ARG 100000     /* Number of steps in the simulation */
#define DEFAULT_NTIME_ARG 0       /* The default is the NSTEPS default */

/* Simulation integration step arguments */
#define DEFAULT_VARSTEP_ARG 0    /* The default is DEFAULT_ISTEP */
#define DEFAULT_ISTEP_ARG 0.05


/* Simulation debug mode arguments */
#define DEFAULT_DEBUG_ARG 4          /* default, the command line debugger */
#define DEFAULT_GUI_DEBUG_ARG 5       /* Run with TkShift */
#define DEFAULT_DBG_SCRIPT_ARG 6   /* Run with debug script */
#define DEFAULT_DB_DUMP_ARG 7      /* Run with dump-to-database option */
#define DEFAULT_DUMP_SMARTPATH_ARG 8



/* argument parsing functions defined here */


sim_argument * create_null_argument(int argument_type);
int processArguments(int argc, char ** argv);
int parseArguments(int argc, char ** argv);
int checkArguments();
int addDefaults();




#endif /* RUN_TIME_H */

/* end of file -- run-time.h -- */
