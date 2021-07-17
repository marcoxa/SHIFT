
/* -*- Mode: C -*- */

/* dump_data.c -- */

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

#include <ctype.h>
#include <math.h>

#include "shift_api_types.h"
#include "shift_debug.h"
#include "shift_api_info.h"
#include "sim_info.h"


extern long tclick;
extern double h;


void
dump_vehicle_state_other_old(FILE* dump_file)
{
  char buffer[5000];
  int current_VREP;
  double vgam_value12, vgam_value13, vgam_value31, vgam_value11;
  double vgam_value33, vgam_value32;
  double pitch, roll, heading, float_value;
  int segment_number, automated_vehicle_number, vehicle_number, index;
  char temp_buffer[50];
  shift_api_user_type *type, *auto_type, *vehicle_type;
  shift_api_instance *instance, *automated_vehicle_instance, *vehicle_instance;
  shift_api_value *value, *controller_value;
  shift_api_mode ** all_modes, *mode;
  shift_api_user_type* vrep_type = shift_api_find_user_type("VREP");
  int modes_gotten = 0;
  int number_of_vehicles = shift_api_count_instances(vrep_type);
  buffer[0] = '\0';
  
  /* Parse the string, and for each item in the string do: */  
  if (number_of_vehicles <= 0)
    {
      fprintf(dump_file, "There is no information at this time\n");
      printf("There is no information at this time\n");
      return;
    }
  else
    {
      auto_type = shift_api_find_user_type("AutomatedVehicle");
      vehicle_type = shift_api_find_user_type("Vehicle");      
      type = shift_api_find_user_type("VREP");      


      for (current_VREP = 0 ;
	   current_VREP < number_of_vehicles ;
	   current_VREP++)
	{

	  instance = shift_api_find_instance(type,
					     current_VREP);
	  if (instance != NULL)		/* Test if VREP is in good state.  */
	    {

	      /* Simulation time */
	      sprintf(buffer, "%f ", tclick*h);

	      /* CarID: Use current_VREP, platoonID is the same thing */
	      temp_buffer[0] = '\0';
	      sprintf(temp_buffer, "%d %d ",
		      (current_VREP+1),
		      (current_VREP+1));
	      strcat(buffer, temp_buffer);
	      temp_buffer[0] = '\0';

	      /* mode: Always automatic, print 0*/
	      strcat(buffer, "0 ");

	      /* global x position */

	      value =shift_api_find_local_value("gxp", instance);
	      GUI_print_variable_value(value, buffer);
	      
	      /*strcat(buffer, get_variable_values("VREP", current_VREP,
						 "gxp", 0));*/

	      /* global y position. We have to negate the value right now */
	      value =shift_api_find_local_value("gyp", instance);
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

	      /* Pitch */

	      value =shift_api_find_local_value("vgam31", instance);
	      vgam_value31 = value->v.value_v->v.number_v;
	      value =shift_api_find_local_value("vgam33", instance);
	      vgam_value33 = value->v.value_v->v.number_v;
	      pitch = -1 * atan(vgam_value31/vgam_value33);
	      sprintf(temp_buffer, "%f", pitch);
	      strcat(buffer, temp_buffer);

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
	      sprintf(temp_buffer, "%d ", segment_number);
	      strcat(buffer, temp_buffer);

	      /* speed */
	      value =shift_api_find_local_value("xDot", instance);
	      GUI_print_variable_value(value, buffer);
	      strcat(buffer, " ");

	      /* accelleration */
	      temp_buffer[0]='\0';
	      value =shift_api_find_local_value("vehicle", instance);
	      automated_vehicle_number = value->v.instance_v->id;
	      sprintf(temp_buffer, "%d ", vehicle_number);
	      /* Find automated vehicle instance */

	      automated_vehicle_instance =
		shift_api_find_instance(auto_type,automated_vehicle_number);
	      /* Find vehicle instance */
	      value=shift_api_find_local_value("vehicle", automated_vehicle_instance);
	      vehicle_number = value->v.instance_v->id;

	      vehicle_instance =
		shift_api_find_instance(vehicle_type,
					vehicle_number);


	      /* Step 1: get data from Vehicle */


	      value =shift_api_find_local_value("xDDot", vehicle_instance);
	      GUI_print_variable_value(value, buffer);
	      strcat(buffer, " ");

	      /* Steering */
	      value =shift_api_find_local_value("steering", vehicle_instance);
	      GUI_print_variable_value(value, buffer);
	      strcat(buffer, " ");

	      /* Nothing for the rest of these */
	      
	      /* Retrieves a particular mode of a user type */
	      if (modes_gotten == 0) {
		controller_value = shift_api_find_local_value("controller", 
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

	      printf("-%s-\n", buffer);
	      fprintf(dump_file, "%s\n", buffer);
	      buffer[0]='\0';
	    }
	}
    }
}

/* end of file -- dump_data.c -- */
