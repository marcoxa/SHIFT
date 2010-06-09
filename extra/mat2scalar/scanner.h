/* -*- Mode: C -*- */

/* scanner.h -- */
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

#ifndef scanner_h
#define scanner_h

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define PRINT_MSG 0
 
#define UNIT       1000 
#define ADD_EXP    1001
#define MULTI_EXP  1002
#define TRANS_EXP  1003
#define PRIME_EXP  1004
#define SUB_EXP    1005
#define AND_EXP    1006
#define EQ_EXP     1007
#define NEGAT_EXP  1008

#define WIDE 80
#define DEEP 8
#define MAX 8
#define nil 0
#define MATRIX 257
#define RESULT 10000
#define M2S_MAX_STACK 100


/* Declare global variables */

extern int debug_msg;
extern int end_of_last_token;
extern int current_insert_position;

typedef struct stack_data
{
  int token;
  struct node * lval;
} stack_data;

#ifndef scanner_i

extern stack_data * stack[];

#endif /* scanner_i */

/*** End of global variables ***/



typedef struct matrix_entry
{
  char entry[RESULT];
} matrix_entry;


typedef struct node
{
  int tag;   
  char * name;
  char * info;
 
  union
  {
    /* Matrix */
    struct
    {
      int columns;
      int rows;
      matrix_entry matrix[MAX][MAX];
    } m;

    /* ID */

    struct
    {
      char * theSymbol;
    } s;

    struct tree_element
    {
      struct node * arg1;
      struct node * arg2;
      int x_size;
      int y_size;
      struct node * matrix;

    } e;
  } u;
} node;


#define tag(x)                 (x->tag)
#define name(x)                (x->name)
#define matrix_entry(x,i,j)    (x->u.m.matrix[i][j])
#define matrix_columns(x)      (x->u.m.columns)
#define matrix_rows(x)         (x->u.m.rows)
#define arg1(x)                (x->u.e.arg1)
#define arg2(x)                (x->u.e.arg2)
#define symbol(x)              (x->u.s.theSymbol)
#define info(x)                (x->info)

typedef node* yystype;
 

/* Declare all functions */

void initialize_array();
void process_array();
int find_next_token();
void grab_token(int);
int is_a_token(char);
void print_array();
void put_buffer_info_into_array(char * buffer, int number);
node * get_matrix(int);
void process_rows();
char * get_matrix_entry();
void insert_token(int token, node * meaning);
void process_matrix_rows(node * n, char * row, int r_num);
int load_info(char *);
node * make_node(int, node*, node*);

/*** End of function declaration ***/

#endif /* scanner_h */

/* end of file -- scanner.h -- */
