/* -*- Mode: C -*- */

/* shift_api_types.h -- */

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

#ifndef SHIFT_API_TYPES_H
#define SHIFT_API_TYPES_H


#include <shift_config.h>
#include <gc_support.h>


typedef enum {
  BUILTIN_K,
  USER_K,
  UNKNOWN_K,
  FOREIGN_K
} shift_api_type_kind;


typedef enum {
  NUMBER_B,
  SYMBOL_B,
  LOGICAL_B,
  ARRAY_B,
  SET_B,
  UNKNOWN_B
} shift_api_builtin_kind;


typedef enum {
  CONSTANT_M,
  ALGEBRAIC_M,
  DIFFERENTIAL_M,
  UNKNOWN_M
} shift_api_mode_kind;


typedef enum {
  GLOBAL_V,
  INPUT_V,
  OUTPUT_V,
  STATE_V,
  UNKNOWN_V
} shift_api_variable_kind;


typedef enum {
  SINGLE_C,
  SET_ONE_C,
  SET_ALL_C,
  UNKNOWN_C
} shift_api_connection_kind;


typedef struct shift_api_mode {
  char* name;				/* SHIFT mode name */
  struct shift_api_user_type* type;	/* Type the mode belongs to */
  int guarded;				/* 0 or 1 */
  struct shift_api_transition** outs;	/* List of outgoing transitions */
  struct shift_api_transition** ins;	/* List of ingoing transitions */
} shift_api_mode;


typedef struct shift_api_exported_event {
  char* name;				/* SHIFT exported event name */
  struct shift_api_transition** trans;	/* List of exporting transitions */
  int backlink_var_offset;              /* No idea what this does */
} shift_api_exported_event;


typedef struct shift_api_external_event {
  char* name;				/* SHIFT external event name */
  int global;				/* 0 or 1 */
  shift_api_connection_kind kind;	/* Kind of connection */
  char* var_name;			/* External variable name */
  int sync_var_offset;                  /* Some idea */
} shift_api_external_event;


typedef struct shift_api_transition {
  shift_api_mode* source;		/* Source mode */
  shift_api_mode* dest;			/* Destination mode */
  shift_api_exported_event** exports;	/* List of local events */
  shift_api_external_event** externs;	/* List of external events */
} shift_api_transition;


typedef struct shift_api_variable {
  char* name;			/* SHIFT name */
  shift_api_variable_kind kind;	/* Kind of variable */
  struct shift_api_type* type;	/* Variable type */
  shift_api_mode_kind possible;	/* Possible modes: 
				 *   CONSTANT means constant only;
				 *   ALGEBRAIC means constant or algebraic;
				 *   DIFFERENTIAL means constant, algebraic,
				 *   or differential.
				 */
} shift_api_variable;


typedef struct shift_api_user_type {
  char* name;				 /* Name of the type */
  shift_api_exported_event** events;	 /* List of exported events */
  shift_api_mode** modes;		 /* List of discrete modes */
  struct shift_api_user_type* parent;	 /* Parent type */
  struct shift_api_user_type** children; /* List of children types */
  shift_api_variable** variables;	 /* List of local variables */
} shift_api_user_type;


typedef struct shift_api_builtin_type {
  shift_api_builtin_kind kind;		/* Kind of builtin type */
  struct shift_api_type* element_type;	/* In case of set or array */
} shift_api_builtin_type;


typedef struct shift_api_type {
  shift_api_type_kind kind;		/* BUILTIN or USER type */
  union {
    shift_api_builtin_type* builtin_t;	/* Builtin type */
    shift_api_user_type* user_t; 	/* User type */
  } t;
} shift_api_type;


typedef struct shift_api_instance {
  shift_api_user_type* type;	/* Type of the instance */
  int id;			/* Unique id number within the type */
} shift_api_instance;


typedef struct shift_api_builtin_value {
  shift_api_builtin_kind kind;			/* Kind of value */
  shift_api_type* element_type;			/* In case of array or set */
  union {
    double number_v;				/* Value of a number */
    char* symbol_v;				/* Value of a symbol */
    int logical_v;				/* Value of a logical */
    struct shift_api_set_value* set_v;		/* Value of a set */
    struct shift_api_array_value* array_v;	/* Value of an array */
  } v;
} shift_api_builtin_value;


typedef struct shift_api_set_value {
  shift_api_type_kind kind;		/* BUILTIN type or USER type */
  int size;				/* Size of the set */
  union {
    shift_api_builtin_value** values_e;	/* List of values of a builtin type */
    shift_api_instance** instances_e;	/* List of instances of a user type */
    void **unknown_v;
    void **foreign_v;
  } e;
} shift_api_set_value;


typedef struct shift_api_array_value {
  shift_api_type_kind kind;		/* BUILTIN type or USER type */
  int size;				/* Size of the array */
  union {
    shift_api_builtin_value** values_e;	/* List of values of a builtin type */
    shift_api_instance** instances_e;	/* List of instances of a user type */
    void **unknown_v;
    void **foreign_v;
  } e;
} shift_api_array_value;


typedef struct shift_api_value {
  shift_api_type_kind kind;		/* BUILTIN type or USER type */
  union {
    shift_api_builtin_value* value_v;	/* Value of a builtin type */
    shift_api_instance* instance_v;	/* Instance of a user type */
    void* unknown_v;
    void* foreign_v;
  } v;
} shift_api_value;



typedef struct shift_api_component_value {
  shift_api_instance * instance;
  shift_api_value ** values;
} shift_api_component_value;


#endif /* SHIFT_API_TYPES_H */
