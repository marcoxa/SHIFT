/* -*- Mode: C -*- */

/* hasher.c -- */

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
 * Originally written by Edward Wang (edward@edcom.com)
 * Modified 1995-96 by Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* #include "lisp.h" */
#include "crscl.h"

/* Symbol Table Support. 
 * We need to allocate a symbol table here for the only benefit of the
 * 'hasher'.
 */

/* lv* shtab[L_HSIZE]; */

INIT_CRSCL_TABLES()


char *concat2(char *, char *);

char *symref(struct lispval *, int);

struct preload
{
  char *pname;
  int sflags;
} preload[] = {
  0
};

	

main(int argc, char** argv)
{
  int c;
  struct lispval *s, *l;
  int i;
  int index;

  if (argc < 2)
    {
      fprintf(stderr, "hasher: usage: hasher symbol-file [output-file]\n");
      exit(1);
    }
  l = read_sexpr(argv[1]);
  index = 1;

  dolist (s, l)
    {
      puts(">>> hasher: ");
      print(s);
      putchar('\n');
      sindex(s) = index++;
    }
  tsilod;
  if (argc == 2)
    {
      struct lispval *new = 0;
      rs* standard_input = make_stream_stream(stdin);
      FILE *fp;

      printf("#include \"%s.h\"\n", argv[1]);
      for (;;)
	{
	  c = getchar();
	  if (c == EOF)
	    break;
	  if (c != '@')
	    {
	      putchar(c);
	      continue;
	    }
	  s = read_c_symbol(standard_input);
	  if (pname(s)[0] == '\0')
	    {
	      /* for yacc on freebsd */
	      putchar('@');
	      continue;
	    }
	  if (sindex(s) == 0)
	    {
	      sindex(s) = index++;
	      push(s, new);
	    }
	  printf("(%s)", symref(s, 0));
	}
      if (new)
	{
	  l = nconc(l, nreverse(new));
	  write_sexpr(l, argv[1]);
	}
      if ((fp = fopen(concat2(argv[1], ".h"), "w")) == 0)
	{
	  perror(concat2(argv[1], ".h"));
	  exit(1);
	}
      dolist (s, l)
	{
	  fprintf(fp, "extern struct lispval %s;\n",
		  symref(s, 1));
	}
      tsilod;
      fclose(fp);
    }
  else
    {
      FILE *fp;
      struct preload *pp;

      for (pp = preload; pp->pname; pp++)
	{
	  s = intern(pp->pname);
	  if (sindex(s) == 0)
	    sindex(s) = index++;
	}
      l = nil;
      for (i = 0; i < crscl_table_size(); i++)
	for (s = crscl_table_ref(i); s; s = shlink(s))
	  push(s, l);
      assert(length(l) == index); /* There was a -1 (probably due to the NIL issue). */

      if ((fp = fopen(argv[2], "w")) == 0)
	{
	  perror(argv[2]);
	  exit(1);
	}
      fputs("#include <stdio.h>\n", fp);
      /* fprintf(fp, "#include \"lisp.h\"\n"); */
      fputs("#include \"crscl.h\"\n", fp);

      dolist (s, l)
	{
	  fprintf(fp,
		  "struct lispval %s = {L_SYMBOL, 0, 0, 0, {",
		  symref(s, 1));
	  print_string(pname(s), fp);
	  fprintf(fp, ", %s, 0, 0}};\n",
		  symref(shlink(s), 0));
	}
      tsilod;
      fprintf(fp, "\nstruct lispval *shtab[] = {\n");
      for (i = 0; i < crscl_table_size(); i++)
	fprintf(fp, "\t%s,\n", symref(crscl_table_ref(i), 0));
      fprintf(fp, "};\n");
      fclose(fp);
    }
  exit(0);
}

char *
symref(struct lispval * s, int deref)
{
  static char buf[32];

  if (s == 0)
    {
      assert(!deref);
      return "0";
    }
  sprintf(buf, "%sL_SYMBOL%d", deref ? "" : "&", sindex(s));
  return buf;
}

print_string(char* p, FILE* fp)
{
  int c;

  putc('"', fp);
  while (c = *((unsigned char *)p++))
    if (c < ' ' || c >= 0x7f || c == '"')
      fprintf(fp, "\\%03o", c);
    else
      putc(c, fp);
  putc('"', fp);
}

/* end of file -- hasher.c */
