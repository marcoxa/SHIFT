/* -*- Mode: C -*- */

/* scanaux.c -- */

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
 * Copyright (c) 1995-96 Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */

/* Log:
 * Added new keywords to take into account the set and array iterators.
 */

#include <stdio.h>
#include <stdlib.h>

/* #include "lisp.h" */
#include "crscl.h"

#include "scanner.h"
#include "parser.h"
#include "shic.h"

struct KEYTOKEN {
  lv* keyword;
  int token;
};


/*
struct KEYTOKEN {
  lv *keyword;
  int token;
} predefinedKeywords[] = {
  { @all, ALL },
  { @array, ARRAY },
  { @ascending, ASCENDING },
  { @bool, LOGICAL },
  { @boolean, LOGICAL },
  { @by, BY },
  { @choose, CHOOSE },
  { @closed, CLOSED },
  { @components, COMPONENTS},
  { @connect, CONNECT },
  { @continuous, CONTINUOUS },
  { @count, COUNT },
  { @create, CREATE },
  { @default, DEFAULT },
  { @define, DEFINE },
  { @descending, DESCENDING },
  { @discrete, DISCRETE },
  { @do, DO },
  { @else, ELSE },
  { @exists, EXISTS },
  { @exit, EXIT },
  { @export, EXPORT },
  { @find, FIND },
  { @flow, FLOW },
  { @function, FUNCTION },
  { @global, GLOBAL },
  { @if, IF },
  { @in, IN },
  { @input, INPUT },
  { @invariant, INVARIANT },
  { @logical, LOGICAL },
  { @maxel, MAXEL },
  { @minel, MINEL },
  { @number, NUMBER },
  { @one, ONE },
  { @open, OPEN },
  { @out, OUT },
  { @output, OUTPUT },
  { @set, SET },
  { @setup, SETUP },
  { @state, STATE },
  { @symbol, SYMBOL },
  { @then, THEN },
  { @transition, TRANSITION },
  { @type, TYPE },
  { @when, WHEN },
  { 0, 0 }
};

struct KEYTOKEN predefinedOperators[] = {
    { intern("**"), PREC10 },
    { intern("/"), PREC9 },
    { intern("*"), PREC9 },
    { intern("+"), PREC8 },
    { intern("-"), PREC8 },
    { intern("<"), PREC7 },
    { intern(">"), PREC7 },
    { intern("<="), PREC7 },
    { intern(">="), PREC7 },
    { intern("/="), PREC7 },
    { intern("="), '=' },
    { @not, PREC5 },
    { @and, PREC4 },
    { @xor, PREC3 },
    { @or, PREC2 },
    { intern("=="), PREC1 },
    { intern("'"), POSTOP },
    { 0, 0 }
};
*/

#define N_PREDEFINED_KWDS 46
#define N_PREDEFINED_OPS 17

struct KEYTOKEN predefinedKeywords[N_PREDEFINED_KWDS];
struct KEYTOKEN predefinedOperators[N_PREDEFINED_OPS];

char errorBuffer[1024];

int line = 1;
char *filename = "<stdin>";

lv *current_file = 0;
lv *current_line = 0;

/*
void
init_scanner()
{
  struct KEYTOKEN *p;
  lv *s;

  for (p = predefinedKeywords; p->keyword != nil; p += 1)
    {
      apush(intern("token"), fixnum(p->token), plist(p->keyword));
    }
  for (p = predefinedOperators; p->keyword != nil; p += 1)
    {
      apush(intern("token"), fixnum(p->token), plist(p->keyword));
    }
}
*/

void
init_keyword_token(const char* token_name, int token_id, int i)
{
  lv* token_symbol = intern((char*) token_name);

  if (i < N_PREDEFINED_KWDS)
    {
      predefinedKeywords[i].keyword = token_symbol;
      predefinedKeywords[i].token   = token_id;
      apush(intern("token"), fixnum(token_id), plist(token_symbol));
    }
  else
    {
      shift_lex_error("SHIFT lexical analysis internal error: too may keywords.");
      exit(11);
    }
}


void
init_operator_token(const char* token_name, int token_id, int i)
{
  lv* token_symbol = intern((char *) token_name);

  if (i < N_PREDEFINED_OPS)
    {
      predefinedOperators[i].keyword = token_symbol;
      predefinedOperators[i].token   = token_id;
      apush(intern("token"), fixnum(token_id), plist(token_symbol));
    }
  else
    {
      shift_lex_error("SHIFT lexical analysis internal error: too may operators.");
      exit(11);
    }
}



void
init_scanner()
{
  /* Keywords. */
  int k = 0;

  init_keyword_token("all", ALL, k++);
  init_keyword_token("array", ARRAY, k++);
  init_keyword_token("ascending", ASCENDING, k++); /* Added. Marco Antoniotti 19970106 */
  init_keyword_token("bool", LOGICAL, k++);        /* Added. Marco Antoniotti 19971003 */
  init_keyword_token("boolean", LOGICAL, k++);     /* Added. Marco Antoniotti 19971003 */
  init_keyword_token("by", BY, k++);			/* Added. Marco Antoniotti 19961230 */
  init_keyword_token("choose", CHOOSE, k++);		/* Added. Marco Antoniotti 19960619 */
  init_keyword_token("closed", CLOSED, k++);
  init_keyword_token("components", COMPONENTS, k++);
  init_keyword_token("connect", CONNECT, k++);
  init_keyword_token("continuous", CONTINUOUS, k++);
  init_keyword_token("count", COUNT, k++);		/* Added. Marco Antoniotti 19960619 */
  init_keyword_token("create", CREATE, k++);
  init_keyword_token("default", DEFAULT, k++);   	/* Added. Marco Antoniotti 19970707 */
  init_keyword_token("define", DEFINE, k++);
  init_keyword_token("descending", DESCENDING, k++);	/* Added. Marco Antoniotti 19970106 */
  init_keyword_token("discrete", DISCRETE, k++);
  init_keyword_token("do", DO, k++);
  init_keyword_token("else", ELSE, k++);
  init_keyword_token("exists", EXISTS, k++);
  init_keyword_token("exit", EXIT, k++);		/* Added. Alain Girault 19970203 */
  init_keyword_token("export", EXPORT, k++);
  init_keyword_token("find", FIND, k++);		/* Added. Marco Antoniotti 19960619 */
  init_keyword_token("flow", FLOW, k++);
  init_keyword_token("function", FUNCTION, k++);
  init_keyword_token("global", GLOBAL, k++);
  init_keyword_token("if", IF, k++);
  init_keyword_token("in", IN, k++);
  init_keyword_token("input", INPUT, k++);
  init_keyword_token("invariant", INVARIANT, k++);
  init_keyword_token("logical", LOGICAL, k++);
  init_keyword_token("maxel", MAXEL, k++);
  init_keyword_token("minel", MINEL, k++);
  init_keyword_token("number", NUMBER, k++);
  init_keyword_token("one", ONE, k++);
  init_keyword_token("open", OPEN, k++);
  init_keyword_token("out", OUT, k++);
  init_keyword_token("output", OUTPUT, k++);
  init_keyword_token("set", SET, k++);
  init_keyword_token("setup", SETUP, k++);
  init_keyword_token("state", STATE, k++);
  init_keyword_token("symbol", SYMBOL, k++);
  init_keyword_token("then", THEN, k++);
  init_keyword_token("transition", TRANSITION, k++);
  init_keyword_token("type", TYPE, k++);
  init_keyword_token("when", WHEN, k++);

  /* Operators. */
  k = 0;
  
  init_operator_token("**", PREC10, k++);
  init_operator_token("/", PREC9, k++);
  init_operator_token("*", PREC9, k++);
  init_operator_token("+", PREC8, k++);
  init_operator_token("-", PREC8, k++);
  init_operator_token("<", PREC7, k++);
  init_operator_token(">", PREC7, k++);
  init_operator_token("<=", PREC7, k++);
  init_operator_token(">=", PREC7, k++);
  init_operator_token("/=", PREC7, k++);
  init_operator_token("=", '=', k++); /* should be PREC1 */
  init_operator_token("not", PREC5, k++);
  init_operator_token("and", PREC4, k++);
  init_operator_token("xor", PREC3, k++);
  init_operator_token("or", PREC2, k++);
  init_operator_token("==", PREC1, k++); /* Added. Marco Antoniotti 19961230 */
  init_operator_token("'", POSTOP, k++);
}




lv *
current_file_line()
{
  return alist2(intern("file"),
		current_file? current_file : (current_file = string(filename)),
		intern("line"),
                current_line? current_line : (current_line = fixnum(line)));
}


lv *
identifier(lv *x)
{
  return node(intern("id"), nil, acons(intern("name"), x, current_file_line()));
}

/* predefinedOrOther -- Return the token for a predefined symbol or
 * TOKEN.  Set yylval as a side effect.
 *
 * Note: DEFAULT must be treated in a special way.  I.e. the Lisp
 * symbol associated to it, must be returned anyway in 'yyval' as any
 * 'user' defined identifier.  This must be done to have default flows
 * work as expected and while having the DEFAULT symbol work also as a
 * SHIFT keyword.
 *
 * Marco Antoniotti 19970707
 */
int
predefinedOrOther(char *s, int token)
{
  lv *sym = intern(s);
  lv *tokv = assoc(intern("token"), plist(sym));
  int t = tokv ? num(tl(tokv)) : token;

  switch (t)
    {
    case EXIT:
    case IDENTIFIER:
    case DEFAULT:		/* This is a little trick. */
    case PREC1:
    case PREC2:
    case PREC3:
    case PREC4:
    case PREC5:
    case PREC6:
    case PREC7:
    case PREC8:
    case PREC9:
    case PREC10:
    case POSTOP:
      if (! tokv)
	{
	  apush(intern("token"), fixnum(token), plist(sym));
	}
      yylval = identifier(sym);
      break;
    }
  return t;
}

int
symliteral(char *s)
{
  yylval = node(intern("symbolic"), nil, acons(intern("name"), intern(s+1),
				      current_file_line()));
  return SYMLITERAL;
}

int
floatliteral(char *s, int is_int)
{
    yylval = node(intern("float"), nil,
		  acons(intern("value"), intern(s),
			acons(intern("type"), number_type, current_file_line())));
    if (is_int) set_attr(intern("convert"), yylval, intern("true"));
    return FLOATLITERAL;
}


/* intliteral --
 * SHIFT uses doubles all over internally.  However, here and there,
 * we need to use pure integers.  So, an integer is always converted
 * (i.e. cast to a 'double'), unless provisions are made to avoid this
 * process. (E.g. in array declarations).
 *
 * Marco Antoniotti 19970506
 */
int
intliteral(char *s)
{
  yylval =
    node(intern("int"), nil,
         acons(intern("value"), fixnum(atoi(s)),
               acons(intern("convert"), intern("true"),
                     acons(intern("type"), discrete_number_type,
                           current_file_line()))));
  return INTLITERAL;
}

int
stringlit(char *s)
{
    yylval = node(intern("string"), nil, acons(intern("value"), string(s), current_file_line()));
    return STRING;
}


/* lex_error --
 */

void
shift_lex_error(const char * fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  vsprintf(errorBuffer, fmt, args);
  va_end(args);
  yyerror(errorBuffer);
}

/* end of file -- scanaux.c -- */
