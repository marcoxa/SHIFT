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

/*
 * This file is part of the SHIFT compiler.
 *
 * Copyright 1995-96 by Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */

%{
#include <stdio.h>
#include "lisp.h"
#include "scanner.h"
#include "parser.h"
#include "shic.h"

#define YYDEBUG 1

YYSTYPE call1(YYSTYPE f, YYSTYPE x)
{
   return node(@call, list2(f, x), nil);
}

YYSTYPE call2(YYSTYPE f, YYSTYPE x, YYSTYPE y)
{
   return node(@call, list3(f, x, y), nil);
}

extern lv *Program;
extern lv *current_file_line();

%}
/* Keywords */

/* Added keyword OUT for FFI.
 * Marco Antoniotti 19970218
 */
%token ALL ARRAY CREATE COMPONENTS DEFINE DISCRETE DO ELSE EXISTS EXPORT
%token FLOW FUNCTION GLOBAL IF IN INPUT
%token INVARIANT LOGICAL MAXEL MINEL NUMBER CONTINUOUS ONE OUTPUT
%token SET SETUP STATE STRING SYMBOL THEN TRANSITION
%token TYPE WHEN CONNECT OUT

/* Added keywords FIND, COUNT and CHOOSE for set and array
 * manipulation.  DEFAULT is also added.
 *
 * Note: DEFAULT was missing from the original grammar.  However I now
 * need it to extend the language for FIND, CHOOSE and COUNT.  This
 * addendum is tricky because of the way default flows are currently
 *  handled. See the note on 'predefinedOrOther' in 'scanaux.c' for
 * more info on this little kludge.
 *
 * Marco Antoniotti 19970707
 */
%token CHOOSE COUNT FIND DEFAULT


/* Added the exit state: Alain Girault 19970203 */
%token EXIT

/* Added for iterators: Marco Antoniotti 19961230 */
%token BY ASCENDING DESCENDING

/* Other tokens
 *
 * Added {INCR,DECR,MULT,DIV}ASSIGN
 * Marco Antoniotti 19970601
 */
%token COLONEQUAL INCRASSIGN DECRASSIGN MULTASSIGN DIVASSIGN
%token FLOATLITERAL IDENTIFIER INTLITERAL LEFTRIGHTARROW
%token RIGHTARROW SYMLITERAL LEFTARROW

/* Added for iterators: Marco Antoniotti 19961230 */
%token TWODOTS

/* Added for event types: Marco Antoniotti and Tunc Simsek 19971211 */
%token CLOSED OPEN
%token '=' ',' '(' ')' '{' '}' ':' ';' '[' ']'
%token '|'			/* Added for iterators: Marco Antoniotti 19961230 */

%left ELSE
%left PREC1  IN ':'
%left PREC2
%left PREC3
%left PREC4
%left PREC5
%left PREC6
%left PREC7 '='
%left PREC8
%left PREC9
%right PREC10
%right UNARY
%left POSTOP
%left '(' '['

%start specification

%%

empty : ;

specification :
	  spec			{ Program = nreverse($1); }

spec :
	  definition		{ $$ = list1($1); }
	| spec definition	{ $$ = cons($2, $1); }
	;

/* Added global_setup_clause
 *
 * This global setup statement is provided to avoid
 * in many situations the initializations of global
 * variables.
 *
 * Note: Modification is backward compatible.
 *
 * Tunc Simsek 19980527
 */
definition :
	  type_definition		{ $$ = $1; }
	| external_function_declaration { $$ = $1; }
	| global_variable_declaration	{ $$ = $1; }
        | global_setup_clause           { $$ = $1; }
	;


/* Added @exttypedef:
 *
 * The syntax now accepts external type definitions so that
 * there is a natural methodology for interfacing SHIFT
 * programs to external libraries.
 *
 * Note: Modification is backward compatible.
 *
 * Tunc Simsek 15th April, 1998
 */ 
type_definition :
	  TYPE IDENTIFIER opt_parent opt_type_clause_list semi_opt
		{ $$ = $4 == nil
		    ? 
		      node(@typedef, nil, alist3(@id, $2, 
                                                 @parent, $3,
		                                 @exttypedef, @true))
		    :
		      hd($4) == @type_clause_nil_delimeter
		       ?
		         node(@typedef, nil, acons(@id, $2,
						   acons(@parent, $3, tl($4))))
		       :
		         node(@typedef, nil, acons(@id, $2,
						   acons(@parent, $3, $4)));}
                   

/* Added opt_type_clause_list:
 *
 * Accomodates for the @exttypedef node added above.
 *
 * Tunc Simsek 15th April, 1998
 */
opt_type_clause_list:
	  empty                    { $$ = nil; }
        | '{' type_clause_list '}' { $$ = $2 == nil
				       ?
				         cons(@type_clause_nil_delimeter, nil)
				       :
	                                 $2; }
        ;

opt_parent :
	  empty			   { $$ = nil; }
	| ':' IDENTIFIER	   { $$ = $2; }

/**SYNTAX**/
opt_semi :
	  empty { yywarn("semicolon is missing"); }
	| ';'
	;

/**SYNTAX**/
semi_opt :
	  empty
	| ';'    { yywarn("semicolon is ignored");}
	;


opt_comma :
	  empty { yywarn("comma is missing"); }
	| ','
	;

type_clause_list :
	  empty					{ $$ = nil; }
	| type_clause_list type_clause		{ $$ = cons($2, $1); }

type_clause :
	  EXPORT export_list opt_semi		{ $$ = cons(@export, $2);}
	| STATE declaration_list opt_semi	{ $$ = cons(@state, $2); }
	| INPUT declaration_list opt_semi	{ $$ = cons(@input, $2); }
	| OUTPUT declaration_list opt_semi	{ $$ = cons(@output, $2); }
	| setup_clause opt_semi			{ $$ = cons(@setup, $1); }
	| FLOW flow_list opt_semi_comma
				{ $$ = cons(@flow, nreverse($2)); }
	| DISCRETE discrete_state_list opt_semi_comma
	/* Old code: $$ = cons(@discrete, nreverse($2));
	 * Now we add the predefined state EXIT to the list of states.
	 * identifier(intern("exit")) is exactly what is returned by lex
	 * when a new identifier is created: see predefinedOrOther.
	 * The EXIT state is created with an empty list of equations and 
	 * with an empty invariant. The construction of the new node is
	 * exactly the same as in discrete_state_clause. Finally the
	 * EXIT state is inserted at the end of the state list: this
	 * way the initial state of the component is not the EXIT
	 * state.
	 *
	 * Alain Girault: 19970204.
	 */
		{ $$ = cons(@discrete,
		            nreverse(cons(node(@discrete,
			                       nil,
				               alist3(@id,
					              identifier(intern("exit")), 
				                      @equations, nil,
				                      @invariant, nil)),
					 $2)));
		}
	| TRANSITION transition_list opt_semi_comma
				{ $$ = cons(@transition, nreverse($2)); }
	;


global_setup_clause:
          setup_clause opt_semi { $$ = $1; }
        ;
 
setup_clause :
          SETUP optional_define_do  cle_actions_or_empty  optional_connect
	  {
	    $$ = node(@setup,
                      nil,
                      acons(@connections, nreverse($4),
                            acons(@do, nreverse($3), $2 )));
          }
	;

cle_actions_or_empty :
	    empty                   { $$ = nil; }
	  | DO '{' cle_actions '}'  { $$ = $3; }
	  ;

optional_connect :
          empty                        { $$ = nil; }
        | CONNECT '{' con_actions '}'
        /*  { $$ = alist1(@connections, nreverse($3)); } */
          { $$ =  nreverse($3); }
        ;
	
/*****************************************************************************
setup_clause :
	  SETUP optional_define_do '{' actions '}'
	  { $$ = node(@setup, nil, acons(@do, nreverse($4), $2)); }
	;
*****************************************************************************/


optional_define_do :
	  empty				{ $$ = nil; }
/*
	| DO				{ $$ = nil; }
	| DEFINE '{' declaration_list opt_semi '}' DO	
*/
	| DEFINE '{' declaration_list opt_semi '}'
			     { $$ = alist1(@define, $3); }


/* Exported event declarations.
 * Modified to account for typed events
 *
 * Marco Antoniotti and Tunc Simsek 19971211
 */
export_list :
          export_clause                             { $$ = $1; }
        | export_list ';' export_clause             { $$ = nconc($1, $3); }
	;

export_clause :
          export_type IDENTIFIER
                         { $$ = list1(node(@declare,
			                   list1($1),
			                   alist1(@id, $2))); }
        | export_clause ',' IDENTIFIER
                         { $$ = cons(node(@declare,
                                          list1(arg1(hd($1))),
                                          alist1(@id, $3)),
	                             $1); }
        ;

export_type :
          OPEN                                      { $$ = open_event_type; }
        | CLOSED                                    { $$ = event_type; }
        | empty                                     { $$ = event_type; }
        ;

declaration_list :
	  declaration_clause			    { $$ = $1; }
	| declaration_list ';' declaration_clause   { $$ = nconc($1, $3); }
	;

declaration_clause :
	  type IDENTIFIER opt_init
			{ $$ = list1(node(@declare,
					  cons($1, $3),
					  alist1(@id, $2))); }
	| declaration_clause ',' IDENTIFIER opt_init
			{ $$ = cons(node(@declare,
					 cons(arg1(hd($1)), $4),
					 alist1(@id, $3)),
				    $1); }


/* Original version
 * Marco Antoniotti 19961127
type :
	  simple_type			{ $$ = $1; }
	| SET '(' simple_type ')'	{ $$ = node(@set, list1($3), nil); }
	| ARRAY '(' simple_type ')'	{ $$ = node(@array, list1($3), nil); }
	;
 */

/* New Version which now allows 'nested' types in arrays and sets.
 * Also added possibility to declare the actual dimension of an
 * array.  This feature is unused except for the Foreign Function
 *  Interface.
 *
 * Marco Antoniotti 19970505
 */
type :
	  simple_type			{ $$ = $1; }
	| SET '(' type ')'		{ $$ = node(@set, list1($3), nil); }
	| ARRAY '(' type ')'		{ $$ = node(@array, list1($3), nil); }
	| ARRAY '(' type ',' INTLITERAL ')'
		{
		  set_attr(@convert, $5, @false);
		  $$ = node(@array, list1($3), alist1(@dimensions, $5));
		}
	;

simple_type :
	  NUMBER			{ $$ = discrete_number_type; }
	| CONTINUOUS NUMBER		{ $$ = number_type; }
	| IDENTIFIER			{ $$ = $1; }
	| SYMBOL			{ $$ = symbol_type; }
	| LOGICAL			{ $$ = logical_type; }
	;

opt_init :
	  empty				{ $$ = nil; }
	| COLONEQUAL expression		{ $$ = list1($2); }
	;

flow_list :
	  flow				{ $$ = list1($1); }
	| flow_list comma_or_semi flow	{ $$ = cons($3, $1); }

opt_semi_comma :    /** semicolon is preferred **/
	  empty         { yywarn("semicolon is missing"); }
	| ','  { yywarn("semicolon should be used instead of comma"); }
        | ';'
	;
semi_or_comma :
	  ','           { yywarn("semicolon should be used instead of comma"); }
	| ';'
	;

comma_or_semi :    /** comma is preferred **/
	  ','
	| ';'           { yywarn("comma should be used instead of semicolon"); }
	;


flow :
	  DEFAULT  '{' eq_list opt_semi_comma '}'
		{ $$ = node(@flow, nreverse($3), alist1(@id, $1)); }
	| IDENTIFIER '{' eq_list opt_semi_comma '}'
		{ $$ = node(@flow, nreverse($3), alist1(@id, $1)); }
	;


discrete_state_list :
	  discrete_state_clause				{ $$ = list1($1); }
	| discrete_state_list comma_or_semi discrete_state_clause
							{ $$ = cons($3, $1); }
	;

discrete_state_clause :
	  IDENTIFIER opt_flow opt_invariant
		{ $$ = node(@discrete, nil, alist3(@id, $1,
						   @equations, $2,
						   @invariant, $3)); }
	;

opt_flow :
	  empty					{ $$ = nil; }
	| '{' eq_list opt_semi_comma '}'	{ $$ = nreverse($2); }

eq_list :
	  equation				{ $$ = list1($1); }
	| eq_list semi_or_comma equation	{ $$ = cons($3, $1); }
	;

equation :
	  expression { $$ = $1; }
	;

opt_invariant :
	  empty			{ $$ = nil; }
	| INVARIANT expression	{ $$ = $2; }
	;

transition_list :
	  transition					{ $$ = list1($1); }
	| transition_list comma_or_semi transition	{ $$ = cons($3, $1); }
	;

transition :
	  expression RIGHTARROW IDENTIFIER '{' event_list '}'
	  transition_clauses
		{ $$ = node(@transition, nil,
			    acons(@from, $1,
				  acons(@to, $3,
					acons(@events, $5, $7)))); }
	| expression RIGHTARROW EXIT '{' event_list '}'
	  transition_clauses
		{ $$ = node(@transition, nil,
			    acons(@from, $1,
				  acons(@to, $3,
					acons(@events, $5, $7)))); }
	;

event_list :
	  empty			{ $$ = nil; }
	| event_list2		{ $$ = nreverse($1); }
	;

event_list2 :
	  event				{ $$ = list1($1); }
	| event_list2 opt_comma event	{ $$ = cons($3, $1); }
	;

event :
	  IDENTIFIER				{ $$ = $1; }
	| IDENTIFIER ':' IDENTIFIER opt_sync_rule
		{ $$ = node(@external_event, list2($1, $3),
			    alist1(@sync_type, $4)); }

opt_sync_rule :
	  empty				{ $$ = nil;  }

	| '(' ONE ')'			{ $$ = @one; }
	| '(' ONE ':' IDENTIFIER ')'	{ $$ = $4;   }
	| '(' IDENTIFIER ')'		{ $$ = $2;   }
	| '(' ALL ')'			{ $$ = @all; }
	;

transition_clauses :
	  empty					{ $$ = nil; }
	| transition_clauses transition_clause	{ $$ = cons($2, $1); }
	;




/******************************************************************************
transition_clause :
	  WHEN expression		{ $$ = cons(@guard, $2); }
	| DO '{' actions '}'		{ $$ = cons(@do, nreverse($3)); }
	| DEFINE '{' actions '}'	{ $$ = cons(@define, nreverse($3)); }
	;
******************************************************************************/
transition_clause :
	  WHEN expression		{ $$ = cons(@guard, $2); }
	| DO '{' cle_actions '}'	{ $$ = cons(@do, nreverse($3)); }
	| DEFINE '{' declaration_list opt_semi '}'	
				    { $$ = cons(@define, $3); }
	;






/*****************************************************************************
actions :
	  action		{ $$ = list1($1); }
	| actions action	{ $$ = cons($2, $1); }
	;

action :
	  sync_list ';'		{ $$ = node(@sync, $1, nil); }
	| expression COLONEQUAL expression ';'
				{ $$ = node(@assign, list2($1, $3), nil); }
	| expression ';'	{ $$ = $1; }
	;
*****************************************************************************/

cle_actions :
	  cle_action             { $$ = list1($1); }
	| cle_actions cle_action { $$ = cons($2, $1); }
	;


cle_action :
	  expression COLONEQUAL expression ';'
		    { $$ = node(@assign, list2($1, $3), nil); }
        | expression INCRASSIGN expression ';'
                    {
                      $$ = node(@opassign,
                                list2($1, $3),
                                alist1(@op, @"+"));
                    }
        | expression DECRASSIGN expression ';'
                    {
                      $$ = node(@opassign,
                                list2($1, $3),
                                alist1(@op, @"-"));
                    }
        | expression MULTASSIGN expression ';'
                    {
                      $$ = node(@opassign,
                                list2($1, $3),
                                alist1(@op, @"*"));
                    }
        | expression DIVASSIGN expression ';'
                    {
                      $$ = node(@opassign,
                                list2($1, $3),
                                alist1(@op, @"/"));
                    }
        ;


con_actions :
	  con_action             { $$ = list1($1); }
	| con_actions con_action { $$ = cons($2, $1); }
	;


con_action :
	   sync_list ';'		{ $$ = node(@sync, $1, nil); }
	|  expression LEFTARROW expression ';'
/*******    { $$ = node(@dlink, list2($1,$3), nil); } ****************/
		    { $$ = call2(identifier(@dlink), $1,$3); }


sync_list :
	  event LEFTRIGHTARROW event		{ $$ = list2($1, $3); }
	| sync_list LEFTRIGHTARROW event	{ $$ = cons($3, $1); }
	;

expression :
	  IDENTIFIER			{ $$ = $1; }
	| INTLITERAL			{ $$ = $1; }
	| FLOATLITERAL			{ $$ = $1; }
	| SYMLITERAL			{ $$ = $1; }
	| ALL				{ $$ = identifier(@all); }
 	| create_expression		{ $$ = $1; }
        | components_of_expression { $$ = $1; }
	| '(' expression ')'		{ $$ = $2; }
	| expression '=' expression
			{ $$ = call2(identifier(@"="), $1, $3); }
	| expression IN expression
			{ $$ = call2(identifier(@in), $1, $3); }
	| expression PREC1 expression	{ $$ = call2($2, $1, $3); }
	| expression PREC2 expression	{ $$ = call2($2, $1, $3); }
	| expression PREC3 expression	{ $$ = call2($2, $1, $3); }
	| expression PREC4 expression	{ $$ = call2($2, $1, $3); }
	| expression PREC5 expression	{ $$ = call2($2, $1, $3); }
	| expression PREC6 expression	{ $$ = call2($2, $1, $3); }
	| expression PREC7 expression	{ $$ = call2($2, $1, $3); }
	| expression PREC8 expression	{ $$ = call2($2, $1, $3); }
	| expression PREC9 expression	{ $$ = call2($2, $1, $3); }
	| expression PREC10 expression	{ $$ = call2($2, $1, $3); }
	| PREC1 expression %prec UNARY	{ $$ = call1($1, $2); }
	| PREC2 expression %prec UNARY	{ $$ = call1($1, $2); }
	| PREC3 expression %prec UNARY	{ $$ = call1($1, $2); }
	| PREC4 expression %prec UNARY	{ $$ = call1($1, $2); }
	| PREC5 expression %prec UNARY	{ $$ = call1($1, $2); }
	| PREC6 expression %prec UNARY	{ $$ = call1($1, $2); }
	| PREC7 expression %prec UNARY	{ $$ = call1($1, $2); }
	| PREC8 expression %prec UNARY	{ $$ = call1($1, $2); }
	| PREC9 expression %prec UNARY	{ $$ = call1($1, $2); }
	| PREC10 expression %prec UNARY	{ $$ = call1($1, $2); }
	| expression POSTOP		{ $$ = call1($2, $1); }
 	| expression '(' expr_list ')'
				{ $$ = node(@call, cons($1, $3), nil); }
	| expression '[' expression ']'
				{ $$ = node(@index, list2($1, $3), nil); }
	/* | '{' expr_list '}'	{ $$ = node(@setcons, $2, nil); } */
	| set_constructor	{ $$ = $1; }
	| array_constructor	{ $$ = $1; }

	/* The following are really 'special forms' which should have been
	 * treated in an homogenous way.
	 * From now on I will add SPECIAL FORMS only. (See 'find' and 'count')
	 *
	 * Marco Antoniotti 19970619
	 */
	| EXISTS IDENTIFIER IN expression ':' expression
		{ $$ = node(@exists, list2($4, $6), alist1(@id, $2)); }
        | EXISTS IDENTIFIER IN array_range ':' expression
		{ $$ = node(@exists, list2($4, $6), alist1(@id, $2)); }
	| MINEL IDENTIFIER IN expression ':' expression
		{ $$ = node(@minel, list2($4, $6), alist1(@id, $2)); }
	| MAXEL IDENTIFIER IN expression ':' expression
		{ $$ = node(@maxel, list2($4, $6), alist1(@id, $2)); }
	| IF expression THEN expression ELSE expression
		{ $$ = node(@if, list3($2, $4, $6), nil); }
	| FIND set_constructor default_option
		{
		  /* The default_option can be 'nil'. Hence the
		   * nconc.
		   *
		   * Marco Antoniotti 19970728
		   */
			$$ = node(@special_form,
			          nconc(list1($2), $3),
				  alist1(@sfid, @find));
		}

	| COUNT set_constructor
		{
			$$ = node(@special_form,
				  list1($2),
				  alist1(@sfid, @count));
		}

	| CHOOSE set_constructor default_option
		{
		  /* See comment about FIND form. */
			$$ = node(@special_form,
			          nconc(list1($2), $3),
				  alist1(@sfid, @choose));
		}
	;

default_option :
	  empty { $$ = nil; }
	| DEFAULT '{' expression '}' { $$ = list1($3); }
	;


expr_list :
	  empty			{ $$ = nil; }
	| expr_list2		{ $$ = nreverse($1); }
	;

expr_list2 :
	  expression			{ $$ = list1($1); }
	| expr_list2 ',' expression	{ $$ = cons($3, $1); }
	;


/* New set and Array constructors via iteration.
 *
 * Marco Antoniotti 19961214
 */


set_constructor :
	  '{' expr_list '}'	{ $$ = node(@setcons, $2, nil); }
	| '{' expression ':' in_exprs as_cons_cond '}'
		{ $$ = node(@setcons2,
			    list3($2, $4, $5),
			    nil); }
	;


components_of_expression :
	  COMPONENTS '(' IDENTIFIER ')'
		{ $$ = node(@setcons, 
		            nil,
		            alist1(@components, $3)); }
	;
	
array_constructor :
	  '[' expr_list ']'	{ $$ = node(@arraycons, $2, nil); }
	| '[' expression ':' in_exprs as_cons_cond ']'
		{ $$ = node(@arraycons2,
			    list3($2, $4, $5),
			    nil); }
	;


as_cons_cond :
	  empty /* return a 'true' condition */
                { $$ = node(@id, nil, acons(@name, @true, nil)); }
	| '|' expression	{ $$ = $2; }
	;


/* Rule 'array_range' -- now needed for new iterator tests. */

array_range :
          '[' expression TWODOTS expression step_options ']'
                { $$ = node(@arrayrange, nil,
		                         alist3(@bound1, $2,
					        @bound2, $4,
					        @step_expr, $5)); }
        ;

/* Old version: 'step_options' in 'array_range' rule used to be 'step_expr'
 * Note that the change implied changes to the rewrites and generating
 * functions.
step_expr :
          empty                 { $$ = nil; }
        | ':' BY expression     { $$ = node(@by_expr, list1($3), nil); }
        ;
 */

step_options :
          empty { $$ = node(@by_expr,
	                    list1(node(@int, nil,
				        alist2(@value, fixnum(1),
                                               @type, discrete_number_type))),
                            alist2(@discrete, @true,
                                   @direction, @ascending));
                }
        | ':' step_options2
                { $$ = node(@by_expr, nil, $2); }
        ;

step_options2 :
          empty { $$ = nil; }
        | step_option { $$ = $1; }
        | step_option ',' step_options2 { $$ = nconc($1, $3); }
        ;

step_option :
          ASCENDING     { $$ = alist1(@direction, @ascending); }
	| DESCENDING    { $$ = alist1(@direction, @descending); }
	| BY expression { $$ = alist1(@step_expr, $2); }
	| DISCRETE      { $$ = alist1(@discrete, @true); }
	| CONTINUOUS    { $$ = alist1(@discrete, @false); }
        ;

in_exprs :
	  empty			{ $$ = nil; }
	| in_exprs2             { $$ = nreverse($1); }
	;

in_exprs2 :
	  in_expr		{ $$ = list1($1); }
	| in_exprs ',' in_expr	{ $$ = cons($3, $1); }
	;

in_expr :
	  IDENTIFIER IN expression
                        { $$ = node(@in_expr, list2($1, $3), nil); }
        | IDENTIFIER IN array_range
                        { $$ = node(@in_expr, list2($1, $3), nil); }
	;

create_expression :
	  CREATE '(' IDENTIFIER initializers ')'
		{ $$ = node(@create, cons($3, $4), current_file_line()); }
	;

initializers :
	  /* empty */	{ $$ = nil; }
	| initializers ',' IDENTIFIER COLONEQUAL expression
		{ $$ = cons(node(@initialize, list1($5), alist1(@id, $3)),
			    $1); }
	;


/* Foreign Function Declaration
 * I need to remember that this is a "foreign" function, not only for
 * the declaration but also for the call proper.  How do I do this?  I
 * simply use the plist machinery to associate the information to the
 * identifier.  Ain't Lisp grand?!? 
 *
 * Marco Antoniotti 19970218
 */
external_function_declaration :
	  FUNCTION IDENTIFIER '(' farg_declaration_list_or_empty ')'
	  RIGHTARROW type opt_semi
		{
                  lv* formals = $4;

                  apush(@foreign, @c_function, plist(attr(@name, $2)));
                  apush(@formals, formals, plist(attr(@name, $2)));
                  $$ = node(@ext_fun_decl, nil,
			    alist3(@id, $2,
				   @formals, formals,
			    	   @return_type, $7));
                }

farg_declaration_list_or_empty :
	  empty			{ $$ = nil; }
	| farg_declaration_list	{ $$ = $1; }
	;


farg_declaration_list :
	  farg_declaration_clause
		{ $$ = $1; }
	| farg_declaration_list ';' farg_declaration_clause
		{ $$ = nconc($1, $3); }
	;

farg_declaration_clause :
	  type io_qual IDENTIFIER opt_init
			{ $$ = list1(node(@declare,
					  cons($1, $4),
					  alist2(@id, $3, @io_qual, $2))); }
	| farg_declaration_clause ',' io_qual IDENTIFIER opt_init
			{ $$ = nconc($1,
				     list1(node(@declare,
						cons(arg1(hd($1)), $5),
						alist2(@id, $4, @io_qual, $3))
					   )); }
        ;


io_qual :
          empty		{ $$ = @in; }
	| IN		{ $$ = @in; }
        /* | IN OUT	{ $$ = @i_o; } This would be too complex to
	 *                             handle
         */
        | OUT           { $$ = @out; }
        ;


global_variable_declaration :
	  GLOBAL declaration_list opt_semi
			{ $$ = node(@glob_var_decl, nreverse($2), nil); }
	;
%%

int
yyerror(char *s)
{
  char *f = current_file ? str(current_file) : filename;
  int l = current_line ? num(current_line) : line;

  fprintf(stderr, "%s:%d: error: %s.\n", f, l, s);
  n_errors++;
  return 0;
}

int
yywarn(char *s)
{
  char *f = current_file ? str(current_file) : filename;
  int l = current_line ? num(current_line) : line; 

  fprintf(stderr, "%s:%d: warning: %s at or before line %d.\n", f, l, s, l);
  return 0;
}



/* end of file -- parser.y -- */
