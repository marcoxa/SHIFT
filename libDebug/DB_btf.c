/* -*- Mode: C -*- */

/* DB_btf.c -- */

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

#ifndef DB_BTF_I
#define DB_BTF_I

#include "shift_debug.h"
#include "discrete.h"
#include "shift_tools.h"
#include "shift_api_info.h"
#include "shift_debug.h"
#include "shift_db_debug.h"


extern int Transition_Info;
extern int Trace_Info;
extern db_cmd* DB_dbg_List;
extern char testing[30];


char*
DBprintTran(Component *c, TransitionDescriptor *trD, int id, char* text);

void
DBprintTran2(Component *c, TransitionDescriptor *trD, int id, char* text);


char*
DBprintCollective(int sequence);

char*
DBprintAttrs(Component* c, VariableDescriptor** myPV, char* a);

FILE*
shift_DB_Find_Log_FD(char* Type, int Instance);

void
DB_btf()
{
  int i; /* Beware!!!! i is used by FOR_ALL_READY_COMPONENTS */
  int printed = 2; /* 0 is to screen, 1 is to file, 2 is not at all */
  static int sequence;
  char buffer[2000];
  char* buff_ptr = buffer;
  int component_traced = 0;
  int component_broken = 0;
  int type_traced = 0;
  int type_broken = 0;
  FILE *Temp_FD; 
  FILE *FD=NULL;
  Temp_FD = Tracer->outfile;
  if ( DB_dbg_List == NULL )
    {
      return;
    }
  FOR_ALL_READY_COMPONENTS {
    component_broken = shift_tools_is_component_traced(c, d);
    component_traced = shift_tools_is_component_broken(c, d);
    type_traced = shift_tools_is_type_traced(c->desc, d);
    type_broken = shift_tools_is_type_broken(c->desc, d);
    if( DB_dbg_List != NULL)
      {
	FD=shift_DB_Find_Log_FD(c->desc->name, c->name);
	if (FD == NULL ) continue;    
	if( FD != NULL)
	  {
	    Tracer->outfile = FD;
	  }
      }
    if ( (c->break_flag == break_e && component_broken)
	  || (c->desc->break_flag == break_e && type_broken) )
      {
	/* Print text that describes the Collective Transition only once ! */
	if (printed == 2)
	  {
	    sprintf(buff_ptr,"%d%s", tclick,FieldSeperator);
	    buff_ptr=buff_ptr + strlen(buff_ptr);
	    sprintf(buff_ptr,"%s", DBprintCollective(sequence));
	    buff_ptr=buff_ptr + strlen(buff_ptr);
	    printed = 0;
	  } 
	sprintf(buff_ptr,"%s", DBprintTran(c, d, sequence, "Before"));
	DBprintTran2(c, d, sequence, "Before");
	buff_ptr=buff_ptr + strlen(buff_ptr);
      }

    if ( (c->trace_flag == both_e && component_traced)
	 || (c->trace_flag == desc_e && component_traced)
	 || (c->desc->trace_flag == both_e && type_traced)
	 || (c->desc->trace_flag == desc_e && type_traced) )
      {
	/* Print text that describes the Collective Transition only once ! */
	if (printed == 2)
	  {
	    fprintf(Tracer->outfile,"%d%s", tclick,FieldSeperator);
	    fprintf(Tracer->outfile, "%s",
		    DBprintCollective(sequence));
	    printed = 1;
	    Transition_Info=1;    
	  }
	fprintf(Tracer->outfile,
		"%s",
		DBprintTran(c, d, sequence, "Before"));
      }
    if( FD != NULL )
      {
	fprintf(Tracer->outfile,"%s",EOR);
      }
  } END_FARC;
  sequence++;

  if (printed == 0)
    {
      Transition_Info=1;    
      serverListen(); 
    }
  fflush(Tracer->outfile);
  Tracer->outfile = Temp_FD;
}


void 
DBprintTran2(Component *c, TransitionDescriptor *trD, int id, char* text)
{

  /* id identifies the collective transition */
  ComponentTypeDescriptor* myT = c->desc;
  Component* tC;
  TypeDescriptor* tD;
  ModeDescriptor* myM = c->M;
  EventDescriptor* eD = trD->events[0];
  ExternalEventDescriptor* eED = trD->sync_list[0];
  VariableDescriptor* myV = myT->variables;
  static char d[OUTBUFSIZE];
  char* a =d;
  int i = 0, stupidMacroShortcoming;
  
  sprintf(testing, "#%s%s%s%s", myM->name,FieldSeperator, \
	  trD->to->name,FieldSeperator);

}


char* 
DBprintTran(Component *c, TransitionDescriptor *trD, int id, char* text)
{

  /* id identifies the collective transition */
  ComponentTypeDescriptor* myT = c->desc;
  Component* tC;
  TypeDescriptor* tD;
  ModeDescriptor* myM = c->M;
  EventDescriptor* eD = trD->events[0];
  ExternalEventDescriptor* eED = trD->sync_list[0];
  VariableDescriptor* myV = myT->variables;
  static char d[OUTBUFSIZE];
  char* a =d;
  int i = 0, stupidMacroShortcoming;
 
  sprintf(a, "%s%s%s%s", myM->name,FieldSeperator,\
	  trD->to->name,FieldSeperator);
  a = a + strlen(a);
 
  while (eD)
    {
      sprintf(a, "%s%s", eD->name,FieldSeperator);
      a = a + strlen(a);
      eD = trD->events[++i];
    }
    i = 0;
  
  while (eED)
    {
      tC = eED->global? _global : c;

      if (eED->connection_type == SINGLE_CONNECTION)
	{
	  Component* ec = DATA_ITEM(Component*, tC, eED->link_var_offset);
	  if (ec)
	    {
	      sprintf(a, " {%s %s %d}",
		      eED->event->name,
		      ec->desc->name,
		      ec->name); 
	      a = a + strlen(a);
	    }
	  else
	    {
	      sprintf(a, "%s%s", eED->event->name,FieldSeperator); 
	      a = a + strlen(a);
	    }
	}
      else if (eED->sync_type == SYNC_ALL)
	{
	  sprintf(a, "%s%s", eED->event->name,FieldSeperator);
	  a = a + strlen(a);
	  stupidMacroShortcoming = eED->link_var_offset;
	  a = printSet(DATA_ADDRESS(GenericSet*, tC, stupidMacroShortcoming),
		       a);

	}
      else if ( (eED->sync_type == SYNC_ONE) & (eED->sync_var_offset != -1))
	{
	  sprintf(a, "%s%s", eED->event->name,FieldSeperator); 
	  a = a + strlen(a);
	  stupidMacroShortcoming = eED->sync_var_offset;
	  a = printComponent(DATA_ADDRESS(Component*,
					  c,
					  stupidMacroShortcoming),
			     a);
	}

      eED = trD->sync_list[++i];
    }
  /* 
  if (c->printVars)
    a = DBprintAttrs(c, c->printVars, a);
  else if (c->desc->printVars)
    a = DBprintAttrs(c, c->desc->printVars, a);
    */
  return d;
 
}

char*
DBprintCollective(int sequence)
{

  int i; /* Beware!!!! i is used by FOR_ALL_READY_COMPONENTS */
  static char text[2000];
  char* a = text;


  sprintf(a, "%d%s", sequence,FieldSeperator);
  a = a + strlen(a);
  FOR_ALL_READY_COMPONENTS {
    sprintf(a, "%s %d%s",c->desc->name, c->name,FieldSeperator);
    a = a + strlen(a);
  } END_FARC;
        
  return text;
}

/* ignore print attrs * NOT relevant to definition of tracing command */
char *
DBprintAttrs(Component* c, VariableDescriptor** myPV, char* a)
{
  int i = 0;
  VariableDescriptor* myV;

  while ((myV = myPV[i++]) != 0)
    {
      sprintf(a, "%s", myV->name); a = a + strlen(a);
      if (myV->type->kind == NUMBER_T)
	a = printNumberVariable(c, myV, a);
      else
	a = printObject(DATA_ADDRESS(void, c, myV->offset), myV->type, a);
    }

  return a;
}
FILE*
shift_DB_Find_Log_FD(char* Type, int Instance)
{

  db_cmd* L_Trace_Type;
  db_cmd* L_Trace_Component;
  db_cmd* ptr1;
  int DB_ID;
  ptr1 = DB_dbg_List;
  L_Trace_Type = L_Trace_Component = (db_cmd*) NULL;
  while ( ptr1 != NULL)
    {
      if( !strcmp(ptr1->cmd,"db_dtt"))
	{
	  if ( !strcmp( ptr1->type, Type))
	    {
	      L_Trace_Type = ptr1;
	    }
	}
      else if ( !strcmp(ptr1->cmd,"db_dtc"))
	{
	  /*get component from dbg command */
	  DB_ID = db_get_component_id(ptr1->tail);
	  if( !strcmp ( ptr1->type, Type))
	    {
	      if ( DB_ID == Instance )
		{
		  L_Trace_Component = ptr1;
		}
	    }
	}
      ptr1=ptr1->next;

    }
  if ( L_Trace_Component != NULL)
    {
       return L_Trace_Component->fd;
    }
  else if ( L_Trace_Type != NULL )
    {
      return  L_Trace_Type->fd;
    }
  else 
    {
      return (FILE*)NULL;
    }
}
#endif /* DB_BTF_I */

/* end of file -- DB_btf.c -- */




