/* -*- Mode: C -*- */

/* communication.c -- */

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

#ifndef COMMUNICATION_I
#define COMMUNICATION_I

#include <shift_config.h>

#include <tcl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tkshift-util.h"

/* stripSpaces:  this function is a helper function for the Execute 1.
   when a line is given it strips off the all the spaces at the beginning
   of the line  */

/* int NOT_REACHED = 0; */

char *
C_AddSpaces(char* list, int list_size)
{
  char *next_element;
  char *var_name;
  char *var_valu;
  char *filled_list;
  int count;
  int num_spaces;

  filled_list = tkshift_safe_malloc(list_size);

  /* this probably won't happen, but . . .*/
  if (strcmp (list, "") != 0)
    {

      /* initial bracket  */
      next_element = strchr(list, '{');

      /* now this is the main loop */
      while (next_element != NULL)
	{
	  /* making of var_name  */
	  var_name = &next_element[1];
	  var_valu = strchr(var_name, ' ');
	  var_valu[0] = '\0';

	  /* making of var_valu */
	  var_valu = &var_valu[1];
   	  next_element = strchr(var_valu, '}');

	  /* this means that it is a set */
	  if (next_element[1] == '}')
	    {
	      next_element = &next_element[1];
	    }
	  next_element[0] = '\0';

	  /* creating new next_element */
	  next_element = &next_element[1];
	  next_element = strchr(next_element, '{');

	  /* now lets fill spaces and append */
	  strcat(filled_list, "{");
	  strcat(filled_list, var_name);
	  num_spaces = 20 - strlen(var_name);
	  for (count = 0; count < num_spaces; count++)
	    {
	      strcat(filled_list, " ");
	    }
	  strcat(filled_list, var_valu);
	  strcat(filled_list, "} ");
	
	}
      return filled_list;
    }
  return "";
}

int
FindNewSizeAfterAddSpaces(char* list)
{
  char *next_element;
  char *var_name;
  char *var_valu;
  int count = 0;

  /* this probably won't happen, but . . .*/
  if (strcmp (list, "") != 0)
    {

      /* initial bracket  */
      next_element = strchr(list, '{');

      /* now this is the main loop */
      while (next_element != NULL)
	{
	  /* making of var_name  */
	  var_name = &next_element[1];
	  var_valu = strchr(var_name, ' ');
	  var_valu[0] = '\0';

	  /* 5 is safety for extra stuff such as spaces and brackets */
	  if (strlen(var_name) > 20)
	    {	
	      count = count + strlen(var_name) + 5;
	    }
	  else
	    {
	      count = count + 20 + 5;
	    }
	
	  /* making of var_valu */
	  var_valu = &var_valu[1];
	  next_element = strchr(var_valu, '}');

	  /* this means that it is a set */
	  if (next_element[1] == '}')
	    {
	      next_element = &next_element[1];
	    }

	  next_element[0] = '\0';

	  if (strlen(var_valu) > 20)
	    {	
	      count = count + strlen(var_valu) + 5;
	    }
	  else
	    {
	      count = count + 20 + 5 ;
	    }

	  /* creating new next_element */
	  next_element = &next_element[1];
	  next_element = strchr(next_element, '{');
	
	}
      return count;
    }
  return 0;
}

int
C_stripSpaces(ClientData clientData, Tcl_Interp *interp,
	    int argc, char *argv[])
{					
  int i; 		
  char *line = (char *) argv[1];

  if	 (argc != 2) {
    interp->result = "Usage: stripSpaces line ";
    return TCL_ERROR;
  }			

  for (i = 0; line[i] == ' '; i++);
  strcpy(interp->result, &line[i]);
  return TCL_OK; 	
}

int
C_ProcessData(ClientData clientData, Tcl_Interp *interp,
	      int argc, char *argv[])
{

  /* this is how it is defined in communication.tcl */

  /* these commands are created to communicate with tcl interpreter */
  int magic_number = 400; /* 400 should be enough for some command or
			   * variable name
			   */
  int tcl_command_size;
  char *tcl_command;
  char *tcl_variable;

  /* temporary pointers needed to manipulate data */
  char *tempstr;
  char *type;
  char *str;	
  char last;
  char *instance;
  char *var_list;
  int timeClick;
  char *time_elapsed;
  char *Time;
  char canvas;
  char group;
  char var_number;

  /* these are global variables */
  char *TimeClick;
  char *TimeVectors;
  char *isStartUp;
  int startup;
  char *channel1;
  char *channel2;
  char *BreakNumber;
  char *TkShiftValues;

  int var_list_size;

  double start_time;
  double end_time;
  double elapsed_time;
  int script_windows_size;
  int pending_scripts_size;
  int script_windows_count;
  int pending_scripts_count;
  char * script_windows;
  char * pending_scripts;
  char * script_windows_list;
  char * pending_scripts_list;
  char * script_windows_index;
  char * pending_scripts_index;
  char * script_type;
  char * script_type_start;
  char * script_type_end;

  char * break_type;
  char * break_data;
  int break_inst;

  char * script_data;
  char * w;
  int number;
  int wait_counter;

  char *data = argv[1];
  char datatype = data[2];

  int data_size;

  data_size = strlen(data);


  tcl_command_size = data_size + magic_number;
  tcl_command = tkshift_safe_malloc(tcl_command_size);

  /*  printf ("data received by ProcessLine is: %s", data); */
  /*  printf("The current size of tcl_command is %d.\n", tcl_command_size); */

  if (argc != 2)
    {
      interp->result = "Usage: ProcessData buffer ";
      return TCL_ERROR;
    }							

  /*  printf ("am i reached? "); */

  /* #message names used so far:
     # A: ParseTypesData 				okay
     # B: Breaktype information				okay
     # C: Get vector data				okay 	
     # E: TimeClick					okay	
     # H: Houston information
     # I: GetInstances but don't put them in a box	okay
     # L: GetInstances and put them in a Show box	okay
     # P: Get Variables
     # Q: tracecomp info				okay
     # R: SmartAHS information
     # X: Exit simulation
     # W: Canvas info with 2 data items			okay
     # Y: Canvas info with 3 data items			okay
     # S: Zeno Warning has been sent.
     # Z: Get variable information 			okay */

  /* main loop */

  if (datatype == 'L')
    {
      /* we want to get what is in the middle of #'s */
      type = strchr(data, '#');
      type = &type[1];
      tempstr = strchr(type, '#');
      str = &tempstr[1];
      tempstr[0] = '\0';
      sprintf(tcl_command, "GetInstances {%s} {%s}", type, str);
      Tcl_Eval(interp, tcl_command);

      tcl_variable = Tcl_GetVar2(interp,
				 "InstanceTypeList",
				 type, TCL_GLOBAL_ONLY);
      if (tcl_variable == "")
	{
	  sprintf(tcl_command, "bell");
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK);
	
	  sprintf(tcl_command,
		  "tk_dialog .info Info \"No Instances of this type exist at this point of time in the simulation. Instances may be created later.\" info 0 OK");
	
	  Tcl_Eval(interp, tcl_command);
	}
      else
	{
	  sprintf(tcl_command, "Show {%s}", type);
	  Tcl_Eval(interp, tcl_command);
	}
    }
  else if (datatype == 'I')
    {
      /* #This next one is similar to the previous case, but it does not pop up
	 #the instance box because it does not call the proc Show. This
	 #function just tries to get an updated set of instances into the
	 #global variable InstanceTypeList  */

      /* we want to get what is in the middle of #'s */
      type = strchr(data, '#');
      type = &type[1];
      tempstr = strchr(type, '#');
      str = &tempstr[1];
      tempstr[0] = '\0';
      sprintf(tcl_command, "GetInstances {%s} {%s}", type, str);
      Tcl_Eval(interp, tcl_command);
    }
  else if (datatype == 'P')
    {
      last = data[3];
      type = strchr(data, '#');
      type[0] = '\0';	
      type = &type[1];

      instance = strchr(type, '#');
      instance[0] = '\0';
      instance = &instance[1];

      str = strchr(instance, '#');
      str[0] = '\0';
      str = &str[1];

      if (strcmp(type, "global") == 0)
	{
	  sprintf(tcl_command, "ProcessGlobalLine {%s}", str);
	  Tcl_Eval(interp, tcl_command);
	  var_list = tkshift_safe_malloc(strlen(interp->result));
	  strcpy(var_list, interp->result);
	  tkshift_free(tcl_command);
	  tcl_command = tkshift_safe_malloc(strlen(var_list) + magic_number);
	  sprintf(tcl_command, "Print {%s} 0 {%s} %c", type, var_list, last);
	  tkshift_free(var_list);
	  Tcl_Eval(interp, tcl_command);
	}	
      else
	{
	  sprintf(tcl_command, "ProcessVariables {%s} {%s}", type, str);
	  Tcl_Eval(interp, tcl_command);
	  var_list = tkshift_safe_malloc(strlen(interp->result));
	  strcpy(var_list, interp->result);
	  tkshift_free(tcl_command);
	  tcl_command = tkshift_safe_malloc(strlen(var_list) + magic_number);
	  sprintf(tcl_command,
		  "Print {%s} {%s} {%s} %c", type, instance, var_list, last);
	  tkshift_free(var_list);
	  Tcl_Eval(interp, tcl_command);
	}	
    }
  else if (datatype == 'Z')
    {
      if (strcmp(data, "##") == 0)
	{
	  return;
	}

      /*commented out by pcooke- debugging statement for linux port
	printf(">>> 'Z' data is : {%s} \n", data);
	fflush(stdout);
      */
      last = data[3];
      type = strchr(data, '#');
      type[0] = '\0';	
      type = &type[1];

      instance = strchr(type, '#');
      instance[0] = '\0';
      instance = &instance[1];

      str = strchr(instance, '#');
      str[0] = '\0';
      str = &str[1];

      if (strcmp(type, "global") == 0)
	{
	  sprintf(tcl_command, "ProcessGlobalLine {%s}", str);
	  Tcl_Eval(interp, tcl_command);
	  var_list = tkshift_safe_malloc(strlen(interp->result));
	  strcpy(var_list, interp->result);
	  tkshift_free(tcl_command);
	  tcl_command = tkshift_safe_malloc(strlen(var_list) + magic_number);
	  sprintf(tcl_command, "Print {%s} 0 {%s} %c", type, var_list, last);
	  tkshift_free(var_list);
	  Tcl_Eval(interp, tcl_command);
	}	
      else
	{
	  /*commented out by pcooke- debugging statement for linux port
	    printf(">>> 'Z' codes before: str \"%s\"\n", str);
	  */
	  tempstr = tkshift_safe_malloc(strlen(str));

	  memset(tempstr, 0, strlen(tempstr));

	  strcpy(tempstr, str);

	  var_list_size = FindNewSizeAfterAddSpaces(tempstr);
	 
	  tkshift_free(tempstr);

	  var_list = C_AddSpaces(str, var_list_size);

	  tkshift_free(tcl_command);

	  tcl_command = tkshift_safe_malloc(var_list_size + magic_number);
	 
	  sprintf(tcl_command, "Print {%s} {%s} {%s} %c",
		  type, instance, var_list, last);
	  /*commented out by pcooke- debugging statement for linux port
	    printf("\n>>> 'Z' codes after:\n>>> tcl_cmd: %s\n>>>str:     %s\n>>>var_lst: %s\n",
	    tcl_command,
		 str,
		 var_list);
	  */
	  tkshift_free(var_list);

	  /*commented out by pcooke- debugging statement for linux port
	  putchar('$'); fflush(stdout);
	  */

	  Tcl_Eval(interp, tcl_command);
	}
      /*commented out by pcooke- debugging statement for linux port
      puts("\n>>> 'Z' handled\n");
      */
      fflush(stdout);
    }
  else if (datatype == 'C')
    {
      sprintf(tcl_command, "FillVectorInfo {%s}", &data[4]);
      Tcl_Eval(interp, tcl_command);
    }
  else if (datatype == 'E')
    {
      TimeClick = &data[4];
      time_elapsed  = strchr(TimeClick, ' ');	
      time_elapsed[0] = '\0';
      time_elapsed = &time_elapsed[1];

      timeClick = atoi(TimeClick);
      sprintf(TimeClick, "%d", timeClick);

      /* since TimeClick is a global variable, I have to update it */
      Tcl_SetVar(interp, "TimeClick", TimeClick, TCL_GLOBAL_ONLY);  	

      /* TimeElapsed is used by the script to keep track of scripts
	 that is time dependent*/
      Tcl_SetVar(interp, "TimeElapsed", time_elapsed, TCL_GLOBAL_ONLY);  	

      /* ## Commented out by Tolga 110697
	 ## This is now done in main.tcl using traces for TimeClick & TimeElapsed
      sprintf (tcl_command, ".middle.main.time configure -text \"%s\"",
	       TimeClick);
      Tcl_Eval (interp, tcl_command); 
      sprintf (tcl_command, ".middle.main.timeel configure -text \"%s\"", 
	       time_elapsed); 
      */

      Tcl_Eval (interp, "update idletasks"); 

      /* ## Have an array containing vectors that get time data, and
	 ## update them all */
	
      if (timeClick > 1)
	{
	  sprintf(tcl_command, "UpdateTimeVectorsPlease %s", time_elapsed);
	  Tcl_GlobalEval(interp, tcl_command);
	
	  /* along with updating the graph as above, we also have to 	
	     see if there is any pending scripts, or to get rid of
	     scripts that has run out of time, right now I am going to
	     implement this in tcl because it seems as though it won't be 	
	     too much faster doing in c */

	  /*
	     sprintf(tcl_command, "CheckForPendingScriptPlease");  	
	     assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
	     */

	  elapsed_time = atof(time_elapsed);
	  sprintf(tcl_command, "array exists pending_scripts");
	  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
	  if (strcmp (interp->result, "1") == 0)
	    {
	      sprintf(tcl_command, "array name pending_scripts");
	      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
	      pending_scripts_list = tkshift_safe_malloc(strlen(interp->result));
	      strcpy(pending_scripts_list, interp->result);

	      sprintf(tcl_command, "llength {%s}", pending_scripts_list);
	      assert(Tcl_Eval(interp,tcl_command) == TCL_OK);
	      pending_scripts_size = atoi(interp->result);
	
	      for (pending_scripts_count = 0;
		   pending_scripts_count < pending_scripts_size;
		   pending_scripts_count++)
		{		
		  sprintf(tcl_command, "lindex {%s} %d", pending_scripts_list,
			  pending_scripts_count);
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
		  pending_scripts_index = tkshift_safe_malloc(strlen(interp->result));
		  strcpy(pending_scripts_index, interp->result);
		
		  sprintf(tcl_command, "lindex $pending_scripts(%s) 0",
			  pending_scripts_index);
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
		  start_time = atof(interp->result);
		
		  if (start_time <= elapsed_time)
		    {
		      sprintf(tcl_command, "lindex $pending_scripts(%s) 1",
			      pending_scripts_index);
		      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
		      end_time = atof(interp->result);

		      sprintf(tcl_command, "lindex $pending_scripts(%s) 2",
			      pending_scripts_index);
		      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
		      script_type = tkshift_safe_malloc(strlen(interp->result));
		      strcpy(script_type, interp->result);

		      sprintf(tcl_command, "lindex $pending_scripts(%s) 3",
			      pending_scripts_index);
		      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
		      number = atoi(interp->result);

		      sprintf(tcl_command, "lindex $pending_scripts(%s) 4",
			      pending_scripts_index);
		      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
		      script_data = tkshift_safe_malloc(strlen(interp->result));
		      strcpy(script_data, interp->result);
		
		      if (strcmp(script_type, "Animation") == 0)
			{
			  sprintf(tcl_command, "lindex {%s} 0",
				  script_data);
			  assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			  if (strcmp(interp->result, "SmartAHS") == 0)
			    {
			      /* smart ahs file. . . this could be tricky */

			      sprintf(tcl_command, "info exists script_windows(%s)",
				      pending_scripts_index);
			      assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			      if (strcmp(interp->result,"1") == 0)
				{
				  /* this means that canvas window is already
				     created */
				
				  sprintf(tcl_command, "info exists SmartAHSData_semaphore");
				  assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				  if (strcmp(interp->result, "1") == 0)
				    {
				      /* we have received the appropriate data */

				      sprintf(tcl_command,
					      "lindex $script_windows(%s) 1", 	
					      pending_scripts_index);
				      assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				      sprintf(tcl_command,
					      "eval DrawSmartAHSAfterInfo %s.animate.canvas",
					      interp->result);
				      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

				      sprintf(tcl_command, "unset pending_scripts(%s)",
					      pending_scripts_index);
				      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				      sprintf(tcl_command, "unset SmartAHSData_semaphore");
				      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				    }
				}
			      else
				{
				  /* this means that it is the first time going
				     through here. */
				
				  sprintf(tcl_command,
					  "set script_windows(%s) [list %f [eval CreateCanvas {%s}]]",
					  pending_scripts_index, end_time, script_data);
				  assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

				  /* instead of using getSmartAHSData, we have to
				     do it manually */

				  sprintf(tcl_command, "catch {unset AHSHighwaySectionData}");
				  assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				
				  sprintf(tcl_command, "catch {unset AHSHighwaySegmentData}");
				  assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				
				  /* getting information from SmartAHS */

				  sprintf(tcl_command, "lappend messageQueue 5900");
				  assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				  sprintf(tcl_command, "lappend messageQueue [padMessage \"Section R length width\"]");
				  assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				  sprintf(tcl_command, "lappend messageQueue 5900");
				  assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				  sprintf(tcl_command, "lappend messageQueue [padMessage \"Segment R length gxa gya orientation curvature section\"]");
				  assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				
				}


			    }
			  else
			    {
			
			      sprintf(tcl_command,
				      "set script_windows(%s) [list %f [eval CreateCanvas {%s}]]",
				      pending_scripts_index, end_time, script_data);
			
			      assert (Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			
			
			      sprintf(tcl_command, "unset pending_scripts(%s)",
				      pending_scripts_index);
			      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			    }
			
			}
		      else if (strcmp(script_type, "Graph") == 0)
			{
			  sprintf(tcl_command, "CreateNewGraph");
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			  w = tkshift_safe_malloc(strlen(interp->result));
			  strcpy(w, interp->result);
			
			  sprintf(tcl_command,
				  "set script_windows(%s) [list %f %s]",
				  pending_scripts_index, end_time, w);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			
			  sprintf(tcl_command, "dispatchGraphPlots %s {%s}", w, script_data);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			  sprintf(tcl_command, "unset pending_scripts(%s)",
				  pending_scripts_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			  sprintf(tcl_command, "ConfigureGraph %s",
				  pending_scripts_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			  tkshift_free(w);
				
			}
		      else if (strcmp(script_type, "Phase") == 0)
			{
			  sprintf(tcl_command, "CreateNewGraph");
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			  w = tkshift_safe_malloc(strlen(interp->result));
			  strcpy(w, interp->result);
			
			  sprintf(tcl_command,
				  "set script_windows(%s) [list %f %s]",
				  pending_scripts_index, end_time, w);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			
			  sprintf(tcl_command, "DispatchPhasePlot %s %s", w, script_data);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			  sprintf(tcl_command, "unset pending_scripts(%s)",
				  pending_scripts_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			  sprintf(tcl_command, "ConfigureGraph %s",
				  pending_scripts_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			  tkshift_free(w);
				
			}
		      else if (strcmp(script_type, "Break") == 0)
			{	
			  sprintf(tcl_command, "lindex {%s} 0", script_data);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			  break_type = tkshift_safe_malloc(strlen(interp->result));
			  strcpy(break_type, interp->result);

			  sprintf(tcl_command, "lindex {%s} 1", script_data);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			  break_inst = atoi(interp->result);
			
			  sprintf(tcl_command, "lindex {%s} 2", script_data);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			  break_data = tkshift_safe_malloc(strlen(interp->result));
			  strcpy(break_data, interp->result);

			  if (break_inst == -1)
			    {
			      /* if break_inst is -1, this means that this script
				 breaks the type, instead of just one instance of
				 it */
			      sprintf(tcl_command, 	
				      "set script_windows(%s) [list %f [list 8002 [padMessage %s]]]",
				      pending_scripts_index, end_time,
				      break_type);
			      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			
			      sprintf(tcl_command, "lappend messageQueue %s", break_data);
			      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			
			      sprintf(tcl_command, "unset pending_scripts(%s)",
				      pending_scripts_index);
			      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			      sprintf(tcl_command, "set TypesBroken(%s) 1",
				      break_type);
			      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			      sprintf(tcl_command, "UpdateTypesBox");
			      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);


			    }	
			  else	
			    {	
			      /* now we have to check if the instance exists at
				 this time */
			
			      /* i just realized that these c functions that is
				 imbeded in tcl/tk is implemented using semaphores
				 especially when you use trace, a function used to
				 execute Execute2, so I am going to go about a
				 different way */
			
			      /* the first time a break is processed it will
				 simply ask the simulation for the list of
				 instances of that type.  With this we will change
				 pending_script data structure a little bit to 	
				 remind us that we already have requested for the
				 instances. */
			
			      if (break_inst == -2)
				{
				  /* this is the second time around */

				  /* first we have to check if we got the instances
				     or not */
				
				  sprintf(tcl_command, "set received_instance(%s)",
					  break_type);
				  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

				  if (strcmp(interp->result, "1") == 0)
				    {
				      sprintf(tcl_command, "lindex {%s} 0", break_data);
				      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				      break_inst = atoi(interp->result);
				
				      sprintf(tcl_command, "lindex {%s} 1", break_data);
				      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				      tkshift_free(break_data);
				      break_data = tkshift_safe_malloc(strlen(interp->result));
				      strcpy(break_data, interp->result);
				
				      sprintf(tcl_command,
					      "lsearch $InstanceTypeList(%s) \"%s %d\"",
					      break_type, break_type, break_inst);
				      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				      if (strcmp(interp->result, "-1") == 0)
					{
					  sprintf(tcl_command, "unset pending_scripts(%s)",
						  pending_scripts_index);
					  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
					  sprintf(tcl_command, "bell");
					  assert(Tcl_Eval(interp, tcl_command) == 0);
					  sprintf(tcl_command, "tk_dialog .error Error \"failure in running break script: %s %d does not exist in simulation at this time\" error 0 OK", break_type, break_inst);
					  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
					  sprintf(tcl_command, "RemoveFromRunningScripts %s", pending_scripts_index);
					  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
					}
				      else
					{
					  sprintf(tcl_command,
						  "set script_windows(%s) [list %f [list 8004 [padMessage \"%s %d\"]]]",
						  pending_scripts_index, end_time,
						  break_type, break_inst);
					  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
					
					  sprintf(tcl_command, "lappend messageQueue %s", break_data);
					  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
					  sprintf(tcl_command, "unset pending_scripts(%s)",
						  pending_scripts_index);
					  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
					  sprintf(tcl_command, "set ComponentsBroken(%s) 1", break_type);
					  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
					  sprintf(tcl_command, "UpdateShowWindow");
					  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

					}
				    }
				}
			      else
				{
				  /* first time around */
			      			
				  sprintf(tcl_command, "lappend messageQueue 5556");
				  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				  sprintf(tcl_command,
					  "lappend messageQueue [padMessage %s]",
					  break_type);
				  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				
				  /* now modify the pending_script to leave a mark that
				     we have went through this once which will be changing
				     instance type to -2 */

				  sprintf(tcl_command, "set pending_scripts(%s) [list %f %f %s %d [list %s -2 [list %d {%s}]]]",
					  pending_scripts_index, start_time,
					  end_time, script_type, number,
					  break_type, break_inst, break_data);
				  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
				  /* set gotinstance to 0, so we will know when we
				     finally get the instances */
				  sprintf(tcl_command, "set received_instance(%s) 0",
					  break_type);
				  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

				}
			    }
			  tkshift_free(break_data);
			  tkshift_free(break_type);
			}
		      else if (strcmp(script_type, "Stop") == 0)
			{	
			  sprintf(tcl_command, "lappend messageQueue $Stop");
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			  sprintf(tcl_command, ".middle.main.continue configure -state normal");
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			  sprintf(tcl_command, ".middle.main.stop configure -state disabled");
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			  sprintf(tcl_command, "unset pending_scripts(%s)",
				  pending_scripts_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);

			  sprintf(tcl_command, "RemoveFromRunningScripts %s",
				  pending_scripts_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			
			}
		      else if (strcmp(script_type, "Trace") == 0)
			{	
			  /* not yet implemented */
			}
		      else
			{
			  /* something is wrong here */
			  printf ("pending script is not known type");
			  assert(0);
			}
		      tkshift_free(script_type);
		      tkshift_free(script_data);
		    }
		  tkshift_free(pending_scripts_index);
		}
	      tkshift_free(pending_scripts_list);
	    }
	
	  sprintf(tcl_command, "array exists script_windows");
	  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
	  if (strcmp (interp->result, "1") == 0)
	    {
	      sprintf(tcl_command, "array name script_windows");
	      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
	      script_windows_list = tkshift_safe_malloc(strlen(interp->result));
	      strcpy(script_windows_list, interp->result);

	      sprintf(tcl_command, "llength {%s}", script_windows_list);
	      assert(Tcl_Eval(interp,tcl_command) == TCL_OK);
	      script_windows_size = atoi(interp->result);
	
	      for (script_windows_count = 0;
		   script_windows_count < script_windows_size;
		   script_windows_count++)
		{		
		  sprintf(tcl_command, "lindex {%s} %d", script_windows_list,
			  script_windows_count);
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
		  script_windows_index
		    = tkshift_safe_malloc(strlen(interp->result));
		  strcpy(script_windows_index, interp->result);

		  /* we have to determine what type it is, since break has 	
		     to be treated differently now */
		  script_type_start = strchr(script_windows_index, ',');
		  script_type_start = (char *) ((int) script_type_start + 1);
		  script_type_end =
		    strchr(script_type_start, ',');
		  script_type =
		    tkshift_safe_malloc((int) script_type_end
					- (int) script_type_start);
		  strncat(script_type, script_type_start,
			  (int) script_type_end - (int) script_type_start);
		
		  sprintf(tcl_command, "lindex $script_windows(%s) 0",
			  script_windows_index);
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
		  end_time = atof(interp->result);
		
		  if (end_time < elapsed_time)
		    {
		      if (strcmp(script_type, "break") == 0)
			{
			  sprintf(tcl_command, "lindex $script_windows(%s) 1",
				  script_windows_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			  script_data = tkshift_safe_malloc(strlen(interp->result));
			  strcpy(script_data, interp->result);
			
			  sprintf(tcl_command, "lappend messageQueue %s", script_data);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			
			  sprintf(tcl_command, "unset script_windows(%s)",
				  script_windows_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			
			  sprintf(tcl_command, "RemoveFromRunningScripts %s",
				  script_windows_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			
			  tkshift_free(script_data);
			}
		      else
			{
			  sprintf(tcl_command, "lindex $script_windows(%s) 1",
				  script_windows_index);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			  w = tkshift_safe_malloc(strlen(interp->result));
			  strcpy(w, interp->result);
		
			  /* in case they have been destroyed already */
			  sprintf(tcl_command, "winfo exists %s", w);
			  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
		
			  if (strcmp(interp->result, "1") == 0)
			    {
			      sprintf(tcl_command, "DestroyScript %s",
				      script_windows_index);
			      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			    }
			  else
			    {
			      sprintf(tcl_command, "unset script_windows(%s)",
				      script_windows_index);
			      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			      sprintf(tcl_command, "RemoveFromRunningScripts %s",
				      script_windows_index);
			      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
			    }

			  tkshift_free(w);
			}
		    }
		  tkshift_free(script_windows_index);
		  tkshift_free(script_type);
		}
	      tkshift_free(script_windows_list);
	    }

	}
			
      else
	{
	  isStartUp = Tcl_GetVar(interp, "isStartUp", TCL_GLOBAL_ONLY);
	
	  if (strcmp(isStartUp, "1") == 0)
	    {
	      sprintf(tcl_command,
		      ".middle.main.continue configure -text \"Continue\" -state normal");	
	      Tcl_Eval(interp, tcl_command); 	

	      TkShiftValues =
		Tcl_GetVar2(interp, "TkShiftValues", "time", TCL_GLOBAL_ONLY);
	      sprintf(tcl_command,
		      ".middle.main.step configure -text \"Step %s\"",
		      TkShiftValues);
	      Tcl_Eval(interp, tcl_command);
	      sprintf(tcl_command,
		      ".middle.main.timestep configure -text \"Time %s\"",
		      TkShiftValues);
	      Tcl_Eval(interp, tcl_command);
	      startup = atoi(isStartUp);
	      startup = startup + 1;
	      sprintf(isStartUp, "%d", startup);
	      Tcl_SetVar(interp, "isStartUp", isStartUp, TCL_GLOBAL_ONLY);

	      Tcl_Eval(interp, ".middle.main.stop configure -state disabled");
	    }
	  else
	    {
	      startup = atoi(isStartUp);
	      startup = startup + 1;
	      sprintf(isStartUp, "%d", startup);
	      Tcl_SetVar(interp, "isStartUp", isStartUp, TCL_GLOBAL_ONLY);
	    }
	}
    }
  else if (datatype == 'A')
    {
      data = &data[3];
      sprintf(tcl_command, "ParseTypesData {%s}", data);
      Tcl_Eval(interp, tcl_command);
      Tcl_Eval(interp, "insert_types_list");
    }
  else if (datatype == 'R')
    {
      data = &data[3];
      data = strchr(data, '#');
      type = &data[1];
      data = strchr(type, '#');
      data[0] = '\0';
      data = &data[1];
      sprintf(tcl_command, "StoreAHSData {%s} {%s}", type, data);
      Tcl_Eval(interp, tcl_command);

    }
  else if (datatype == 'B')
    {
      data = &data[3];
      if (data[0] == '\0')
	{
	  return;
	}
      else
	{	
	  sprintf(tcl_command, "discrete_break_window .break BreakType {%s}", data);
	  Tcl_Eval(interp, tcl_command);
	  Tcl_GlobalEval(interp, "incr BreakNumber");
	  Tcl_Eval(interp, "update");
	  Tcl_Eval(interp, ".middle.main.continue configure -state normal");
	  Tcl_Eval(interp, ".middle.main.stop configure -state disabled");
	}
    }
  else if(datatype == 'Q')
    {
      data = &data[3];
      if (data[0] == '\0') 	
	{	
	  return;
	}
      else
	{
	  sprintf(tcl_command, "discrete_trace_window .break2 BreakType {%s}", data);
	  Tcl_Eval(interp, tcl_command);
	  Tcl_Eval(interp, "update idletasks");
	}
    }
  else if (datatype == 'X')
    {
      Tcl_Eval(interp, "trace vdelete messageQueue w Execute2");

      channel1 = Tcl_GetVar(interp, "channel1", TCL_GLOBAL_ONLY);
      sprintf(tcl_command, "close %s", channel1);
      Tcl_Eval(interp, tcl_command);

      channel2 = Tcl_GetVar(interp, "channel2", TCL_GLOBAL_ONLY);
      sprintf(tcl_command, "close %s", channel2);
      Tcl_Eval(interp, tcl_command);

      Tcl_Eval(interp, "destroy .");
      Tcl_Eval(interp, "exit");
    }
  else if (datatype == 'W')
    {
      canvas = data[4];
      group = data[6];
      data = &data[7];

      if (strlen(data) < 8)
	{
	  return TCL_OK;
	}

      sprintf(tcl_command, "UpdateItem %c %c {%s}", canvas, group, data);
      Tcl_Eval(interp, tcl_command);
    }
  else if (datatype == 'Y')
    {
      canvas = data[4];
      group = data[6];
      var_number = data[8];
      data = &data[9];

      if (strlen(data) < 8)
	{
	  return TCL_OK;
	}	

      sprintf(tcl_command, "UpdateItem %c %c {%s}", canvas, group, data);
      Tcl_Eval(interp, tcl_command);

    }	
  else if (datatype == 'H')
    {
      data = &data[3];
      sprintf(tcl_command, "ProcessHoustonInfo {%s}", data);
      Tcl_Eval(interp, tcl_command);
    }
  else if (datatype == 'S')
    {
      data = &data[3]; 	
      sprintf (tcl_command, "bell");
      assert(Tcl_Eval(interp, tcl_command) == TCL_OK);
      sprintf (tcl_command, "tk_dialog .info Warning \"There is possible zeno behavior in the program.\" warning 0 OK");
      Tcl_Eval(interp, tcl_command);
    }
  else
    {
      /* error */
      assert(0);
    }	

  tkshift_free(tcl_command);

  return TCL_OK;

}
	
#endif
