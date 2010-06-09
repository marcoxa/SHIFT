/* -*- Mode: C -*- */

/* shift_db_debug.h -- */

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

/* -*- Mode: C -*- */

/* shift_db_debug.h --
 * Debug aux functions
 */
 
#ifndef SHIFT_DB_DEBUG_H
#define SHIFT_DB_DEBUG_H

typedef struct db_command{
  FILE *fd;
  char cmd[32];
  char type[256];
  char* tail;
  struct db_command* next;
} *db_cmd_pt,db_cmd;

char *FieldSeperator;
char *FieldSeperator2;
char *EOR;
char *SOR;


/* Takes a pointer to node in DB_cmd_List.  Flushes Tracer->outfile,
   temporarily assigns outfile to be that of proper log file ( stored
   in debug command list)  call printing for transition or component
   based on "command".  (DB_Print_Component, DB_Print_Transition).
   restores Tracer->outfile
   */
void shift_DB_Print_Command (db_cmd *dbg_cmd);




/* given head of singly linked list of debug commands traverse
list, call shift_DB_Print_Command */
void shift_DB_Traverse_CmdList();




/* inserts a new node at end of  debug command list.  Called in
   dispachRequest.  format of node above in db_command
   in directory "log0" create a unique logfile based on debug command
   and user type debug command is opperationg on.
   */
db_cmd*  shift_DB_New_cmd(db_cmd* DB_cmd_list,\
		 char *cmd,\
		 char *rS);




/* Not used at this time. Will be used if format changes.*/
/* allows shift_DB_New_cmd to print a header at creation time of
   logfile. */
int  shift_db_print_header(db_cmd*);



/* Do what we need to do to start this thing.  BIG ONE-
   build log dirctory and move logdirs up "log0-9" up by 1 for ever
   run of simulation.  That way runs of simulation don't clober log
   dir.
   */
db_cmd* shift_DB_initalize();


/* to allow for for format changes */
int shift_DB_setFieldSeperator( char* string );
int shift_DB_setFieldSeperator2( char* string );
int shift_DB_setEOR( char* string );
int shift_DB_setSOR( char* string );

/* In the building of the command list the component ID can be 
   hidden in the "tail" section of the db_command struct */
int get_component_id(char* tail_of_command);



int shift_DB_Print_Transition( db_cmd* db_cmd, int instance );
int shift_DB_Print_Component(db_cmd* dbg_cmd, int instance );
int shift_DB_Print_Transitions( db_cmd* db_cmd);
int shift_DB_Print_Components(db_cmd* dbg_cmd );

#endif /* SHIFT_DB_DEBUG_H */
/* end of file -- shift_db_debug.c -- */










































