/* -*- Mode: C -*- */

/* server.c -- */

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

#ifndef shift_ge_server_i
#define shift_ge_server_i

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef OS_SOLARIS
#include <sys/filio.h>
#endif

#include "run-time.h"
#include "shift_debug.h"
#include "shift_api_info.h"
#include "shift_api_control.h"
#include "names.h"
#include "run-time.h"
extern sim_argument *simulation_args[];

#define MAXSIZE 512
int Canvas_On = 0;
extern double CurrentTime; 
extern int serverSock1 , serverSock2;
extern int clientSock1, clientSock2;

extern name_list * NameList;
extern name_list * GraphList;
extern Canvas_Data * Canvas_Data_List;

extern char Transition_Data[20000];
extern char Trace_Data[20000];
extern int Transition_Info;
extern int Trace_Info;
extern int Zeno_Warning;

extern char testing[30];

int continue_steps;
double time_step = 1;
double stop_time = 1;
void translate(char *);
VariableDescriptor *descriptor;

void
sigsegv_handler(int s)
{
  fprintf(stderr, "xxxxx: error, caught signal SIGSEGV %d\n", s);
  send_close_info();
  exit(3);
}

void
sigbus_handler(int s)
{
  fprintf(stderr, "xxxxx: error, caught signal SIGBUS %d\n", s);
  send_close_info();
  fflush(stderr);
  exit(3);
}


static int dots_per_line = 50;
static int dots_output = 0;

void
serverListen()
{
  int cnt;

  static int stopFlag = 0;
  static int continue_mode = 0;
  static int time_mode = 0;

  char message[20];
  char data_message[128];
  char buffer[100000];
  char info[20000];
  char Instance_ID[10];
  char Var_Name[30];
  char typeName[50];
  char type_number[10];
  char ID[10];
  char graph_type[50];
  long nsteps;
  int bytes1, bytes2;
  int instance_number;

  char *env_var;
  Component *component, *Comp;

  /* If the GUI is displaying some instances then each timeclick it needs all 
   * the new varaible data on these instances.  The instances are in the 
   * NameList list, and this loop goes through the list, gets the data, and
   * sends it to the GU
   */
 
  nsteps = simulation_args[NSTEPS_ARG]->long_arg;
  nsteps = nsteps - 1;
  
  if (Zeno_Warning == 1)
    {
      send_zeno_warning();
      Zeno_Warning = 0;
      stopFlag = 1; 
      continue_mode = 0;
    }

  if (Transition_Info == 1)
    {
      send_transition_data(Transition_Data);
      Transition_Data[0] = '\0';
      Transition_Info = 0;
      stopFlag = 1; 
      continue_mode = 0;       
    }


  if (Trace_Info == 1)
    {
      send_trace_data(Trace_Data);
      Trace_Data[0] = '\0';
      Trace_Info = 0;
      /*stopFlag = 1; 
	continue_mode = 0;	*/
    }

  if (tclick > 0)
    {
      send_timeclick(tclick);
      if (NameList != NULL)
	send_instance_data(NameList);
      if (GraphList != NULL)
	send_graph_data(GraphList);
      if (Canvas_On == 1)
	GUI_Send_All_Position_Info(Canvas_Data_List);
    }
  else
    {
      send_timeclick(tclick);
      continue_mode = 0;
      stopFlag = 1; 
    }
  
  if (continue_mode == 1)
    {
      if (time_mode)
	{
	  if (CurrentTime <= stop_time)
	    {
	      return;
	    }
	  else
	    {
	      continue_mode = 0;
	      stopFlag = 1;
	      time_mode = 0;
	    }
	} else { /* not time mode */

	ComponentList* CompEl = Tracer-> timeTraced;
	Component* Comp;
	ComponentTypeDescriptor* myT;
  
	/* Dump state of all components in time Traced */
	/*      while (CompEl != NULL)
		{
		Comp = CompEl->thisComp;
		fprintf(Tracer->outfile, "%s\n", display(Comp));
		CompEl = CompEl->nextComp;
		}
		*/
	/* Now iterate thru all components and dump them if their type
	 * is traced
	 */
	/*      for (Comp = continuous_components; Comp; Comp = Comp->live_next)
		{ 
		myT = Comp->desc;	
		if (myT->output_flag == diff_e || myT->output_flag == both_e)
		fprintf(Tracer->outfile, "%s\n", display(Comp));
		}
		*/
	if ((Tracer->stopInN > 0) && (Tracer->returnControl == TRUE))
	  {
	    (Tracer->stopInN)--;
	    
	    if (dots_output % dots_per_line == 0)
	      {
		printf("\n[%d]", tclick);
		dots_output = 0;
	      }
	    dots_output++;
	    putchar('.');
	    return;
	  }
	else
	  {
	    continue_mode = 0;
	    stopFlag = 1;
	  }
      }
    }
  
  if (continue_mode == 0)
    {
      do {
	message[0]='\0';
	if ((cnt = receive_command(serverSock2, message)) > 0 ) {
	  
	  translate(message);

	  if (!strcmp(message, "Stop"))	  /* Stop simulation and loop */
	    {
	      stopFlag = 1;
	    }
	  else if (!strcmp(message, "Cont")) /* Restart simulation */
	    {
	      continue_mode = 0;
	      stopFlag = 0;
	    }
	  else if (!strcmp(message, "Step"))
	    {
	      Tracer->stopInN = continue_steps;
	      Tracer->lastLength = Tracer->stopInN;
	      Tracer->returnControl = TRUE;
	      continue_mode = 1;
	      stopFlag = 0;
	    }
	  else if (!strcmp(message, "Time"))
	    {
	      continue_mode = 1;
	      time_mode=1;
	      stop_time=CurrentTime + time_step;
	      stopFlag = 0;
	    }
	  else if ( (!strcmp(message, "ListComponents")) ||
		    (!strcmp(message, "ListInstances")))
	    {
	      data_message[0]='\0';
	      if ((!strcmp(message, "ListComponents")))
		insert_code(buffer,'L');
	      else
		insert_code(buffer,'I');

	      info[0]='\0';

	      receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      strcat(buffer, "#");
	      strcat(buffer, data_message);
	      strcat(buffer, "#");
	      if (!strcmp(data_message,"global"))
		{
		  strcat(info, buffer);
		  strcat(info, " {global 0}");
		}
	      else
		{
		  sprintf(info, "%s", GUI_ListComponents(data_message,
							 buffer));
		}
	      make_smart_packet(info);
	    }
	  else if (!strcmp(message, "PrintInstance"))
	    {
	      char temp_print_buffer[1000];

	      data_message[0]='\0';
	      receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      sscanf(data_message,"%s %d", typeName,
		     &instance_number);

	      insert_code(buffer,'Z');
	      strcat(buffer, "#");
	      strcat(buffer, typeName);
	      sprintf(temp_print_buffer, "#%d", instance_number);
	      strcat(buffer,  temp_print_buffer);
	      strcat(buffer, "#");

	      if (!strcmp(typeName, "global")) { 
		temp_print_buffer[0] = '\0';
		GUI_printGlobal(temp_print_buffer);
		strcat(buffer, temp_print_buffer);
	      }
	      else 
		GUI_get_component_values(typeName,
					 instance_number, buffer);
	      
	      NameList = add_instance(typeName,
				      NameList,
				      instance_number); 
	      make_smart_packet(buffer);
	    }
	  else if (!strcmp(message, "RemoveInstance"))
	    {
	      data_message[0]='\0';
	      receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      sscanf(data_message,"%s %d", typeName,
		     &instance_number);

 	      NameList = remove_instance(typeName, instance_number, 
 					 NameList);   
	    } 
	  else if (!strcmp(message, "PrintGType"))
	    {
	      data_message[0]='\0';
	      receive_data(serverSock2, data_message, 128);
	  
	      extract_ID_and_var(graph_type, Instance_ID, 
				 Var_Name, ID, data_message);
	      process_data(data_message);
	      GraphList = add_graph_instance(graph_type,
					     atoi(Instance_ID),
					     GraphList,
					     Var_Name,
					     atoi(ID));
	      
	    }
	  else if (!strcmp(message, "RemoveGType"))
	    {
	      data_message[0]='\0';
	      receive_data(serverSock2, data_message, 128);
	      getInfo(typeName, type_number,ID,data_message);
	      process_data(data_message);

	      GraphList = remove_graph_instance(typeName,
						atoi(ID),
						GraphList);
	    }
	  else if (!strcmp(message, "ChangeStep"))
	    {
	      receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      continue_steps = atoi(data_message);
	    }
	  else if (!strcmp (message, "ChangeTime"))
	    {
	      receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      time_step = atof(data_message);
	      /*	      printf("Changed time step to %d\n", time_step);*/
	    }
	  else if (!strcmp (message, "ChangeZeno"))
	    {

	      receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      /*zeno_step = atoi(data_message);*/
	      setZeno(data_message);
	      /*	      printf("Changed time step to %d\n", time_step);*/
	    }
	  else if (!strcmp(message, "BreakType"))
	    {
	      shift_api_exported_event** events;
	      shift_api_user_type* type;
	      receive_data(serverSock2, data_message, 128);
	      getNameAndNumber(data_message, typeName, ID);
	      type = shift_api_find_user_type(typeName);	      
	      events = getBreakpointEvents(atoi(ID),type);
	      shift_api_break_type(type,events);
	      printf("Breaking type %s.\n", typeName);

	      /*setTypeTrace(data_message, break_e);*/
	    }
	  else if (!strcmp(message, "UnbreakType"))
	    {
	      shift_api_user_type* type;
	      receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      /* Need to change this */
	      type = shift_api_find_user_type(data_message);
	      shift_api_unbreak_type(type);
	      
	    }
	  else if (!strcmp(message, "BreakComp"))
	    {
	      shift_api_exported_event** events;
	      shift_api_user_type* type;
	      shift_api_instance* instance;
	      int i, instanceNo, event_number;

	      receive_data(serverSock2, data_message, 128);
	      /*printf("To break %s %s.\n", data_message);*/
	      process_data(data_message);
	      sscanf(data_message,"%s %d %d", typeName, &instanceNo, &event_number);
	      /*getNameAndNumber(data_message, typeName, ID);*/
	      /*printf("To break %s %d with event string %d.\n",
		typeName, instanceNo, event_number);*/
	      type = shift_api_find_user_type(typeName);
	      events = getBreakpointEvents(event_number,type);
	      instance = shift_api_find_instance(type, instanceNo);
	      shift_api_break_instance(instance, events);

	    }
	  else if (!strcmp(message, "ClearComp"))
	    {
	      shift_api_exported_event** events;
	      shift_api_user_type* type;
	      shift_api_instance* instance;
	      int i;
	      events = (shift_api_exported_event**)malloc(sizeof(shift_api_exported_event*) * 1);
	     
	      events[0] = (shift_api_exported_event*) NULL;

	      receive_data(serverSock2, data_message, 128);
	      getNameAndNumber(data_message, typeName, ID);

	      type = shift_api_find_user_type(typeName);
	      instance = shift_api_find_instance(type, atoi(ID));
	      if (instance != NULL) {
		shift_api_unbreak_instance(instance);
	      }
	      /*	  printf("Received data: %s\n", data_message);*/
	    }
	  else if (!strcmp(message, "ClearCompTrace"))
	    {
	      shift_api_user_type* type;
	      shift_api_instance* instance;

	      receive_data(serverSock2, data_message, 128);
	      getNameAndNumber(data_message, typeName, ID);
	      if (strcmp(typeName,"global")) {
		type = shift_api_find_user_type(typeName);
		instance = shift_api_find_instance(type, atoi(ID));
		if (instance != NULL) {
		  shift_api_untrace_instance(instance);
		}
	      }
	      /*	  printf("Received data: %s\n", data_message);*/
	    }
	  else if (!strcmp(message, "TraceComp"))
	    {
	      shift_api_instance* instance;
	      shift_api_exported_event** events;
	      shift_api_variable** variables;
	      shift_api_user_type* type;
	      events =
		(shift_api_exported_event**)malloc(sizeof(shift_api_exported_event*) * 1);
	      variables = (shift_api_variable**)malloc(sizeof(shift_api_variable*) * 1);

	      events[0] = (shift_api_exported_event*) NULL;
	      variables[0] = (shift_api_variable*) NULL;
	      receive_data(serverSock2, data_message, 128);
	      getNameAndNumber(data_message, typeName, ID);
	      if (strcmp(typeName,"global")) {
		type = shift_api_find_user_type(typeName);	      
		instance = shift_api_find_instance(type, atoi(ID));
		/*events = getBreakpointEvents(atoi(ID),type);*/
		if (instance != NULL) {
		  shift_api_trace_instance(instance,events,variables,both_e);
		}
	      }
	    }
	  else if (!strcmp(message, "Quit"))
	    {
	      send_close_info();
	      printf("Received Exit signal from client. Exiting...\n");
	      exit(1);
	    }
	  else if (!strcmp(message, "CanvasOn"))
	    {
	      bytes1 = receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      parse_and_add_string(data_message);
	      data_message[0]='\0';
	      bytes2 = receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      parse_and_add_string(data_message);
	      Canvas_On = 1;
	    }
	  else if (!strcmp(message, "RemoveAnim"))
	    {
	      int group, canvas;
	      bytes1 = receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      sscanf(data_message,"%d %d", &group, &canvas);
	      remove_Canvas_Data(group, canvas);
	    }
	  else if (!strcmp(message, "PrintValue"))
	    {
	      /* This is being added for the Houston canvas */
	      getHoustonInformation();
	    }
	  else if (!strcmp(message, "ValuesOfAll"))
	    {
	      char type[100], return_code[4], variable_string[100];
	      int l, index, length;
	      receive_data(serverSock2, data_message, 128);
	      process_data(data_message);
	      /*printf("Message is %s.\n", data_message);*/
	      sscanf(data_message,"%s %s", type, return_code);

	      l = strlen(return_code) + strlen(type) + 2;
	      index = l;
	      length = strlen(data_message);
	      while (index <= length) {
		variable_string[index-l] = data_message[index];
		index++;
	      }
	      /*printf("We have read type %s, code %s and vars %s.\n",
		     type, return_code, variable_string);*/
	      
	      
	      insert_code(buffer,return_code[0]);
	      strcat(buffer, "#");
	      strcat(buffer, type);
	      strcat(buffer, "#");

	      GUI_print_all_variables_of_all_instances(type,
						      variable_string,
						      buffer);
	      /*printf("Buffer: %s.\n", buffer);*/
	      make_smart_packet(buffer);
	    }
	  else {
	    /*	  printf("Unknown command: %s\n", message);*/
	  }
	  message[0]='\0';
	}
      } while (stopFlag == 1); /* do {...} while ... ; */
    }

  if (tclick == nsteps)
    {
      send_close_info();
      /* closeServer();*/
    }
}
    

void
translate(char * msg)
{

  int m;
  char result[20];
  char message[1024];

  m = atoi(msg);

  switch (m)
    {
    case 9999: strcpy(result, "Quit");
      result[4]='\0';
      break;
    case 1111: strcpy(result, "Cont");
      result[4]='\0';
      break;
    case 2222: strcpy(result, "Stop");
      result[4]='\0';
      break;
    case 5599: strcpy(result, "PrintValue");
      result[10]='\0';
      break;
    case 3333: strcpy(result, "PrintTypes");
      result[10]='\0';
      break;
    case 4444: strcpy(result, "ListComponents");
      result[14]='\0';
      break;
    case 5505: strcpy(result, "PrintInstance");
      result[13]='\0';
      break;
    case 5900: strcpy(result, "ValuesOfAll");
      result[11]='\0';
      break;
    case 5777: strcpy(result, "RemoveInstance");
      result[14]='\0';
      break;
    case 5778: strcpy(result, "RemoveGType");
      result[11]='\0';
      break;
    case 5550: strcpy(result, "PrintGType");
      result[10]='\0';
      break;
    case 5556: strcpy(result, "ListInstances");
      result[13]='\0';
      break;
    case 6001: strcpy(result, "Step");
      result[4]='\0';
      break;
    case 6002: strcpy(result, "Time");
      result[4]='\0';
      break;
    case 6003: strcpy(result, "ChangeZeno");
      result[10]='\0';
      break;
    case 6005: strcpy(result, "ChangeStep");
      result[10]='\0';
      break;
    case 6006: strcpy(result, "ChangeTime");
      result[10]='\0';
      break;
    case 8000: strcpy(result, "BreakType");
      result[9]='\0';
      break;
    case 8002: strcpy(result, "UnbreakType");
      result[11]='\0';
      break;
    case 8003: strcpy(result, "BreakComp");
      result[9]='\0';
      break;
    case 8004: strcpy(result, "ClearComp");
      result[9]='\0';
      break;
    case 8005: strcpy(result, "TraceComp");
      result[9]='\0';
      break;
    case 8006: strcpy(result, "ClearCompTrace");
      result[14]='\0';
      break;
    case 8765: strcpy(result, "CanvasOn");
      result[8]='\0';
      break;
    case 8900: strcpy(result, "RemoveAnim");
      result[10]='\0';
      break;
    default: strcpy(result, "Dunno");
      result[5]='\0';
      break;
    }
  strcpy(msg, result);
}


void 
insert_code(char * str, char code) {


  str[0]='$';
  str[1]=' ';
  str[2]=code;
  str[3]=' ';
  str[4]='\0';

}


void
make_smart_packet(char * buffer)
{

  int l;
  char stamp[7];
  char *info;
  char * info2;

  stamp[0]='\0';
  l = strlen(buffer);
  info2 = (char *)malloc(l + 1);
  info2[0]='\0';
  strncpy(info2, buffer, l);
  info2[l] = '\0';
  sprintf(stamp, "%d", l);
  pad_buffer(stamp, 6);
  info = (char *)malloc(l + 6 + 1);
  info[0]='\0';
  strncpy(info, stamp, 7);
  strcat(info, info2);
  info[l + 6]='\0';

  transmit(serverSock1, info, serverSock2);
}

void
pad_buffer(char * str, int size)
{

  int l, i;
  
  l = strlen(str);
  if (l < 1024)
    {
      for (i = l; i< size; i++)
	str[i] = ' ';
      str[size] = '\0';
    }
  else
    {
      printf("We have an array greater than 1024 spots\n");
      exit(0);
    }
}


void
process_data(char * msg)
{

  char data[128];
  int i = 0;

  strcpy(data, msg);

  while (data[i] != '!')
    i++;

  msg[i] = '\0';
}

void 
getInfo(char *typeName, char *type_number, char *ID,
	char *data_message)  {

  int i, j;

  i = 0;
  while (data_message[i] != ':')
    {
      typeName[i] = data_message[i];
      i++;
    }
  i++;
  typeName[i] = '\0';

  j = 0;
  while (data_message[i] != ':')
    {
      type_number[j] = data_message[i];
      i++;
      j++;
    }
  type_number[j]='\0';

  i++;
  j = 0;
  while (data_message[i] != '!')
    {
      ID[j] = data_message[i];
      i++;
      j++;
    }
  ID[j] = '\0';

}

void
extract_ID_and_var(char * graph_Type,
		   char *Instance_ID,
		   char *Var_Name,
		   char *ID,
		   char *data_message)
{
  int i, j;

  i = 0;
  while (data_message[i] != ':')
    {
      graph_Type[i] = data_message[i];
      i++;
    }
  graph_Type[i] = '\0';
  i++;


  j = 0;
  while (data_message[i] != ':')
    {
      Instance_ID[j] = data_message[i];
      i++;
      j++;
    }
  Instance_ID[j]='\0';

  i++;
  j = 0;
  while (data_message[i] != ':')
    {
      Var_Name[j] = data_message[i];
      i++;
      j++;
    }
  Var_Name[j] = '\0';

  i++;
  j = 0;
  while (data_message[i] != '!')
    {
      ID[j] = data_message[i];
      i++;
      j++;
    }
  ID[j] = '\0';
}



void
getNameAndNumber(char * data_message,
		   char *name,
		 char *ID) 

{
  int i, j;

  i = 0;
  while (data_message[i] != ' ')
    {
      name[i] = data_message[i];
      i++;
    }
  name[i] = '\0';
  i++;

  j = 0;
  while (data_message[i] != '!')
    {
      ID[j] = data_message[i];
      i++;
      j++;
    }
  ID[j]='\0';
}




#endif /* shift_ge_server_i */

/* end of file -- server.c -- */
