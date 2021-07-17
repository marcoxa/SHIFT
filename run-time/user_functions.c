/* -*- Mode: C -*- */

/* user_functions.c -- */

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


#ifndef user_functions_i
#define user_functions_i

#include <shift_config.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shift_debug.h"
#include "sim_info.h"
#include "shift_api_info.h"
#include "shift_api_types.h"
#include "shift_tools.h"
#include "run-time.h"

extern long tclick;
extern double h;


/* If you want to customize the output of s SHIFT simulation, this is
   where you insert your code. You can:
      -register functions that are going to be called at set intervals
      -set breakpoints
      -set traces
*/
      
void get_stuff();
int do_stuff();
int do_things();
void get_junk();
void get_trash();
void dump_vehicle_state();
void dump_controller_discrete_states();


int smartpath_step;
int click;
FILE *smartpath_file; 
extern sim_argument *simulation_args[];


void
register_user_functions()
{
  char* shift_dump_to_smartpath = 0;
  char* shift_smartpath_data = 0;
  char* shift_smartpath_step = 0;
  int dump_smartpath_data = 0;

  /*
    This example shows how one can output smartpath Data to a file 
   */
  if (simulation_args[DUMP_SMARTPATH_ARG]->value == 1)
    {
      dump_smartpath_data = 1;

      click = 0;
      smartpath_step = 1;
      smartpath_file = fopen(simulation_args[DUMP_SMARTPATH_ARG]->string_arg, "w");
      if (smartpath_file == 0)
	{
	  runtime_error("unable able to open smartpath data file.\n");
	}
      registerCallback(dump_vehicle_state, smartpath_step, 0, 0);
      registerCallback(dump_controller_discrete_states, 1, 1, 1);
    }
}


void
register_user_functions_old()
{
  char* shift_dump_to_smartpath = 0;
  char* shift_smartpath_data = 0;
  char* shift_smartpath_step = 0;
  char* shift_gui_debug = 0;
  int dump_smartpath_data = 0;

  /*
    This example shows how one can output smartpath Data to a file 
   */
  shift_dump_to_smartpath = getenv("SHIFT_DUMP_SMARTPATH");
  if (shift_dump_to_smartpath != 0 && atoi(shift_dump_to_smartpath) != 0)
    {
      shift_gui_debug = getenv("SHIFT_GUI_DEBUG");
      if (shift_gui_debug == 0) {
	shift_api_initialize_user_types();
	shift_api_initialize_global_variables();
      }
      dump_smartpath_data = 1;
      shift_smartpath_data = getenv("SHIFT_SMARTPATH_DATA");
      shift_smartpath_step = getenv("SHIFT_SMARTPATH_STEP");

      if (shift_smartpath_data == 0)
	{
	  shift_smartpath_data = (char*) safe_malloc(80 * sizeof(char));
	  strcpy(shift_smartpath_data, "smartpath.data");
	}

      if (shift_smartpath_step == 0)
	smartpath_step = 5;
      else
	smartpath_step = atoi(shift_smartpath_step);

      click = 0;
      smartpath_file = fopen(shift_smartpath_data, "w");
      if (smartpath_file == 0)
	{
	  runtime_error("unable able to open smartpath data file.\n");
	}
      registerCallback(dump_vehicle_state, smartpath_step, 0, 0);
      registerCallback(dump_controller_discrete_states, 1, 0, 1);
    }

  /*
    This is example code that shows how to break a component.
    The do_stuff() and do_things() functions appear below  */

  /*  
      break_component("Ball", 0, do_stuff);
      
      break_component_at_event("Ball", 1, "raiseSpeed",do_things); */

  /*   This code shows how to register functions at the beginning of 
       the simulation run so that they get called periodically */
  /*

    registerCallback(get_stuff, 15, 0, 0);
    registerCallback(get_junk, 15, 1, 50);
    registerCallback(get_trash, 1, 0, 100);
    */
}




void 
dump_controller_discrete_states()
{
  shift_api_user_type *controller_type;
  shift_api_mode **all_modes;
  FILE * string_file;
  int index;

  controller_type = shift_api_find_user_type("merge_controller");   
  if (controller_type != NULL) {
    all_modes=shift_api_find_modes(controller_type);

    string_file = fopen("controller_strings", "w");
    fprintf(string_file, "NULL\n");

    index = 0;
    while (all_modes[index] != NULL) {
      fprintf(string_file, "%s\n", all_modes[index]->name);
      index++;
    }
    fclose(string_file);
  } else {
    fprintf(stderr, "Warning: There is no merge_controller in this ");
    fprintf(stderr, "simulation for the smartAHS output.\n");
  }
}


void 
trace_some_data_from_some_component()
{
  /* This registers the function example_outputData() to be called every 15
     timeclicks during the while simulation starting at timeclick 0 */

  /*
    registerCallback(example_outputData, 15, 0, 100);
    */
}


void 
example_outputData()
{
  /*
  Component* c;
  char buffer[5000];
  
  c = shift_tools_find_component("SomeType", SomeComponentNumber);
  sprintf(buffer, "%s\n", get_component_values(c, "speed"));
  sprintf(buffer, "%s\n", get_component_values(c, "accelleration"));
  other_buffer = get_component_values(c, "emergency_state");
  if (atoi(other_buffer) == 1)
    {
      course = get_component_values(c, "collision_course");
      call_emergency_loop(course);
    }
    */
}



void
dump_vehicle_state()
{
  char buffer[5000];
  int current_VREP;
  double vgam_value12, vgam_value13, vgam_value31, vgam_value11;
  double vgam_value33, vgam_value32;
  double pitch, roll, heading, float_value;
  int segment_number, automated_vehicle_number, vehicle_number, index;
  char temp_buffer[50];
  shift_api_user_type *type, *auto_type, *vehicle_type, *controller_type;
  shift_api_instance *controller_instance, *instance, 
    *automated_vehicle_instance, *vehicle_instance;
  shift_api_value *value, *controller_value;
  shift_api_mode ** all_modes, *mode;
  shift_api_user_type* vrep_type = shift_api_find_user_type("VREP");
  shift_api_instance ** instances;
  int modes_gotten = 0;
  int number_of_vehicles;
  int loop;

  buffer[0] = '\0';

  /* If there is no VREP type around, return instead of seg fault */
  if (vrep_type == NULL) 
    return; 

  number_of_vehicles = shift_api_count_instances(vrep_type);
  /* Parse the string, and for each item in the string do: */  
  if (number_of_vehicles <= 0)
    {
      /*fprintf(smartpath_file, "There is no information at this time\n");*/
      /*printf("There is no information at this time\n");*/
      return;
    }
  else
    {
      auto_type = shift_api_find_user_type("M_Vehicle");
      controller_type = shift_api_find_user_type("merge_controller");

      /*vehicle_type = shift_api_find_user_type("Vehicle");     */
      vehicle_type = shift_api_find_user_type("k_vehicle_dynamics");   
      
      type = shift_api_find_user_type("VREP");      
      number_of_vehicles = shift_api_count_instances(vrep_type);
      
      instances = shift_api_find_instances(type);
      loop = 0;
      while (instances[loop]) {

	/*(for (current_VREP = 0 ;
	  current_VREP < number_of_vehicles ;
	  current_VREP++)
	*/
	  
	/*instance = shift_api_find_instance(type,
	  current_VREP);*/
	instance = instances[loop];
	      
	/* Get information about the vehicle's AutomatedVehicle
	   and its controller. These are used to get information
	   that we need to print */

	value =shift_api_find_local_value("vehicle", instance);
	automated_vehicle_number = value->v.instance_v->id;

	/* Find automated vehicle instance */

	automated_vehicle_instance =
	  shift_api_find_instance(auto_type,automated_vehicle_number);

	controller_instance =
	  shift_api_find_instance(controller_type,automated_vehicle_number);

	
	/* Find vehicle instance */
	value=shift_api_find_local_value("the_platoon", controller_instance);
	vehicle_number = value->v.instance_v->id;

	vehicle_instance =
	  shift_api_find_instance(vehicle_type,
				  vehicle_number);


	controller_value = shift_api_find_local_value("m_controller", 
						      automated_vehicle_instance);
	      

	/* 
	   Here we start printing the line of information about
	   a vehicle. The order is:
	   Time CarID PlatoonID Mode X Y Z h p r SegmentID Speed
	   Accel TireAngle CoordMsgID RegMsgID LinkMsgID
	   PartnerID SensorFront SensorLeft SensorRight 
	*/



	/* Simulation time */
	sprintf(buffer, "%f ", tclick*h);

	/* CarID: Use current_VREP, platoonID is the same thing */
	temp_buffer[0] = '\0';
	sprintf(temp_buffer, "%d %d ",
		instance->id,
		instance->id);
	strcat(buffer, temp_buffer);
	temp_buffer[0] = '\0';
	      
	/* mode: Always automatic, print 0*/
	value =shift_api_find_local_value("in_platoon",
					  controller_value->v.instance_v);
	if (value == NULL) {
	  strcat(buffer, " 0 ");
	} else {
	  GUI_print_variable_value(value, buffer);
	  strcat(buffer, " ");
	}
	
	/* global x position */

	value =shift_api_find_local_value("gxp", instance);
	/*buffer = shift_tools_print_value(value, buffer);*/
	GUI_print_variable_value(value, buffer);
	strcat(buffer, " ");
	/*strcat(buffer, get_variable_values("VREP", current_VREP,
	  "gxp", 0));*/
	
	/* global y position. We have to negate the value right now */
	value =shift_api_find_local_value("gyp", instance);
	/*temp_buffer = shift_tools_print_value(value, temp_buffer);*/
	GUI_print_variable_value(value, temp_buffer);
	float_value=atof(temp_buffer)*-1;
	sprintf(temp_buffer, "%f", float_value);
	strcat(buffer, temp_buffer);

	/* strcat(buffer, get_variable_values("VREP", current_VREP,
	   "gyp", 0));
	*/
	strcat(buffer, " ");

	/* global z position.  */
	value =shift_api_find_local_value("gzp", instance);
	/*buffer = shift_tools_print_value(value, buffer);*/
	GUI_print_variable_value(value, buffer);
	/*
	  strcat(buffer, get_variable_values("VREP", current_VREP,
	  "gzp", 0));
	*/
	strcat(buffer, " ");

	/* heading */
	value =shift_api_find_local_value("vgam12", instance);
	vgam_value12 = value->v.value_v->v.number_v;
	value =shift_api_find_local_value("vgam11", instance);
	vgam_value11 = value->v.value_v->v.number_v;
	heading = -1 * atan(vgam_value12/vgam_value11);
	sprintf(temp_buffer, "%f", heading);
	strcat(buffer, temp_buffer);
	strcat(buffer, " ");
	/* Pitch */

	value =shift_api_find_local_value("vgam31", instance);
	vgam_value31 = value->v.value_v->v.number_v;
	value =shift_api_find_local_value("vgam33", instance);
	vgam_value33 = value->v.value_v->v.number_v;
	pitch = -1 * atan(vgam_value31/vgam_value33);
	sprintf(temp_buffer, "%f", pitch);
	strcat(buffer, temp_buffer);
	strcat(buffer, " ");
	/* Roll */

	value =shift_api_find_local_value("vgam32", instance);
	vgam_value32 = value->v.value_v->v.number_v;
	roll = -1 * atan(vgam_value32/vgam_value33);
	sprintf(temp_buffer, "%f", roll);
	strcat(buffer, temp_buffer);
	strcat(buffer, " ");

	/* segment */
	temp_buffer[0]='\0';
	value =shift_api_find_local_value("segment", instance);
	segment_number = value->v.instance_v->id;
	sprintf(temp_buffer, " 0 ");
	/*sprintf(temp_buffer, "%d ", segment_number);*/
	strcat(buffer, temp_buffer);

	/* speed */
	value =shift_api_find_local_value("xDot", instance);
	GUI_print_variable_value(value, buffer);
	strcat(buffer, " ");

	/* accelleration */
	temp_buffer[0]='\0';


	/* Step 1: get data from Vehicle */


	value =shift_api_find_local_value("xDDot", vehicle_instance);
	GUI_print_variable_value(value, buffer);
	strcat(buffer, " ");

	/* Steering */
	/*value =shift_api_find_local_value("steering", vehicle_instance);
	  GUI_print_variable_value(value, buffer);*/
	strcat(buffer, " 0.0  ");

	/* Nothing for the rest of these */
	      
	/* Retrieves a particular mode of a user type */
	if (modes_gotten == 0) {
	  controller_value = shift_api_find_local_value("m_controller", 
							automated_vehicle_instance);
	  all_modes=shift_api_find_modes(controller_value->v.instance_v->type);
	  modes_gotten = 1;
	}

	      
	/*coordMessageID: nothing */
	mode = shift_api_find_current_mode(controller_value->v.instance_v);
	index = 0;
	while (mode->name != all_modes[index]->name)
	  index++;
	sprintf(temp_buffer, " %d ", index+1);
	strcat(buffer, temp_buffer);
	/*regMessageID: nothing */
	strcat(buffer, "0 ");
	/*linkMessageID: nothing */
	strcat(buffer, "0 ");
	/*partnerId: nothing */
	strcat(buffer, "0 ");
	/*sensorFront: nothing */
	strcat(buffer, "0 ");
	/*sensorLeft: nothing */
	strcat(buffer, "0 ");
	/*sensorRight: nothing */
	strcat(buffer, "0 ");


	/* BEWARE: Added Oct 15, DW. Carl needs the back_gap. This
	   introduces incompatibility with the smartPATH format.
	*/
	value =shift_api_find_local_value("back_gap",
					  controller_value->v.instance_v);
	if (value == NULL) {
	  strcat(buffer, " 0 ");
	} else {
	  GUI_print_variable_value(value, buffer);
	  strcat(buffer, " ");
	}

	/* BEWARE: Added Oct 15, DW. Carl needs the back_gap. This
	   introduces incompatibility with the smartPATH format.
	*/
	value =shift_api_find_local_value("same_lane_gap",
					  controller_value->v.instance_v);
	if (value == NULL) {
	  strcat(buffer, " 0 ");
	} else {
	  GUI_print_variable_value(value, buffer);
	  strcat(buffer, " ");
	}


	fprintf(smartpath_file, "%s\n", buffer);
	fflush(smartpath_file);
	buffer[0]='\0';
	loop++;
      }
      
    }
}

/* 
 * dump_vehicle_state_old -- This now works only for the 'merge protocol'
 * simulation.
 */


void 
dump_vehicle_state_old()
{
  char buffer[4096];		/* 4Kb */
  int current_VREP;
  char string_one[20], string_two[20];
  int segment;
  double vgam_value1, vgam_value2;
  char temp_buffer[50];
  shift_api_user_type* vrep_type;
  int number_of_vehicles;
  shift_api_instance** vrep_instances;

  buffer[0] = '\0';
  vrep_type = shift_api_find_user_type("VREP");
  number_of_vehicles = shift_api_count_instances(vrep_type);
  vrep_instances = shift_api_find_instances(vrep_type);
  /* Parse the string, and for each item in the string do: */  
  if (number_of_vehicles < 0)
    {
      fprintf(smartpath_file, "There is no information at this time\n");
      printf("There is no information at this time\n");
      return;
    }
  else
    {
      int iter = 0;

      while (vrep_instances[iter])
	{
	  /* Current vrep number */
	  current_VREP = vrep_instances[iter]->id;
	  iter++;

	  /* Simulation time */
	  sprintf(buffer, "%.3f ", tclick*h);

	  /* CarID: Use current_VREP, platoonID is the same thing */
	  temp_buffer[0] = '\0';
	  sprintf(temp_buffer, "%d %d ", (current_VREP+1), (current_VREP+1));
	  strcat(buffer, temp_buffer);
	  temp_buffer[0] = '\0';
      
	  /* mode: Always automatic, print 0*/
	  strcat(buffer, "0 ");

	  /* global x position */
	  strcat(buffer, get_variable_values("VREP", current_VREP,
					     "gxp", 0));
	  strcat(buffer, " -");

	  /* global y position. We have to negate the value right now */
	  strcat(buffer, get_variable_values("VREP", current_VREP,
					     "gyp", 0));
	  strcat(buffer, " ");

	  /* global z position.  */
	  strcat(buffer, get_variable_values("VREP", current_VREP,
					     "gzp", 0));
	  strcat(buffer, " ");

	  /* heading */
	  sprintf(temp_buffer, "%s ",
		  get_variable_values("VREP", current_VREP,
				      "vgam12",0));
	  sscanf(temp_buffer, "%lf", &vgam_value1);

	  sprintf(temp_buffer, "%s ",
		  get_variable_values("VREP", current_VREP,
				      "vgam11",0));

	  sscanf(temp_buffer, "%lf", &vgam_value2);
	  sprintf(temp_buffer, "-%.3f ", atan(vgam_value1/vgam_value2));
	  strcat(buffer, temp_buffer);

	  /* Pitch */
	  sprintf(temp_buffer, "%s ",
		  get_variable_values("VREP", current_VREP,
				      "vgam31",0));
	  sscanf(temp_buffer, "%lf", &vgam_value1);
	  sprintf(temp_buffer, "%s ",
		  get_variable_values("VREP", current_VREP,
				      "vgam33",0));

	  sscanf(temp_buffer, "%lf", &vgam_value2);
	  sprintf(temp_buffer, "-%.3f ", atan(vgam_value1/vgam_value2));
	  strcat(buffer, temp_buffer);

	  /* Roll */
	  sprintf(temp_buffer, "%s ",
		  get_variable_values("VREP", current_VREP,
				      "vgam32", 0));
	  sscanf(temp_buffer, "%lf", &vgam_value1);
	  sprintf(temp_buffer, "-%.3f ", atan(vgam_value1/vgam_value2));
	  strcat(buffer, temp_buffer);
      
	  strcat(buffer, " ");

	  /* segment */

	  temp_buffer[0] = '\0';
	  sprintf(temp_buffer, "%s",
		  get_variable_values("VREP", current_VREP,
				      "segment", 0));
	  sscanf(temp_buffer, "(%s %d @%s)", string_one, &segment,
		 string_two);
	  temp_buffer[0] = '\0';
	  segment = 0;		/* Set to 0 on Delnaz's request to
				 * make the SmartPath animator happy.
				 *
				 * Marco Antoniotti 19970316
				 */
	  sprintf(temp_buffer, "%d ", segment);

	  strcat(buffer, temp_buffer);

	  /* speed */
	  strcat(buffer, get_variable_values("VREP", current_VREP,
					     "xDot", 0));
	  strcat(buffer, " ");

	  /* accelleration */
      
	  temp_buffer[0] = '\0';
	  sprintf(temp_buffer, "%s",
		  get_variable_values("VREP", current_VREP,
				      "vehicle", 0));
	  sscanf(temp_buffer, "(%s %d @%s)", string_one, &segment,
		 string_two);
	  sprintf(temp_buffer, "%s",
		  get_variable_values("AutomatedVehicle", segment,
				      "the_platoon", 0));
	  sscanf(temp_buffer, "(%s %d @%s)", string_one, &segment,
		 string_two);

	  /* Step 1: get data from Vehicle */
	  /* The following code works only for the detailed vehicle
	   * model.
	   * In the merge simulation we do not have the same
	   * parameters.
	   * Marco Antoniotti 19970319

	   temp_buffer[0] = '\0';
	   sprintf(temp_buffer, "%s", get_variable_values("Vehicle", segment,
	   "xDDot", 0));
      
	   strcat(buffer, temp_buffer);
	   strcat(buffer, " ");
	   / * Steering * /

	   temp_buffer[0] = '\0';
	   sprintf(temp_buffer, "%s", get_variable_values("Vehicle", segment,
	   "steering", 0));
      
	   strcat(buffer, temp_buffer);
	   strcat(buffer, " ");
	  */
	  temp_buffer[0] = '\0';
	  sprintf(temp_buffer, "%s", get_variable_values("platoon", segment,
							 "xDDot", 0));
      
	  strcat(buffer, temp_buffer);
	  strcat(buffer, " ");
	  /* Steering */

	  temp_buffer[0] = '\0';
	  sprintf(temp_buffer, "%s", "0.0"); /* platoon does not have
					      * "steering".
					      */
      
	  strcat(buffer, temp_buffer);
	  strcat(buffer, " ");

	  /* Nothing for the rest of these */
	  /*coordMessageID: nothing */
	  strcat(buffer, "0 ");
	  /*regMessageID: nothing */
	  strcat(buffer, "0 ");
	  /*linkMessageID: nothing */
	  strcat(buffer, "0 ");
	  /*partnerId: nothing */
	  strcat(buffer, "0 ");
	  /*sensorFront: nothing */
	  strcat(buffer, "0 ");
	  /*sensorLeft: nothing */
	  strcat(buffer, "0 ");
	  /*sensorRight: nothing */
	  strcat(buffer, "0 ");

	  /* printf("-%s-\n", buffer); */
	  fprintf(smartpath_file, "%s\n", buffer);
	  fflush(smartpath_file);
	  /* printf("%s\n", buffer); */
	  buffer[0] = '\0';
	}
    }
}


void 
get_stuff()
{
  char buffer[5000];

  buffer[0] = '\0';
  sprintf(buffer, "Ball information: x: ", tclick*h);
  strcat(buffer, get_variable_values("Ball", 0, "x", 0));
  printf("%s\n", buffer);
  buffer[0] = '\0';
}


int 
do_stuff(Component* c)
{
  char buffer[5000];

  buffer[0] = '\0';
  sprintf(buffer, "Ball information: x: ");
  /*
    strcat(buffer, get_variable_values("Ball", 0, "x", 0));
    */
  printf("Component Ball was broken.\n");
  buffer[0] = '\0';
  return 1;
}


int
do_things()
{
  printf("\nComponent Ball1 was broken at time %.3f.\n", tclick*h);
  return 1;
}


void 
get_junk()
{
  char buffer[5000];

  buffer[0] = '\0';
  sprintf(buffer, "Once in a lifetime ball information: y: ");
  strcat(buffer, get_variable_values("Ball", 0, "y", 0));
  printf("%s\n", buffer);
  buffer[0] = '\0';
}


void 
get_trash()
{
  char buffer[5000];
  buffer[0]='\0';

  buffer[0] = '\0';
  sprintf(buffer, "frequent ball information: speed = ");
  strcat(buffer, get_variable_values("Ball", 0, "speed", 0));
  printf("%s\n", buffer);
  buffer[0] = '\0';
}


#endif /* user_functions_i */

/* end of file -- user_functions.c -- */
