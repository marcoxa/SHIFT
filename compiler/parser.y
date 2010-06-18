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

/* #include "lisp.h" */
#include "crscl.h"

#include "scanner.h"
#include "parser.h"
#include "shic.h"

#define YYDEBUG 1

YYSTYPE call1(YYSTYPE f, YYSTYPE x)
{
   return node(intern("call"), list2(f, x), nil);
}

YYSTYPE call2(YYSTYPE f, YYSTYPE x, YYSTYPE y)
{
   return node(intern("call"), list3(f, x, y), nil);
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


/* Added intern("exttypedef"):
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
		      node(intern("typedef"), nil, alist3(intern("id"), $2, 
                                                 intern("parent"), $3,
		                                 intern("exttypedef"), intern("true")))
		    :
		      hd($4) == intern("type_clause_nil_delimeter")
		       ?
		         node(intern("typedef"), nil, acons(intern("id"), $2,
						   acons(intern("parent"), $3, tl($4))))
		       :
		         node(intern("typedef"), nil, acons(intern("id"), $2,
						   acons(intern("parent"), $3, $4)));}
                   

/* Added opt_type_clause_list:
 *
 * Accomodates for the intern("exttypedef") node added above.
 *
 * Tunc Simsek 15th April, 1998
 */
opt_type_clause_list:
	  empty                    { $$ = nil; }
        | '{' type_clause_list '}' { $$ = $2 == nil
				       ?
				         cons(intern("type_clause_nil_delimeter"), nil)
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
	  EXPORT export_list opt_semi		{ $$ = cons(intern("export"), $2);}
	| STATE declaration_list opt_semi	{ $$ = cons(intern("state"), $2); }
	| INPUT declaration_list opt_semi	{ $$ = cons(intern("input"), $2); }
	| OUTPUT declaration_list opt_semi	{ $$ = cons(intern("output"), $2); }
	| setup_clause opt_semi			{ $$ = cons(intern("setup"), $1); }
	| FLOW flow_list opt_semi_comma
				{ $$ = cons(intern("flow"), nreverse($2)); }
	| DISCRETE discrete_state_list opt_semi_comma
	/* Old code: $$ = cons(intern("discrete"), nreverse($2));
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
		{ $$ = cons(intern("discrete"),
		            nreverse(cons(node(intern("discrete"),
			                       nil,
				               alist3(intern("id"),
					              identifier(intern("exit")), 
				                      intern("equations"), nil,
				                      intern("invariant"), nil)),
					 $2)));
		}
	| TRANSITION transition_list opt_semi_comma
				{ $$ = cons(intern("transition"), nreverse($2)); }
	;


global_setup_clause:
          setup_clause opt_semi { $$ = $1; }
        ;
 
setup_clause :
          SETUP optional_define_do  cle_actions_or_empty  optional_connect
	  {
	    $$ = node(intern("setup"),
                      nil,
                      acons(intern("connections"), nreverse($4),
                            acons(intern("do"), nreverse($3), $2 )));
          }
	;

cle_actions_or_empty :
	    empty                   { $$ = nil; }
	  | DO '{' cle_actions '}'  { $$ = $3; }
	  ;

optional_connect :
          empty                        { $$ = nil; }
        | CONNECT '{' con_actions '}'
        /*  { $$ = alist1(intern("connections"), nreverse($3)); } */
          { $$ =  nreverse($3); }
        ;
	
/*****************************************************************************
setup_clause :
	  SETUP optional_define_do '{' actions '}'
	  { $$ = node(intern("setup"), nil, acons(intern("do"), nreverse($4), $2)); }
	;
*****************************************************************************/


optional_define_do :
	  empty				{ $$ = nil; }
/*
	| DO				{ $$ = nil; }
	| DEFINE '{' declaration_list opt_semi '}' DO	
*/
	| DEFINE '{' declaration_list opt_semi '}'
			     { $$ = alist1(intern("define"), $3); }


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
                         { $$ = list1(node(intern("declare"),
			                   list1($1),
			                   alist1(intern("id"), $2))); }
        | export_clause ',' IDENTIFIER
                         { $$ = cons(node(intern("declare"),
                                          list1(arg1(hd($1))),
                                          alist1(intern("id"), $3)),
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
			{ $$ = list1(node(intern("declare"),
					  cons($1, $3),
					  alist1(intern("id"), $2))); }
	| declaration_clause ',' IDENTIFIER opt_init
			{ $$ = cons(node(intern("declare"),
					 cons(arg1(hd($1)), $4),
					 alist1(intern("id"), $3)),
				    $1); }


/* Original version
 * Marco Antoniotti 19961127
type :
	  simple_type			{ $$ = $1; }
	| SET '(' simple_type ')'	{ $$ = node(intern("set"), list1($3), nil); }
	| ARRAY '(' simple_type ')'	{ $$ = node(intern("array"), list1($3), nil); }
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
	| SET '(' type ')'		{ $$ = node(intern("set"), list1($3), nil); }
	| ARRAY '(' type ')'		{ $$ = node(intern("array"), list1($3), nil); }
	| ARRAY '(' type ',' INTLITERAL ')'
		{
		  set_attr(intern("convert"), $5, intern("false"));
		  $$ = node(intern("array"), list1($3), alist1(intern("dimensions"), $5));
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
		{ $$ = node(intern("flow"), nreverse($3), alist1(intern("id"), $1)); }
	| IDENTIFIER '{' eq_list opt_semi_comma '}'
		{ $$ = node(intern("flow"), nreverse($3), alist1(intern("id"), $1)); }
	;


discrete_state_list :
	  discrete_state_clause				{ $$ = list1($1); }
	| discrete_state_list comma_or_semi discrete_state_clause
							{ $$ = cons($3, $1); }
	;

discrete_state_clause :
	  IDENTIFIER opt_flow opt_invariant
		{ $$ = node(intern("discrete"), nil, alist3(intern("id"), $1,
						   intern("equations"), $2,
						   intern("invariant"), $3)); }
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
		{ $$ = node(intern("transition"), nil,
			    acons(intern("from"), $1,
				  acons(intern("to"), $3,
					acons(intern("events"), $5, $7)))); }
	| expression RIGHTARROW EXIT '{' event_list '}'
	  transition_clauses
		{ $$ = node(intern("transition"), nil,
			    acons(intern("from"), $1,
				  acons(intern("to"), $3,
					acons(intern("events"), $5, $7)))); }
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
		{ $$ = node(intern("external_event"), list2($1, $3),
			    alist1(intern("sync_type"), $4)); }

opt_sync_rule :
	  empty				{ $$ = nil;  }

	| '(' ONE ')'			{ $$ = intern("one"); }
	| '(' ONE ':' IDENTIFIER ')'	{ $$ = $4;   }
	| '(' IDENTIFIER ')'		{ $$ = $2;   }
	| '(' ALL ')'			{ $$ = intern("all"); }
	;

transition_clauses :
	  empty					{ $$ = nil; }
	| transition_clauses transition_clause	{ $$ = cons($2, $1); }
	;




/******************************************************************************
transition_clause :
	  WHEN expression		{ $$ = cons(intern("guard"), $2); }
	| DO '{' actions '}'		{ $$ = cons(intern("do"), nreverse($3)); }
	| DEFINE '{' actions '}'	{ $$ = cons(intern("define"), nreverse($3)); }
	;
******************************************************************************/
transition_clause :
	  WHEN expression		{ $$ = cons(intern("guard"), $2); }
	| DO '{' cle_actions '}'	{ $$ = cons(intern("do"), nreverse($3)); }
	| DEFINE '{' declaration_list opt_semi '}'	
				    { $$ = cons(intern("define"), $3); }
	;






/*****************************************************************************
actions :
	  action		{ $$ = list1($1); }
	| actions action	{ $$ = cons($2, $1); }
	;

action :
	  sync_list ';'		{ $$ = node(intern("sync"), $1, nil); }
	| expression COLONEQUAL expression ';'
				{ $$ = node(intern("assign"), list2($1, $3), nil); }
	| expression ';'	{ $$ = $1; }
	;
*****************************************************************************/

cle_actions :
	  cle_action             { $$ = list1($1); }
	| cle_actions cle_action { $$ = cons($2, $1); }
	;


cle_action :
	  expression COLONEQUAL expression ';'
		    { $$ = node(intern("assign"), list2($1, $3), nil); }
        | expression INCRASSIGN expression ';'
                    {
                      $$ = node(intern("opassign"),
                                list2($1, $3),
                                alist1(intern("op"), intern("+")));
                    }
        | expression DECRASSIGN expression ';'
                    {
                      $$ = node(intern("opassign"),
                                list2($1, $3),
                                alist1(intern("op"), intern("-")));
                    }
        | expression MULTASSIGN expression ';'
                    {
                      $$ = node(intern("opassign"),
                                list2($1, $3),
                                alist1(intern("op"), intern("*")));
                    }
        | expression DIVASSIGN expression ';'
                    {
                      $$ = node(intern("opassign"),
                                list2($1, $3),
                                alist1(intern("op"), intern("/")));
                    }
        ;


con_actions :
	  con_action             { $$ = list1($1); }
	| con_actions con_action { $$ = cons($2, $1); }
	;


con_action :
	   sync_list ';'		{ $$ = node(intern("sync"), $1, nil); }
	|  expression LEFTARROW expression ';'
/*******    { $$ = node(intern("dlink"), list2($1,$3), nil); } ****************/
		    { $$ = call2(identifier(intern("dlink")), $1,$3); }


sync_list :
	  event LEFTRIGHTARROW event		{ $$ = list2($1, $3); }
	| sync_list LEFTRIGHTARROW event	{ $$ = cons($3, $1); }
	;

expression :
	  IDENTIFIER			{ $$ = $1; }
	| INTLITERAL			{ $$ = $1; }
	| FLOATLITERAL			{ $$ = $1; }
	| SYMLITERAL			{ $$ = $1; }
	| ALL				{ $$ = identifier(intern("all")); }
 	| create_expression		{ $$ = $1; }
        | components_of_expression { $$ = $1; }
	| '(' expression ')'		{ $$ = $2; }
	| expression '=' expression { $$ = call2(identifier(intern("=")), $1, $3); }
	| expression IN expression
			{ $$ = call2(identifier(intern("in")), $1, $3); }
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
				{ $$ = node(intern("call"), cons($1, $3), nil); }
	| expression '[' expression ']'
				{ $$ = node(intern("index"), list2($1, $3), nil); }
	/* | '{' expr_list '}'	{ $$ = node(intern("setcons"), $2, nil); } */
	| set_constructor	{ $$ = $1; }
	| array_constructor	{ $$ = $1; }

	/* The following are really 'special forms' which should have been
	 * treated in an homogenous way.
	 * From now on I will add SPECIAL FORMS only. (See 'find' and 'count')
	 *
	 * Marco Antoniotti 19970619
	 */
	| EXISTS IDENTIFIER IN expression ':' expression
		{ $$ = node(intern("exists"), list2($4, $6), alist1(intern("id"), $2)); }
        | EXISTS IDENTIFIER IN array_range ':' expression
		{ $$ = node(intern("exists"), list2($4, $6), alist1(intern("id"), $2)); }
	| MINEL IDENTIFIER IN expression ':' expression
		{ $$ = node(intern("minel"), list2($4, $6), alist1(intern("id"), $2)); }
	| MAXEL IDENTIFIER IN expression ':' expression
		{ $$ = node(intern("maxel"), list2($4, $6), alist1(intern("id"), $2)); }
	| IF expression THEN expression ELSE expression
		{ $$ = node(intern("if"), list3($2, $4, $6), nil); }
	| FIND set_constructor default_option
		{
		  /* The default_option can be 'nil'. Hence the
		   * nconc.
		   *
		   * Marco Antoniotti 19970728
		   */
			$$ = node(intern("special_form"),
			          nconc(list1($2), $3),
				  alist1(intern("sfid"), intern("find")));
		}

	| COUNT set_constructor
		{
			$$ = node(intern("special_form"),
				  list1($2),
				  alist1(intern("sfid"), intern("count")));
		}

	| CHOOSE set_constructor default_option
		{
		  /* See comment about FIND form. */
			$$ = node(intern("special_form"),
			          nconc(list1($2), $3),
				  alist1(intern("sfid"), intern("choose")));
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
	  '{' expr_list '}'	{ $$ = node(intern("setcons"), $2, nil); }
	| '{' expression ':' in_exprs as_cons_cond '}'
		{ $$ = node(intern("setcons2"),
			    list3($2, $4, $5),
			    nil); }
	;


components_of_expression :
	  COMPONENTS '(' IDENTIFIER ')'
		{ $$ = node(intern("setcons"), 
		            nil,
		            alist1(intern("components"), $3)); }
	;
	
array_constructor :
	  '[' expr_list ']'	{ $$ = node(intern("arraycons"), $2, nil); }
	| '[' expression ':' in_exprs as_cons_cond ']'
		{ $$ = node(intern("arraycons2"),
			    list3($2, $4, $5),
			    nil); }
	;


as_cons_cond :
	  empty /* return a 'true' condition */
                { $$ = node(intern("id"), nil, acons(intern("name"), intern("true"), nil)); }
	| '|' expression	{ $$ = $2; }
	;


/* Rule 'array_range' -- now needed for new iterator tests. */

array_range :
          '[' expression TWODOTS expression step_options ']'
                { $$ = node(intern("arrayrange"), nil,
		                         alist3(intern("bound1"), $2,
					        intern("bound2"), $4,
					        intern("step_expr"), $5)); }
        ;

/* Old version: 'step_options' in 'array_range' rule used to be 'step_expr'
 * Note that the change implied changes to the rewrites and generating
 * functions.
step_expr :
          empty                 { $$ = nil; }
        | ':' BY expression     { $$ = node(intern("by_expr"), list1($3), nil); }
        ;
 */

step_options :
          empty { $$ = node(intern("by_expr"),
	                    list1(node(intern("int"), nil,
				        alist2(intern("value"), fixnum(1),
                                               intern("type"), discrete_number_type))),
                            alist2(intern("discrete"), intern("true"),
                                   intern("direction"), intern("ascending")));
                }
        | ':' step_options2
                { $$ = node(intern("by_expr"), nil, $2); }
        ;

step_options2 :
          empty { $$ = nil; }
        | step_option { $$ = $1; }
        | step_option ',' step_options2 { $$ = nconc($1, $3); }
        ;

step_option :
          ASCENDING     { $$ = alist1(intern("direction"), intern("ascending")); }
	| DESCENDING    { $$ = alist1(intern("direction"), intern("descending")); }
	| BY expression { $$ = alist1(intern("step_expr"), $2); }
	| DISCRETE      { $$ = alist1(intern("discrete"), intern("true")); }
	| CONTINUOUS    { $$ = alist1(intern("discrete"), intern("false")); }
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
                        { $$ = node(intern("in_expr"), list2($1, $3), nil); }
        | IDENTIFIER IN array_range
                        { $$ = node(intern("in_expr"), list2($1, $3), nil); }
	;

create_expression :
	  CREATE '(' IDENTIFIER initializers ')'
		{ $$ = node(intern("create"), cons($3, $4), current_file_line()); }
	;

initializers :
	  /* empty */	{ $$ = nil; }
	| initializers ',' IDENTIFIER COLONEQUAL expression
		{ $$ = cons(node(intern("initialize"), list1($5), alist1(intern("id"), $3)),
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

                  apush(intern("foreign"), intern("c_function"), plist(attr(intern("name"), $2)));
                  apush(intern("formals"), formals, plist(attr(intern("name"), $2)));
                  $$ = node(intern("ext_fun_decl"), nil,
			    alist3(intern("id"), $2,
				   intern("formals"), formals,
			    	   intern("return_type"), $7));
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
			{ $$ = list1(node(intern("declare"),
					  cons($1, $4),
					  alist2(intern("id"), $3, intern("io_qual"), $2))); }
	| farg_declaration_clause ',' io_qual IDENTIFIER opt_init
			{ $$ = nconc($1,
				     list1(node(intern("declare"),
						cons(arg1(hd($1)), $5),
						alist2(intern("id"), $4, intern("io_qual"), $3))
					   )); }
        ;


io_qual :
          empty		{ $$ = intern("in"); }
	| IN		{ $$ = intern("in"); }
        /* | IN OUT	{ $$ = intern("i_o"); } This would be too complex to
	 *                             handle
         */
        | OUT           { $$ = intern("out"); }
        ;


global_variable_declaration :
	  GLOBAL declaration_list opt_semi
			{ $$ = node(intern("glob_var_decl"), nreverse($2), nil); }
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
