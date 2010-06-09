/* -*- Mode: C -*- */

/* names.h -- */

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

#ifndef names_h
#define names_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shift_debug.h>

#define BUFFER_SIZE 5000
#define BIG_BUFFER_SIZE 10000



typedef struct name_list
{
  char name[50];
  VariableDescriptor *descriptor;
  char typeName[50];
  char variableName[50];
  int ID;
  int componentNo;
  Component * component;
  struct name_list * next;
} name_list;


typedef struct Canvas_Data
{
  char type_name[50];
  char number[50];
  char variable_name[50];
  VariableDescriptor * descriptor;
  struct Canvas_Data * next;
  name_list * data;
  int canvas_number;
  char length[50];
  char width[50];
  int frequency;
  int group;
  char container_type[50];
  char container_variable[50];
  int container_instance;
  char desired_state[30];
  char orientation[30];
} Canvas_Data;

  
void process_data(char *);
char* GUITypeStructure();
char* GUI_debugger_display(Component*);
name_list *add_instance(char *, name_list*, int);
VariableDescriptor* getVarName(Component*, char*Var_Name);
char * getVarValue(Component *, VariableDescriptor*, char*);
void extract_ID_and_var(char * type,
			char *Instance_ID,
			char *Var_Name,
			char *ID,
			char *data_message);
void make_smart_packet(char *);
void pad_buffer(char * buffer, int);
void send_close_info();
Canvas_Data * add_Canvas_Data(char * type_name,
			      char * number,
			      char * variable_name,
			      Canvas_Data * Canvas_Data_List,
			      int,
			      char*,
			      char*,
			      int,
			      int,
			      char * container_type,
			      char * container_variable,
			      int container_instance,
			      char * desired_state,
			      char *);

char * GUI_printGlobal();
name_list *add_graph_instance(char *component_name, int,
			      name_list * instanceList,
			      char *,
			      int id);
name_list *remove_graph_instance(char * component_name,
				 int id,
				 name_list * instanceList);
name_list * remove_instance(char *, int,name_list *);
void insert_code(char * str, char code);
void serverListen();  
void getInfo(char *Instance_ID, char *Var_Name, char *ID, char *data_message);
void init_string(char *, int);
void createInstanceLists(Canvas_Data *);
void getNameAndNumber(char * data_message,
		      char *name,
		      char *ID);
shift_api_exported_event** getBreakpointEvents(int, shift_api_user_type *);


#endif /* names_h */

/* end of file -- names.h -- */
