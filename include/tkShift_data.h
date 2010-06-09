/* -*- Mode: C -*- */

/* Gui_info.c -- */

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

#ifndef TKSHIFT_DATA_H
#define TKSHIFT_DATA_H

#include "shift_api_info.h"
#include "shift_api_types.h"
#include "shift_api_internal.h"

char* GUI_listComps(char* typeName,char * buffer);
int GUI_get_component_values(char * typeName, int id, char *);
int GUI_print_variable_value(shift_api_value * value, char *);
int GUI_printBuiltin(shift_api_builtin_value * builtin_value, char *);
int GUI_printUser(shift_api_instance * instance, char *);
int GUI_printNumber(double number, char *);
int GUI_printSymbol(char * symbol, char *);
int GUI_printLogical(int logical, char *);
int GUI_printArray(shift_api_array_value * array, char *);
int GUI_printSet(shift_api_set_value * set, char *);
int GUI_printArrayValues(shift_api_builtin_value ** values, char *);
int GUI_printArrayInstances(shift_api_instance ** instances, char *);
int GUI_printSetValues(shift_api_builtin_value ** values, char *);
int GUI_printSetInstances(shift_api_instance ** instances, char *);
int GUI_get_variable_value(char *typeName, int id, 
		    char *variableName, char *);
char * find_multiple_instance_information(Canvas_Data * data,char *);
char * find_single_instance_information(Canvas_Data * data,char *);
     
int get_multiple_instance_animation_info(shift_api_instance ** instances, 
					   Canvas_Data * data,char *);


int make_header_information(Canvas_Data * data,char *);
int print_animation_information(shift_api_instance * instance, 
			    char * typeName,
			    char * XvariableName,
			    char * YvariableName,
			    char * length,
			    char * width,char *);

int
get_single_instance_animation_information(shift_api_instance * instance, 
					  Canvas_Data * data,char *);

int get_single_instance_set_animation_info(Canvas_Data * data,char *);

void
GUI_print_breakpoints(shift_debug_breakpoint* bp_list,
			      int tnb,
			      int tclick);


void
GUI_print_breakpoint(char *,
			     shift_debug_breakpoint* bp,
			     int tnb);

#endif
