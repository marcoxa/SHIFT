/* -*- Mode: C -*- */

/* shift_api_control.h -- */

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

#ifndef SHIFT_API_CONTROL_H
#define SHIFT_API_CONTROL_H


#include "shift_api_types.h"
#include "shift_debug.h"


/* Clears a type of all breakpoints */
int
shift_api_unbreak_type(shift_api_user_type*);


/* Clears a type of all tracepoints */
int
shift_api_untrace_type(shift_api_user_type*);


/* Sets a breakpoint on a user type */
int
shift_api_break_type(shift_api_user_type*, shift_api_exported_event**);


/* Sets several trace information on a user type */
int
shift_api_trace_type(shift_api_user_type*,
		     shift_api_exported_event**,
		     shift_api_variable**,
		     traceType);


/* Clears an instance of all breakpoints */
int
shift_api_unbreak_instance(shift_api_instance*);


/* Clears an instance of all tracepoints */
int
shift_api_untrace_instance(shift_api_instance*);


/* Sets a breakpoint on an instance */
int
shift_api_break_instance(shift_api_instance*, shift_api_exported_event**);


/* Sets several trace information on an instance */
int
shift_api_trace_instance(shift_api_instance*,
			 shift_api_exported_event**,
			 shift_api_variable**,
			 traceType);


/* Registers a callback function for breakpoints */
int
shift_api_register_breakpoint_callback(void (*f)(shift_debug_breakpoint*,
						 int,
						 int));


#endif /* SHIFT_API_CONTROL_H */
