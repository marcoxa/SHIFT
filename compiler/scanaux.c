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
#include "lisp.h"
#include "scanner.h"
#include "parser.h"
#include "shic.h"

struct KEYTOKEN {
  lv *keyword;
  int token;
} predefinedKeywords[] = {
  { @all, ALL },
  { @array, ARRAY },
  { @ascending, ASCENDING },	/* Added. Marco Antoniotti 19970106 */
  { @bool, LOGICAL },		/* Added. Marco Antoniotti 19971003 */
  { @boolean, LOGICAL },	/* Added. Marco Antoniotti 19971003 */
  { @by, BY },			/* Added. Marco Antoniotti 19961230 */
  { @choose, CHOOSE },		/* Added. Marco Antoniotti 19960619 */
  { @closed, CLOSED },
  { @components, COMPONENTS},
  { @connect, CONNECT },
  { @continuous, CONTINUOUS },
  { @count, COUNT },		/* Added. Marco Antoniotti 19960619 */
  { @create, CREATE },
  { @default, DEFAULT },	/* Added. Marco Antoniotti 19970707 */
  { @define, DEFINE },
  { @descending, DESCENDING },	/* Added. Marco Antoniotti 19970106 */
  { @discrete, DISCRETE },
  { @do, DO },
  { @else, ELSE },
  { @exists, EXISTS },
  { @exit, EXIT },		/* Added. Alain Girault 19970203 */
  { @export, EXPORT },
  { @find, FIND },		/* Added. Marco Antoniotti 19960619 */
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
    { @"**", PREC10 },
    { @"/", PREC9 },
    { @"*", PREC9 },
    { @"+", PREC8 },
    { @"-", PREC8 },
    { @"<", PREC7 },
    { @">", PREC7 },
    { @"<=", PREC7 },
    { @">=", PREC7 },
    { @"/=", PREC7 },
    { @"=", '=' },		/* should be PREC1 */
    { @not, PREC5 },
    { @and, PREC4 },
    { @xor, PREC3 },
    { @or, PREC2 },
    { @"==", PREC1 },		/* Added. Marco Antoniotti 19961230 */
    { @"'", POSTOP },
    { 0, 0 }
};

char errorBuffer[1024];

int line = 1;
char *filename = "<stdin>";

lv *current_file = 0;
lv *current_line = 0;

void
init_scanner()
{
  struct KEYTOKEN *p;
  lv *s;

  for (p = predefinedKeywords; p->keyword != nil; p += 1)
    {
      apush(@token, fixnum(p->token), plist(p->keyword));
    }
  for (p = predefinedOperators; p->keyword != nil; p += 1)
    {
      apush(@token, fixnum(p->token), plist(p->keyword));
    }
}

lv *
current_file_line()
{
  return alist2(@file,
		current_file? current_file : (current_file = string(filename)),
		@line,
                current_line? current_line : (current_line = fixnum(line)));
}

lv *
identifier(lv *x)
{
  return node(@id, nil, acons(@name, x, current_file_line()));
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
  lv *tokv = assoc(@token, plist(sym));
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
	  apush(@token, fixnum(token), plist(sym));
	}
      yylval = identifier(sym);
      break;
    }
  return t;
}

int
symliteral(char *s)
{
  yylval = node(@symbolic, nil, acons(@name, intern(s+1),
				      current_file_line()));
  return SYMLITERAL;
}

int
floatliteral(char *s, int is_int)
{
    yylval = node(@float, nil,
		  acons(@value, intern(s),
			acons(@type, number_type, current_file_line())));
    if (is_int) set_attr(@convert, yylval, @true);
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
    node(@int, nil,
         acons(@value, fixnum(atoi(s)),
               acons(@convert, @true,
                     acons(@type, discrete_number_type,
                           current_file_line()))));
  return INTLITERAL;
}

int
stringlit(char *s)
{
    yylval = node(@string, nil, acons(@value, string(s), current_file_line()));
    return STRING;
}

/* end of file -- scanaux.c -- */
