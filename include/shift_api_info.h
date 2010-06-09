/* -*- Mode: C -*- */

/* shift_api_info.h -- */

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

#ifndef SHIFT_API_INFO_H
#define SHIFT_API_INFO_H


#include <shift_config.h>
#include "shift_api_types.h"


/* Retrieves the list of all user types */
shift_api_user_type**
shift_api_find_user_types();


/* Retrieves a particular user type */
shift_api_user_type*
shift_api_find_user_type(char*);


/* Retrieves the current mode of an instance of a user type */
shift_api_mode*
shift_api_find_current_mode(shift_api_instance*);


/* Retrieves the list of outgoing transitions of a mode */
shift_api_transition**
shift_api_find_outgoing_transitions(shift_api_mode*);


/* Retrieves the list of ingoing transitions of a mode */
shift_api_transition**
shift_api_find_ingoing_transitions(shift_api_mode*);


/* Retrieves the list of exporting transitions of an exported event */
shift_api_transition**
shift_api_find_exporting_transitions(shift_api_exported_event*);


/* Retrieves the list of exported events of a transition */
shift_api_exported_event**
shift_api_find_transition_exported_events(shift_api_transition*);


/* Retrieves the list of external events of a transition */
shift_api_external_event**
shift_api_find_external_events(shift_api_transition*);


/* Retrieves the list of exported events of a user type */
shift_api_exported_event**
shift_api_find_type_exported_events(shift_api_user_type*);


/* Retrieves a particular exported event of a user type */
shift_api_exported_event*
shift_api_find_type_exported_event(char*, shift_api_user_type*);


/* Retrieves the list of modes of a user type */
shift_api_mode**
shift_api_find_modes(shift_api_user_type*);


/* Retrieves a particular mode of a user type */
shift_api_mode*
shift_api_find_mode(char*, shift_api_user_type*);


/* Retrieves the list of children user types of a user type */
shift_api_user_type**
shift_api_find_children(shift_api_user_type*);


/* Retrieves the list of local variables of a user type */
shift_api_variable**
shift_api_find_local_variables(shift_api_user_type*);


/* Retrieves a particular local variable of a user type */
shift_api_variable*
shift_api_find_local_variable(char*, shift_api_user_type*);


/* Retrieves the list of global variables */
shift_api_variable**
shift_api_find_global_variables();


/* Retrieves a particular global variable */
shift_api_variable*
shift_api_find_global_variable(char*);


/* Retrieves the list of instances of a user type */
shift_api_instance**
shift_api_find_instances(shift_api_user_type*);


/* Counts the number of instances of a user type */
int
shift_api_count_instances(shift_api_user_type*);


/* Retrieves a particular instance of a user type */
shift_api_instance*
shift_api_find_instance(shift_api_user_type*, int);


/* Retrieves the value of a global variable */
shift_api_value*
shift_api_find_global_value(char*);


/* Retrieves the value of a local variable of an instance of a user type */
shift_api_value*
shift_api_find_local_value(char*, shift_api_instance*);

/* List of all user types */
shift_api_user_type** shift_api_user_types;


/* List of the global variables */
shift_api_variable** shift_api_global_variables;


/* DW: Added May 12
   This procedure is called when one prints transition information
   when the extrnal event is in the (one) case */

shift_api_instance *
shift_api_find_single_item_in_transition(shift_api_instance * instance,
					 shift_api_external_event *
					 event); 

/* Proposed additions to the API (DW 6/24/97) */

/* 
 * This function takes a string and a shift_api_instance* as
 * arguments. The string contains the names of one or more variable
 * names. This function returns a list of values 
 */

shift_api_value**
shift_api_find_local_variables_values(char *, shift_api_instance*);

/*
 * This function takes a string and a shift_api_type as arguments.
 * The string contains the name of a variable, the type is a
 * user-defined shift type. This function returns a list of
 * shift_api_values from every component in the simulation of that
 * type.
 */

shift_api_component_value**
shift_api_find_all_variable_values_of_a_type(char **, shift_api_user_type*);

/* 
   This function takes a string and a list of instances as
   arguments. It returns a list of values 
 */

shift_api_value**
shift_api_find_some_variable_values(char *, shift_api_instance**);




#endif /* SHIFT_API_INFO_H */
