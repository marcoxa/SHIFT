/* -*- Mode: Text -*- */

%{
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
 * Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */

/* The rules for CPP (the C preprocessor) are meant to take into
 * account the way the '#line' and '# [0-9]*' directives are produced
 * by CPP. (No, we do not want to pass the '-P' option to CPP.)
 *
 * Marco Antoniotti 19961022
 *
 * Changed the rule for recognizing INTLITERALS (now we use the
 * function scanaux.c:intliteral instead of
 * scanaux.c:floatliteral with a second argument of 1). Also changed
 * the scanaux.c:intliteral function. See the comments there for an
 * explanation.
 *
 * Marco Antoniotti 19970506
 */

#include <stdio.h>
#include <strings.h>
#include "lisp.h"
#include "scanner.h"
#include "parser.h"
%}

OP	[-+*/<>=&|'`]
ID	[a-zA-Z_][a-zA-Z_0-9]*
%s I INSTRING INCOMMENT CPP
%%
%{
   BEGIN I;
%}
<INSTRING>\"			{ BEGIN I; return stringlit(yytext); }
<INSTRING>\\\"			{ yymore(); }
<INSTRING>\n			{ line += 1; current_line = 0; yymore(); }
<INSTRING>.			{ yymore(); }
<INCOMMENT>"*/"			{ BEGIN I; }
<INCOMMENT>\n			{ line += 1; current_line = 0; }
<INCOMMENT>.			{ }
<CPP>" "[0-9]+			{ line = atoi(yytext + 1); current_line = 0; }
<CPP>"line "[0-9]+[ \t]*\n      { line = atoi(yytext + 5);
                                  current_line = 0;
                                  BEGIN I; }
<CPP>"line "[0-9]+		{ line = atoi(yytext + 5); current_line = 0; }
<CPP>" "\".*\".*\n		{ *rindex(yytext, '"') = 0;
				  current_file = string(yytext +2 );
				  BEGIN I; }
<I>^#				{ BEGIN CPP; }
<I>\"				{ BEGIN INSTRING; yymore(); }
<I>"/*"				{ BEGIN INCOMMENT; }
<I>"\n"				{ line += 1; current_line = 0; }
<I>[ \t]			{ /* white space */ }
<I>"//".*$			{ /* single-line comment */ }
<I>{ID}				{ return
				    predefinedOrOther(yytext, IDENTIFIER); }
<I>"$"{ID}			{ return symliteral(yytext); }
<I>"->"				{ return RIGHTARROW; }
<I>"<-"				{ return LEFTARROW; }
<I>"<->"			{ return LEFTRIGHTARROW; }
<I>":="				{ return COLONEQUAL; }
<I>"+:="			{ return INCRASSIGN; }
<I>"-:="			{ return DECRASSIGN; }
<I>"*:="			{ return MULTASSIGN; }
<I>"/:="			{ return DIVASSIGN; }
<I>".."                         { return TWODOTS; }
<I>[0-9]+[.]?[0-9]*[eE][-+]?[0-9]+	{ return floatliteral(yytext, 0); }
<I>[.][0-9]+[eE][-+]?[0-9]+		{ return floatliteral(yytext, 0); }
<I>[0-9]+[.][0-9]*			{ return floatliteral(yytext, 0); }
<I>[.][0-9]+				{ return floatliteral(yytext, 0); }
<I>[0-9]+				{ return intliteral(yytext); }
<I>[{}(),:;=|]			{ return yytext[0]; }
<I>"["				{ return yytext[0]; }
<I>"]"				{ return yytext[0]; }
<I>{OP}+|\({OP}+\)|\[{OP}+\]|\{{OP}+\}	{ return predefinedOrOther(yytext, PREC1); }
.					{ shift_lex_error("Illegal character: %c (0x%x)",
					                  yytext[0],
							  yytext[0]); }

%%
/*
.				{ LEXERROR "Illegal character: %c (0x%x)", 
				    yytext[0], yytext[0] ENDERROR; }
*/

/*
\"[^"]*\"			{ return string(yytext); }
*/
