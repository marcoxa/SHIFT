/* -*- Mode: C -*- */

/* Tk_communications.c -- */

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

#ifndef TK_COMMUNICATIONS_I
#define TK_COMMUNICATIONS_I

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef OS_SOLARIS
#include <sys/filio.h>
#endif

#include <netdb.h>
#include <netinet/in.h>

#include <shift_debug.h>
#include <names.h>
#include <shift_api_info.h>
#include <shift_api_types.h>
#include <shift_api_internal.h> 
#include <tkShift_data.h>  
#include "sim_info.h"

 
extern int serverSock1, serverSock2;
extern char testing[30];
extern Canvas_Data * Canvas_Data_List;
extern double CurrentTime;

void
send_instance_data(name_list * list)
{
  static char buffer[BIG_BUFFER_SIZE];
  static char info[BIG_BUFFER_SIZE];
  char temp_buffer[10];


  while (list != NULL)
    {
      insert_code(buffer,'Z');	

      strcat(buffer, "#");
      strcat(buffer, list->name);
      sprintf(temp_buffer, "#%d", list->ID);
      strcat(buffer, temp_buffer);
      strcat(buffer, "#");
    
      if (!strcmp(list->name, "global")) {
	info[0] = '\0';
	GUI_printGlobal(info);
	strcat(buffer, info);
      }

      else
	GUI_get_component_values(list->name,list->ID, buffer);
    
      make_smart_packet(buffer);
      list = list->next;
    }
}

/* This function sends the current timeclick and the CurrentTime
 * everytime it is called.
 */  

void
send_timeclick(long time)
{

  char buffer[50];
  char extra[50];
 
  insert_code(buffer, 'E');
  extra[0] = '\0';
  sprintf(extra, "%d %f", time, CurrentTime);
  strcat(buffer, extra);

  make_smart_packet(buffer);
}

void
send_zeno_warning()
{

  char buffer[20];
 
  insert_code(buffer, 'S');
  make_smart_packet(buffer);
}


/* This function sends all the information requested by the TkShift
 *   graphs. It looks at the list of graph-data requests and
 *   concatenates all of the requested info and sends off the string
 */

void
send_graph_data(name_list * list)
{

  static char buffer[BUFFER_SIZE];
  char temp_buffer[50];


  insert_code(buffer,'C');
   
  if (strlen(testing) > 0) 
    strcat(buffer, testing);
  testing[0] = '\0';

  strcat(buffer," { ");
  while (list != NULL)
    {
      sprintf(temp_buffer, " { %d  ", list->ID);
      strcat(buffer, temp_buffer);
      if (!strcmp(list->typeName, "global")) {
	GUI_get_global_variable_value(list->typeName, 
			       list->componentNo,
			       list->variableName,
			       buffer);
      } else {
	GUI_get_variable_value(list->typeName, 
			       list->componentNo,
			       list->variableName,
			       buffer);
      }
      strcat(buffer," } ");
      list = list->next;
    }
  /*  printf("Gonna send %s\n", buffer);*/
  /*printf("Thiat ");*/
  strcat(buffer," } ");
  make_smart_packet(buffer);
}


/* This function here will have to be redone, pending the creation of 
   the shift_api_write.h document 
   */

void
send_transition_data(char *Transition_Data)
{

  static char buffer[BIG_BUFFER_SIZE];
  char * buffer_I;

  buffer_I = buffer;

  insert_code(buffer, 'B');
  strcat(buffer, Transition_Data);
  make_smart_packet(buffer);
}

void
send_trace_data(char *Trace_Data)
{

  static char buffer[BIG_BUFFER_SIZE];
  char * buffer_I;

  buffer_I = buffer;

  insert_code(buffer, 'Q');
  strcat(buffer, Trace_Data);
  make_smart_packet(buffer);
}


void 
send_close_info()
{
  char buffer[15];

  insert_code(buffer, 'X');
   
  make_smart_packet(buffer);
}


/* 
 *  This function sends the canvas data requested by TkShift. 
 *
 */


void
GUI_Send_All_Position_Info(Canvas_Data * canvas_data_List)
{

  Canvas_Data * data;
  static char buffer[20000];


  data = canvas_data_List; 
  buffer[0]='\0';

  while (data != NULL)
    {
      if (!strcmp(data->number, "ALL"))
	find_multiple_instance_information(data, buffer);
      else 
 	find_single_instance_information(data, buffer);

      make_smart_packet(buffer);
      data=data->next->next;
    }
  /*printf("C buffer is %s\n", buffer);*/

}

/*
 *
 * 
 */ 


char *
find_multiple_instance_information(Canvas_Data * data, char * buffer)
{
  shift_api_user_type * type;
  shift_api_instance ** instances;
  shift_api_instance * instance;
  shift_api_value * value;

  if (strcmp(data->container_type, "Null"))
    {
      if (!(strcmp(data->container_type,"global"))) {
	value = shift_api_find_global_value(data->container_variable);
      } 
	else 
	{
	  type = shift_api_find_user_type(data->container_type);
	  instance = shift_api_find_instance(type, 
					   data->container_instance);
	  
	  value = shift_api_find_local_value(data->container_variable, 
					 instance);
	}
      make_header_information(data, buffer);

	/* Should maybe add a check to make sure it is a set of
	   the instances that are being requested. I now assume this */
	if (value->v.value_v->kind == SET_B) {
	  if (value->v.value_v->v.set_v != NULL) {
	    instances = value->v.value_v->v.set_v->e.instances_e;
	    get_multiple_instance_animation_info(instances,data, buffer);
	  }
	}
	else 
	  if (value->v.value_v->v.array_v != NULL) {
	    instances =  value->v.value_v->v.array_v->e.instances_e;
	    get_multiple_instance_animation_info(instances,data, buffer);
	  }
    
	
      if (data->next->frequency == 1)
	data->next->frequency = 0;
    }
  else
    {
      type = shift_api_find_user_type(data->type_name);
      instances = shift_api_find_instances(type);
      make_header_information(data, buffer);
      get_multiple_instance_animation_info(instances,
					   data,
					   buffer);
      if (data->frequency == 1)
	data->frequency=0;
    }
  return buffer;
}



/* 
 *   This function gets the information of as single instance to
 *   animate, and return it to GUI_Send_All_Position_Info
 */


char *
find_single_instance_information(Canvas_Data * data, char * buffer)
{
  shift_api_user_type * type;
  shift_api_value * value;
  shift_api_instance ** instances;
  shift_api_instance * instance;


  if (!strcmp(data->container_type, "Null")) 
    {
      type = shift_api_find_user_type(data->type_name);
      instance = shift_api_find_instance(type, atoi(data->number));
      if (instance != NULL) {
	make_header_information(data, buffer);
	get_single_instance_animation_information(instance,
						  data,
						  buffer);
      }
    }
  else
    {
      make_header_information(data, buffer);
      get_single_instance_set_animation_info(data, buffer);
    }

  return buffer;
}



int
get_single_instance_set_animation_info(Canvas_Data * data, char * buffer)
{

  shift_api_user_type * type;
  shift_api_instance * instance;
  shift_api_instance ** instances;
  shift_api_value * value;
  int loop;

  loop=0;
  /* this is case 5, when we are sending a single
     thing iif it is in a set and is in any state */

  type = shift_api_find_user_type(data->container_type);
  instance=shift_api_find_instance(type, 
				   data->container_instance);
  value=shift_api_find_local_value(data->container_variable, 
				  instance);
  if (value->v.value_v->kind== SET_B)
    instances = value->v.value_v->v.set_v->e.instances_e;
  else 
    instances = value->v.value_v->v.array_v->e.instances_e;


  while(instances[loop] != NULL) {
    if (instances[loop]->id == atoi(data->number))
      get_single_instance_animation_information(instances[loop],
						data,
						buffer);
    loop++;
    
  }
  return 1;
}

/* 
 * This function 
 *
 *
 */

int
get_multiple_instance_animation_info(shift_api_instance ** instances, 
				     Canvas_Data * data, 
				     char * buffer)
{
  int loop;
  shift_api_mode * curr_state;
  loop = 0;

  strcat(buffer, " { ");
  while (instances[loop] != NULL) {
    if (!strcmp(data->desired_state, "All-States")) {
      if ((shift_api_find_instance(instances[loop]->type,
				   instances[loop]->id)) != NULL)
	GUI_print_animation_information(instances[loop], 
					data->type_name,
					data->variable_name,
					data->orientation,
					data->next->variable_name,
					data->next->length, 
					data->next->width,
					buffer);
    }
    else
      {
	curr_state=shift_api_find_current_mode(instances[loop]);
	if (!strcmp(data->desired_state, curr_state->name))
	  if ((shift_api_find_instance(instances[loop]->type,
				       instances[loop]->id)) != NULL)
	    GUI_print_animation_information(instances[loop],
					    data->type_name,
					    data->variable_name,
					    data->orientation,
					    data->next->variable_name,
					    data->next->length, 
					    data->next->width,
					    buffer);
      }
  loop++;    
  }
  strcat(buffer, " } ");
  return 1;
}

/* 
 *   This function looks at the state argument that is passed to
 * it. If the state is "AllStates", then it calls the
 * print_animation_information function, otherwise it only calls the
 * function if the state argument and the current state (mode) of the
 * instance are the same
 */


int
get_single_instance_animation_information(shift_api_instance * instance, 
					  Canvas_Data * data,
					  char * buffer)
{

  shift_api_mode * curr_state;

  if (!strcmp(data->desired_state, "All-States"))
    GUI_print_animation_information(instance, 
				    data->type_name,
				    data->variable_name,
				    data->orientation,
				    data->next->variable_name,
				    data->next->length, 
				    data->next->width,
				    buffer);
  
  else
    {
      curr_state=shift_api_find_current_mode(instance);
      if (!strcmp(data->desired_state, curr_state->name))
	GUI_print_animation_information(instance, 
					data->type_name,
					data->variable_name,
					data->orientation,
					data->next->variable_name,
					data->next->length, 
					data->next->width,
					buffer);
    }
  return 1;
}


/* 
 * This function prints the information used by TkShift into the
 * buffer. It either prints just the variableName, or also the length
 * and the width fields if these are not null (NONE)
 */


int
GUI_print_animation_information(shift_api_instance * instance, 
				char * typeName,
				char * XvariableName,
				char * orientation,
				char * YvariableName,
				char * length,
				char * width,
				char * buffer)
{
  
  shift_api_value * value;
  char temp[30];

  strcat(buffer, " { ");
  strcat(buffer, " {");
  /*
  strcat(buffer, XvariableName);
  strcat(buffer, " ");
  */
  value=shift_api_find_local_value(XvariableName, instance);
  GUI_print_variable_value(value, buffer);
  strcat(buffer, "} ");
  
  strcat(buffer, " {");
  /*
  strcat(buffer, YvariableName);
  strcat(buffer, " ");
  */
  value=shift_api_find_local_value(YvariableName, instance);
  GUI_print_variable_value(value, buffer);
  strcat(buffer, "} ");

  if (strcmp(orientation, "Nil")) {
      value=shift_api_find_local_value(orientation, instance);
      strcat(buffer, " {");
      /*
      strcat(buffer, orientation);
      strcat(buffer, " ");
      */
      GUI_print_variable_value(value,buffer);
      strcat(buffer, "} ");
  }

  if (strcmp(length, "NONE")) {
      value=shift_api_find_local_value(length, instance);
      strcat(buffer, " {");
      /*
      strcat(buffer, length);
      strcat(buffer, " ");
      */
      GUI_print_variable_value(value,buffer);
      strcat(buffer, "} ");

      if (strcmp(width, "NONE")) {
	strcat(buffer, " {");
	/*
	strcat(buffer, width);
	strcat(buffer, " ");
	*/
	value=shift_api_find_local_value(width, instance);
	GUI_print_variable_value(value,buffer);
	strcat(buffer, "} ");
      }
  }
  sprintf(temp, "{%s %d} ", typeName, instance->id);
  strcat(buffer, temp);
  strcat(buffer, "} "); 
  return 1;
}

/*
 * This function creates the header information that needs to precede
 * the animation information. This header information is used by TkShift
 * to understand what kind of data that is is receiving
 */
 
int
make_header_information(Canvas_Data * data, char * buffer) {


  char temp[3];
  int vars;

  if (data->next->frequency > 0)
    {
      insert_code(buffer, 'Y');
      sprintf(temp,"%d ", data->canvas_number);
      strcat(buffer, temp);

      sprintf(temp,"%d ", data->group);
      strcat(buffer, temp);      

      if (!strcmp(data->next->width, "NONE"))
	vars=3;
      else
	vars=4;
      sprintf(temp,"%d", vars);
      strcat(buffer, temp);
      
    }
  else 
    {
      insert_code(buffer, 'W');
      sprintf(temp,"%d ", data->canvas_number);
      strcat(buffer, temp);

      sprintf(temp,"%d", data->group);
      strcat(buffer, temp);      
    }
}



shift_api_exported_event**
getBreakpointEvents(int number, shift_api_user_type * type)
{
  shift_api_exported_event* current_event;
  char event_name[50];
  char * data;
  char * eventString;
  shift_api_exported_event** events;
  int number_of_events, event_number,loop, position;

  data =(char *)malloc((number + 1));

  if (number == 0) {
    events = (shift_api_exported_event**)malloc(sizeof(shift_api_exported_event*) * 1);
    events[0] = (shift_api_exported_event*) NULL;
    return events;
  }

  
  data = (char *) malloc ((number + 1)*sizeof(char));
  receive_data(serverSock2, data, number);
  number_of_events = getNumberOfEvents(data);
  events = (shift_api_exported_event**)malloc(sizeof(shift_api_exported_event*) * (number_of_events+1));
  loop = 0;
  eventString = data;
  position = 0;
  while (loop < number_of_events) {
    position = getEventName(event_name,data, position);
    current_event= shift_api_find_type_exported_event(event_name,
						      type);
    events[loop]= current_event;
    loop++;
  }
  
  events[number_of_events] = (shift_api_exported_event*) NULL;
  
  return events;
}


/*
 *
 *
 */

void 
parse_and_add_string(char *data_message) {

  char more_data_message[256];
  char length[30];
  char width[30];
  char type[30];
  char variable[30];
  char container_type[30];
  char container_variable[30];
  char desired_state[30];
  char orientation[30];
  int container_instance;

  char number[10];
  int dummy, dummy2, freq, canvas, group;
  type[0] = '\0';  
  number[0] = '\0';
  variable[0] = '\0';
  length[0] = '\0';
  width[0] = '\0';
  container_type[0] = '\0';
  container_variable[0] = '\0';
  desired_state[0] = '\0';
  orientation[0] = '\0';
  more_data_message[0] = '\0';

  sscanf(data_message,
	 "%s %d %s %d %s %s %s %d %s %s",
	 type,
	 &canvas,
	 variable,
	 &group,
	 number,
	 container_type,
	 container_variable,
	 &container_instance,
	 desired_state,
	 orientation);
  
  if (!(strcmp(number, "MORE")))
    {
      receive_data(serverSock2, more_data_message, 128);
      sscanf(more_data_message, "%s %s %d", length, width, &freq);
      Canvas_Data_List = add_Canvas_Data(type,
					 number,
					 variable,
					 Canvas_Data_List,
					 canvas,
					 length,
					 width,
					 freq,
					 group,
					 container_type,
					 container_variable,
					 container_instance,
					 desired_state,
					 orientation);
    }
  else
    Canvas_Data_List = add_Canvas_Data(type,
				       number,
				       variable,
				       Canvas_Data_List,
				       canvas,
				       "NONE",
				       "NONE",
				       0,
				       group,
				       container_type,
				       container_variable,
				       container_instance,
				       desired_state,
				       orientation);
}


Canvas_Data * 
add_Canvas_Data(char *type_name,
		char * number,
		char * variable_name,
		Canvas_Data * Canvas_Data_List,
		int canvas_number,
		char * length,
		char * width,
		int frequency,
		int group,
		char * container_type,
		char * container_variable,
		int container_instance,
		char * desired_state,
		char * orientation)
{

  Canvas_Data * n;
  Canvas_Data * l;

  n = (Canvas_Data*)malloc(sizeof(Canvas_Data));
  strcpy(n->type_name, type_name);
  strcpy(n->number, number);
  strcpy(n->variable_name, variable_name);
  n->canvas_number = canvas_number;
  strcpy(n->length, length);
  strcpy(n->width, width);
  n->frequency = frequency;
  n->next = NULL;
  n->group = group;
  strcpy(n->container_type, container_type);
  strcpy(n->container_variable, container_variable);
  n->container_instance=container_instance;
  strcpy(n->desired_state, desired_state);
  strcpy(n->orientation, orientation);
 
  l = Canvas_Data_List;

  if (Canvas_Data_List == NULL) 
    Canvas_Data_List = n;
  else
    {
      while (l->next != NULL)
	{
	  l = l->next;
	}
      l->next = n;
    }
  return Canvas_Data_List;
}


void
remove_Canvas_Data (int group, int canvas)
{

  Canvas_Data * l;
  Canvas_Data * n;

  l = Canvas_Data_List;
  n = l;
  /*printf("Wanting to remove group %d in canvas %d.\n", group, canvas);*/

  if ((l->next->group == group) && (l->next->canvas_number == canvas))
    {
      l = l->next->next;
      Canvas_Data_List = l;
    }
  else
    {
      l = l->next->next;
      while ((l->next->next != NULL))
	{
	  if ((l->next->group == group) && (l->next->canvas_number == canvas))
	    {
	      n->next->next = l->next->next;
	    }
	  l = l->next->next;
	  n = n->next->next;
	}
      if (l != NULL) 
	if ((l->next->group == group) && (l->next->canvas_number == canvas))
	  n->next->next = l->next->next;
    }
}


name_list *
add_instance(char *typeName,
	     name_list *instanceList,
	     int componentNo)
{

  name_list * n;
  name_list *l;
  Component * component, *Comp;

  for (Comp = continuous_components; Comp; Comp = Comp->continuous_next)
    if (!strcmp(typeName, Comp->desc->name) && Comp->name ==
	componentNo ) {
      component=Comp;
    }

  n = (name_list*)malloc(sizeof(name_list));
  strcpy(n->name, typeName);
  n->ID=componentNo;
  n->component= component;
  n->next = NULL;
 
  l = instanceList;

  if (instanceList == NULL) 
    instanceList = n;
  else
    {
      while (l->next != NULL)
	{
	  l = l->next;
	}
      l->next = n;
    }
  return instanceList;
}


name_list *
remove_instance(char * component_name, int instanceNo, name_list *instanceList)
{
  name_list * n;
  name_list * l;
  static char buffer[BIG_BUFFER_SIZE];
  static char info[BIG_BUFFER_SIZE];
  char temp_print_buffer[10];

  if (instanceList == NULL)
    {
      printf("WARNING: remove_instance illegaly called. Exiting...\n");
      return instanceList;
      /*
      exit(0); */
    }
  else
    {
      if (instanceList->next == NULL)
	{
	  if (!(strcmp(component_name, instanceList->name)) && 
	      (instanceList->ID == instanceNo))
	    {
	      buffer[0] = '$';
	      buffer[1] = ' ';
	      buffer[2] = 'P';
	      buffer[3] = 'L';
	      buffer[4] = '\0';

	      strcat(buffer, "#");
	      strcat(buffer, instanceList->name);
 	      sprintf(temp_print_buffer, "#%d", instanceList->ID);
	      strcat(buffer, temp_print_buffer);
	      strcat(buffer, "#");
	      if (!strcmp(instanceList->name, "global")) {
		info[0] = '\0';
		GUI_printGlobal(info);
		strcat(buffer, info);
	      }
	      else
		GUI_get_component_values(instanceList->name,
					 instanceList->ID,
					 buffer);

	      make_smart_packet(buffer);
	      return NULL;
	    }
	  else
	    {
	      printf("WARNING: remove_instance called with non-existing instance. Exiting...\n");
	      /*
	      exit(0); */
	    }
	}
      else
	{
	  l = instanceList;
	  if (!(strcmp(component_name, l->name)) && 
	      (l->ID == instanceNo))
	    {
	      buffer[0] = '$';
	      buffer[1] = ' ';
	      buffer[2] = 'P';
	      buffer[3] = 'L';  /* Last element */
	      buffer[4] = '\0';

	      strcat(buffer, "#");
	      strcat(buffer, l->name);
	      sprintf(temp_print_buffer, "#%d", l->ID);
	      strcat(buffer, temp_print_buffer);
	      strcat(buffer, "#");
	      if (!strcmp(l->name, "global")) {
		info[0] = '\0';
		GUI_printGlobal(info);
		strcat(buffer, info);
	      }
	      else
		GUI_get_component_values(l->name,
					 l->ID,
					 buffer);

	      make_smart_packet(buffer);
	      return l->next;
	    }
	  else
	    {
	      n = instanceList;
	      l = instanceList->next;
	      while (l != NULL && !(!(strcmp(component_name, l->name)) &&
		     (l->ID == instanceNo)) )
		{
		  n = n->next;
		  l = l->next;
		}

	      if (l!= NULL)
		{
		  n->next = l->next;
		  buffer[0] = '$';
		  buffer[1] = ' ';
		  buffer[2] = 'P';
		  buffer[3] = 'L';
		  buffer[4] = '\0';
	
		  strcat(buffer, "#");
		  strcat(buffer, l->name);
		  sprintf(temp_print_buffer, "#%d", l->ID);
		  strcat(buffer, temp_print_buffer);
		  strcat(buffer, "#");
		  if (!strcmp(l->name, "global")) {
		    info[0] = '\0';
		    GUI_printGlobal(info);
		    strcat(buffer, info);
		  }
		  else
		    GUI_get_component_values(l->name,l->ID,
					     buffer);

		  make_smart_packet(buffer);
	  
		  return instanceList;
		}
	      else
		{

		    printf("WARNING: component not found. Internal \
error. Exiting...\n");
		    return instanceList;
		  /*
		  exit(0); */
		}
	    }
	}
    }
}

name_list *
add_graph_instance(char *type_name,
		   int componentNo,
		   name_list *instanceList,
		   char * variableName,
		   int vector_id)
{
  
  name_list * n;
  name_list *l;

  n = (name_list*)malloc(sizeof(name_list));
  strcpy(n->typeName, type_name);
  strcpy(n->variableName, variableName);
  n->componentNo= componentNo;
  n->ID = vector_id;
  n->next = NULL;
 
  /*  printf("Request to add type %s %d, ID %d\n", type_name, componentNo,
	 vector_id); */
  l = instanceList;

  if (instanceList == NULL) 
    instanceList = n;
  else
    {
      while (l->next != NULL)
	{
	  l = l->next;
	}
      l->next = n;
    }
  return instanceList;
}


name_list *
remove_graph_instance(char * component_name,
		      int id,
		      name_list * instanceList)
{
  name_list * n;
  name_list * l;

  /*printf("Request to remove type %s, ID %d\n", component_name, id);*/
  if (instanceList == NULL)
    {
      printf("WARNING: remove_instance illegaly called on empty list. Exiting...\n");
      /*
      exit(0);*/
    }
  else
    {
      if (instanceList->next == NULL)
	{
	  if ( (!(strcmp(component_name, instanceList->typeName)))
	       && (id == instanceList->ID)) {
	    /*	    printf("Gonna remove %s %d\n", instanceList->typeName, instanceList->ID);*/
	    return NULL;
	  }
	  else
	    {
	      printf("WARNING: remove_instance called with non-existing\
instance. \n");
	      /*
              exit(0); */
	    }
	}
      else
	{
	  l = instanceList;
	  if ((!(strcmp(component_name, l->typeName)))
	      && (id == l->ID)) {
	    /*	    printf("Gonna remove %s %d\n", l->typeName, l->ID);*/
	    return l->next;
	  }
	  else
	    {
	      n = instanceList;
	      l = instanceList->next;
	      while (l != NULL
		     && !( (!strcmp(component_name, l->typeName))
		      && (id == l->ID)) )

		{
		  n = n->next;
		  l = l->next;
		}
	      if (l!= NULL)
		{
		  n->next = l->next;
		  /*		  printf("Gonna remove %s %d\n", l->typeName, l->ID);*/
		  return instanceList;
		}
	      else
		{
		  
		  printf("WARNING: component not found. Internal error. Exiting...\n");
		  /*
		  exit(0);*/
		}
	    }
	}
    }
}


void 
init_string(char * str, int i) {

int j;

j=0;
for (j=0; j<i; j++)
  str[j]='\0';
}


int 
getHoustonInformation()
{

  char myType[30], myVariable[30], myTemp[50];
  int myInstance;
  char buffer[300];
  char data_message[128];
  buffer[0] = '$';
  buffer[1] = ' ';
  buffer[2] = 'H';
  buffer[3] = ' ';
  buffer[4] = '\0';


  receive_data(serverSock2, data_message, 128);
  process_data(data_message);
  extract_information(myType, &myInstance, 
		     myVariable, data_message);
  myTemp[0]='\0';
  sprintf(myTemp, "%s", get_variable_values(myType, myInstance,
					    myVariable, 1));
  /*printf("We have %s\n", myTemp);*/
  if (strlen(myTemp) < 3) {
    strcat(buffer, "{S1 100} {S2 100} {S3 100} {S4 100} {S5 100}");
    make_smart_packet(buffer);
    return;
  }
    
  strcat(buffer, get_variable_values(myType, myInstance,
				    myVariable, 1));
  
  strcat(buffer," ");
  
  receive_data(serverSock2, data_message, 128);
  process_data(data_message);
  extract_information(myType, &myInstance, 
		     myVariable, data_message);
  strcat(buffer, get_variable_values(myType, myInstance,
				    myVariable, 1));
  strcat(buffer," ");
  receive_data(serverSock2, data_message, 128);
  process_data(data_message);
  extract_information(myType, &myInstance, 
		     myVariable, data_message);
  strcat(buffer, get_variable_values(myType, myInstance,
				    myVariable, 1));
  strcat(buffer," ");
  receive_data(serverSock2, data_message, 128);
  process_data(data_message);
  extract_information(myType, &myInstance, 
		     myVariable, data_message);
  strcat(buffer, get_variable_values(myType, myInstance,
				    myVariable, 1));
  strcat(buffer," ");
  receive_data(serverSock2, data_message, 128);
  process_data(data_message);
  extract_information(myType, &myInstance, 
		     myVariable, data_message);
  strcat(buffer, get_variable_values(myType, myInstance,
				    myVariable, 1));
  strcat(buffer," ");

  make_smart_packet(buffer);
  
}

int 
extract_information(char * type, int *instance, char * var, char * str) 
{

  type[0]='\0';
  var[0]='\0';

  sscanf(str, "%s %d %s", type, instance, var);

}



char*
GUI_Send_Type_Structure()
{
  static char buffer[20000];

  int loop, var_loop;
  shift_api_user_type ** types;
  shift_api_mode ** modes;
  shift_api_variable ** variables;
  shift_api_exported_event** events;
  char temp[50];
  loop=0;
  var_loop=0;
  
  insert_code(buffer,'A');	
  types = shift_api_find_user_types();
  
  while (types[loop] != NULL) {
    strcat(buffer, " {");
    strcat(buffer, types[loop]->name);
    strcat(buffer, " ");
    variables = shift_api_find_local_variables(types[loop]);
    strcat(buffer, " {");
    var_loop=0;
    while (variables[var_loop] != NULL) {
      strcat(buffer, " {");
      strcat(buffer, variables[var_loop]->name);
      strcat(buffer, " ");
      temp[0]='\0';
      GUI_printVariableType(variables[var_loop], temp);
      strcat(buffer, temp);
      strcat(buffer, "} ");
      var_loop++;
    }
    strcat(buffer, " }");
    var_loop=0;
    modes = shift_api_find_modes(types[loop]);
    strcat(buffer, " {");
    while (modes[var_loop] != NULL) {
      strcat(buffer, modes[var_loop]->name);
      strcat(buffer, " ");
      var_loop++;
    }
    strcat(buffer, " }");

    
    var_loop=0;
    events = shift_api_find_type_exported_events(types[loop]);
    strcat(buffer, " {");
    while (events[var_loop] != NULL) {
      strcat(buffer, events[var_loop]->name);
      strcat(buffer, " ");
      var_loop++;
    }
    strcat(buffer, " }");
    
    strcat(buffer, " }");
    loop++;
  }


  strcat(buffer, " {");
  strcat(buffer, "global");
  strcat(buffer, " ");
  variables = shift_api_find_global_variables();
  strcat(buffer, " {");
  var_loop = 0;

  while (variables[var_loop] != NULL) {
    strcat(buffer, " {");
    strcat(buffer, variables[var_loop]->name);
    strcat(buffer, " ");
    temp[0]='\0';
    GUI_printVariableType(variables[var_loop], temp);
    strcat(buffer, temp);
    strcat(buffer, "} ");
    var_loop++;
  }
  strcat(buffer, " }");
  strcat(buffer, " {");
  strcat(buffer, " }");
  strcat(buffer, " }");

  make_smart_packet(buffer);
}

  


char*
GUI_ListComponents(char* typeName,char * buffer)
{

  char temp[5];
  int loop;
  shift_api_user_type * type;
  shift_api_instance **  instances;

  loop=0;
  type = shift_api_find_user_type(typeName);
  instances = shift_api_find_instances(type);

  while (instances[loop] != NULL) {
    strcat(buffer, " {");
    strcat(buffer, instances[loop]->type->name);
    strcat(buffer, " ");
    temp[0]='\0';
    sprintf(temp, "%d",  instances[loop]->id);
    strcat(buffer, temp);
    strcat(buffer, "}");
    loop++;
  }
  return buffer;
}




void
initGuiDebug()
{
    
  extern TraceStatus *Tracer;
  char buffer[20000];
  
  /* Initialize the Tracer */
  Tracer = (TraceStatus*)malloc(sizeof(TraceStatus));
  Tracer->returnControl = TRUE;
  Tracer->lastLength = 0;
  Tracer->stopInN = 0; 
  InitializeServer();
  shift_api_callback = (shift_debug_function*)
    malloc(sizeof(shift_debug_function));


  /* Server has been initialized and the client has connected. Now we
     send the program structure to the client */
  buffer[0]='$';
  buffer[1]=' ';
  buffer[2]='A';
  buffer[3]=' ';
  buffer[4]='\0';
  
  GUI_Send_Type_Structure();
}


#endif /* TK_COMMUNICATIONS_I */
