/* -*- Mode: C -*- */

/* scanner.c -- */
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
#ifndef scanner_i
#define scanner_i

#include "scanner.h"

char filename[25];

int end_of_last_token_X;
int end_of_last_token_Y;
int current_token_type=0;
int current_insert_position;
int total_tokens;

char data[WIDE][DEEP];
stack_data * stack[M2S_MAX_STACK];


/* load_info -- This function gets called from main. It is responsible
 * for parsing the input file and getting chunks of data out of it.  
 */

int
load_info(char * filename)
{

  FILE * f;
  int pos, line_number;
  char  c;
  char buffer[500];
  int i, index;


  current_insert_position = 0;   /* This is current index of the token array */
  initialize_array();
  line_number = 0;
  pos = 0;
  index = 0;

  f = fopen(filename, "r");  
#if PRINT_MSG
  printf(".%s.\n", filename );
#endif
  c = getc(f);


  while (c != EOF)
    {

      buffer[index] = c;
      index++;
      if (c == '\n')
	{
	  c = getc(f);
	  if ((c == '\n') || (c == EOF))
	    {
	      end_of_last_token_X = 0;
	      end_of_last_token_Y = 0;
	      current_token_type = 0;
	      put_buffer_info_into_array(buffer, index);
	      process_array();
	      index = 0;
	      buffer[0] = '\0'; /* Empty buffer */
	      initialize_array();
	    }
	  else
	    {
	      buffer[index] = c;
	      index++;
	    }
	}
#ifndef ACCHISTIAMOTUTTIPAZZI
      if (c == '\t')
	{
	  fprintf(stderr, "mat2scalar: tabs not accepted. Exiting...\n");
	  exit(1);
	}
#else
      if (c == '\t')
	{
	  buffer[index] = ' ';
	  index++;
	}
#endif /* ACCHISTIAMOTUTTIPAZZI */
  
      c = getc(f);
    }

  fclose(f);

#if 0
  for (i = 0; i < current_insert_position; i++)
    printf("In cell %d we have token %d\n", i, stack[i]->token);
#endif      

  total_tokens = current_insert_position;

  return 1;
}



/* process_array -- This function looks at the array and is
 * responsible for calling all the functions that extract the
 * information from the array
 */

void
process_array()
{

  int i, j;
  int line_total, this_line, pos;
  int tot_ops;

  line_total = 0;
  if (PRINT_MSG)
    print_array();

  /* This looks at all the lines of the array, and tells us the
   * maximum number of bars that it encountered.  That is the number
   * of matrices * 2
   */
  for (j = 0; j < DEEP; j++)
    {
      this_line = 0;
      for (i = 0; i < WIDE; i++)
	if (data[i][j] == '|')
	  this_line++;
      if (this_line > line_total)
	line_total = this_line;
    }

#if PRINT_MSG  
  printf("Total matrices * 2: %d\n", line_total);
#endif
  tot_ops = 0;

  for (j = 0; j < DEEP; j++)
    for (i = 0; i < WIDE; i++)
      switch (data[i][j])
	{
	case '*':
	case '+':
	case '-':
	case '~':
	case '=':
	case ';':
	case '(':
	case ')':
	  tot_ops++;
	  break;

	default:		/* Do nothing */
	  break;
	}

#if PRINT_MSG   
  printf("Total operation tokens: %d\n", tot_ops);
#endif
  pos = find_next_token();

  while (pos != -1)
    {
      grab_token(pos);

#if PRINT_MSG
      printf("Token taken!\n");
#endif

      pos = find_next_token();
    }
} /* process_array -- */


/* find_next_token -- */

int
find_next_token()
{
  int i;
  int column;

   
  int no_token_found;
#if PRINT_MSG
  printf( "End of last token is now %d.\n", end_of_last_token_X );
#endif

  if ((end_of_last_token_Y + 2) >= DEEP)
    {
      end_of_last_token_X++;
      end_of_last_token_Y = 0;
      i = end_of_last_token_Y;
    } 
  else
    {
      i = end_of_last_token_Y;
    }
  column = end_of_last_token_X;

  no_token_found = 0;

  while (no_token_found == 0)
    {
      if (is_a_token(data[column][i]) == 1)
	{
	  no_token_found = 1;
	  end_of_last_token_Y = i;

#if PRINT_MSG
	  printf("Seems like that %c is a token at column %d and row %d.\n",
		 data[column][i],
		 column,
		 i);
#endif

	}
      else
	{
	  if ((i + 1) < DEEP)
	    i++;
	  else
	    {
	      i = 0;
	      if (column < WIDE)
		column++;
	      else
		return -1;
	    }
	}
    }

  return column;
} /* find_next_token */


/* grab_token -- This should be rewritten! */

void
grab_token(int token_x_pos)
{

  node * matrix;

  if (current_token_type == 1)
    {
      matrix = get_matrix(token_x_pos);
      insert_token(MATRIX, matrix);
      current_token_type = -1;
    }
  else if (current_token_type == 2)
    {
      insert_token('-', NULL);
      end_of_last_token_X = token_x_pos;
      end_of_last_token_Y = end_of_last_token_Y + 1;
      current_token_type = -1;
    }
  else if (current_token_type == 3)
    {
      insert_token('+', NULL);
      end_of_last_token_X = token_x_pos;
      end_of_last_token_Y = end_of_last_token_Y + 1;
      current_token_type = -1; 
    }
  else if (current_token_type == 4)
    {
      insert_token('*', nil); 
      end_of_last_token_X = token_x_pos;
      end_of_last_token_Y = end_of_last_token_Y + 1;
      current_token_type = -1;
    }
  else if (current_token_type == 5)
    {
      insert_token('~', NULL); 
      end_of_last_token_Y = end_of_last_token_Y + 1;
      end_of_last_token_X = token_x_pos;
      current_token_type = -1;
    }
  else if (current_token_type == 6)
    {
      insert_token('=', NULL); 
      end_of_last_token_X = token_x_pos;
      end_of_last_token_Y = end_of_last_token_Y + 1;
      current_token_type = -1;
    }
  else if (current_token_type == 7)
    {
      insert_token(';', NULL); 
      end_of_last_token_X = token_x_pos;
      end_of_last_token_Y = end_of_last_token_Y + 1;
      current_token_type = -1;
    }
  else if (current_token_type == 8)
    {
      insert_token('\'', NULL); 
      end_of_last_token_Y = end_of_last_token_Y + 1;
      end_of_last_token_X = token_x_pos;
      current_token_type = -1;
    }
  else if (current_token_type == 9)
    {
      insert_token('(', NULL); 
      end_of_last_token_X = token_x_pos;
      end_of_last_token_Y = end_of_last_token_Y + 1;
      current_token_type = -1;
    }
  else if (current_token_type == 10)
    {
      insert_token(')', NULL); 
      end_of_last_token_Y = end_of_last_token_Y + 1;
      end_of_last_token_X = token_x_pos;
      current_token_type = -1;
    }
} /* grab_token */


/* insert_token -- This function gets a token and optionally a node as
 * arguments. It mallocs some space and puts the token on the stack of
 * tokens.  If the token is a matrix, the node holds the matrix
 * values.
 */

void
insert_token(int token, node * meaning)
{

  stack_data * s;
 
  s = (stack_data *)malloc(sizeof(stack_data));

  s->token = token;
  s->lval = meaning;

  stack[current_insert_position] = s;
  current_insert_position++;
} /* insert_token -- */


/* get_matrix -- start_x_pos is the column number */

node*
get_matrix(int start_x_pos)
{

  int i;
  int last, start_matrix, end_matrix;
  char  *row1, *row2, *row3, *row4, *row5, *row6, *row7;
  int start_y_pos;
 
  node * n;


  start_matrix = start_x_pos;

#if PRINT_MSG
  printf( "Start:%d\n", start_matrix );
#endif

  start_y_pos = 0;
  while (data[start_x_pos][start_y_pos] != '|')
    start_y_pos++;

  /* looking for the row number where the matrix starts */

  i = start_x_pos + 1;

  while (data[i][start_y_pos] != '|')
    i++;
 
  end_matrix = i;

#if PRINT_MSG
  printf( "End:%d\n", end_matrix );
#endif

  end_of_last_token_X = end_matrix + 1;

#if PRINT_MSG
  printf( "I have just made the end matrix equal %d\n", end_of_last_token_X );
#endif

  end_of_last_token_Y = 0;
  i = start_y_pos;
  last = 0;
  while (data[start_x_pos][i] == '|')
    {
      i++;
      last++;
    }

  row1 = (char *)malloc(end_matrix - start_matrix + 1);
  row2 = (char *)malloc(end_matrix - start_matrix + 1);
  row3 = (char *)malloc(end_matrix - start_matrix + 1);
  row4 = (char *)malloc(end_matrix - start_matrix + 1);
  row5 = (char *)malloc(end_matrix - start_matrix + 1);
  row6 = (char *)malloc(end_matrix - start_matrix + 1);
  row7 = (char *)malloc(end_matrix - start_matrix + 1);

  row1[0] = '\0';
  row2[0] = '\0';
  row3[0] = '\0';
  row4[0] = '\0';
  row5[0] = '\0';
  row6[0] = '\0';
  row7[0] = '\0';

  for (i = start_matrix + 1; i < end_matrix; i++)
    row1[i - start_matrix - 1] = data[i][start_y_pos];
  row1[end_matrix - start_matrix - 1] = '\0';

  if (last > 1)
    {
      for (i = start_matrix + 1; i < end_matrix; i++)
	row2[i - start_matrix - 1] = data[i][start_y_pos + 1];
      row2[end_matrix - start_matrix - 1] = '\0';
    }

  if (last > 2)
    {
      for (i = start_matrix + 1; i < end_matrix; i++)
	row3[i - start_matrix - 1] = data[i][start_y_pos + 2];
      row3[end_matrix - start_matrix - 1] = '\0';
    }

  if (last > 3)
    {
      for (i = start_matrix + 1; i < end_matrix; i++)
	row4[i - start_matrix - 1] = data[i][start_y_pos + 3]; 
      row4[end_matrix - start_matrix - 1] = '\0';

    }

  if (last > 4)
    {
      for (i = start_matrix + 1; i < end_matrix; i++)
	row5[i - start_matrix - 1] = data[i][start_y_pos + 4];
      row5[end_matrix - start_matrix - 1] = '\0';
    }

  if (last > 5)
    {
      for (i = start_matrix + 1; i < end_matrix; i++)
	row6[i - start_matrix - 1] = data[i][start_y_pos + 5];
      row6[end_matrix - start_matrix - 1] = '\0';
    }
  if (last > 6)
    {
      for (i = start_matrix + 1; i < end_matrix; i++)
	row7[i - start_matrix - 1] = data[i][start_y_pos + 6]; 
      row7[end_matrix - start_matrix - 1] = '\0';
    }



#if PRINT_MSG
  printf( "Row 1:%s,\nRow 2:%s,\nRow 3:%s,\nRow 4:%s.",
	  row1, row2, row3, row4 );
  printf( "Last = %d\n", last );
#endif

  n = (node *)malloc(sizeof(node));

  n->tag = MATRIX;
  n->u.m.rows = last;
  process_matrix_rows(n, row1, 0);
  if (last > 1)
    process_matrix_rows(n, row2, 1);
  if (last > 2)
    process_matrix_rows(n, row3, 2);
  if (last > 3)
    process_matrix_rows(n, row4, 3);
  if (last > 4)
    process_matrix_rows(n, row5, 4);
  if (last > 5)
    process_matrix_rows(n, row6, 5);
  if (last > 6)
    process_matrix_rows(n, row7, 6);
  return n;
} /* get_matrix -- */


void
process_matrix_rows(node * n, char * row, int r_num)
{

  int l, i, current_entry, pos, row_number;

  strcat(row, "#");
  l = strlen(row);
  row_number = r_num;
  current_entry = 0;
  i = 0;

#if PRINT_MSG
  printf( "This is my data:%s---\n", row );
#endif

  while (row[i] != '#')
    {

      pos = 0;
      while (isspace(row[i]))
	{
	  i++;
	}
  
      while (isalnum(row[i])
	     || row[i] == '\''
	     || row[i] == '-'
	     || row[i] == '*'
	     || row[i] == '('
	     || row[i] == ')'
	     || row[i] == '_'
	     || row[i] == '/'
	     || row[i] == '+')
	{
	  n->u.m.matrix[row_number][current_entry].entry[pos] = row[i];
	  pos++;
	  i++;
	}

      while (isspace(row[i]))
	{ 
	  i++;
	}
      n->u.m.matrix[row_number][current_entry].entry[pos] = '\0';
      current_entry++;
    }
  n->u.m.columns = current_entry;
} /* process_matrix_rows -- */


/* is_a_token */

int
is_a_token(char c)
{
  switch (c)
    {
    case '|':
      current_token_type = 1;
      return 1;
    case '-':
      current_token_type = 2;
      return 1;
    case '+':
      current_token_type = 3;
      return 1;
    case '*':
      current_token_type = 4;
      return 1;
    case '~':
      current_token_type = 5;
      return 1;
    case '=':
      current_token_type = 6;
      return 1;
    case ';':
      current_token_type = 7;
      return 1;
    case '\'':
      current_token_type = 8;
      return 1;
    case '(':
      current_token_type = 9;
      return 1;
    case ')':
      current_token_type = 10;
      return 1;
    default:
      return 0;
    }
} /* is_a_token */


/* make_node -- */
 
node *
make_node(int node_type, node * n1, node * n2)
{

  node * n;

  n = (node *)malloc(sizeof(struct node));
  n->tag = node_type;
  n->u.e.arg1 = n1;
  n->u.e.arg2 = n2;

  return n;
} /* make_node */
   

void
initialize_array()
{

  int i, j;

  for (i = 0; i < DEEP; i++)
    for (j = 0; j < WIDE; j++) 
      data[j][i] = 'X';
}


void
print_array()
{

  int i, j;
  for (i = 0; i < DEEP; i++)
    for (j = 0; j < WIDE; j++) {
      printf("%c", data[j][i]);
      if (j == WIDE-1) 
	printf("\n");
    }
}


/* put_buffer -- The buffer is where we put things when we read the
 * file.  In order to process this information, we put everything into
 * an array, which is easier to reference to
 */

void
put_buffer_info_into_array(char * buffer, int number)
{

  int i, index, line;

  line = 0;
  index = 0;

  for (i = 0; i < number; i++)
    {
      if (buffer[i] == '\n')
	{
	  line++;
	  index = 0;
	}
      else
	{
	  data[index][line] = buffer[i];
	  index++;
	} 
    }
}


#endif /* scanner_i */

/* end of file -- scanner.c -- */
