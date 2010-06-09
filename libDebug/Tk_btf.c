/* -*- Mode: C -*- */

/* GUI_btf.c -- */

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

#ifndef GUI_BTF_I
#define GUI_BTF_I

#include "shift_debug.h"
#include "discrete.h"
#include "shift_tools.h"
#include "shift_api_info.h"
#include "shift_debug.h"


extern int Transition_Info;
extern int Trace_Info;

extern char testing[30];


char*
GUIprintTran(Component *c, TransitionDescriptor *trD, int id, char* text);

void
GUIprintTran2(Component *c, TransitionDescriptor *trD, int id, char* text);


char*
GUIprintCollective(int sequence);

char*
GUIprintAttrs(Component* c, VariableDescriptor** myPV, char* a);



void
GUI_btf(char* buffer)
{
  int i; /* Beware!!!! i is used by FOR_ALL_READY_COMPONENTS */
  int printed = 2; /* 0 is to screen, 1 is to file, 2 is not at all */
  static int sequence;
  char* buff_ptr = buffer;
  int component_traced = 0;
  int component_broken = 0;
  int type_traced = 0;
  int type_broken = 0;
  
  FOR_ALL_READY_COMPONENTS {
    component_broken = shift_tools_is_component_traced(c, d);
    component_traced = shift_tools_is_component_broken(c, d);
    type_traced = shift_tools_is_type_traced(c->desc, d);
    type_broken = shift_tools_is_type_broken(c->desc, d);

    if ( (c->break_flag == break_e && component_broken)
	  || (c->desc->break_flag == break_e && type_broken) )
      {
	/* Print text that describes the Collective Transition only once ! */
	if (printed == 2)
	  {
	    sprintf(buff_ptr,"{Time is %d}", tclick);
	    buff_ptr=buff_ptr + strlen(buff_ptr);
	    sprintf(buff_ptr,"%s", GUIprintCollective(sequence));
	    buff_ptr=buff_ptr + strlen(buff_ptr);
	    printed = 0;
	  } 
	sprintf(buff_ptr," {%s }", GUIprintTran(c, d, sequence, "Before"));
	GUIprintTran2(c, d, sequence, "Before");
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
	    fprintf(Tracer->outfile, " {%s }", GUIprintCollective(sequence));
	    printed = 1;
	    Transition_Info=1;    
	  }
	fprintf(Tracer->outfile,
		"%s",
		GUIprintTran(c, d, sequence, "Before"));
      }
  } END_FARC;

  sequence++;

  if (printed == 0) {
    Transition_Info=1;    
    serverListen(); 
  }
}


void 
GUIprintTran2(Component *c, TransitionDescriptor *trD, int id, char* text)
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
  
  sprintf(testing, "{#%s -> %s#}", myM->name, trD->to->name);

}


char* 
GUIprintTran(Component *c, TransitionDescriptor *trD, int id, char* text)
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

  sprintf(a, " {%s %d}", myT->name, c->name);
  a = a + strlen(a);
  sprintf(a, " {%s CollectiveTransition %d}", text, id);
  a = a + strlen(a);
  sprintf(a, " {Transition %s -> %s}", myM->name, trD->to->name);
  a = a + strlen(a);
  sprintf(a, " {local "); a = a + strlen(a);
 
  while (eD)
    {
      sprintf(a, "%s ", eD->name); a = a + strlen(a);
      eD = trD->events[++i];
    }
  a = a - 1; /* Get rid of space */
  sprintf(a, "} {external "); a = a + strlen(a);
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
	      sprintf(a, "{%s 0}", eED->event->name); 
	      a = a + strlen(a);
	    }
	}
      else if (eED->sync_type == SYNC_ALL)
	{
	  sprintf(a, " {%s ", eED->event->name); a = a + strlen(a);
	  stupidMacroShortcoming = eED->link_var_offset;
	  a = printSet(DATA_ADDRESS(GenericSet*, tC, stupidMacroShortcoming),
		       a);

	  sprintf(a, "}"); a = a + strlen(a); 
	}
      else if ( (eED->sync_type == SYNC_ONE) & (eED->sync_var_offset != -1))
	{
	  sprintf(a, " {%s ", eED->event->name); a = a + strlen(a);
	  stupidMacroShortcoming = eED->sync_var_offset;
	  a = printComponent(DATA_ADDRESS(Component*,
					  c,
					  stupidMacroShortcoming),
			     a);
	  sprintf(a, "}"); a = a + strlen(a); 
	}

      eED = trD->sync_list[++i];
    }
  
  sprintf(a, "}"); a = a + strlen(a);

  if (c->printVars)
    a = GUIprintAttrs(c, c->printVars, a);
  else if (c->desc->printVars)
    a = GUIprintAttrs(c, c->desc->printVars, a);

  /*sprintf(a, "}");*/
  return d;
 
}

char*
GUIprintCollective(int sequence)
{

  int i; /* Beware!!!! i is used by FOR_ALL_READY_COMPONENTS */
  static char text[2000];
  char* a = text;


  sprintf(a, " {CollectiveTransition %d {", sequence); a = a + strlen(a);
  FOR_ALL_READY_COMPONENTS {
    sprintf(a, " {%s %d}", c->desc->name, c->name); a = a + strlen(a);
  } END_FARC;
  sprintf(a, "} }"); a = a + strlen(a);
      
  return text;
}

char *
GUIprintAttrs(Component* c, VariableDescriptor** myPV, char* a)
{
  int i = 0;
  VariableDescriptor* myV;

  while ((myV = myPV[i++]) != 0)
    {
      sprintf(a, "{%s ", myV->name); a = a + strlen(a);
      if (myV->type->kind == NUMBER_T)
	a = printNumberVariable(c, myV, a);
      else
	a = printObject(DATA_ADDRESS(void, c, myV->offset), myV->type, a);

      sprintf(a, "}"); a = a + strlen(a);
    }

  return a;
}

void
GUI_btf2()
{
  int i; /* Beware!!!! i is used by FOR_ALL_READY_COMPONENTS */
  int collective_traced = 1;
  int collective_broken = 1;
  static int sequence;
  shift_debug_breakpoint* bp_list = (shift_debug_breakpoint*) NULL;
  shift_debug_breakpoint* bp_prev = (shift_debug_breakpoint*) NULL;
  int something_is_broken_or_traced = 0;
  
  FOR_ALL_READY_COMPONENTS {
    int component_broken = shift_tools_is_component_broken(c, d);
    int component_traced = shift_tools_is_component_traced(c, d);
    int type_broken = shift_tools_is_type_broken(c->desc, d);
    int type_traced = shift_tools_is_type_traced(c->desc, d);
    shift_api_user_type* type = shift_api_find_user_type(c->desc->name);
    shift_api_instance* instance = shift_api_find_instance(type, c->name);

    /* 1. Check if the component must break */
    if ( (c->break_flag == break_e && component_broken)
	 || (c->desc->break_flag == break_e && type_broken) )
      {
	something_is_broken_or_traced = 1;
      }
    /* 2. Check if the component must trace */
    if ( (c->trace_flag == both_e && component_traced)
	 || (c->trace_flag == desc_e && component_traced)
	 || (c->desc->trace_flag == both_e && type_traced)
	 || (c->desc->trace_flag == desc_e && type_traced) )
      {
	something_is_broken_or_traced = 1;
      }
  } END_FARC;

  if (something_is_broken_or_traced) {
    FOR_ALL_READY_COMPONENTS {
      int component_broken = shift_tools_is_component_broken(c, d);
      int component_traced = shift_tools_is_component_traced(c, d);
      int type_broken = shift_tools_is_type_broken(c->desc, d);
      int type_traced = shift_tools_is_type_traced(c->desc, d);
      shift_api_user_type* type = shift_api_find_user_type(c->desc->name);
      shift_api_instance* instance = shift_api_find_instance(type, c->name);

      /* 1. Check if the component must break */
      if ( (c->break_flag == break_e && component_broken)
	   || (c->desc->break_flag == break_e && type_broken) )
	{
	  shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);
	
	  /* If this is the first breakpoint, store it as the list head */
	  if (!bp_list)
	    bp_list = bp_curr;

	  /* Set the fields of the breakpoint */
	  bp_curr->instance = instance;
	  bp_curr->trans = d->api_trans;
	  bp_curr->trace_flag = NOT_P;
	  if (c->break_flag == break_e && component_broken)
	    if (c->desc->break_flag == break_e && type_broken)
	      bp_curr->break_flag = BOTH_P;
	    else
	      bp_curr->break_flag = COMP_P;
	  else
	    bp_curr->break_flag = TYPE_P;
	  bp_curr->next = (shift_debug_breakpoint*) NULL;
	
	  /* Do the linking with the previous breakpoint */
	  if (bp_prev)
	    bp_prev->next = bp_curr;
	  bp_prev = bp_curr;
	
	  /* Record the fact that the transition is broken */
	  if (collective_broken)
	    collective_broken = 0;
	}

      /* 2. Check if the component must trace */
      if ( (c->trace_flag == both_e && component_traced)
	   || (c->trace_flag == desc_e && component_traced)
	   || (c->desc->trace_flag == both_e && type_traced)
	   || (c->desc->trace_flag == desc_e && type_traced) )
	{
	  shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);

	  /* If this is the first breakpoint, store it as the list head */
	  if (!bp_list)
	    bp_list = bp_curr;

	  /* Set the fields of the breakpoint */
	  bp_curr->instance = instance;
	  bp_curr->trans = d->api_trans;
	  bp_curr->break_flag = NOT_P;
	  if ( (c->trace_flag == both_e && component_traced)
	       || (c->trace_flag == desc_e && component_traced) )
	    if ( (c->desc->trace_flag == both_e && type_traced)
		 || (c->desc->trace_flag == desc_e && type_traced) )
	      bp_curr->trace_flag = BOTH_P;
	    else
	      bp_curr->trace_flag = COMP_P;
	  else
	    bp_curr->trace_flag = TYPE_P;
	  bp_curr->next = (shift_debug_breakpoint*) NULL;

	  /* Do the linking with the previous breakpoint */
	  if (bp_prev)
	    bp_prev->next = bp_curr;
	  bp_prev = bp_curr;

	  /* Record the fact that the transition is traced */
	  if (collective_traced)
	    collective_traced = 0;
	}
    } END_FARC;
    
    FOR_ALL_READY_COMPONENTS {
      int component_broken = shift_tools_is_component_broken(c, d);
      int component_traced = shift_tools_is_component_traced(c, d);
      int type_broken = shift_tools_is_type_broken(c->desc, d);
      int type_traced = shift_tools_is_type_traced(c->desc, d);
      shift_api_user_type* type = shift_api_find_user_type(c->desc->name);
      shift_api_instance* instance = shift_api_find_instance(type, c->name);

      /* 1. Check if the component must break */
      if (!( (c->break_flag == break_e && component_broken)
	   || (c->desc->break_flag == break_e && type_broken)) )
	if (!( (c->trace_flag == both_e && component_traced)
	   || (c->trace_flag == desc_e && component_traced)
	   || (c->desc->trace_flag == both_e && type_traced)
	   || (c->desc->trace_flag == desc_e && type_traced)) )
	  {
	    shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);
	    
	    /* If this is the first breakpoint, store it as the list head */
	    if (!bp_list)
	      bp_list = bp_curr;

	    /* Set the fields of the breakpoint */
	    bp_curr->instance = instance;
	    bp_curr->trans = d->api_trans;
	    bp_curr->break_flag = NOT_P;
	    bp_curr->trace_flag = NOT_P;
	    bp_curr->next = (shift_debug_breakpoint*) NULL;
	    
	    /* Do the linking with the previous breakpoint */
	    if (bp_prev)
	      bp_prev->next = bp_curr;
	    bp_prev = bp_curr;
	  }
    } END_FARC;

  }  

  /* 4. If the transition is either traced or broken,
   *    call the callback function
   */
  if (!collective_broken || !collective_traced)
    shift_api_callback->function(bp_list, sequence, tclick);

  /* 5. Increment the transition counter */
  sequence++;

  /* 6. Give back the hand to the user */
  if (!collective_traced) {
    Trace_Info=1;
  }
  if (!collective_broken) {
    Transition_Info=1;    
    serverListen(); 
  }

}

void
old_GUI_tf2()
{
  int i; /* Beware!!!! i is used by FOR_ALL_READY_COMPONENTS */
  int collective_traced = 1;
  int collective_broken = 1;
  static int sequence;
  shift_debug_breakpoint* bp_list = (shift_debug_breakpoint*) NULL;
  shift_debug_breakpoint* bp_prev = (shift_debug_breakpoint*) NULL;
  
  FOR_ALL_READY_COMPONENTS {
    int component_broken = shift_tools_is_component_broken(c, d);
    int component_traced = shift_tools_is_component_traced(c, d);
    int type_broken = shift_tools_is_type_broken(c->desc, d);
    int type_traced = shift_tools_is_type_traced(c->desc, d);
    shift_api_user_type* type = shift_api_find_user_type(c->desc->name);
    shift_api_instance* instance = shift_api_find_instance(type, c->name);

    /* 1. Check if the component must break */
    if ( (c->break_flag == break_e && component_broken)
	  || (c->desc->break_flag == break_e && type_broken) )
      {
	shift_debug_breakpoint* bp_curr = (shift_debug_breakpoint*)malloc(sizeof(shift_debug_breakpoint));

	/* If this is the first breakpoint, store it as the list head */
	if (!bp_list)
	  bp_list = bp_curr;

	/* Set the fields of the breakpoint. */
	bp_curr->instance = instance;
	bp_curr->trans = d->api_trans;
	bp_curr->trace_flag = NOT_P;
	if (c->break_flag == break_e && component_broken)
	  if (c->desc->break_flag == break_e && type_broken)
	    bp_curr->break_flag = BOTH_P;
	  else
	    bp_curr->break_flag = COMP_P;
	else
	  bp_curr->break_flag = TYPE_P;
	bp_curr->next = (shift_debug_breakpoint*) NULL;

	/* Do the linking with the previous breakpoint */
	if (bp_prev)
	  bp_prev->next = bp_curr;
	bp_prev = bp_curr;

	/* Record the fact that the transition is broken */
	if (collective_broken)
	  collective_broken = 0;
      }

    /* 2. Check if the component must trace */
    if ( (c->trace_flag == both_e && component_traced)
	 || (c->trace_flag == desc_e && component_traced)
	 || (c->desc->trace_flag == both_e && type_traced)
	 || (c->desc->trace_flag == desc_e && type_traced) )
      {
	shift_debug_breakpoint* bp_curr = (shift_debug_breakpoint*)malloc(sizeof(shift_debug_breakpoint));


	/* If this is the first breakpoint, store it as the list head */
	if (!bp_list)
	  bp_list = bp_curr;

	/* Set the fields of the breakpoint */
	bp_curr->instance = instance;
	bp_curr->trans = d->api_trans;
	bp_curr->break_flag = NOT_P;
	if ( (c->trace_flag == both_e && component_traced)
	     || (c->trace_flag == desc_e && component_traced) )
	  if ( (c->desc->trace_flag == both_e && type_traced)
	       || (c->desc->trace_flag == desc_e && type_traced) )
	    bp_curr->trace_flag = BOTH_P;
	  else
	    bp_curr->trace_flag = COMP_P;
	else
	  bp_curr->trace_flag = TYPE_P;
	bp_curr->next = (shift_debug_breakpoint*) NULL;

	/* Do the linking with the previous breakpoint */
	if (bp_prev)
	  bp_prev->next = bp_curr;
	bp_prev = bp_curr;

	/* Record the fact that the transition is traced */
	if (collective_traced)
	  collective_traced = 0;
      }

    /* 3. Even if the component neither broke nor traced, it must still
     *    appear in the collective transition
     */
    if (collective_broken && collective_traced)
      {
	shift_debug_breakpoint* bp_curr = new(shift_debug_breakpoint);

	/* If this is the first breakpoint, store it as the list head */
	if (!bp_list)
	  bp_list = bp_curr;

	/* Set the fields of the breakpoint */
	bp_curr->instance = instance;
	bp_curr->trans = d->api_trans;
	bp_curr->break_flag = NOT_P;
	bp_curr->trace_flag = NOT_P;
	bp_curr->next = (shift_debug_breakpoint*) NULL;

	/* Do the linking with the previous breakpoint */
	if (bp_prev)
	  bp_prev->next = bp_curr;
	bp_prev = bp_curr;
      }
  } END_FARC;



  /* 4. If the transition is either traced or broken,
   *    call the callback function
   */
  if (!collective_broken || !collective_traced)
    shift_api_callback->function(bp_list, sequence, tclick);

  /* 5. Increment the transition counter */
  sequence++;

  /* 6. Give back the hand to the user */
  if (!collective_traced) {
    Trace_Info=1;
  }
  if (!collective_broken) {
    Transition_Info=1;    
    serverListen(); 
  }

}

#endif /* GUI_BTF_I */

/* end of file -- GUI_btf.c -- */
