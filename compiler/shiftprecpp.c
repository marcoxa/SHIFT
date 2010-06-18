 /* -*- Mode: C -*- */

/* shiftprecpp.c -- */

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

/*==========================================================================*/
#if 0
/*
   This is code for a PRE-preprocessor to handle the differential SHIFT "'"
   operator and allow Cpp symbols to be expanded properly on the same line.
  
   Here is the problem: if, for example, you have
  
  	#define RATE 1
  	...
  		t' = RATE;
  
   the symbol RATE won't get expanded.  This is a foible of
   both /lib/cpp and Gnu Cpp - character constants start
   and end with "'", and these C preprocessors want to see
   a closing quote before they expand anything else afterward
   on the same line.
  
   This is ordinarily a feature - if, for example, you say
  
  	#define n <something>
  
   you don't want '\n' (newline) expanding to '\<something>'.
   But these preprocessors aren't smart enough to get to the end of
   the line, and say "gee, I didn't see a closing quote", go back
   to the quote mark and expand symbols from there.
  
   This PRE-preprocessor fixes it up so that the above equation
   looks like this before it goes into the C preprocessor
  
  	__d_dt__(t) = RATE;
  
   where __d_dt__ is a macro that expands through Cpp as:
  
  	t' /* '*/ = 1;
  
   How does this work?  Like this:
  
   The space between the first (differential) tick and the
   beginning of the comment is necessary, because the shic
   scanner otherwise tries to interpret "'/*" as a single token.
   (Sloppily, in my view, but who asked me?)

   The second space (inside the comment) avoids the whole comment
   being taken as a single token as well.  At least, I could not
   get anything but a syntax error from shic without this extra space.

   The second tick (quote), embedded in the comment, is necessary
   because gcc C preprocessor is assuming a character
   constant " /* ", and complains if it does not get a closing
   quote; furthermore, /lib/cpp will not expand symbols for the
   rest of the line if you do not have this closing quote.

   The upshot: C preprocessors will see the above line as
  
  	t'<weird-but-acceptable-character-constant>'*/ = RATE;

   and will pass the text of the character constant through and expand
   RATE properly; after that, shic will be dealing only with           /*

  	t' /* <weird-but-acceptable-comment-character>*/ = 1;
  
   which it can apparently handle.
  
   CAVEATS:
  
  	- if array-references can take part in equations in
  	  some future SHIFT, allowing equations to take d/dt
  	  of array references, e.g.,					/*
  
  		v[0]' = ....
  
	  then the problem will come back, and with a vengeance; a
	  PRE-preprocessor approach gets much more complex in
	  that case;

	- changing shic's scanner so that it either doesn't
	  process comments or processes only "//"-type comments will
	  break this fix;

	- other C preprocessors might not even be able to
	  handle this, since they might scan for comments before
	  character constants, and excise them;

	- newer C preprocessors might try some tricky UNICODE semantics
	  on " /* ", and either barf it back up or digest it and pass
	  the excreta onto the poor mystified SHIFT compiler;

	- older C preprocessors might just plain barf on a "character
	  constant" like " /* ";

	- even older ones,....well,....

    IMPLEMENTATION:

	Look for an identifier, followed optionally by whitespace, then
	a "'", and change it to the above macro invocation on the
	identifier.
*/
#endif
/*==========================================================================*/

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#define PRINTF (void) printf
extern int errno;
extern void exit();

#define D_DT_MACRO "__d_dt__(%s)"
#define QUOTE  '\''
#define D_DT_CHAR QUOTE


static char id[256];
static char *ipd;
static in_id;

void
add_id_char(c)
	char c;
{
	if (in_id == 0)
		ipd = id;

	*ipd++ = c;
	in_id = 1;
}

void
emit_id(fmt)
	char *fmt;
{
	assert (in_id);

	*ipd = '\0';
	PRINTF (fmt, id);
	in_id = 0;
}

emit_d_dt_define()
{
 /* formal parameter to definition */
	char d_dt_arg = '_';

 /* emit definition header */
	PRINTF ("#define ");
	add_id_char(d_dt_arg);
	emit_id (D_DT_MACRO);

 /* emit definition body */
	PRINTF (" %c%c /* %c*/\n", d_dt_arg, QUOTE, QUOTE);
}

void
reset_place(line_no, filename)
	int line_no;
	char *filename;
{
	assert (filename != 0);
	assert (filename[0] != 0);

	PRINTF ("#line %d \"%s\"\n", line_no, filename);
}


int is_starting_id_char(c)
	char c;
{
	return isalpha(c) || c=='_';
}

int
is_id_char(c)
	char c;
{
	return isalnum(c) || c=='_';
}

output_char (c)
	int c;
{
	if (c != EOF)
		(void) putchar (c);
}

int
main(int argc, char **argv)
{
  register int c;
  FILE *fp;
  char *filename;

  assert (D_DT_CHAR == QUOTE);	/* only if this is the problem */

  assert (argc == 2);
  filename = argv[1];

  fp = fopen (filename, "r");

  if (fp == NULL)
    {
      perror (filename);
      exit (errno);
      /*NOTREACHED*/
    }

  emit_d_dt_define();

  reset_place (1,filename);

  while ((c = fgetc(fp)) != EOF)
    {
      if (!in_id)
	{
	  if (is_starting_id_char(c))
	    add_id_char(c);
	  else
	    output_char (c);
	}
      else if (is_id_char(c))
	add_id_char(c);
      else if (c == D_DT_CHAR)
	emit_id (D_DT_MACRO);
      else if (c == ' ' || c == '\t')
	add_id_char(c);
      else
	{
	  emit_id ("%s");
	  output_char (c);
	}
    }

  exit (0);
  /*NOTREACHED*/
}
