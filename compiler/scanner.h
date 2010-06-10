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

/*
 * This file is part of the SHIFT compiler.
 *
 * Copyright 1995-96 Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */

#include <stdio.h>
#include <stdarg.h>

#define LEXERROR  { (void) sprintf(errorBuffer, 
#define ENDERROR  ); (void) yyerror(errorBuffer); }

extern char errorBuffer[];
extern char *filename;
extern int line;
extern lv *current_line, *current_file;

void initScanner();

#define YYSTYPE yystype
typedef lv *yystype;
extern YYSTYPE yylval;

int predefinedOrOther(char *, int);
int symbolicConstant(char *);
int number(char *);
int intnumber(char *);
int stringlit(char *);
lv *identifier(lv *);

void shift_lex_error(const char*, ...);

/* end of file -- scanner.h -- */
