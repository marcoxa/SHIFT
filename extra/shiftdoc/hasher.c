/* -*- Mode: C -*- */

/* hasher.c -- */

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
#include "lisp.h"

/* Symbol Table Support. 
 * We need to allocate a symbol table here for the only benefit of the
 * 'hasher'.
 */

lv* shtab[L_HSIZE];


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
      sindex(s) = index++;
    }
  tsilod;
  if (argc == 2)
    {
      struct lispval *new = 0;
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
	  s = read_c_symbol(stdin);
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
      l = 0;
      for (i = 0; i < L_HSIZE; i++)
	for (s = shtab[i]; s; s = shlink(s))
	  push(s, l);
      assert(length(l) == index - 1);

      if ((fp = fopen(argv[2], "w")) == 0)
	{
	  perror(argv[2]);
	  exit(1);
	}
      fprintf(fp, "#include <stdio.h>\n");
      fprintf(fp, "#include \"lisp.h\"\n");
      fprintf(fp, "\n");
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
      for (i = 0; i < L_HSIZE; i++)
	fprintf(fp, "\t%s,\n", symref(shtab[i], 0));
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
