/* -*- Mode: C -*- */

/* parser.y -- */

/*
 * Copyright (c) 1996, 1997 Regents of the University of California.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and the California PATH Program.
 * 4. Neither the name of the University nor of the California PATH
 *    Program may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * This file is part of the Shiftdoc distribution.
 *
 * 19970922 Marco Zandonadi
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"
#include "scanner.h"
#include "parser.h"
#include "shic.h"
#include "dcScanaux.h"

#define YYDEBUG 1

YYSTYPE call1(YYSTYPE f, YYSTYPE x)
{
   return node(@call, list2(f, x), nil);
}

YYSTYPE call2(YYSTYPE f, YYSTYPE x, YYSTYPE y)
{
   return node(@call, list3(f, x, y), nil);
}

/* This variable is used as a state variable. It is set to 1 when a
 * DocComment is found. When the DocComment has been processed and is
 * no longer needed, it is set to 0. If dcFound is set to 1, all the
 * actions update the HTML file, otherwise they don't.
 * 
 * Marco Zandonadi 19970908
 */
int dcFound = 0;

/* This variable is used to remember that a DocComment was found in a
 * high level clause. By checking it we can understand wether to
 * write a subsequent part of a statement to the HTML file or not.
 *
 * E.g. a doccommented transition statement is a complex one; if it
 * synchronizes with some events, which are doccommented, then dcFound
 * is overwritten and one can no longer tell if the "when" clause is to
 * be written to the file or not. For this reason it is necessary to
 * remember that a doccommented transition was found.
 *
 * Marco Zandonadi 19970912
 */
int dcUpperFound = 0;

/* This variable is used to manage entities which are not
 * doccommentable per se (e.g. expressions). These entities are
 * usually part of higher level commands and there is not one single
 * policy to decide wheter to write the to file or not. It depends on
 * the context. If forceWrite is set to 1 then the entity is written,
 * otherwise it is not.
 * 
 * Marco Zandonadi 19970912
 */
int forceWrite = 0;

/* Set/unset the current level DocComment flag. It is usually set at the
 * beginning of a DocComment statement and unset at the end.
 * 
 * Marco Zandonadi 19970923
 */
#define SET_CL_DC dcFound = 1
#define UNSET_CL_DC dcFound = 0

/* If a current level DocComment was found, it returns 1, otherwise 0.
 * Marco Zandonadi 19970923 */ 
#define CL_DC_P (dcFound)

/* Set/unset the upper level DocComment flag. It is usually set at the
 * beginning of composite statements (e.g. transitions) and unset at the end.
 * It is useful to keep track that we are traversing the composite statement.
 * 
 * Marco Zandonadi 19970923
 */
#define SET_UL_DC dcUpperFound = 1
#define UNSET_UL_DC dcUpperFound = 0

/* If an upper level DocComment was found, it returns 1, otherwise 0.
 * Marco Zandonadi 19970923 */ 
#define UL_DC_P (dcUpperFound)

/* Set/unset the force write flag. It MUST be used explicitly to decide
 * whether low-level statements (e.g. expressions) are written to the HTML
 * file or not.
 * 
 * Marco Zandonadi 19970923
 */
#define SET_FW forceWrite = 1
#define UNSET_FW forceWrite = 0

/* If force write is set, it returns 1, otherwise 0.
 * Marco Zandonadi 19970923 */ 
#define FW_P (forceWrite)

/* This variable is used to keep track of the type of the internal
 * clause we are traversing (state, input, etc.). It is actually
 * needed only for input, output and state clauses: all of these
 * clauses have declaration_list in their productions.
 * When we traverse a declaration list we need to know if it is a 
 * state, a input or a output DL, in order to select the correct 
 * docBuffer for writing the output.
 *
 * Marco Zandonadi 19970919 
 */
int currTypeClause = 0;
				/* Values for currIntClause */
				/* They can be used as indexes for docBuffer */
#define TYPE_CL		0	/* Type Clause		*/
#define INPUT_CL	2	/* Input Clause		*/
#define OUTPUT_CL	3 	/* Output Clause	*/
#define STATE_CL	4 	/* State Clause		*/
#define DISCRETE_CL	5 	/* Discrete Clause	*/
#define EXPORT_CL	6 	/* Export Clause	*/
#define FLOW_CL		7 	/* Flow Clause		*/
#define TRANSITION_CL	8 	/* Transition Clause	*/
#define SETUP_CL	10	/* Setup Clause	*/
				/*                           */
				/* Marco Zandonadi 19970919  */

#define TYPE_BUF docBuffer[0]		/* Type begin buffer         */
#define FOOT_BUF docBuffer[1]		/* Type begin buffer         */
#define IN_BUF   docBuffer[2]		/* Input clause buffer       */
#define OUT_BUF  docBuffer[3]		/* Output clause buffer      */
#define STT_BUF  docBuffer[4]		/* State clause buffer       */
#define DIS_BUF  docBuffer[5]		/* Discrete clause buffer    */
#define EXP_BUF  docBuffer[6]		/* Export clause buffer      */
#define FLW_BUF  docBuffer[7]		/* Flow clause buffer        */
#define TRN_BUF  docBuffer[8]		/* Transition clause buffer  */
#define DC_BUF   docBuffer[9]		/* Current DocComment buffer */
#define STP_BUF  docBuffer[10]		/* Setup clause buffer       */
					/*                           */
					/* Marco Zandonadi 19970919  */

#define SET_BUF(buf_type) currTypeClause = (buf_type); 
#define GET_CUR_BUF docBuffer[currTypeClause]

FILE * htmlOut ={stdout};
extern char *docBuffer[NUM_PARSE_BUFFERS];
extern long yyinSize;

extern lv *Program;

%}

/* Keywords */

/* Added keyword OUT for FFI.
 * Marco Antoniotti 19970218
 */
%token ALL ARRAY CREATE DEFINE DISCRETE DO ELSE EXISTS EXPORT
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

%token '=' ',' '(' ')' '{' '}' ':' ';' '[' ']'
%token '|'			/* Added for iterators: Marco Antoniotti 19961230 */

/* Added for Doc Comments. Marco Zandonadi 19970903 */
%token DOCSTART DOCEND DOCWORD DOCAUTHOR DOCVERSION DOCSINCE DOCSEE DOCSPACE


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

/* Productions for DocComments
 *
 * Marco Zandonadi 19970916
 */
opt_doccomment :
	  empty			{ UNSET_CL_DC; $$ = nil; }
	| doc_comment
		{ 
		  SET_CL_DC;
		  $$ = string(DC_BUF); 
		}
	;

doc_comment :
	DOCSTART
		{ DC_BUF[0] = '\0'; } 
	doc_body DOCEND
	;

doc_body :
	  empty
	| doc_body DOCSPACE
		{ }
	| doc_body DOCWORD	
		{ strcat(DC_BUF, " ");
		  strcat(DC_BUF, str($2));
		  strcat(DC_BUF, " "); 
		}
	| doc_body doc_command
	;

doc_command :
	  DOCAUTHOR oom_docspaces 
		{ strcat(DC_BUF, " <B> Author:</B> <I> "); }
	  oom_docwords '$' 
		{ strcat(DC_BUF, " </I> <P>\n"); }
	| DOCVERSION oom_docspaces
		{ strcat(DC_BUF, " <B> Version:</B> <I> "); }
	  DOCWORD '$' 
		{ strcat(DC_BUF, str($4));
		  strcat(DC_BUF, " </I> <P>\n"); }
	| DOCSINCE oom_docspaces
		{ strcat(DC_BUF, " <I> Since: "); }
	  oom_docwords '$' 
		{ strcat(DC_BUF, " </I> <P>\n"); }
	| DOCSEE oom_docspaces DOCWORD '$'
		{ strcat(DC_BUF, " <A HREF=\""); 
		  strcat(DC_BUF, str($3));
		  strcat(DC_BUF, ".html\"> ");
		  strcat(DC_BUF, str($3));
		  strcat(DC_BUF, " </A> ");
		}
	;

oom_docspaces: 
	  DOCSPACE
	| DOCSPACE oom_docspaces
	;

oom_docwords: 
	  DOCWORD
		{ strcat(DC_BUF, " ");
		  strcat(DC_BUF, str($1));
		  strcat(DC_BUF, " "); 
		}
	| DOCWORD 
		{ strcat(DC_BUF, " ");
		  strcat(DC_BUF, str($1));
		  strcat(DC_BUF, " "); 
		}
	  oom_docspaces oom_docwords
	;

/* Shift grammar, modified to support DocComments. Marco Zandonadi 19970923 */

empty : ;

specification :
	  spec			{ Program = nreverse($1); }

spec :
	  definition		{ $$ = list1($1); }
	| spec definition	{ $$ = cons($2, $1); }
	;

definition :
	  type_definition		{ $$ = $1; }
	| external_function_declaration { $$ = $1; }
	| global_variable_declaration	{ $$ = $1; }
	;

/* opt_doccomment should be first. Marco Zandonadi 19970904 */

type_definition :
	  opt_doccomment TYPE IDENTIFIER opt_parent
		{
		  openOutputFile (pname(attr(@name, $3)));
		  eraseBuffers ();

		  SET_BUF(TYPE_CL);

		  if ($4) {
			writeHeaderHTML (
				TYPE_BUF, 
				pname(attr(@name, $3)), 
				pname(attr(@name,$4)));
			writeTypeHTML (
				TYPE_BUF, 
				pname(attr(@name, $3)), 
				pname(attr(@name,$4)));
		  } else {
			writeHeaderHTML (
				TYPE_BUF,
				pname(attr(@name, $3)), NULL);
			writeTypeHTML (
				TYPE_BUF,
				pname(attr(@name, $3)), NULL);
		  }
		  if (CL_DC_P)
			writeDocTextHTML (TYPE_BUF, str($1));
		}
	  '{' type_clause_list '}'semi_opt
		{ 
		  $$ = node(@typedef, nil, acons(@id, $3,
						 acons(@parent, $4, $7)));

		  writeFooterHTML (FOOT_BUF);

		  fprintf (htmlOut, "%s\n", TYPE_BUF);
		  fprintf (htmlOut, "%s\n", STP_BUF);
		  fprintf (htmlOut, "%s\n", IN_BUF);
		  fprintf (htmlOut, "%s\n", OUT_BUF);
		  fprintf (htmlOut, "%s\n", STT_BUF);
		  fprintf (htmlOut, "%s\n", EXP_BUF);
		  fprintf (htmlOut, "%s\n", FLW_BUF);
		  fprintf (htmlOut, "%s\n", DIS_BUF);
		  fprintf (htmlOut, "%s\n", TRN_BUF);
		  fprintf (htmlOut, "%s\n", FOOT_BUF);

/*		  fprintf (stdout, "TYPE:\n%s\n", TYPE_BUF);
		  fprintf (stdout, "STP:\n%s\n", STP_BUF);
		  fprintf (stdout, "IN:\n%s\n", IN_BUF);
		  fprintf (stdout, "OUT:\n%s\n", OUT_BUF);
		  fprintf (stdout, "STT:\n%s\n", STT_BUF);
		  fprintf (stdout, "EXP:\n%s\n", EXP_BUF);
		  fprintf (stdout, "FLW:\n%s\n", FLW_BUF);
		  fprintf (stdout, "DIS:\n%s\n", DIS_BUF);
		  fprintf (stdout, "TRN:\n%s\n", TRN_BUF);
		  fprintf (stdout, "FOOT:\n%s\n", FOOT_BUF);
*/
		  closeOutputFile();
		}

opt_parent :
	  empty			{ $$ = nil; }
	| ':' IDENTIFIER	{ $$ = $2; }

opt_semi :
	  empty { yywarn("semicolon is missing"); }
	| ';'
	;

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
	;

type_clause :
	  EXPORT
		{ if (strlen(EXP_BUF) == 0) 
			writeClauseHeaderHTML(EXP_BUF, "Export");
		  SET_BUF(EXPORT_CL);
		}
	  export_list opt_semi
		{ $$ = cons(@export, $3);
		  writeClauseFooterHTML(EXP_BUF); }

	| STATE 
		{ if (strlen(STT_BUF) == 0) 
			writeClauseHeaderHTML(STT_BUF, "State");
		  SET_BUF(STATE_CL); 
		}
	  declaration_list opt_semi	
		{ $$ = cons(@state, $3); 
		  writeClauseFooterHTML(STT_BUF); }

	| INPUT
		{ if (strlen(IN_BUF) == 0) 
			writeClauseHeaderHTML(IN_BUF, "Input");
		  SET_BUF(INPUT_CL); 
		}
	  declaration_list opt_semi
		{ $$ = cons(@input, $3);
		  writeClauseFooterHTML(IN_BUF); }

	| OUTPUT 
		{ if (strlen(OUT_BUF) == 0) 
			writeClauseHeaderHTML(OUT_BUF, "Output"); 
		  SET_BUF(OUTPUT_CL); 
		}
	  declaration_list opt_semi 
		{ $$ = cons(@output, $3);
		  writeClauseFooterHTML(OUT_BUF); }

	| setup_clause opt_semi			{ $$ = cons(@setup, $1); }

	| FLOW
		{ if (strlen(FLW_BUF) == 0) 
			writeClauseHeaderHTML(FLW_BUF, "Flow");
		  SET_BUF(FLOW_CL);
		}
	  flow_list opt_semi_comma
		{ $$ = cons(@flow, nreverse($3));
		  writeClauseFooterHTML(FLW_BUF); }

	| DISCRETE
		{ if (strlen(DIS_BUF) == 0) 
			writeClauseHeaderHTML(DIS_BUF, "Discrete");
		  SET_BUF(DISCRETE_CL);
		}
	  discrete_state_list opt_semi_comma
		{ $$ = cons(@discrete,
			nreverse(cons(node(@discrete, nil,
			alist3(@id, identifier(intern("exit")), 
			@equations, nil,
			@invariant, nil)),
			$3)));
		  writeClauseFooterHTML(DIS_BUF); }

	| TRANSITION
		{ if (strlen(TRN_BUF) == 0) 
			writeClauseHeaderHTML(TRN_BUF, "Transition");
		  SET_BUF(TRANSITION_CL);
		}
	  transition_list opt_semi_comma
		{ $$ = cons(@transition, nreverse($3));
		  writeClauseFooterHTML(TRN_BUF); }
	;


setup_clause :
          SETUP 
		{ if (strlen(STP_BUF) == 0) 
			writeClauseHeaderHTML(STP_BUF, "Setup");
		  SET_BUF(SETUP_CL);
		}
	  optional_define_do  cle_actions_or_empty  optional_connect
		{ $$ = node(@setup,
			nil,
			acons(@connections, nreverse($5),
				acons(@do, nreverse($4), $3 )));

		  writeClauseFooterHTML(STP_BUF); 
		}
	;

cle_actions_or_empty :
	  empty           
		{ $$ = nil; }
	| DO '{'
		{ writeRawHTML (STP_BUF, "<H4> Do </H4>\n");

		  /* This trick is required because the condition on 
		   * cle_actions is UL_DC_P && CL_DC_P.
		   *
		   * Marco Zandonadi 19970921
		   */
		  SET_UL_DC; } 
	  cle_actions '}'  
		{ $$ = $4;
		  UNSET_UL_DC;
		}
	  ;

optional_connect :
          empty                        { $$ = nil; }
        | CONNECT '{' 
		{ writeRawHTML (STP_BUF, "<H4> Connect </H4>\n"); }
	  con_actions '}'
          { $$ =  nreverse($4); }
        ;
	
optional_define_do :
	  empty				{ $$ = nil; }
	| DEFINE '{'
		{ writeRawHTML (STP_BUF, "<H4> Define </H4>\n"); }
	  declaration_list opt_semi '}'
		{ $$ = alist1(@define, $4); }

export_list :
	  opt_doccomment IDENTIFIER	
		{ $$ = list1($2);
		  if (CL_DC_P) {
			writeExportHeaderHTML(
				GET_CUR_BUF, 
				pname(attr(@name, $2)));
			writeDocTextHTML (GET_CUR_BUF, str($1));
			writeExportFooterHTML(GET_CUR_BUF);
		  }
		}
	| export_list opt_comma opt_doccomment IDENTIFIER
		{ $$ = cons($4, $1);
		  if (CL_DC_P) {
			writeExportHeaderHTML(
				GET_CUR_BUF, 
				pname(attr(@name, $4)));
			writeDocTextHTML (GET_CUR_BUF, str($3));
			writeExportFooterHTML(GET_CUR_BUF); 
		  }
		}
	;

declaration_list :
	  declaration_clause
		{ $$ = $1; }
	| declaration_list ';' declaration_clause
		{ $$ = nconc($1, $3); }
	;

declaration_clause :
	  opt_doccomment 
			{ if (CL_DC_P) writeRawHTML(GET_CUR_BUF, "<LI>\n"); }
	  type IDENTIFIER 
			{ if (CL_DC_P) {
				writeVarNameHTML(
					GET_CUR_BUF,
					pname(attr(@name, $4))); 
				writeRawHTML (GET_CUR_BUF, "<P>\n"); 
				writeDocTextHTML (GET_CUR_BUF, str($1));
			  }
			}
	  opt_init
			{ $$ = list1(node(@declare,
					  cons($3, $6),
					  alist1(@id, $4))); 
			  if (CL_DC_P) writeRawHTML (GET_CUR_BUF, "</LI>\n"); 
			}
	| declaration_clause ',' IDENTIFIER opt_init
			{ $$ = cons(node(@declare,
					 cons(arg1(hd($1)), $4),
					 alist1(@id, $3)),
				    $1); }

/* Added support for DocComments. The old production has been didived 
 * in two ones. The second one describes the index type for arrays and
 * is necessary in order to avoid shift-reduce errors in Yacc.
 *
 * Marco Zandonadi 19970909 
 */
type :
          simple_type                   { $$ = $1; }
        | SET '('
		{ if(CL_DC_P) writeVarTypeHTML (GET_CUR_BUF, "set ("); }
	  type ')'
		{ if(CL_DC_P) writeVarTypeHTML(GET_CUR_BUF,	")"); 
		  $$ = node(@set, list1($4), nil); }
        | ARRAY '('
		{ if(CL_DC_P) writeVarTypeHTML (GET_CUR_BUF, "array ("); }
	  array_index_type ')'
		{ if(CL_DC_P) writeVarTypeHTML (GET_CUR_BUF, ")");
		  $$ = $4; }
        ;

array_index_type:
	  type
		{ $$ = node(@array, list1($1), nil); }
        | type ',' INTLITERAL
                { set_attr(@convert, $3, @false);
                  $$ = node(@array, list1($1), alist1(@dimensions, $3)); 
		  if (CL_DC_P)
		    {
		      char * tempStr;

		      tempStr
			= malloc(getIntLength(num(attr(@value, $3)) + 20)
				 * sizeof(char));
		      sprintf(tempStr, "%ld", num(attr(@value, $3)));
		      writeVarTypeHTML (GET_CUR_BUF, ", ");
		      writeVarTypeHTML (GET_CUR_BUF, tempStr);

		      free (tempStr);
		    }
		}
	;

simple_type :
	  NUMBER
	{ $$ = discrete_number_type;
		  if(CL_DC_P) writeVarTypeHTML (GET_CUR_BUF, "number"); }
	| CONTINUOUS NUMBER
		{ $$ = number_type; 
		  if(CL_DC_P) writeVarTypeHTML (
			GET_CUR_BUF, 
			"continuous number");
		}

	| IDENTIFIER			
		{ $$ = $1; 
		  if(CL_DC_P)
			writeVarTypeHTML (
				GET_CUR_BUF, 
				pname(attr(@name,($1)))); 
		}
	| SYMBOL
		{ $$ = symbol_type; 
		  if(CL_DC_P) writeVarTypeHTML (GET_CUR_BUF, "symbol"); }
	| LOGICAL			
		{ $$ = logical_type;
		  if(CL_DC_P) writeVarTypeHTML (GET_CUR_BUF, "logical"); }
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
	  opt_doccomment DEFAULT
		{ if (CL_DC_P) {
			SET_UL_DC;
			writeFlowHeaderHTML(GET_CUR_BUF, "Default"); 
			writeDocTextHTML (GET_CUR_BUF, str($1));
		  }
		}
	  '{' eq_list opt_semi_comma '}'
		{ $$ = node(@flow, nreverse($5), alist1(@id, $2));
		  if (UL_DC_P) {
			writeFlowFooterHTML(GET_CUR_BUF); 
			UNSET_UL_DC;
		  }
		}
	| opt_doccomment IDENTIFIER
		{ if (CL_DC_P) {
			SET_UL_DC;
			writeFlowHeaderHTML (GET_CUR_BUF, pname(attr(@name, $2)));
			writeDocTextHTML (GET_CUR_BUF, str($1)); 
		  }
		}
	  '{' eq_list opt_semi_comma '}'
		{ $$ = node(@flow, nreverse($5), alist1(@id, $2));
		  if (UL_DC_P) {
			writeFlowFooterHTML(GET_CUR_BUF);
			UNSET_UL_DC;
		  }
		}
	;

discrete_state_list :
	  discrete_state_clause				{ $$ = list1($1); }
	| discrete_state_list comma_or_semi discrete_state_clause
							{ $$ = cons($3, $1); }
	;

discrete_state_clause :
	  opt_doccomment IDENTIFIER
		{ if (CL_DC_P) { 
			writeStateHeaderHTML (
				GET_CUR_BUF, 
				pname(attr(@name, $2)));
			writeDocTextHTML (GET_CUR_BUF, str($1)); 
			SET_UL_DC;
			SET_FW;
		  }
		}
	  opt_flow opt_invariant
		{ $$ = node(@discrete, nil, alist3(@id, $2,
						   @equations, $4,
						   @invariant, $5));
		  if (UL_DC_P) {
			writeStateFooterHTML(GET_CUR_BUF); 
			UNSET_UL_DC;
			UNSET_FW;
		  }
		}
	;

opt_flow :
	  empty					{ $$ = nil; }
	| '{'
		{ if (UL_DC_P)
			writeRawHTML (
				GET_CUR_BUF, 
				"<H4><I> Flow </I></H4>\n<UL>\n"); 
		}
	  eq_list opt_semi_comma '}'	
		{ $$ = nreverse($3); 
		  if (UL_DC_P) writeRawHTML (GET_CUR_BUF, "</UL>\n");
		}

eq_list :
	  opt_doccomment 
		{ if (CL_DC_P) {
			SET_FW;
			writeRawHTML (GET_CUR_BUF, "<LI>\n");
		  }
		}
	  equation
		{ $$ = list1($3); 
		  if (CL_DC_P) {
			writeRawHTML (GET_CUR_BUF, "<P>"); 
			writeDocTextHTML (GET_CUR_BUF, str($1));
			writeRawHTML (GET_CUR_BUF, "</LI>\n");
			UNSET_FW;
		  }
		}
	| eq_list semi_or_comma opt_doccomment 
		{ if (CL_DC_P) {
			SET_FW; 
			writeRawHTML (GET_CUR_BUF, "<LI>\n");
		  }
		}
	  equation
		{ $$ = cons($5, $1);
		  if (CL_DC_P) {
			writeRawHTML (GET_CUR_BUF, "<P>");
			writeDocTextHTML (GET_CUR_BUF, str($3)); 
			writeRawHTML (GET_CUR_BUF, "</LI>\n");
			UNSET_FW;
		  }
		}
	;

equation :
	  expression { $$ = $1; }
	;

opt_invariant :
	  empty			{ $$ = nil; }
	| INVARIANT expression	{ $$ = $2; }
	;

transition_list :
	  opt_doccomment 
		{ if (CL_DC_P) { 
			writeRawHTML (GET_CUR_BUF, "<LI>\n<H3> "); 
			SET_UL_DC;
			SET_FW;
		  }
		} 
	  transition
		{ $$ = list1($3);
		  if (UL_DC_P) {
			writeDocTextHTML (GET_CUR_BUF, str($1)); 
			writeRawHTML (GET_CUR_BUF, "</LI>\n"); 
			UNSET_UL_DC;
			UNSET_FW;
		  }
		}
	| transition_list comma_or_semi opt_doccomment
		{ if (CL_DC_P) {
			writeRawHTML (GET_CUR_BUF, "<LI>\n<H3> ");
			SET_UL_DC;
			SET_FW;
		  }
		}
	  transition
		{ $$ = cons($5, $1);
		  if (UL_DC_P) {
			writeDocTextHTML (GET_CUR_BUF, str($3));
			writeRawHTML (GET_CUR_BUF, "</LI>\n");
			UNSET_UL_DC;
			UNSET_FW;
		  }
		}
	;

transition :
	  expression RIGHTARROW IDENTIFIER '{' 
		{ if (UL_DC_P) {
			writeRawHTML (GET_CUR_BUF, " -> ");
			writeRawHTML (GET_CUR_BUF, pname(attr(@name, $3)));
			writeRawHTML (GET_CUR_BUF, " </H3> \n");
			writeRawHTML (
				GET_CUR_BUF, 
				"<H4> Synchronizes with: </H4>\n<UL> \n");
		  }
		}
	  event_list '}'
		{ if (UL_DC_P) writeRawHTML (GET_CUR_BUF, "</UL> \n"); }
	  transition_clauses
		{ $$ = node(@transition, nil,
			    acons(@from, $1,
				  acons(@to, $3,
					acons(@events, $6, $9)))); }
	| expression RIGHTARROW EXIT '{' 
		{ if (UL_DC_P) {
			writeRawHTML (GET_CUR_BUF, " -> ");
			writeRawHTML (GET_CUR_BUF, " EXIT ");
			writeRawHTML (GET_CUR_BUF, " </H3> \n");
			writeRawHTML (
				GET_CUR_BUF, 
				"<H4> Synchronizes with: </H4>\n<UL> \n");
		  }
		}
	  event_list '}'
		{ if (UL_DC_P) writeRawHTML (GET_CUR_BUF, "</UL> \n"); }
	  transition_clauses
		{ $$ = node(@transition, nil,
			    acons(@from, $1,
				  acons(@to, $3,
					acons(@events, $6, $9)))); }
	;

event_list :
	  empty			{ $$ = nil; }
	| event_list2		{ $$ = nreverse($1); }
	;

event_list2 :
	  opt_doccomment 
		{ if (CL_DC_P && UL_DC_P) SET_FW;
		  else UNSET_FW;
		}	
	  event
		{ $$ = list1($3); 
		  if (CL_DC_P && UL_DC_P) {
			writeRawHTML (GET_CUR_BUF, "<BR> "); 
			writeDocTextHTML (GET_CUR_BUF, str($1)); 
		  }
		}
	| event_list2 opt_comma opt_doccomment
		{ if (CL_DC_P && UL_DC_P) SET_FW;
		  else UNSET_FW;
		}	
	  event
		{ $$ = cons($5, $1); 
		  if (CL_DC_P && UL_DC_P) { 
			writeRawHTML (GET_CUR_BUF, "<BR> "); 
			writeDocTextHTML (GET_CUR_BUF, str($3));
		  }
		}
	;

event :
	  IDENTIFIER
		{ $$ = $1;
		  if (CL_DC_P && UL_DC_P) {
			writeRawHTML (GET_CUR_BUF, "<LI> "); 
			writeRawHTML (GET_CUR_BUF, pname(attr(@name, $1)));	
			writeRawHTML (GET_CUR_BUF, "\n"); 
		  }
		}
	
	| IDENTIFIER ':' IDENTIFIER 
		{
		  if (CL_DC_P && UL_DC_P) {
			writeRawHTML (GET_CUR_BUF, "<LI> "); 
			writeRawHTML (GET_CUR_BUF, pname(attr(@name, $1)));
			writeRawHTML (GET_CUR_BUF, " : ");
			writeRawHTML (GET_CUR_BUF, pname(attr(@name, $3)));
		  }
		}
	  opt_sync_rule
		{ $$ = node(@external_event, list2($1, $3),
			    alist1(@sync_type, $5)); 
		  if (CL_DC_P && UL_DC_P) writeRawHTML (GET_CUR_BUF, "\n"); 
		}
opt_sync_rule :
	  empty				{ $$ = nil;  }

	| '(' ONE ')'
		{ $$ = @one; 
		  if (CL_DC_P && UL_DC_P) 
			writeRawHTML (GET_CUR_BUF, " ( one ) "); }
	| '(' ONE ':' IDENTIFIER ')'
		{ $$ = $4;
		  if (CL_DC_P && UL_DC_P) {
			writeRawHTML (GET_CUR_BUF, " ( one : "); 
			writeRawHTML (GET_CUR_BUF, pname(attr(@name, $4))); 
			writeRawHTML (GET_CUR_BUF, " ) ");
		  }
		}
	| '(' IDENTIFIER ')'
		{ $$ = $2;
		  if (CL_DC_P && UL_DC_P) {
			writeRawHTML (GET_CUR_BUF, " ( ");
			writeRawHTML (GET_CUR_BUF, pname(attr(@name, $2)));
			writeRawHTML (GET_CUR_BUF, " ) ");
		  }
		}
	| '(' ALL ')'
		{ $$ = @all; 
		  if (CL_DC_P && UL_DC_P) 
			writeRawHTML (GET_CUR_BUF, " ( ALL ) "); }
	;

transition_clauses :
	  empty					{ $$ = nil; }
	| transition_clauses transition_clause	{ $$ = cons($2, $1); }
	;

transition_clause :
	  WHEN 
		{ if (UL_DC_P) {
			writeRawHTML (GET_CUR_BUF, "<H4> when "); 
			SET_FW;
		  }
		}
	  expression
		{ $$ = cons(@guard, $3);
		  if (UL_DC_P) {
			UNSET_FW;
			writeRawHTML(GET_CUR_BUF, "</H4> <P>\n");
		  }
		}
	| DO
		{
		  if (UL_DC_P) writeRawHTML(
			GET_CUR_BUF, 
			"<H4> Do </H4>\n<UL>\n");
		} 
	  '{' cle_actions '}'
		{ $$ = cons(@do, nreverse($4));
		  if (UL_DC_P) writeRawHTML(GET_CUR_BUF, "</UL>\n<P>\n");
		}
	| DEFINE 
		{
		  if (UL_DC_P) writeRawHTML(
			GET_CUR_BUF,
			"<H4> Define </H4>\n<UL>\n");
		}
	  '{' declaration_list opt_semi '}'	
		{ $$ = cons(@define, $4); 
		  if (UL_DC_P) writeRawHTML(GET_CUR_BUF, "</UL>\n<P>\n");
		}
	;

cle_actions :
	  opt_doccomment
		{ if (CL_DC_P && UL_DC_P) {
			SET_FW; 
			writeRawHTML(GET_CUR_BUF, "<LI>\n");
		  }
		}
	  cle_action
		{ $$ = list1($3); 
		  if (CL_DC_P && UL_DC_P) {
			writeRawHTML(GET_CUR_BUF, "<P>\n");
			writeDocTextHTML(GET_CUR_BUF, str($1));
			writeRawHTML(GET_CUR_BUF, "</LI>\n<P>\n");
			UNSET_FW;
		  }
		}
	| cle_actions opt_doccomment 
		{ if (CL_DC_P && UL_DC_P) {
			SET_FW; 
			writeRawHTML(GET_CUR_BUF, "<LI>\n");
		  }
		}
	  cle_action 
		{ $$ = cons($4, $1);
		  if (CL_DC_P && UL_DC_P) {
			writeRawHTML(GET_CUR_BUF, "<P>\n");
			writeDocTextHTML(GET_CUR_BUF, str($2));
			writeRawHTML(GET_CUR_BUF, "</LI>\n<P>\n");
			UNSET_FW; 
		  }
		}
	;


cle_action :
	  expression COLONEQUAL 
		{ if (CL_DC_P && UL_DC_P) writeRawHTML(GET_CUR_BUF, " := "); }
	  expression ';'
		{ $$ = node(@assign, list2($1, $4), nil); }
        | expression INCRASSIGN 
		{ if (CL_DC_P && UL_DC_P) writeRawHTML(GET_CUR_BUF, " += "); }
	  expression ';'
		{
		  $$ = node(@opassign,
			list2($1, $4),
			alist1(@op, @"+"));
		}
        | expression DECRASSIGN 
		{ if (CL_DC_P && UL_DC_P) writeRawHTML(GET_CUR_BUF, " -= "); }
	  expression ';'
                    {
                      $$ = node(@opassign,
                                list2($1, $4),
                                alist1(@op, @"-"));
                    }
        | expression MULTASSIGN 
		{ if (CL_DC_P && UL_DC_P) writeRawHTML(GET_CUR_BUF, " *= "); }
	  expression ';'
                    {
                      $$ = node(@opassign,
                                list2($1, $4),
                                alist1(@op, @"*"));
                    }
        | expression DIVASSIGN 
		{ if (CL_DC_P && UL_DC_P) writeRawHTML(GET_CUR_BUF, " /= "); }
	expression ';'
                    {
                      $$ = node(@opassign,
                                list2($1, $4),
                                alist1(@op, @"/"));
                    }
        ;


con_actions :
	  opt_doccomment 
		{ if (CL_DC_P) {
			SET_FW; 
			writeRawHTML(GET_CUR_BUF, "<LI>\n");
		  }
		}
	  con_action             
		{ $$ = list1($3); 
		  if (CL_DC_P) {
			writeRawHTML(GET_CUR_BUF, "<P>\n");
			writeDocTextHTML(GET_CUR_BUF, str($1));
			writeRawHTML(GET_CUR_BUF, "</LI>\n<P>\n");
			UNSET_FW;
		  }
		}
	| con_actions opt_doccomment 
		{ if (CL_DC_P) {
			SET_FW; 
			writeRawHTML(GET_CUR_BUF, "<LI>\n");
		  }
		}
	  con_action 
		{ $$ = cons($4, $1); 
		  if (CL_DC_P) {
			writeRawHTML(GET_CUR_BUF, "<P>\n");
			writeDocTextHTML(GET_CUR_BUF, str($2));
			writeRawHTML(GET_CUR_BUF, "</LI>\n<P>\n");
			UNSET_FW;
		  }
		}
	;


con_action :
	  sync_list ';'		{ $$ = node(@sync, $1, nil); }
	| expression LEFTARROW 
		{ if (CL_DC_P) writeRawHTML (GET_CUR_BUF, " <- "); }
	  expression ';'
		{ $$ = call2(identifier(@dlink), $1,$4); }


sync_list :
	  event LEFTRIGHTARROW 
		{ if (CL_DC_P) writeRawHTML (GET_CUR_BUF, " <-> "); }
	  event		
		{ $$ = list2($1, $4); }
	| sync_list LEFTRIGHTARROW 
		{ if (CL_DC_P) writeRawHTML (GET_CUR_BUF, " <-> "); }
	  event	
		{ $$ = cons($4, $1); }
	;

expression :
	  IDENTIFIER
		{ $$ = $1; 
		  if (FW_P)
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); 
		}
	| INTLITERAL
	{ $$ = $1;
	  if (FW_P) {
		char * tempStr;

		tempStr = malloc (
			getIntLength(
			((num(attr(@value, $1))) + 20) * sizeof(char)));
		sprintf(tempStr, "%ld", num(attr(@value, $1)));
		writeExprHTML (GET_CUR_BUF, tempStr);

		free (tempStr);
	  }
	}
	| FLOATLITERAL
		{ $$ = $1;
		  if (FW_P) 
			writeExprHTML (GET_CUR_BUF, pname(attr(@value, $1)));
		}
	| SYMLITERAL
		{ $$ = $1;
		  if (FW_P) 
			writeExprHTML (GET_CUR_BUF, pname($1)); }
	| ALL
		{ $$ = identifier(@all); 
		  if (FW_P) 
			writeExprHTML (GET_CUR_BUF, "all"); }
	| create_expression		{ $$ = $1; }

	| '('
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, "("); }
	  expression ')'
		{ $$ = $3; 
		  if (FW_P) writeExprHTML(GET_CUR_BUF, ")"); }
	| expression '=' 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " = "); }
	  expression
		{ $$ = call2(identifier(@"="), $1, $4); }
	| expression IN 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, "in"); }
	  expression
		{ $$ = call2(identifier(@in), $1, $4); }
	| expression PREC1
		{ if (FW_P)
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| expression PREC2
		{ if (FW_P)
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| expression PREC3 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| expression PREC4 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| expression PREC5 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| expression PREC6 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| expression PREC7 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| expression PREC8
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| expression PREC9 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| expression PREC10 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
	  expression
		{ $$ = call2($2, $1, $4); }
	| PREC1
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY	
		{ $$ = call1($1, $3); }
	| PREC2 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY
		{ $$ = call1($1, $3); }
	| PREC3 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY
		{ $$ = call1($1, $3); }
	| PREC4 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY
		{ $$ = call1($1, $3); }
	| PREC5 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY
		{ $$ = call1($1, $3); }
	| PREC6 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY
		{ $$ = call1($1, $3); }
	| PREC7 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY
		{ $$ = call1($1, $3); }
	| PREC8 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY
		{ $$ = call1($1, $3); }
	| PREC9 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY
		{ $$ = call1($1, $3); }
	| PREC10 
		{ if (FW_P) 
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $1))); }
	  expression %prec UNARY
		{ $$ = call1($1, $3); }
	| expression POSTOP
		{ $$ = call1($2, $1);
		  if (FW_P)
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2))); }
 	| expression '(' 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " ("); }
	  expr_list ')'
		{ $$ = node(@call, cons($1, $4), nil);
		  if (FW_P) writeExprHTML(GET_CUR_BUF, ") "); }
	| expression '[' 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " ["); }
	  expression ']'
		{ $$ = node(@index, list2($1, $4), nil);
		  if (FW_P) writeExprHTML(GET_CUR_BUF, "] "); }
	| set_constructor	{ $$ = $1; }
	| array_constructor	{ $$ = $1; }

/*** TODO: THE FOLLOWING TWO RULES GENERATE SR ERRORS */

	| EXISTS IDENTIFIER IN
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, "exists ");
		  if (FW_P) writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2)));
		  if (FW_P) writeExprHTML(GET_CUR_BUF, " in "); }
	  expression_or_array_range ':' 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " : "); }
	  expression
		{ $$ = node(@exists, list2($5, $8), alist1(@id, $2)); }
/*        | EXISTS IDENTIFIER IN 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, "exists ");
		  if (FW_P) writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2)));
		  if (FW_P) writeExprHTML(GET_CUR_BUF, " in "); }
	  array_range ':' 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " : "); }
	  expression
		{ $$ = node(@exists, list2($4, $6), alist1(@id, $2)); }
*/	| MINEL IDENTIFIER IN
		{ if (FW_P) {
			writeExprHTML(GET_CUR_BUF, "minel ");
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2)));
			writeExprHTML(GET_CUR_BUF, " in "); 
		  }
		}
	  expression ':' 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " : "); }
	  expression
		{ $$ = node(@minel, list2($5, $8), alist1(@id, $2)); }
	| MAXEL IDENTIFIER IN 
		{ if (FW_P) { 
			writeExprHTML(GET_CUR_BUF, "maxel ");
			writeExprHTML(GET_CUR_BUF, pname(attr(@name, $2)));
			writeExprHTML(GET_CUR_BUF, " in "); 
		  }
		}
	  expression ':'
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " : "); }
	  expression
		{ $$ = node(@maxel, list2($5, $8), alist1(@id, $2)); }
	| IF 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " if "); }
	  expression THEN 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " then "); }
	  expression ELSE 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " else "); }
	  expression
		{ $$ = node(@if, list3($3, $6, $9), nil); }
	| FIND 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " find "); }
	  set_constructor default_option
		{
			$$ = node(@special_form,
			          nconc(list1($3), $4),
				  alist1(@sfid, @find));
		}

	| COUNT 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " count "); }
	  set_constructor
		{
			$$ = node(@special_form,
				  list1($3),
				  alist1(@sfid, @count));
		}

	| CHOOSE 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " choose "); }
	  set_constructor default_option
		{
			$$ = node(@special_form,
			          nconc(list1($3), $4),
				  alist1(@sfid, @choose));
		}
	;

expression_or_array_range :
	  expression
	| array_range
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


set_constructor :
	  '{' expr_list '}'	{ $$ = node(@setcons, $2, nil); }
	| '{' expression ':' in_exprs as_cons_cond '}'
		{ $$ = node(@setcons2,
			    list3($2, $4, $5),
			    nil); }
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
          '[' expression TWODOTS 
		{ if (FW_P) writeExprHTML(GET_CUR_BUF, " .. "); }
	  expression step_options ']'
                { $$ = node(@arrayrange, nil,
		                         alist3(@bound1, $2,
					        @bound2, $5,
					        @step_expr, $6)); 
		}
        ;


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
			{ $$ = node(@create, cons($3, $4), nil); }
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
        | OUT           { $$ = @out; }
        ;


global_variable_declaration :
	  GLOBAL global_declaration_list opt_semi
			{ $$ = node(@glob_var_decl, nreverse($2), nil); }
	;

global_declaration_list :
	  global_declaration_clause
		{ $$ = $1; }
	| global_declaration_list ';' global_declaration_clause
		{ $$ = nconc($1, $3); }
	;

global_declaration_clause :
	  type IDENTIFIER opt_init
			{ $$ = list1(node(@declare,
					  cons($1, $3),
					  alist1(@id, $2))); 
			}
	| global_declaration_clause ',' IDENTIFIER opt_init
			{ $$ = cons(node(@declare,
					 cons(arg1(hd($1)), $4),
					 alist1(@id, $3)),
				    $1); }
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

void 
openOutputFile (char * typeName)
{
	char * htmlOutName = 
		(char*) malloc(sizeof(char) * strlen(typeName) + 5);
	int i = 0;

	if (htmlOutName == NULL) 
	{
		fprintf(stderr, "shiftdoc: cannot allocate memory.\n");
		exit(1);
	}
	strcpy (htmlOutName, typeName);
	strcat (htmlOutName, ".html");

	htmlOut = fopen(htmlOutName, "w");
	if (htmlOut == NULL) {
		fprintf(stderr, "shiftdoc: ");
		perror(htmlOutName);
		exit(1);
	}

	free(htmlOutName);
}

void 
closeOutputFile ()
{
	fclose(htmlOut);
}

void 
eraseBuffers ()
{
	int i;

	for (i = 0; i < NUM_PARSE_BUFFERS; i++)
		docBuffer[i][0] = '\0';
}

/* Returns the number of digits in an integer
 *
 * Marco Zandonadi 19970922
 */
int
getIntLength (int intPar)
{
	int i;
	int intParCopy = intPar;

	for (i = 0; abs(intParCopy) >= 1; intParCopy /= 10, i++);

	if (i == 0) i++;
	if (intPar < 0) i++;

	return i;
}

/* end of file -- parser.y -- */







