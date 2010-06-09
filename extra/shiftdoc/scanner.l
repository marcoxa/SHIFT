/* -*- Mode: Text -*- */

%{
/*
Copyright (c) 1996, 1997 Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:
     This product includes software developed by the University of
     California, Berkeley and the California PATH Program.
4. Neither the name of the University nor of the California PATH
   Program may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/
/*
 * This file is part of the Shiftdoc distribution.
 *
 * 19970922 Marco Zandonadi
 */

/* The rules for CPP (the C preprocessor) are meant to take into
 * account the way the '#line' and '# [0-9]*' directives are produced
 * by CPP. (No, we do not want to pass the '-P' option to CPP.)
 *
 * Marco Antoniotti 19961022
 */

#include <stdio.h>
#include <strings.h>
#include "lisp.h"
#include "scanner.h"
#include "parser.h"
#include "dcScanaux.h"

/* This is used to tell if it's the first time we're calling the scanner.
 *
 * Marco Zandonadi 19970917
 */
static int firstTimeFlag = 1;
%}

OP	[-+*/<>=&|'`]
ID	[a-zA-Z_][a-zA-Z_0-9]*
%s I INSTRING INCOMMENT CPP INDOC
%%
%{
	if (firstTimeFlag) {
		BEGIN I;
		firstTimeFlag = 0;
	}
%}
<INDOC>\**"*/"		{ BEGIN I;
			  /*printf ("1 Lineno: %d; %s\n", line, yytext);*/
			  return DOCEND; }
<INDOC>\n		{ line += 1; current_line = 0; 
			  /*printf ("2 Lineno: %d; newline\n", line);*/ }
<INDOC>[ \t]		{ /*printf ("3 Lineno: %d; whitespace\n", line);*/
			  return DOCSPACE;}
<INDOC>^[ \t]*\**[ \t]+	{ /*printf ("3 Lineno: %d; whitespace\n", line);*/
			  return DOCSPACE;}
<INDOC>"$AUTHOR"	{ /*printf ("5 Lineno: %d; %s\n", line, yytext);*/
			  return DOCAUTHOR; }
<INDOC>"$VERSION"	{ /*printf ("6 Lineno: %d; %s\n", line, yytext);*/
			  return DOCVERSION; }
<INDOC>"$SINCE"		{ /*printf ("7 Lineno: %d; %s\n", line, yytext);*/
			  return DOCSINCE; }
<INDOC>"$SEE"		{ /*printf ("8 Lineno: %d; %s\n", line, yytext);*/
			  return DOCSEE; }
<INDOC>"$"		{ return yytext[0]; }
<INDOC>[^ \t\n$]*	{ /*printf ("9 Lineno: %d; %s\n", line, yytext);*/
			  return docword(yytext); }
<INSTRING>\"			{ BEGIN I; return stringlit(yytext); }
<INSTRING>\\\"			{ yymore(); }
<INSTRING>\n			{ line += 1; current_line = 0; yymore(); }
<INSTRING>.			{ yymore(); }
<INCOMMENT>"*/"			{ BEGIN I; /*printf("%s", yytext);*/ }
<INCOMMENT>\n			{ line += 1; current_line = 0;
				  /*printf("%s", yytext);*/ }
<INCOMMENT>.			{ /*printf("%s", yytext);*/ }
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
<I>"/*"				{ BEGIN INCOMMENT; 
				  /*printf("StartComm\n");*/ }
<I>"/**"                        { BEGIN INDOC;
				  /*printf("StartDocComm\n");*/ 
				  return DOCSTART; }
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
.				{ LEXERROR "Illegal character: %c (0x%x)", 
				    yytext[0], yytext[0] ENDERROR; }
%%

/*
\"[^"]*\"			{ return string(yytext); }
*/



