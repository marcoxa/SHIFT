/* -*- Mode: C -*- */

/* parser.y -- */

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

%{
#include <stdio.h>
#include "scanner.h"
#include "parser.h"

#define YYSTYPE yystype
extern int total_tokens;
YYSTYPE yylval;


int current_token=0;

extern node * Equations;
extern int total_equations;

#define YYDEBUG 1
%}

%token MATRIX SCALAR ID

%left '-' '+'
%left '*'
%left '('
%right '\'' '~'
%right UNARY

%start specification

%%
specification:
	statement_list
		{
			Equations = $1;
			if (debug_msg)
			  printf("Got to the top\n");
		}
		;


statement_list:
	  statement { $$ = $1; total_equations++; }
					        
	| statement_list statement { $$ = make_node(AND_EXP, $2, $1); }
	;


statement:
	expression '=' expression ';' { $$ = make_node(EQ_EXP, $1,$3); }
	;


expression:
	  expression '+' expression	{ $$  = make_node(ADD_EXP, $1,$3); }
	| expression '-' expression	{ $$ =  make_node(SUB_EXP, $1,$3); }
	| expression '*'  expression	{ $$ =  make_node(MULTI_EXP,$1, $3); }
	| MATRIX	{ $$ = $1; }
	| ID		{ $$ = $1; }
	| '(' expression ')'		{ $$ = $2; }
	| expression '\''		{ $$ = make_node(PRIME_EXP,$1, nil); }
	| expression '~'		{ $$ = make_node(TRANS_EXP, $1, nil); }
	| '-' expression %prec UNARY	{ $$ = make_node(NEGAT_EXP, $2, nil); }
	;	

%%

int
yylex()
{
  node* l;

  current_token++;
  if (current_token <= total_tokens)
    { 
      yylval = stack[current_token-1]->lval;
      return (stack[current_token-1]->token);
    }
  else
    return 0;
}


int
yyerror(char *s)
{
  fprintf(stderr, "mat2scalar: syntax error.\n");
  exit(1);
}

/* end of file -- parser.y -- */
