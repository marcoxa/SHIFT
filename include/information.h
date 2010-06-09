/* -*- Mode: C -*- */

/* information.h -- */

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


#ifndef INFORMATION_H
#define INFORMATION_H


typedef struct API_function_information {
  int call_every_n_clicks;	/* The function in invoked every n
				   timeclicks */
  int clicks_before_call;	/* How many clicks to wait until we
				   call it again */
  int once;			/* (1==call periodically), (0==call once
				   and forget) */
  int breakID;			/* This item's breakpoint ID */
  int first_call;		/* The user may not want to start invoking
				   the function until a certain
				   timeclick */
  void (*function_to_call)();	/* Function to invoke */
  void *arg;			/* Function's argument */
  struct API_function_information * next; /* Next function in list */
  struct API_function_information * previous; /* Next function in list */

} API_function_information;


API_function_information *API_callbackList;
API_function_information *Debug_callbackList;

void
information_initialize();
void
initialize_debugger_functions();
void
information_functions();
void
execute_debugger_functions();
void
execute_user_functions();
int
registerCallback(void (*funct)(), int, int, long);
int
registerDebugCallback(void (*funct)(), int, int, long);
int  
unregisterCallback(int);


#endif /* INFORMATION_H */
