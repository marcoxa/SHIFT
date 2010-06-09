/* -*- Mode: C -*- */

/* shift_debug.h -- */

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

#ifndef SHIFT_DEBUG_H
#define SHIFT_DEBUG_H


#include <shift_config.h>
#include <gc_support.h>
#include <shifttypes.h>
#include <collection.h>
#include <stdio.h>
#include <string.h>
#include "shift_api_types.h"


#define CMDBUFSIZE 1000
#define OUTBUFSIZE 20000
#define NAMESIZE 1000
#define LISTSIZE 2000
#define IDSIZE 8 /* Max eight digits for component id */


extern long nb_consecutive_transitions;
extern long shift_zeno_threshold;


typedef enum shift_debug_cmd_name {
  empty_e,
  breakcomp_e,
  breaktype_e,
  unbreakcomp_e,
  untracecomp_e,
  unbreaktype_e,
  untracetype_e,
  cont_e,
  ctracecomp_e,
  ctracetype_e,
  db_ctracecomp_e,
  db_ctracetype_e,
  delete_e,
  descd_e,
  desct_e,
  dtracecomp_e,
  dtracetype_e,
  db_dtracecomp_e,
  db_dtracetype_e,
  exit_e,
  quit_e,
  help_e,
  parent_e,
  children_e,
  current_e,
  discrete_e,
  localvar_e,
  globalvar_e,
  globalval_e,
  list_e,
  listsub_e,
  listtypes_e,
  print_e,
  printglobal_e,
  setfile_e,
  setprecision_e,
  setzeno_e,
  setFS_e,
  setFS2_e,
  setSOR_e,
  setEOR_e,
  status_e,
  stop_e,
  tracetype_e,
  tracecomp_e,
  set_docfile_e,
  set_logdir_e,
 invalid_command_e
} commandName; 


typedef enum {
  none_e = 0,
  unbreak_e,
  untrace_e,
  diff_e,
  desc_e,
  both_e,
  break_e
} traceType;


struct ComponentList;


typedef struct ComponentList {
  Component* thisComp;
  struct ComponentList* nextComp;
  struct ComponentList* prevComp;
} ComponentList;


typedef struct TraceStatus {
  int returnControl; 	/* TRUE returns control, FALSE cheks inputs */
  int stopInN; 	/* Will stop simulation after stopInN  clicks */
  int lastLength; 	/* sim run lastLength clicks */
  struct ComponentList* timeTraced; /* List of time-traced components */
  FILE* outfile; 	/* File in which info is printed */
} TraceStatus;


typedef enum {
  NOT_P,
  TYPE_P,
  COMP_P,
  BOTH_P,
  UNKNOWN_P
} shift_debug_break_kind;


typedef struct shift_debug_breakpoint {
  shift_api_instance* instance;		/* Instance participating in
					   the collective transition */
  shift_api_transition* trans;		/* Transition fired */
  shift_debug_break_kind break_flag;	/* Breaking status */
  shift_debug_break_kind trace_flag;	/* Tracing status */
  struct shift_debug_breakpoint* next;	/* Next breakpoint */
} shift_debug_breakpoint;


typedef struct shift_debug_function {
  void (*function)(shift_debug_breakpoint*, int, int);
} shift_debug_function;


#define TRUE 1
#define FALSE 0


extern int shift_debug_precision;
extern TraceStatus* Tracer;
extern char* trace_file_name;
extern shift_debug_function* shift_api_callback;


extern int getCommandLine(char*);
extern int getToken(char*, char*, char*, char*);
extern commandName getCommand(char*);
extern void dbgControl();
extern char* display(Component*);
extern char* displayAll(Component*);
extern void initDebug();
extern void userInteract();
extern char* printNumberVariable(Component*, VariableDescriptor*, char*);
extern char* printNumber(double*, char*);
extern char* printComponent(Component**, char*);
extern char* printSet(GenericSet**, char*);
extern char* printObject(void*,  TypeDescriptor*, char *);
extern char* printArray(GenericArray**, char*);
extern void userInteract();
extern void shift_debug_prompt();
extern void btf();
extern char* printCollective(int sequence);
extern VariableDescriptor** constructAttrList(ComponentTypeDescriptor* myT,
					      char* rS);

extern char* printGlobal();
extern char* printSymbol(int* x, char*a);
extern char* getCompId(char* input, int* index);
extern char* printAttrs(Component* c, VariableDescriptor** myPV, char* a);
extern void setFile(char* filename);
extern void printCmdLineHelp();


#endif /* SHIFT_DEBUG_H */

/* end of file -- shift_debug.h -- */
