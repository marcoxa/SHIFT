/* -*- Mode: C -*- */

/* shift_api_internal.h -- */

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

#ifndef SHIFT_API_INTERNAL_H
#define SHIFT_API_INTERNAL_H


#include <shift_config.h>
#include <gc_support.h>
#include "shift_api_types.h"
#include "collection.h"


/* Reports an error and exits */
void
shift_api_error(char*);


/* Reports a warning */
void
shift_api_warning(char*);


/* Creates a mode from a ModeDescriptor */
shift_api_mode*
shift_api_create_mode(ModeDescriptor*, shift_api_user_type*);


/* Retrieves a mode of a user type */
shift_api_mode*
shift_api_find_mode(char*, shift_api_user_type*);


/* Creates an exported event from an EventDescriptor */
shift_api_exported_event*
shift_api_create_exported_event(EventDescriptor*);


/* Creates an external event from an ExternalEventDescriptor */
shift_api_external_event*
shift_api_create_external_event(ExternalEventDescriptor*);


/* Creates a transition from a TransitionDescriptor */
shift_api_transition*
shift_api_create_transition(TransitionDescriptor*, shift_api_user_type*);


/* Creates a variable from a VariableDescriptor */
shift_api_variable*
shift_api_create_variable(VariableDescriptor*);


/* Creates a user type from a ComponentTypeDescriptor */
shift_api_user_type*
shift_api_create_user_type(ComponentTypeDescriptor*);


/* Creates a builtin type from a TypeDescriptor */
shift_api_builtin_type*
shift_api_create_builtin_type(TypeDescriptor*);


/* Creates a type from a TypeDescriptor */
shift_api_type*
shift_api_create_type(TypeDescriptor*);


/* Tests if an event is exported by a transition */
int
shift_api_is_event_exported(shift_api_exported_event*, shift_api_transition*);


/* Makes a value */
shift_api_value*
shift_api_make_value(void*, shift_api_type*);


/* Makes a builtin type value */
shift_api_builtin_value*
shift_api_make_builtin_value(void*, shift_api_builtin_type*);


/* Makes a number value */
double
shift_api_make_number_value(double*);


/* Makes a symbol value */
char*
shift_api_make_symbol_value(int*);


/* Makes a logical value */
int
shift_api_make_logical_value(int*);


/* Makes a set value */
shift_api_set_value*
shift_api_make_set_value(GenericSet**, shift_api_type*);


/* Makes an array value */
shift_api_array_value*
shift_api_make_array_value(GenericArray**, shift_api_type*);


/* Makes a user type value */
shift_api_instance*
shift_api_make_user_value(Component**, shift_api_user_type*);


/* Initializes the list of all user types */
void
shift_api_initialize_user_types();


/* Initializes the list of global variables */
void
shift_api_initialize_global_variables();


/* External variables defined by each c file produced by SHIC */
extern char* shift_symbol_table[];
extern int first_shift_symbol;
extern int last_shift_symbol;


#endif /* SHIFT_API_INTERNAL_H */
