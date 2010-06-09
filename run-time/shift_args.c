/* -*- Mode: C -*- */

/* shift_args.c -- */

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

/* This file contains the functions that parse the command-line
   arguments given to the shift simulation 
*/

#ifndef SHIFT_ARGS_I
#define SHIFT_ARGS_I

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "run-time.h"
#include "names.h"
#include "shift_debug.h"


extern sim_argument *simulation_args[];

/* 
   function: processArguments().
   This function receives the argc and argv from main. It calls
   parseArguments, checkArguments, and finally, addDefaults. If all
   these functions return successfully, the arguments are good and it 
   returns 1. If it returns 0, main will exit. If it returns 2, it
   means that the --help function was invoked, and we do not run the
   simulation. 
   */

int 
processArguments(int argc, char ** argv)
{
  int status;

  status = parseArguments(argc, argv);
  if (status != 1)
    return status;
  
  status = checkArguments();
  if (status != 1)
    return status;
  
  status = addDefaults();

  return status;
}
  

/* parseArguments() --
 * This function goes through the list of arguments and puts them into
 * the global simulation_args[] array. It checks to make sure that the
 * parameters given to the arguments are of the correct type, but does
 * not check to make sure the parameters are compatible. 
 */
int 
parseArguments(int argc, char **argv)
{
  int arg_counter;
  double double_arg;
  int int_arg;
  long long_arg;
  char * char_arg;
  int success = 1;
  int parse;
  char **ptr = '\0';
  sim_argument * new_arg;

  arg_counter = 0;

  /* Loop through argv list */
  for (arg_counter =1; arg_counter < argc; arg_counter++)
    {
      if (!strcmp(argv[arg_counter], "-nt")
	  || !strcmp(argv[arg_counter], "--ntime"))
	{
	  /* Option 1: Time of simulation option */

	  /* Check if there is another argument in the list. If there is,
	     get it and make it a double. */
	  if (arg_counter + 1 < argc)
	    {
	      double_arg = strtod(argv[arg_counter+1], ptr);
	    }
	  else
	    {
	      runtime_error("the %s option syntax is -nt <number> or --ntime <number>.", 
			    argv[arg_counter]);
	      success = 0;
	      break;
	    }
      
	  if (ptr != NULL)
	    {
	      if (! **ptr == '\0')
		{
		  fprintf(stderr, "ERROR: invalid \'%s\' argument for %s option.\n", 
			  argv[arg_counter + 1],
			  argv[arg_counter]);
		  success = 0;
		} 
	    }
	  else if (double_arg == 0)
	    {
	      fprintf(stderr, "ERROR: invalid argument for %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	    }
	  else if (errno == ERANGE)
	    {
	      fprintf(stderr, "ERROR: invalied argument for %s option.\n",
		      argv[arg_counter]);
	      success = 0;
	    }
	  else if (double_arg < 0)
	    {
	      fprintf(stderr, "ERROR: %s value must be positive.\n", 
		      argv[arg_counter]);
	      success = 0;
	    }

	  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
	  new_arg->type = NTIME_ARG;
	  new_arg->value = 1;
	  new_arg->double_arg = double_arg;
	  simulation_args[NTIME_ARG] = new_arg;
	  arg_counter++;

	  /* End of time of simulation option */

	} 
      else if (!strcmp(argv[arg_counter], "-ns")
	       || !strcmp(argv[arg_counter], "--nsteps"))
	{

	  /* NSTEPS option: this takes an extra argument that specifies
	     the number of steps to simulate for */
	  if (arg_counter + 1 < argc)
	    {
	      long_arg = strtol(argv[arg_counter+1], ptr, 0);
	    }
	  else
	    {
	      fprintf(stderr, "ERROR: missing argument for the %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	      break;
	    }
	  if (long_arg == 0)
	    {
	      fprintf(stderr, "ERROR: invalid argument for %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	    } 
	  if (ptr != NULL)
	    {
	      if (! **ptr == '\0')
		{
		  fprintf(stderr,  "ERROR: invalid \'%s\' argument for %s option.\n", 
			  argv[arg_counter+1], argv[arg_counter]);
		  success = 0;
		} 
	    }
	  else if (long_arg == 0)
	    {
	      fprintf(stderr, "ERROR: invalid argument for %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	    }
	  else if (errno == ERANGE)
	    {
	      fprintf(stderr, "ERROR: invalied argument for %s option.\n",
		      argv[arg_counter]);
	      success = 0;
	    }
	  else if (long_arg < 0)
	    {
	      fprintf(stderr, "ERROR: %s value must be positive.\n", 
		      argv[arg_counter]);
	      success = 0;
	    }

	  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
	  new_arg->type = NSTEPS_ARG;
	  new_arg->value = 1;
	  new_arg->long_arg = long_arg;
	  simulation_args[NSTEPS_ARG] = new_arg;
	  arg_counter++;      

	  /* End of the NSTEPs option */

	} 
      else if (!strcmp(argv[arg_counter], "-vs")
	       || !strcmp(argv[arg_counter], "--varstep"))
	{
      	  /* Variable integration option */

	  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
	  new_arg->type = VARSTEP_ARG;
	  new_arg->value = 1;
	  simulation_args[VARSTEP_ARG] = new_arg;

	} 
      else if (!strcmp(argv[arg_counter], "-is")
	       || !strcmp(argv[arg_counter], "--istep"))
	{
	  /* Integration step option */

	  if (arg_counter + 1 < argc)
	    {
	      double_arg = strtod(argv[arg_counter+1], ptr);
	    }
	  else
	    {
	      fprintf(stderr, "ERROR: missing argument for the %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	      break;
	    }
	  if (ptr != NULL)
	    {
	      if (!  **ptr == '\0')
		{
		  fprintf(stderr,  "ERROR: invalid \'%s\' argument for %s option.\n", 
			  argv[arg_counter+1], argv[arg_counter]);
		  success = 0;
		} 
	    }
      
	  else if (double_arg == 0)
	    {
	      fprintf(stderr, "ERROR: invalid argument for %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	    }
	  else if (errno == ERANGE)
	    {
	      fprintf(stderr, "ERROR: invalied argument for %s option.\n",
		      argv[arg_counter]);
	      success = 0;
	    }
	  else if (double_arg < 0)
	    {
	      fprintf(stderr, "ERROR: %s value must be positive.\n", 
		      argv[arg_counter]);
	      success = 0;
	    }

	  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
	  new_arg->type = ISTEP_ARG;
	  new_arg->value = 1;
	  new_arg->double_arg = double_arg;
	  simulation_args[ISTEP_ARG] = new_arg;
	  arg_counter++;
      
	  /* End of ISTEP option */
	}
      else if (!strcmp(argv[arg_counter], "-dbg")
	       || !strcmp(argv[arg_counter], "--line_debug"))
	{
       
	  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
	  new_arg->type = DEBUG_ARG;
	  new_arg->value = 1;
	  simulation_args[DEBUG_ARG] = new_arg;

	} 
      else if (!strcmp(argv[arg_counter], "--smartPATH")
	       || !strcmp(argv[arg_counter], "-sp"))
	{
      	  if ((arg_counter +1) < argc)
	    {
	      double_arg = strtod(argv[arg_counter+1], ptr);
	    }
	  else {
	    fprintf(stderr, "ERROR: missing argument for the %s option.\n", 
		    argv[arg_counter]);
	    success = 0;
	    break;
	  }
      
	  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
	  new_arg->type = DUMP_SMARTPATH_ARG;
	  new_arg->value = 1;
	  new_arg->string_arg
	    = safe_malloc(sizeof(strlen(argv[arg_counter+1]) + 1));
	  strcpy(new_arg->string_arg, argv[arg_counter+1]);
	  simulation_args[DUMP_SMARTPATH_ARG] = new_arg;
	  arg_counter++;

	}
      else if (!strcmp(argv[arg_counter], "-nd")
	       || !strcmp(argv[arg_counter], "--no-debug"))
	{
      
	  /* Variable integration option */

	  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
	  new_arg->type = NO_DEBUG_ARG;
	  new_arg->value = 1;
	  simulation_args[NO_DEBUG_ARG] = new_arg;
	}
      else if (!strcmp(argv[arg_counter], "-v")
	       || !strcmp(argv[arg_counter], "--visual"))
	{
	  if (arg_counter + 1 < argc)
	    {
	      long_arg = strtol(argv[arg_counter+1], ptr, 0);
	    }
	  else
	    {
	      fprintf(stderr, "ERROR: missing argument for the %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	      break;
	    }
	  if (long_arg == 0)
	    {
	      fprintf(stderr, "ERROR: invalid argument for %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	    } 
	  if (ptr != NULL)
	    {
	      if (! **ptr == '\0')
		{
		  fprintf(stderr,  "ERROR: invalid \'%s\' argument for %s option.\n", 
			  argv[arg_counter+1], argv[arg_counter]);
		  success = 0;
		} 
	    }
	  else if (long_arg == 0)
	    {
	      fprintf(stderr, "ERROR: invalid argument for %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	    }
	  else if (errno == ERANGE)
	    {
	      fprintf(stderr, "ERROR: invalied argument for %s option.\n",
		      argv[arg_counter]);
	      success = 0;
	    }
	  else if (long_arg < 5000 || long_arg > 10000)
	    {
	      fprintf(stderr, "ERROR: Unix protocol dictates the socket address must be between 5000 and 10000.\n");
	      success = 0;
	    }

	  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
	  new_arg->type = GUI_DEBUG_ARG;
	  new_arg->value = 1;
	  new_arg->long_arg = long_arg;
	  simulation_args[GUI_DEBUG_ARG] = new_arg;
	  arg_counter++;      

	} 
      else if (!strcmp(argv[arg_counter], "-dbd")
	       || !strcmp(argv[arg_counter], "--db_dump"))
	{
	  if (arg_counter + 1 < argc)
	    {
	      double_arg = strtod(argv[arg_counter+1], ptr);
	    }
	  else
	    {
	      fprintf(stderr, "ERROR: missing argument for the %s option.\n", 
		      argv[arg_counter]);
	      success = 0;
	      break;
	    }
      
	  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
	  new_arg->type = DB_DUMP_ARG;
	  new_arg->value = 1;
	  new_arg->string_arg
	    = safe_malloc(sizeof(strlen(argv[arg_counter+1]) + 1));
	  strcpy(new_arg->string_arg, argv[arg_counter+1]);
	  simulation_args[DB_DUMP_ARG] = new_arg;
	  arg_counter++;
	}
      else if (!strcmp(argv[arg_counter], "-dbs")
	       || !strcmp(argv[arg_counter], "--dbg_script"))
	{
	  printf("-- The %s option is not supported (yet).\n", 
		 argv[arg_counter]);

	}
      else if (!strcmp(argv[arg_counter], "--help")
	       || !strcmp(argv[arg_counter], "-h")
	       || !strcmp(argv[arg_counter], "-?"))
	{
	  printf("The valid options are:\n");
	  printf("Simulation Length Parameters:\n");
	  printf(" --nsteps (-ns) steps:\n");
	  printf("\tNumber of timeclicks to run simulation for.\n");

	  printf(" --ntime (-nt) time:\n");
	  printf("\t Number of units of time to run simulation for.\n\n");

	  printf("Simulation Integration Parameters:\n");
	  printf(" --istep (-is) step-size:\n");
	  printf("\tIntegration step size for Runge-Kutta integrator\n");
	  printf("\t(default step size is 0.05).\n");

	  printf(" --varstep (-vs):\n");
	  printf("\tVariable step Runge-Kutta integration. \n\n");

	  printf("Simulation Debugger and Printing Parameters:\n");
	  printf(" --no-debug (-nd):\n\t Do not invoke a debugger. \n");
	  printf(" --line-debug (-dbg):\n");
	  printf("\tUse command-line debugger (default) \n");

	  printf(" --visual (-v) socket-address:\n");
	  printf("\tAllows one to link to the TkShift visual debugger\n");
	  printf("\tusing the  'socket_address' socket. \n");

	  printf(" --db_dump (-dbd) filename:\n");
	  printf("\tUse the commands in filename to do a database dump.\n");

	  printf(" --smartPATH (-sp) filename:\n");
	  printf("\tDump SmartPATH formatted data to the file \'filename\'\n");

	  printf(" --help (-h):\n\t Prints this information \n");
	  if (success == 1) 
	    success = 2;
	} 
      else
	{
	  printf("ERROR: I do not recognize the %s argument.\n",
		 argv[arg_counter]);
	  success = 0;
	  /*some unknown thingy: error?*/
	}
    }
  return success; /* Maybe no success */
}


/* checkArguments --
 * This funtions looks at the arguments given to main, and issues an
 * error if any of them are incompatible.
 * Incompatibilities:
 * -different debuggers
 * -different integration steps
 * -different times to run simulation
 */
int 
checkArguments()
{
  int counter;
  int debuggers = 0;
  int success = 1;

  /* Check for incompatibilities with the NSTEPS and NTIME */
  if (simulation_args[NSTEPS_ARG] != NULL
      && simulation_args[NTIME_ARG] != NULL)
    {
      fprintf(stderr, "ERROR, you may only specify one of the ");
      fprintf(stderr, "simulation length parameters.\n");
      success = 0;
    }

  if (simulation_args[VARSTEP_ARG] != NULL
      && simulation_args[ISTEP_ARG] != NULL)
    {
      fprintf(stderr, "ERROR, you may only specify one of the ");
      fprintf(stderr, "simulation integration step parameters.\n");
      success = 0;
    }
  if (simulation_args[NO_DEBUG_ARG] != NULL) 
    debuggers++;
  if (simulation_args[DEBUG_ARG] != NULL) 
    debuggers++;
  if (simulation_args[GUI_DEBUG_ARG] != NULL) 
    debuggers++;
  if (debuggers > 1) 
    {
      fprintf(stderr, "ERROR, you may only specify one of the ");
      fprintf(stderr, "simulation debugger parameters.\n");
      success = 0;
    }

  /* Initialize the shift_api if one of the debuggers is called */

  if (simulation_args[DEBUG_ARG] != NULL
      || simulation_args[GUI_DEBUG_ARG] != NULL
      || simulation_args[DUMP_SMARTPATH_ARG] != NULL)
    {
      shift_api_initialize_user_types();
      shift_api_initialize_global_variables();
    }
  return success;
}


/* addDefaults --
 * This functions adds the default parameters that the users did not
 * specify to the simulation_args array.
 */
int
addDefaults()
{
  int counter;
  sim_argument* new_arg;

  for (counter = 0; counter < TOTAL_ARGS; counter++)
    {
      if (simulation_args[counter] == NULL)
	{
	  switch (counter)
	    {
	    case NSTEPS_ARG:
	      if (simulation_args[NTIME_ARG] == NULL)
		{
		  new_arg = create_null_argument(NSTEPS_ARG);
		  new_arg->value = 1;
		  new_arg->long_arg = DEFAULT_NSTEPS_ARG;
		  simulation_args[NSTEPS_ARG] = new_arg;
		}
	      else
		{
		  simulation_args[NSTEPS_ARG]
		    = create_null_argument(NTIME_ARG);
		}
	      break;

	    case NTIME_ARG:
	      simulation_args[NTIME_ARG] = create_null_argument(NTIME_ARG);
	      break;

	    case ISTEP_ARG:
	      if (simulation_args[VARSTEP_ARG] == NULL)
		{
		  new_arg = create_null_argument(ISTEP_ARG);
		  new_arg->value = 1;
		  new_arg->double_arg = DEFAULT_ISTEP_ARG;
		  simulation_args[ISTEP_ARG] = new_arg;
		}
	      else if (simulation_args[VARSTEP_ARG]->value == 0)
		{
		  new_arg = create_null_argument(ISTEP_ARG);
		  new_arg->value = 1;
		  new_arg->double_arg = DEFAULT_ISTEP_ARG;
		  simulation_args[ISTEP_ARG] = new_arg;
		}
	      else
		{
		  simulation_args[ISTEP_ARG]
		    = create_null_argument(VARSTEP_ARG);
		}
	      break;

	    case VARSTEP_ARG:
	      simulation_args[VARSTEP_ARG] = create_null_argument(VARSTEP_ARG);
	      break;

	    case DEBUG_ARG:
	      /* make sure visual and no-debug are not set */
	      if (simulation_args[GUI_DEBUG_ARG] == NULL
		  && simulation_args[NO_DEBUG_ARG] == NULL)
		{
		  new_arg = create_null_argument(DEBUG_ARG);
		  new_arg->value = 1;
		  simulation_args[DEBUG_ARG] = new_arg;
		}
	      else
		{
		  simulation_args[DEBUG_ARG]
		    = create_null_argument(DEBUG_ARG);
		}
	      break;

	    case GUI_DEBUG_ARG:
	      simulation_args[GUI_DEBUG_ARG]
		= create_null_argument(GUI_DEBUG_ARG);
	      break;

	    case NO_DEBUG_ARG:
	      simulation_args[NO_DEBUG_ARG]
		= create_null_argument(NO_DEBUG_ARG);
	      break;

	    case DBG_SCRIPT_ARG:
	      simulation_args[DBG_SCRIPT_ARG]
		= create_null_argument(DBG_SCRIPT_ARG);
	      break;

	    case DB_DUMP_ARG:
	      simulation_args[DB_DUMP_ARG]
		= create_null_argument(DB_DUMP_ARG);
	      break;

	    case DUMP_SMARTPATH_ARG:
	      simulation_args[DUMP_SMARTPATH_ARG]
		= create_null_argument(DUMP_SMARTPATH_ARG);
	      break;

	    default:
	      fprintf(stderr, " Default argument, should not be here...");
	      break;
	    }
	}
    }
  return;
}


/* create_null_argument --
 * This function mallocs a sim_argument, and initializes it
 */
sim_argument *
create_null_argument(int argument_type)
{
  sim_argument *new_arg;

  new_arg = (sim_argument*)safe_malloc(sizeof(sim_argument));
  new_arg->type = argument_type;
  new_arg->value = 0;

  return new_arg;
}


#endif /* SHIFT_ARGS_I */

/* end of file -- shift_args.c -- */
