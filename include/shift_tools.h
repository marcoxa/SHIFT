/* -*- Mode: C -*- */

/* shift_tools.h -- */

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

#ifndef SHIFT_TOOLS_H
#define SHIFT_TOOLS_H


/* Raises an error */
void
shift_tools_error(char*);


/* Raises a warning */
void
shift_tools_warning(char*);


/* Prints the informations relative to a variable */
char*
shift_tools_print_variable(shift_api_variable*);


/* Prints the informations relative to a type */
char*
shift_tools_print_type(shift_api_type*);


/* Prints the informations relative to a user type */
char*
shift_tools_print_user_type(shift_api_user_type*);


/* Prints the informations relative to a builtin type */
char*
shift_tools_print_builtin_type(shift_api_builtin_type*);


/* Prints a value */
char*
shift_tools_print_value(shift_api_value*, char*);


/* Prints a user type value (i.e., an instance) */
char*
shift_tools_print_instance(shift_api_instance*, char*);


/* Prints a builtin type value */
char*
shift_tools_print_builtin_value(shift_api_builtin_value*, char*);


/* Prints a set */
char*
shift_tools_print_set(shift_api_set_value*, char*);


/* Prints an array */
char*
shift_tools_print_array(shift_api_array_value*, char*);


/* Prints the values of all the local variables and the current mode 
 * of an instance 
 */
void
shift_tools_print_all_values(FILE*, shift_api_instance*);


/* Prints a list of breakpoints */
void
shift_tools_print_breakpoints(shift_debug_breakpoint*, int, int);


/* Prints one breakpoint to a file */
void
shift_tools_print_breakpoint(FILE*, shift_debug_breakpoint*, int);


/* Marks an event as tracing variables */
void
shift_tools_trace_event(char*, ComponentTypeDescriptor*);


/* Marks an event as causing a break point */
void
shift_tools_break_event(char*, ComponentTypeDescriptor*);


/* Tests if a variable belongs to a list */
int
shift_tools_is_var_in_list(char*, VariableDescriptor**);


/* Tests if an exported event belongs to a list */
int
shift_tools_is_event_in_list(char*, char**);


/* Retrieves a ComponentTypeDescriptor */
ComponentTypeDescriptor*
shift_tools_find_type(char*);


/* Retrieves a Component */
Component*
shift_tools_find_component(char*, int);


/* Retrieves a VariableDescriptor in a ComponentTypeDescriptor */
VariableDescriptor*
shift_tools_find_variable_descriptor(char*, ComponentTypeDescriptor*);


/* Retrieves an EventDescriptor in a ComponentTypeDescriptor */
EventDescriptor*
shift_tools_find_event_descriptor(char*, ComponentTypeDescriptor*);


/* Extracts a list of variables from a space separated string */
shift_api_variable**
shift_tools_extract_variables(shift_api_user_type*, char*);


/* Extracts a list of exported events from a space separated string */
shift_api_exported_event**
shift_tools_extract_events(shift_api_user_type*, char*);


/* Tests if a type must trace on a transition */
int
shift_tools_is_type_traced(ComponentTypeDescriptor*, TransitionDescriptor*);


/* Tests if a type must break on a transition */
int
shift_tools_is_type_broken(ComponentTypeDescriptor*, TransitionDescriptor*);


/* Tests if a component must trace on a transition */
int
shift_tools_is_component_traced(Component*, TransitionDescriptor*);


/* Tests if a component must break on a transition */
int
shift_tools_is_component_broken(Component*, TransitionDescriptor*);


#endif /* SHIFT_TOOLS_H */
