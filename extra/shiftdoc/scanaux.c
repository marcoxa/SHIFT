/* -*- Mode: C -*- */

/* scanaux.c -- */

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
