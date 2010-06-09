/* -*-Mode: C -*- */

/* analyzer.c -- */
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
#ifndef analyzer_i
#define analyzer_i

#include <stdio.h>
#include <stdarg.h>
#include "scanner.h"
#include "analyzer.h"


node * Equations;
node * equation_array[50];
int total_equations;
int debug_msg = 0;

void
fatal_error(char* msg, ...)
{
  va_list format_args;

  va_start(format_args, msg);
  fprintf(stderr, "mat2scalar: error: ");
  vfprintf(stderr, msg, format_args);
  va_end(format_args);
  putc('\n', stderr);
  exit(1);
}


int
main(int argc, char** argv)
{

 
  char filename[30];
  int i;
  FILE * file;

  if (argc < 2) 
    {
      fatal_error("no matrix file specified.");
    }
   
  strcpy(filename, argv[1]);

  load_info(filename);		/* This makes the lexer read the matrix file
				 * and convert it into tokens, which yyparse()
				 * below will use
				 */

  yyparse();

  if (PRINT_MSG)
    print_tree(Equations, 1);
  total_equations = 0;

  insert_into_array(Equations);

  for (i = 0; i < total_equations; i++)
    {
#if PRINT_MSG
      printf( "Equation #%d\n", i );
#endif
      check_equations(equation_array[i]);
    }

  for (i = 0; i < total_equations; i++)
    {
#if PRINT_MSG
      printf( "Equation #%d\n", i );
#endif
      output_scalar(equation_array[i]);
    }

  file = fopen("results.mx", "w");
  fprintf(file, "*/\n");

  for (i = total_equations-1; i >= 0 ; i--)
    {
      matrix_to_file(file, equation_array[i]);
    }
  fprintf(file, "%c%c\n", '/', '*'); /* Stupid, but otherwis it messes
				     * up editor coloring
				     */
  printf("Wrote equations to file...\n");

  return 1;
}


void
matrix_to_file(FILE * f, node * eq)
{

  int i,j,rows, columns;

  rows = eq->u.e.matrix->u.m.rows;
  columns = eq->u.e.matrix->u.m.columns;

  for (i = 0; i< rows; i++)
    for (j = 0; j < columns; j++)
      fprintf(f, "%s;\n", eq->u.e.matrix->u.m.matrix[i][j]);
}


void
output_scalar(node * eq)
{

  node * mat1, *mat2;

  switch (tag(eq))
    {
    case 1001:

#if PRINT_MSG
      printf( "ADD " );
#endif
    
      if (tag(arg1(eq)) == MATRIX)
	{
	  mat1 = arg1(eq);  
	}
      else 
	{
	  output_scalar(arg1(eq));
	  mat1 = eq->u.e.arg1->u.e.matrix;
	}

      if (tag(arg2(eq)) == MATRIX) 
	{
	  mat2 = arg2(eq);  
	}
      else 
	{
	  output_scalar(arg2(eq));
	  mat2 = eq->u.e.arg2->u.e.matrix;
	}
      eq->u.e.matrix = compute_matrix_addition(mat1, mat2);
      break;

    case 1002:

#if PRINT_MSG
      printf( "MULTIPLY " );
#endif

      if (tag(arg1(eq)) == MATRIX) 
	{
	  mat1 = arg1(eq);  
	}
      else 
	{
	  output_scalar(arg1(eq));
	  mat1 = eq->u.e.arg1->u.e.matrix;
	}

      if (tag(arg2(eq)) == MATRIX) 
	{
	  mat2 = arg2(eq);  
	}
      else 
	{
	  output_scalar(arg2(eq));
	  mat2 = eq->u.e.arg2->u.e.matrix;
	}

      eq->u.e.matrix = compute_matrix_multiplication(mat1, mat2);
      break;

    case 1003:

#if PRINT_MSG
      printf( "TRANSPOSE " );
#endif
      eq->u.e.matrix = compute_transpose(eq->u.e.arg1);
      break;

    case 1004:                         /* PRIME CASE */
#if PRINT_MSG
      printf( "PRIME " );
#endif
      eq->u.e.matrix = compute_prime_matrix(eq->u.e.arg1);
      break;

    case 1005:                          /* SUBTRACT CASE */
#if PRINT_MSG
      printf( "SUBTRACT " );
#endif
      if (tag(arg1(eq)) == MATRIX) 
	{
	  mat1 = arg1(eq);  
	}
      else 
	{
	  output_scalar(arg1(eq));
	  mat1 = eq->u.e.arg1->u.e.matrix;
	}

      if (tag(arg2(eq)) == MATRIX) 
	{
	  mat2 = arg2(eq);  
	}
      else 
	{
	  output_scalar(arg2(eq));
	  mat2 = eq->u.e.arg2->u.e.matrix;
	}
      eq->u.e.matrix = compute_matrix_subtraction(mat1, mat2);
      break;

    case 1007:                       /* EQUAL CASE */
#if PRINT_MSG
      printf( "EQUAL " );
#endif
      if (tag(arg1(eq)) == MATRIX) 
	{
	  mat1 = (arg1(eq));
#if PRINT_MSG
	  printf( "This\n" );
#endif
	} 
      else if (tag(arg1(eq)) == PRIME_EXP) 
	{
	  output_scalar(arg1(eq));
	  mat1 = eq->u.e.arg1->u.e.matrix;
	}        
      else
	{
	  fatal_error("left hand side of the equation must be a matrix");
	}

      if (tag(arg2(eq)) == MATRIX) 
	{
	  mat2 = arg2(eq);
	}
      else 
	{
	  output_scalar(arg2(eq));
	  mat2 = eq->u.e.arg2->u.e.matrix;
	}
      eq->u.e.matrix = compute_equations(mat1, mat2);
      break;

    case 1008:                         /* PRIME CASE */
#if PRINT_MSG
      printf( "NEGATIVE " );
#endif
      eq->u.e.matrix = compute_negatives(eq->u.e.arg1);
      break;
    }
}



node *
compute_matrix_addition(node * n1, node * n2) 
{

  int i,j, k,l;
  int columns;
  int rows;
  char result[RESULT];

  node * n;


  rows = n1->u.m.rows;
  columns = n1->u.m.columns;
  
  n = (node *)malloc(sizeof(node));

  n->tag = MATRIX;
  n->u.m.rows = rows;
  n->u.m.columns = columns;
#if PRINT_MSG
  printf( "Matrix string %s---\n", n1->u.m.matrix[1][0].entry );
#endif

  for (i = 0; i <rows; i++) 
    {
      for (j = 0; j < columns; j++) 
	{
	  result[0] = '\0';
	  strcat(result, "(");
	  strcat(result, n1->u.m.matrix[i][j].entry);
	  strcat(result, " + ");
	  strcat(result, n2->u.m.matrix[i][j].entry);
	  strcat(result, ")");
	  strcpy(n->u.m.matrix[i][j].entry, result);
#if PRINT_MSG
	  printf( "Addition result is %s---\n", result );
#endif
	}
    }
  return n;
}


node *
compute_matrix_subtraction(node * n1, node * n2) 
{
  int i,j, k,l;
  int columns;
  int rows;
  char result[RESULT];

  node * n;


  rows = n1->u.m.rows;
  columns = n1->u.m.columns;
  
  n = (node *)malloc(sizeof(node));

  n->tag = MATRIX;
  n->u.m.rows = rows;
  n->u.m.columns = columns;

#if PRINT_MSG
  printf( "Matrix string %s---\n", n1->u.m.matrix[1][0].entry );
#endif

  for (i = 0; i <rows; i++) 
    {
      for (j = 0; j < columns; j++) 
	{
	  result[0] = '\0';
	  strcat(result, "(");
	  strcat(result, n1->u.m.matrix[i][j].entry);
	  strcat(result, " - ");
	  strcat(result, n2->u.m.matrix[i][j].entry);
	  strcat(result, ")");
	  strcpy(n->u.m.matrix[i][j].entry, result);

#if PRINT_MSG
	  printf( "Result is %s\n", result );
#endif
	}
    }
  return n;
}


node *
compute_equations(node * n1, node * n2) 
{

  int i,j, k,l;
  int columns;
  int rows;
  char result[RESULT];

  node * n;


  rows = n1->u.m.rows;
  columns = n1->u.m.columns;
  
  n = (node *)malloc(sizeof(node));

  n->tag = MATRIX;
  n->u.m.rows = rows;
  n->u.m.columns = columns;

  for (i = 0; i <rows; i++) 
    {
      for (j = 0; j < columns; j++) 
	{
	  result[0] = '\0';
     
	  strcat(result, n1->u.m.matrix[i][j].entry);
	  strcat(result, "  =  ");
	  strcat(result, n2->u.m.matrix[i][j].entry);
	  strcpy(n->u.m.matrix[i][j].entry, result);

#if PRINT_MSG
	  printf( "Result is %s\n", result );
#endif
	}
    }
  return n;
}


node *
compute_matrix_multiplication(node * n1, node * n2) 
{
  int i,j, k,l;
  int columns_one, columns_two;
  int rows_one, rows_two;
  char result[RESULT];
  char result2[RESULT];

  node * n;    


  rows_one = n1->u.m.rows;
  columns_one = n1->u.m.columns;
  
  rows_two = n2->u.m.rows;
  columns_two = n2->u.m.columns;
 
  n = (node *)malloc(sizeof(node));

  n->tag = MATRIX;
  n->u.m.rows = rows_one;
  n->u.m.columns = columns_two;
  

  for (i = 0; i <rows_one; i++) 
    {
      for (j = 0; j < columns_two; j++) 
	{
	  result[0] = '\0';
	  result2[0] = '(';
	  result2[1] = '\0';    
	  for (k = 0; k< columns_one; k++) 
	    {
	      strcat(result, "(");
	      strcat(result, n1->u.m.matrix[i][k].entry);
	      strcat(result, " * ");
	      strcat(result, n2->u.m.matrix[k][j].entry);
	      strcat(result, ")");
	      strcat(result,"+");    
	    }
	  result[(strlen(result)-1)] = '\0';
	  strcat(result2, result);
	  strcat(result2, ")");
#if PRINT_MSG
	  printf( "Result is %s\n", result2 );
#endif
	  strcpy(n->u.m.matrix[i][j].entry, result2);
	}
    }
  return n;
}


node *
compute_negatives(node * n1) 
{

  int i,j, rows, columns;
  char result[RESULT];
  node * n;

  rows = n1->u.m.rows;
  columns = n1->u.m.columns;
  n = (node *)malloc(sizeof(node));
  n->tag = MATRIX;
  n->u.m.rows = rows;
  n->u.m.columns = columns;

  for (i = 0; i <rows; i++) 
    {
      for (j = 0; j < columns; j++) 
	{
	  result[0] = '\0';
	  strcat(result, " -");
	  strcat(result, n1->u.m.matrix[i][j].entry);
	  strcpy(n->u.m.matrix[i][j].entry, result);
#if PRINT_MSG
	  printf( "Result is %s\n", result );
#endif
	}
    }
  return n;
}


node *
compute_transpose(node * n1) 
{
  
  int i,j, k,l;
  int columns;
  int rows;
  char result[RESULT];

  node * n;


  rows = n1->u.m.rows;
  columns = n1->u.m.columns;
  
  n = (node *)malloc(sizeof(node));

  n->tag = MATRIX;
  n->u.m.rows = columns;
  n->u.m.columns = rows;


  for (i = 0; i < rows; i++) 
    {
      for (j = 0; j < columns; j++) 
	{
	  result[0] = '\0';
	  strcat(result, n1->u.m.matrix[i][j].entry);
     
	  strcpy(n->u.m.matrix[j][i].entry, result);
#if PRINT_MSG
	  printf( "Result is %s\n", result );
#endif
	}
    }
  return n;
}


node *
compute_prime_matrix(node * n1) 
{
  
  int i,j, k,l;
  int columns;
  int rows;
  char result[RESULT];

  node * n;


  rows = n1->u.m.rows;
  columns = n1->u.m.columns;
  
  n = (node *)malloc(sizeof(node));

  n->tag = MATRIX;
  n->u.m.rows = rows;
  n->u.m.columns = columns;


  for (i = 0; i <rows; i++) 
    {
      for (j = 0; j < columns; j++) 
	{
	  result[0] = '\0';
	  strcat(result, n1->u.m.matrix[i][j].entry);
	  strcat(result, "'");
	  strcpy(n->u.m.matrix[i][j].entry, result);
#if PRINT_MSG
	  printf( "Result is %s\n", result );
#endif
	}
    }
  return n;

}


void
check_equations(node * eq) 
{

  int m1_columns, m1_rows,m2_columns,m2_rows;

 
  switch (tag(eq)) 
    {
    case 1000:
#if PRINT_MSG
      printf( "UNIT" );
#endif
      check_equations(arg1(eq));
      if (arg2(eq) !=  nil)
	check_equations(arg2(eq));
      break;

    case 1001:
#if PRINT_MSG
      printf( "ADD " );
#endif
   
      if (tag(arg1(eq)) == MATRIX) 
	{
	  m1_columns = (matrix_columns(arg1(eq)));
	  m1_rows = (matrix_rows(arg1(eq))); 
	}
      else 
	{
	  check_equations(arg1(eq)); 
	  m1_columns = eq->u.e.arg1->u.e.x_size;
	  m1_rows = eq->u.e.arg1->u.e.y_size;
	}
    
      if (tag(arg2(eq)) == MATRIX) 
	{
	  m2_columns = (matrix_columns(arg2(eq)));
	  m2_rows = (matrix_rows(arg2(eq))); 
	}
      else 
	{
	  check_equations(arg2(eq)); 
	  m2_columns = eq->u.e.arg2->u.e.x_size;
	  m2_rows = eq->u.e.arg2->u.e.y_size;
	} 

      if (operation_ok(ADD_EXP, m1_columns, m1_rows, m2_columns, m2_rows)) 
	{
#if PRINT_MSG
	  printf( "Valid matrix addition seen\n" );
#endif
	  eq->u.e.x_size = m1_columns;
	  eq->u.e.y_size = m1_rows;
	}
      else 
	{
	  fatal_error("illegal matrix addition found\n");   
	}
      break;

    case 1002:
#if PRINT_MSG
      printf( "MULTIPLY " );
#endif
      if (tag(arg1(eq)) == MATRIX) 
	{
	  m1_columns = (matrix_columns(arg1(eq)));      
	  m1_rows = (matrix_rows(arg1(eq))); 
	}
      else 
	{
	  check_equations(arg1(eq)); 
	  m1_columns = eq->u.e.arg1->u.e.x_size;
	  m1_rows = eq->u.e.arg1->u.e.y_size;
	}
    
      if (tag(arg2(eq))  == MATRIX) 
	{
	  m2_columns = (matrix_columns(arg2(eq)));
	  m2_rows = (matrix_rows(arg2(eq))); 
	}
      else 
	{
	  check_equations(arg2(eq)); 
	  m2_columns = eq->u.e.arg2->u.e.x_size;
	  m2_rows = eq->u.e.arg2->u.e.y_size;
	} 

      if (operation_ok(MULTI_EXP, m1_columns, m1_rows, m2_columns, m2_rows)) 
	{
#if PRINT_MSG
	  printf( "Valid matrix multiplication seen\n" );
#endif
	  eq->u.e.x_size = m2_columns;
	  eq->u.e.y_size = m1_rows;
	}
      else 
	{
	  fatal_error("illegal matrix multiplication found.");
	}
      break;

    case 1003:
#if PRINT_MSG
      printf( "TRANSPOSE " );
#endif
      if (tag(arg1(eq)) == MATRIX) 
	{
	  eq->u.e.y_size = (matrix_columns(arg1(eq)));
	  eq->u.e.x_size = (matrix_rows(arg1(eq))); 
	} 
      break;

    case 1004:
#if PRINT_MSG
      printf( "PRIME " );
#endif
      if (tag(arg1(eq)) == MATRIX) 
	{
	  eq->u.e.x_size = (matrix_columns(arg1(eq)));
	  eq->u.e.y_size = (matrix_rows(arg1(eq))); 
	}
      else 
	{
	  printf("We've got a prime of a non-matrix here...\n");
	}
     
      /*     eq->u.e.x_size = (matrix_columns(arg1(eq))); */
      /*     eq->u.e.y_size = (matrix_rows(arg1(eq))); */
      break;

    case 1005:
#if PRINT_MSG
      printf( "SUBTRACT " );
#endif
      if (tag(arg1(eq)) == MATRIX) 
	{
	  m1_columns = (matrix_columns(arg1(eq)));
	  m1_rows = (matrix_rows(arg1(eq))); 
	}
      else 
	{
	  check_equations(arg1(eq)); 
	  m1_columns = eq->u.e.arg1->u.e.x_size;
	  m1_rows = eq->u.e.arg1->u.e.y_size;
	}
    
      if (tag(arg2(eq)) == MATRIX) 
	{
	  m2_columns = (matrix_columns(arg2(eq)));
	  m2_rows = (matrix_rows(arg2(eq))); 
	}
      else 
	{
	  check_equations(arg2(eq)); 
	  m2_columns=eq->u.e.arg2->u.e.x_size;
	  m2_rows = eq->u.e.arg2->u.e.y_size;
	} 

      if (operation_ok(ADD_EXP, m1_columns, m1_rows, m2_columns, m2_rows)) 
	{  /* ADD and SUB are same */
#if PRINT_MSG
	  printf( "Valid matrix addition seen\n" );
#endif
	  eq->u.e.x_size = m1_columns;
	  eq->u.e.y_size = m1_rows;
	}
      else 
	{
	  fatal_error("illegal matrix subtraction found.");
	}
      break;

    case 1007:
#if PRINT_MSG
      printf( "EQUAL " );
#endif
      if (tag(arg1(eq)) == MATRIX) 
	{
	  m1_columns = (matrix_columns(arg1(eq)));
	  m1_rows = (matrix_rows(arg1(eq))); 
	} 
      else if (tag(arg1(eq)) == PRIME_EXP) 
	{ 
	  check_equations(arg1(eq));
	  m1_columns = eq->u.e.arg1->u.e.x_size;
	  m1_rows = eq->u.e.arg1->u.e.y_size;
	}        
      else
      
	{
	  fatal_error("left hand side of the equation must be a matrix");
	}
      if (tag(arg2(eq)) == MATRIX) 
	{
	  m2_columns = (matrix_columns(arg2(eq)));
	  m2_rows = (matrix_rows(arg2(eq))); 
	}
      else 
	{
	  check_equations(arg2(eq)); 
	  m2_columns = eq->u.e.arg2->u.e.x_size;
	  m2_rows = eq->u.e.arg2->u.e.y_size;
	} 
      if (operation_ok(EQ_EXP, m1_columns, m1_rows, m2_columns, m2_rows)) 
	{  /* ADD and EQUAL are same */
#if PRINT_MSG
	  printf( "Valid matrix equality seen\n" );
#endif
	  eq->u.e.x_size = m1_columns;
	  eq->u.e.y_size = m1_rows;
	}
      else 
	{
	  fatal_error("illegal matrix equality found.");
	}
      break;

    case 1008:
#if PRINT_MSG
      printf( "NEGATIVE " );
#endif
      if (tag(arg1(eq)) == MATRIX) 
	{
	  eq->u.e.x_size = (matrix_columns(arg1(eq)));
	  eq->u.e.y_size = (matrix_rows(arg1(eq))); 
	}
      else 
	{
	  check_equations(arg1(eq));  
	  eq->u.e.x_size = (matrix_columns(arg1(eq)));
	  eq->u.e.y_size = (matrix_rows(arg1(eq))); 
	}
      break;

    default: 
      printf("UNDEFINED %d---",tag(eq));
    }
}


int
operation_ok(int operation, int column1, int row1, int column2, int row2) 
{
  int OK = 0;

  if (operation  == ADD_EXP || operation == SUB_EXP || operation == EQ_EXP)
    {
      if ((column1 == column2) && (row1 == row2))
	OK = 1;
    }
  else if ( (operation == MULTI_EXP))
    if (column1 == row2)
	OK = 1;
  
  if (OK == 0) 
    {
#if PRINT_MSG
      printf("The sizes of the matrices have been found wrong.\n");
      printf("Please check and try again.\n" );
      printf("This is what we have:");
      printf(" column1 = %d, row1 = %d, column2 = %d, row2 = %d\n",
	     column1, row1, column2, row2);
#endif
    }
  return OK;
}


void
insert_into_array(node * tree) 
{
  if (tag(tree) == AND_EXP) 
    {
      insert_into_array(arg1(tree)); 
      insert_into_array(arg2(tree));
    }
  else if (tag(tree) == EQ_EXP) 
      {
	equation_array[total_equations] = tree;
	total_equations++;
      }
}


void
print_tree(node * top, int space) 
{
  int i;
  
  for (i = 0; i< space; i++)
    printf(" ");
 
  print_type(top);
  
  printf("\n");
  if (tag(top) != MATRIX) 
    {
      if (arg1(top) != nil)
	print_tree(arg1(top), space+1);
      if (arg2(top) != nil)
	print_tree(arg2(top), space+1);
    }
}


void
print_type(node * eq) 
{
  switch (tag(eq)) 
    {
    case 1000:
      printf("UNIT ");
      break;
    case 1001:
      printf("ADD ");
      break;
    case 1002:
      printf("MULTIPLY ");
      break;
    case 1003:
      printf("TRANSPOSE ");
      break;
    case 1004:
      printf("PRIME ");
      break;
    case 1005:
      printf("SUBTRACT ");
      break;
    case 1006:
      printf("AND ");
      break;
    case 1007:
      printf("EQUAL ");
      break;
    case 1008:
      printf("NEGATIVE ");
      break;
    case 257:
      printf("MATRIX ");
      printf("of size -%dx%d-", eq->u.m.rows, eq->u.m.columns); 
      break;
    default: 
      printf("UNDEFINED %d---",tag(eq));
    }
}

#endif /* analyzer_i */

/* end of file -- analyzer.c -- */
