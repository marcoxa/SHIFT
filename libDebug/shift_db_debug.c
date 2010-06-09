/* -*- Mode: C -*- */

/* shift_db_debug.c -- */

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

/* -*- Mode: C -*- */

/* shift_db_debug.c --
 * Debug aux functions
 */

#ifndef SHIFT_DB_DEBUG_I
#define SHIFT_DB_DEBUG_I

#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "shift_debug.h"
#include "discrete.h"
#include "shift_tools.h"
#include "shift_api_info.h"
#include "shift_db_debug.h"


static  char  LOGDIR[80];
extern Component* live_components;
static int db_initalized = 0;
extern db_cmd* DB_dbg_List;
static int command_number;
static int SET_LOGDIR=0;

void
shift_DB_Print_Command (db_cmd *dbg_cmd)
{ 
  FILE *Tmp_Log_FD;
  /* record origional log file informaiton and set tracer log
     to dbg_cmd->fd */
  fflush(Tracer->outfile);
  Tmp_Log_FD = Tracer->outfile;
  Tracer->outfile = dbg_cmd->fd;
  if (strcmp (dbg_cmd->cmd,"db_ctt")==0)
    {
      DB_Print_Components (dbg_cmd);
    }
  /* For component <type id>, dump all continuous variables every */
  if (strcmp (dbg_cmd->cmd,"db_ctc")==0)
    {
      int component = db_get_component_id(dbg_cmd->tail);
      DB_Print_Component ( dbg_cmd ,component);
    }
  /* For all components of type <type>, dump all transition
     information */
  if (strcmp (dbg_cmd->cmd,"db_dtt")==0)
    {
      shift_DB_Print_Transitions(dbg_cmd);
    }
      /*For component <type id>, dump all transition information
	before any transition */
  if (strcmp (dbg_cmd->cmd,"db_dtc")==0)
    {
      int component = db_get_component_id(dbg_cmd->tail);
      shift_DB_Print_Transition (dbg_cmd , component);  
    }
      /* repeat for db_dtt */
  /* Put tracer outfile back */

  Tracer->outfile = Tmp_Log_FD;
}

void
shift_DB_Traverse_CmdList()
{
  db_cmd *ptr;
  ptr=DB_dbg_List;
  while ( ptr != NULL)
    {
      shift_DB_Print_Command(ptr);
      ptr = ptr->next;
    }
}


db_cmd*
shift_DB_New_cmd(db_cmd* DB_cmd_list,\
		 char *cmd,\
		 char *rS)
{
  char tmpfile[64];
  FILE* FILE_D;                  
  db_cmd *new_node, *ptr;
  char type[256];
  char* tail = NULL ;
  char tmpstr[32];
  extern db_cmd *DB_dbg_List;
  
  if ( !db_initalized )
    {
      DB_dbg_List=shift_DB_initalize();
    }
  if ( (strchr(rS,' ')) != NULL )
    {
      tail = (char*) safe_malloc(strlen (rS));
      getToken(rS,type,tail," ");
    }
  else
    {
      strcpy(type,rS);
    }
  sprintf (tmpfile,"%s",LOGDIR);
  strcat (tmpfile, cmd);
  strcat (tmpfile, "_");
  strcat (tmpfile, type);
  sprintf(tmpstr,"%d", command_number);
  command_number++;
  strcat (tmpfile, tmpstr);
  FILE_D = fopen (tmpfile,"w");

  new_node = (db_cmd*) safe_malloc( sizeof(db_cmd));
  strcat (new_node->cmd,  cmd);
  strcat (new_node->cmd, "\0");
  strcpy ( new_node->type, type );
  strcat ( new_node->type,"\0");
 
  if ( tail !=NULL)
    { 
      new_node->tail = tail;
    }
  if( FILE_D  != NULL )
    {
      new_node->fd = FILE_D;

      if ( !strcmp(cmd,"db_ctt") || !strcmp(cmd,"db_ctc"))
	{
	  shift_DB_Print_Header_comp(new_node); 
	}
      else
	{
	  shift_DB_Print_Header_trans(new_node);
	}
    }
  else
    { 
      fprintf(stderr," could not open log file:\n");
      return (db_cmd*)NULL;
    }
  ptr =  DB_cmd_list;
  if ( ptr ==  NULL ) 
    {
      return (db_cmd*)new_node;
    }
  else
    {
      while ( ptr->next != NULL)
	{
	  ptr=ptr->next;
	}
      ptr->next = new_node;
    }

  return (db_cmd*) DB_cmd_list;
}


db_cmd*
shift_DB_initalize()
{
  int x=0;
  extern char *FieldSeperator, *EOR;
  db_initalized = 1;
  
  if (! SET_LOGDIR)shift_DB_Set_LOGDIR((char*)NULL);
  /* set field seperator */
  if ( shift_DB_setFieldSeperator(":") != 0)
    {
      fprintf (stderr," could not set field seperator\n");
    }
 if ( shift_DB_setFieldSeperator2(",") != 0)
    {
      fprintf (stderr," could not set field seperator\n");
    }
  if ( shift_DB_setEOR("\n") != 0 )
    {
      fprintf (stderr," could not set DB EOR\n");
    }
  if ( shift_DB_setSOR("") != 0 )
    {
      fprintf (stderr," could not set DB EOR\n");
    }
  return (db_cmd*) NULL ;
}

int
shift_DB_setFieldSeperator( char* string )
{
  int i=0;
  int j=0;
 
 if ( db_initalized == 0 )
    {
      DB_dbg_List=shift_DB_initalize();
    }
  if ( string == NULL )
    {
      fprintf(stderr,"usage: fs <string>\n");
      return (1);
    }
  free(FieldSeperator);
  FieldSeperator = (char*) safe_malloc(sizeof(string));
  while ( string[i] )
    {
      if ( string[i] == '\\')
	{
	  if(string[i+1] == 'n')
	    {
	      FieldSeperator[j] = '\n';
	      i = i + 2;
	      j++;
	    }
	  else if (string[i+1] == 't')
	    {
	      FieldSeperator[j] = '\t';
	      i = i + 2 ;
	      j++;
	    }

	  else if ( string[i+1] ==  '\\' )
	    {
	      FieldSeperator[j] = '\\';
	      i = i + 2;
	      j++;
	    }
	}
      else 
	{
	 FieldSeperator[j] = string[i];
	 i++;
	 j++;
	}
    }
  return (0);
}

int
shift_DB_setFieldSeperator2( char* string )
{
  int i=0;
  int j=0;

 if ( db_initalized == 0  )
    {
      DB_dbg_List=shift_DB_initalize();
    }

  if ( string == NULL )
    {
      fprintf(stderr,"usage: fs <string>\n");
      return (1);
    }
  free(FieldSeperator2);
  FieldSeperator2 = (char*) safe_malloc(sizeof(string));
  while ( string[i] )
    {
      if ( string[i] == '\\')
	{
	  if(string[i+1] == 'n')
	    {
	      FieldSeperator2[j] = '\n';
	      i = i + 2;
	      j++;
	    }
	  else if (string[i+1] == 't')
	    {
	      FieldSeperator2[j] = '\t';
	      i = i + 2 ;
	      j++;
	    }
	  else if ( string[i+1] ==  '\\' )
	    {
	      FieldSeperator2[j] = '\\';
	      i = i + 2;
	      j++;
	    }
	}
      else 
	{
	 FieldSeperator2[j] = string[i];
	 i++;
	 j++;
	}
    }
  return (0);
}

int
shift_DB_setEOR( char* string )
{
  int i=0;
  int j=0;

 if ( db_initalized == 0 )
    {
      DB_dbg_List=shift_DB_initalize();
    }

  if ( string == NULL )
    {
      fprintf(stderr,"usage: eor <string>\n");
      return (1);
    }
  free(EOR);
  EOR = (char*) safe_malloc(sizeof(string));
  
  while ( string[i])
    {
      if ( string[i] == '\\')
	{
	  if(string[i+1] == 'n')
	    {
	      EOR[j] = '\n';
	      i = i+2;
	      j++;
	    }
	  if (string[i+1] == 't')
	    {
	      EOR[j] = '\t';
	      i = i+2;
	      j++;
	    }
	  if (string[i+1] == '\\')
	    {
	      EOR[j] = '\\';
	      i = i+2;
	      j++;
	    }
	}
      else 
	{
	  EOR[j] = string[i];
	  i++;
	  j++;
	}
    }
  return (0);
}


int
shift_DB_setSOR( char* string )
{
  int i=0;
  int j=0;

  
if ( db_initalized  == 0 )
    {
      DB_dbg_List=shift_DB_initalize();
    }

  if ( SOR == NULL)
    {
      SOR =  (char*) safe_malloc(1 * sizeof(char));
      strcpy(SOR,string);
    }
  else
    {
      free(SOR);
      SOR = (char*) safe_malloc(sizeof(string));
    
  
      while ( string[i])
	{
	  if ( string[i] == '\\')
	    {
	      if(string[i+1] == 'n')
		{
		  SOR[j] = '\n';
		  i = i+2;
		  j++;
		}
	      if (string[i+1] == 't')
		{
		  SOR[j] = '\t';
		  i = i+2;
		  j++;
		}
	      if (string[i+1] == '\\')
		{
		  SOR[j] = '\\';
		  i = i+2;
		  j++;
		}
	    }
	  else 
	    {
	      SOR[j] = string[i];
	      i++;
	      j++;
	    }
	}
    }
  return (0);
}

int
db_get_component_id(char* tail_of_command)
{
  char BUF[10], rest[100];
  if ( strstr(tail_of_command," ") != NULL)
    {
      sscanf (tail_of_command,"%s %s", BUF,rest);
    }
  else
    {
      strcpy (BUF,tail_of_command);
    }
  return (int) atoi(BUF);
}




int  
DB_Print_Components(db_cmd* dbg_cmd )
{
  char* char_ptr;
  char BUFFER[5000];
  int a;
  int j=0;
  int num_instances = -1;
  shift_api_user_type* db_type_ptr;
  shift_api_variable** type_variables;
  shift_api_instance*  instance;
  shift_api_instance** instances_ptr;
  shift_api_value      *var_value;
  shift_api_mode*      inst_mode;
      /* for each instance of a type */
  db_type_ptr     = shift_api_find_user_type( dbg_cmd->type );
  num_instances   = shift_api_count_instances(db_type_ptr);
  type_variables  = shift_api_find_local_variables(db_type_ptr);
  instances_ptr   = shift_api_find_instances( db_type_ptr);

  if ( num_instances == 0 )
    {
      fprintf (Tracer->outfile,"%d\n", tclick);
      return(0);
    }
  /*
   *  I blew it
   *  19980202  Peter M.cooke find all components then print component
   *  ID insead of for id 0-> num_instances
   */
    while ( *instances_ptr != NULL )
      {
	DB_Print_Component(dbg_cmd,( *instances_ptr)->id);
	instances_ptr++;
      }
  fflush(Tracer->outfile);
  return(0);
}

int
shift_DB_Print_Transitions( db_cmd* dbg_cmd )
{
  /* don't do anything.  Handled by DB_btf */
}



int 
shift_DB_Print_Header_comp(db_cmd* node)
{
  char *ptr;
  int i=0;
  shift_api_variable **type_variables;
  shift_api_user_type *local_type; 

#ifdef DB_SEPERATE_HEADER
  char tmpfile[128],tmpstr[7];
  FILE *fd;
  sprintf( tmpfile, "%s" , LOGDIR);
  strcat(  tmpfile, node->cmd);
  strcat(  tmpfile, "_");
  strcat(  tmpfile, node->type);
  sprintf( tmpstr,  "%d", (command_number-1));
  strcat(  tmpfile, tmpstr);
  strcat(  tmpfile, ".hdr");
  fd=fopen(tmpfile, "w");
  if( fd == NULL)
    {
      fprintf(stderr,"cannot opend DB logfile\n");
      return (-1);
    }
  fprintf(fd,"%stime%sInsance#%smode%s",\
	  SOR,FieldSeperator,FieldSeperator,\
	  FieldSeperator);

#else
  fprintf(node->fd,"%stime%sInsance#%smode%s",\
	  SOR,FieldSeperator,FieldSeperator,\
	  FieldSeperator);
#endif
  if (node->tail != NULL )
    {  
      ptr = strchr(node->tail, '-');
      if ( ptr  != NULL )
	{  
	  ptr = ptr +3;
	  while ( ptr[i] != '\0')
	    {
	      if ( ptr[i] == ' ')
		{
#ifdef DB_SEPERATE_HEADER
		  fprintf(fd,"%s",FieldSeperator);
#else
		  fprintf(node->fd,"%s",FieldSeperator);
#endif
		  i++;
		}
	      else
		{
#ifdef DB_SEPERATE_HEADER
		  fprintf(fd,"%c",ptr[i]);
#else
		  fprintf(node->fd,"%c",ptr[i]);
#endif
		  i++;
		}
	    }
#ifdef DB_SEPERATE_HEADER
	  fprintf(fd,"%s",EOR);
	  fflush(fd);
	  fclose(fd);
#else
	  fprintf(node->fd,"%s",EOR);
	  fflush(node->fd);
#endif
	  return(0);
	}
    }

  local_type = shift_api_find_user_type(node->type);
  type_variables = shift_api_find_local_variables(local_type);
      
  i = 0;
  if ( type_variables == NULL )
    {
      fprintf ( stderr, \
		"shift_DB_Print_Header: cannot find varriables\n");
      return (-1);
    } 
  while ( type_variables[i] )
    {
      if (  node->tail != NULL )
	{
	  ptr =(char*) safe_malloc ( strlen(node->tail) + \
				     strlen (type_variables[i]->name) +	 2);
	  strcpy ( ptr, node->tail);
	  free   ( node->tail );
	  strcat ( ptr, " ");
	}
      else
	{
	  ptr = (char*) safe_malloc( strlen (type_variables[i]->name) + 2);
	}
      strcat ( ptr, type_variables[i]->name );
      strcat ( ptr,"\0");
      node->tail =   ptr;
 
#ifdef DB_SEPERATE_HEADER
      fprintf (fd,"%s",type_variables[i]->name);
#else
      fprintf (node->fd,"%s",type_variables[i]->name);
#endif
      if (type_variables[i+1] == NULL)
	{
	  break;
	}
#ifdef DB_SEPERATE_HEADER
       fprintf(fd,"%s", FieldSeperator);
#else
      fprintf(node->fd,"%s", FieldSeperator);
#endif
      i++;
    }
#ifdef DB_SEPERATE_HEADER
  fprintf(fd,"%s",EOR);
  fflush(fd);
  fclose(fd);
#else
   fprintf(node->fd,"%s",EOR);
 fflush(node->fd);
#endif
 
  return (0);
}

int 
shift_DB_Print_Header_trans(db_cmd* node)
{
  char *ptr;
  int i=0;
  shift_api_variable **type_variables;
  shift_api_user_type *local_type; 


#ifdef DB_SEPERATE_HEADER
  char tmpfile[128],tmpstr[7];
  FILE *fd;
  sprintf( tmpfile, "%s" , LOGDIR);
  strcat(  tmpfile, node->cmd);
  strcat(  tmpfile, "_");
  strcat(  tmpfile, node->type);
  sprintf( tmpstr,  "%d", (command_number-1));
  strcat(  tmpfile, tmpstr);
  strcat(  tmpfile, ".hdr");
  fd=fopen(tmpfile, "w");
  if( fd == NULL)
    {
      fprintf(stderr,"cannot opend DB logfile\n");
      return (-1);
    }
  fprintf(fd,"%stime%sTransition#%sInsance#%smode1%smode2%sevent%s",\
	  SOR,FieldSeperator,FieldSeperator,FieldSeperator,\
	  FieldSeperator,FieldSeperatorq,EOR);

  fflush(fd);
#else
  fprintf(node->fd,"%stime%sTransition#%sInsance#%smode1%smode2%sevent%s",\
	  SOR,FieldSeperator,FieldSeperator,FieldSeperator,\
	  FieldSeperator,FieldSeperator,EOR);
  fflush(node->fd);
#endif
  return(0);
	  
}

int
shift_DB_Set_LOGDIR(char* string)
{
  char DIR[16];
  int x = 0;
  struct stat biff_dir;
  if ( SET_LOGDIR )
    {
      fprintf (stderr, \
	       "Log Directory can only be set before simulation starts\n");
      return 0;
    }
  SET_LOGDIR = 1;
  if ( db_initalized == 0 )
    {
      DB_dbg_List=shift_DB_initalize();
    }
  
  if ( string != NULL )
    {
      if ( stat(string,&biff_dir) != -1)
	    {
	      fprintf(stderr,
		      "logdirectory:%s alreadyexists\n",string);
	      return 0;
	    }
      sprintf (LOGDIR,"%s/", string);
      mkdir(LOGDIR, S_IRWXU | S_IRGRP | S_IROTH);
      fprintf (stderr, "Log directory is set to %s\n",string);
      return 1;
    }
  else
    {
      for (;;)
	{
	  strcpy (DIR,"");
	  sprintf(DIR,"./LOG%d/",x);
	  if ( stat(DIR,&biff_dir) == -1)
	    {
	      strcpy(LOGDIR,DIR);
	      mkdir(LOGDIR, S_IRWXU | S_IRGRP | S_IROTH);
	      break;
	    }
	  x++;
	}
    }
  return 1;
}

int
shift_DB_Set_description_file(char* string)
{
  char desc_file_name[128];
  FILE *fd;
  char *mytime;
  time_t t;

   if ( db_initalized == 0 )
    {
      DB_dbg_List=shift_DB_initalize();
    }
  sprintf(desc_file_name,"%s/sim_description",LOGDIR);
  fd=fopen(desc_file_name,"w"); 
  if ( fd == NULL)
    {
      fprintf(stderr,
	      "shift_DB_Set_description_file: could not open description");
      return 0;
    }
  time(&t);
  mytime=ctime(&t);
  fprintf(fd,"%s",mytime);
  fprintf(fd,"%s", string);
  return 1;
}


int
DB_Print_Component(db_cmd* dbg_cmd, int instance )
{
  char* char_ptr;
  char* ptr_char;
  int j=0;
  int num_instances = -1;
  char BUFFER[5000];
  shift_api_user_type* db_type_ptr;
  shift_api_variable** type_variables;
  shift_api_value **   variable_values;
  shift_api_instance*  instance_ptr;
  shift_api_value*     var_value;
  shift_api_mode*      inst_mode;
  db_type_ptr     = shift_api_find_user_type( dbg_cmd->type );
  num_instances   = shift_api_count_instances(db_type_ptr);
  type_variables  = shift_api_find_local_variables(db_type_ptr);
   if ( num_instances < 0 )
    {
      fprintf (stderr,"cannot track invalid incident\n");
      return(-1);
    }
 
  j=0;

  instance_ptr = shift_api_find_instance(db_type_ptr , instance );

  /* This check prevents a crash when the instance is null
   * (e.g. this happens when a component is forced to exit).
   * 19980128 Marco Zandonadi
   * 19980202 Pcooke - modified conditional
   */
  if (instance_ptr == NULL) return -1;

  inst_mode = shift_api_find_current_mode(instance_ptr);
  fprintf(Tracer->outfile,"%s%d%s%d%s%s%s",SOR,tclick,FieldSeperator,\
	   instance, FieldSeperator,\
	  inst_mode->name,FieldSeperator);
  while( type_variables[j] ) 
    {
      if (( char_ptr = strstr(dbg_cmd->tail, "-v")) != NULL)
	    {
	      char_ptr = char_ptr +3;
	    }
	  else
	    {
	      char_ptr = dbg_cmd->tail;
	    }
	  
      ptr_char = strstr(char_ptr,type_variables[j]->name);
	if ( ptr_char != NULL )
	  {
	    if (!strncmp(ptr_char,type_variables[j]->name, 
		      sizeof(type_variables[j]->name)))
	      {
		var_value = \
		  shift_api_find_local_value(type_variables[j]->name,\
					     instance_ptr);
		strcpy(BUFFER, "");
		GUI_print_variable_value(var_value,BUFFER);
		fprintf(Tracer->outfile,"%s%s",BUFFER,FieldSeperator);
	      }
	  }
      j++;
    }
  fprintf(Tracer->outfile,"%s",EOR);
  return(0);
}

 int
 shift_DB_Print_Transition( db_cmd* dbg_cmd, int instance )
 {
   /*
     Don't do anything.  Handeled by DB_btf();
  */
}
#endif /* SHIFT_DB_DEBUG_I */


/* end of file -- shift_db_debug.c -- */
