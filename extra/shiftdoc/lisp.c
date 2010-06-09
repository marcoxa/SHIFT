/* -*- Mode: C -*- */

/* lisp.c -- */

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

#ifndef SHIFTC_LISP_I
#define SHIFTC_LISP_I

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <limits.h>

/* Added <ctype.h> for character conversion
 * routines.
 */
#include <ctype.h>

#include "lisp.h"

/* shtab -- Symbol table support. Each program that uses the functions
 * in this library must allocate a 'shtab'.
 * 'hasher' does so and the compiler does as well with the
 * automagically generated 'symbols.c'.
 * This works now well with compilers other than GCC. (E.g. SGI's cc).
 *
 * Marco Antoniotti 19971023
 */
extern lv* *shtab[/* L_HSIZE */];


/* List all the interface functions */

lv *intern(char *);
int shash(char *);

lv *read_sexpr(char *);
lv *read_sexpr1(FILE *);
lv *read_sexpr_list(FILE *);
lv *list_to_attrs(lv *);
lv *read_sexpr_string(FILE *);
lv *read_sexpr_symbol(FILE *);
lv *read_c_symbol(FILE *);
int write_sexpr(lv *, char *);
void print(lv *);
void write_sexpr1(lv *, FILE *);
lv *cons(lv *, lv *);
lv *string(const char *);
lv *stringl(char *, int);
lv *fixnum(int);
lv *symbol(char *);
lv *node(lv *, lv *, lv *);
lv *attr(lv *, lv *);
#define attr2(a1, a2, n) attr(a1, attr(a2, n))
#define attr3(a1, a2, a3, n) attr2(a1, a2, attr(a3, n))
void set_attr(lv *attr, lv *node, lv *value);
void push_attr(lv *attr, lv *node, lv *value);
lv *other(void *x);
lv *assoc(lv *, lv *);
int length(lv *);
lv *nth(int, lv *);
lv *list(lv *, ...);
lv *copy_tree(lv *);
lv *copy_list(lv *);
lv *nreverse(lv *);
lv *nreverse2(lv *, lv *);
lv *nconc(lv *, lv *);
lv *append(lv *, lv *);
lv *del_el(lv *, lv *);
lv *mapcar(lv *(*)(lv *), lv *);
lv *mapcarx(lv *(*)(), lv *, lv *);
lv *mapcan(lv *(*)(lv *), lv *);
void mapc(void (*)(lv *), lv *);
void mapcx(void (*)(lv *, lv*), lv *, lv *);
lv *member_op(lv *, lv *);
lv *memq(lv *, lv *);
lv *lalloc(void);
void gc_set_root(lv *);
void gc_clear_root(void);
void gc();
void new_node_marker();
void mark_node(lv* node);
int node_marked_p(lv* node);


/*
 * A data-structure package that provides Lisp-like features
 */

int
lv_type_of(lv* v)
{
  if (v != 0)
    return v->type;
  else
    return L_CONS;
}


void
print_lv_type(lv* v)
{
  switch (lv_type_of(v))
    {
    case L_FREE:
      fputs("L_FREE", stdout);
      break;
    case L_CONS:
      fputs("L_CONS", stdout);
      break;
    case L_STRING:
      fputs("L_STRING", stdout);
      break;
    case L_SYMBOL:
      fputs("L_SYMBOL", stdout);
      break;
    case L_NODE:
      fputs("L_NODE", stdout);
      break;
    case L_FIXNUM:
      fputs("L_FIXNUM", stdout);
      break;
    default:
      fputs("L_OTHER", stdout);
      break;
    }
  putc('\n', stdout);
}


/*
 * Reader
 */

lv *
read_sexpr(char* file)
{
  FILE *fp;
  lv *s;
    
  if ((fp = fopen(file, "r")) == 0)
    return 0;
  s = read_sexpr1(fp);
  fclose(fp);
  return s;
}


lv *
read_sexpr1(FILE* fp)
{
  int c;
  lv *s;
    
again:
  for (; (c = getc(fp)) != EOF && (c == ' ' || c == '\t' || c == '\n');)
    ;
  switch (c)
    {
    case ';':
      for (; (c = getc(fp)) != EOF && c != '\n';)
	;
      goto again;
    case EOF:
      s = 0;
      break;
    case '(':
      s = read_sexpr_list(fp);
      break;
    case '"':
      s = read_sexpr_string(fp);
      break;
    default:
      ungetc(c, fp);
      s = read_sexpr_symbol(fp);
    }
  return s;
}


lv *
read_sexpr_list(FILE* fp)
{
  int c;
  lv *s;
    
  for (; (c = getc(fp)) != EOF && (c == ' ' || c == '\t' || c == '\n');)
    ;
  switch (c)
    {
    case EOF:
    case ')':
      s = 0;
      break;
    default:
      ungetc(c, fp);
      s = read_sexpr1(fp);
      s = cons(s, read_sexpr_list(fp));
      break;
    }
  return s;
}


static char *read_buf;
static read_bufsiz;


lv *
read_sexpr_string(FILE* fp)
{
  int c;
  int i = 0;
    
  if (read_buf == 0)
    {
      read_buf = malloc(read_bufsiz = 1024);
      if (read_buf == 0)
	{
	  fputs("lisp.c:read_sexpr_string: unable to allocate heap memory.\n",
		stderr);
	  fflush(stderr);
	  exit(1);
	}
    }
  for (; (c = getc(fp)) != EOF && c != '"';)
    {
      if (c == '\\' && (c = getc(fp)) == EOF)
	break;
      read_buf[i++] = c;
      if (i >= read_bufsiz)
	{
	  read_buf = realloc(read_buf, read_bufsiz *= 2);
	  if (read_buf == 0)
	    {
	      fputs("lisp.c:read_sexpr_string: unable to allocate heap memory.\n",
		    stderr);
	      fflush(stderr);
	      exit(1);
	    }
	}
    }
  read_buf[i++] = 0;
  return string(read_buf);
}


lv *
read_sexpr_symbol(FILE* fp)
{
  int c;
  int i = 0;
    
  if (read_buf == 0)
    {
      read_buf = malloc(read_bufsiz = 1024);
      if (read_buf == 0)
	{
	  fputs("lisp.c: unable to allocate heap memory.\n", stderr);
	  fflush(stderr);
	  exit(1);
	}
    }

  for (;;)
    {
      c = getc(fp);
      switch (c)
	{
	case ';':
	case '(':
	case ')':
	case '"':
	case '&':
	  ungetc(c, fp);
	case EOF:
	case '\n':
	case '\r':
	case '\t':
	case '\f':
	case ' ':
	  goto out;
	case '\\':
	  if ((c = getc(fp)) == EOF)
	    goto out;
	}
      read_buf[i++] = c;
      if (i >= read_bufsiz)
	{
	  read_buf = realloc(read_buf, read_bufsiz *= 2);
	  if (read_buf == 0)
	    {
	      fputs("lisp.c: unable to allocate heap memory.\n", stderr);
	      fflush(stderr);
	      exit(1);
	    }
	}
    }
out:
  read_buf[i++] = 0;
  return intern(read_buf);
}


lv *
read_quoted_symbol(FILE *fp)
{
  int c;
  int i = 0;
    
  if (read_buf == 0)
    {
      read_buf = malloc(read_bufsiz = 1024);
      if (read_buf == 0)
	{
	  fputs("lisp.c: unable to allocate heap memory.\n", stderr);
	  fflush(stderr);
	  exit(1);
	}
    }
  for (;;)
    {
      c = getc(fp);
      switch (c)
	{
	case '|':
	  goto out;
	case '\\':
	  if ((c = getc(fp)) == EOF)
	    goto out;
	}
      read_buf[i++] = c;
      if (i >= read_bufsiz)
	{
	  read_buf = realloc(read_buf, read_bufsiz *= 2);
	  if (read_buf == 0)
	    {
	      fputs("lisp.c: unable to allocate heap memory.\n", stderr);
	      fflush(stderr);
	      exit(1);
	    }
	}
    }
out:
  read_buf[i++] = 0;
  return intern(read_buf);
}


lv *
read_c_symbol2(FILE *fp)
{
  int c;
  int i = 0;
    
  if (read_buf == 0)
    {
      read_buf = malloc(read_bufsiz = 1024);
      if (read_buf == 0)
	{
	  fputs("lisp.c: unable to allocate heap memory.\n", stderr);
	  fflush(stderr);
	  exit(1);
	}
    }
  for (;;)
    {
      c = getc(fp);
      switch (c)
	{
	case ',':
	case ':':
	case ';':
	case '(':
	case ')':
	case '"':
	case '&':
	case '\n':
	case '\r':
	case '\t':
	case '\f':
	case ' ':
	  ungetc(c, fp);
	case EOF:
	  goto out;
	case '\\':
	  if ((c = getc(fp)) == EOF)
	    goto out;
	}
      read_buf[i++] = c;
      if (i >= read_bufsiz)
	{
	  read_buf = realloc(read_buf, read_bufsiz *= 2);
	  if (read_buf == 0)
	    {
	      fputs("lisp.c: unable to allocate heap memory.\n", stderr);
	      fflush(stderr);
	      exit(1);
	    }
	}
    }
out:
  read_buf[i++] = 0;
  return intern(read_buf);
}


lv *
read_c_symbol(FILE* fp)
{
  int c;

  if ((c = getc(fp)) == '"')
    {
      lv *x = read_sexpr_string(fp);
      return intern(str(x));
    }
  else
    {
      ungetc(c, fp);
      return read_c_symbol2(fp);
    }
}


/*
 * Print functions
 */

int
write_sexpr(lv* s, char* file)
{
  FILE *fp;
    
  if ((fp = fopen(file, "w")) == 0)
    return -1;
  write_sexpr1(s, fp);
  fclose(fp);
  return 0;
}


void
print(lv* s)
{
  write_sexpr1(s, stdout);
}


/* For debugging.
 */
void
p(lv* s)
{
  print(s);
  fflush(stdout);
}


/* For debugging.
 */
void
f()
{
  fflush(stdout);
}

/* For debugging.
 */
void
pattr(char* as, lv* n)
{
  p(attr(intern(as), n));
}

static int dont_loop = 1;
static int print_addresses = 0;
static int print_level = 10;

static int sexpr_mark = 1;

void
write_sexpr1_(lv* s, FILE* fp, int level)
{
  char *p;
    
  if (s == 0)
    {
      fprintf(fp, "()");
      return;
    }
  if (print_addresses) fprintf(fp, "0x%x: ", s);
  if (level >= print_level)
    {
      switch (s->type)
	{
	case L_CONS:
	  fprintf(fp, "(...)");
	  return;
	case L_NODE:
	  fprintf(fp, "#[...]");
	  return;
	}
    }
  if (dont_loop && s->mark)
    {
      switch (s->type)
	{
	case L_CONS:
	  fprintf(fp, "#=%d()", s->mark);
	  return;
	case L_NODE:
	  fprintf(fp, "#=%d#[...]", s->mark);
	  return;
	}
    }
  if (dont_loop) s->mark = sexpr_mark++;

  switch (s->type)
    {
    case L_CONS:
      fprintf(fp, "(");
      write_sexpr1_(hd(s), fp, level + 1);
      while (s = tl(s))
	{
	  if (s->type == L_CONS)
	    {
	      fprintf(fp, " ");
	      write_sexpr1_(hd(s), fp, level + 1);
	    }
	  else
	    {
	      fprintf(fp, " . ");
	      write_sexpr1_(s, fp, level + 1);
	    }
	}
      fprintf(fp, ")\n");
      break;

    case L_STRING:
      putc('"', fp);
      for (p = str(s); *p; p++)
	{
	  if (*p == '"')
	    putc('\\', fp);
	  putc(*p, fp);
	}
      putc('"', fp);
      break;

    case L_FIXNUM:
      printf("%d", num(s));
      break;

    case L_SYMBOL:
      for (p = pname(s); *p; p++)
	{
	  switch (*p)
	    {
	    case ';':
	    case '(':
	    case ')':
	    case '"':
	    case '&':
	    case '\n':
	    case '\r':
	    case '\t':
	    case '\f':
	    case ' ':
	    case '\\':
	      putc('\\', fp);
	    }
	  putc(*p, fp);
	}
      break;

    case L_NODE:
      fprintf(fp, "#[");
      write_sexpr1_(op(s), fp, level + 1);
      if (args(s))
	{
	  fprintf(fp, " ARGUMENTS: ");
	  write_sexpr1_(args(s), fp, level + 1);
	}
      if (attrs(s))
	{
	  fprintf(fp, " ATTRIBUTES: ");
	  write_sexpr1_(attrs(s), fp, level + 1);
	}
      fprintf(fp, "]\n");
      break;

    case L_OTHER:
      fprintf(fp, "#<other 0x%x>", oth(s));
      break;
    }
}

static void markv(lv *, int);

void write_sexpr1(lv *s, FILE* fp)
{
  write_sexpr1_(s, fp, 0);
  if (dont_loop) markv(s, 0);	/* unmark, really. */
}


/*
 * Basic constructors
 */

lv *
cons(lv *a, lv *b)
{
  lv *s;
    
  s = lalloc();
  s->type = L_CONS;
  hd(s) = a;
  tl(s) = b;
  return s;
}


lv *
string(const char* p)
{
  lv *s;
    
  s = lalloc();
  s->type = L_STRING;
  str(s) = malloc(strlen(p) + 1);
  if (str(s) == 0)
    {
      fputs("lisp.c: unable to allocate heap memory.\n", stderr);
      fflush(stderr);
      exit(1);
    }
  strcpy(str(s), p);
  return s;
}


lv *
stringl(char *p, int n)
{
  lv *s;
    
  s = lalloc();
  s->type = L_STRING;
  str(s) = malloc(n + 1);
  if (str(s) == 0)
    {
      fputs("lisp.c: unable to allocate heap memory.\n", stderr);
      fflush(stderr);
      exit(1);
    }
  memcpy(str(s), p, n);
  str(s)[n] = 0;
  return s;
}


lv *
fixnum(int v)
{
  lv *s;
    
  s = lalloc();
  s->type = L_FIXNUM;
  num(s) = v;
  return s;
}


lv *
symbol(char* pname)
{
  lv *s;
    
  s = lalloc();
  s->type = L_SYMBOL;
  pname(s) = malloc(strlen(pname) + 1);
  if (pname(s) == 0)
    {
      fputs("lisp.c: unable to allocate heap memory.\n", stderr);
      fflush(stderr);
      exit(1);
    }
  strcpy(pname(s), pname);
  sindex(s) = 0;
  plist(s) = 0;
  return s;
}


lv *
node(lv* op, lv* args, lv* attrs)
{
  lv *s;
  
  s = lalloc();
  s->type = L_NODE;
  op(s) = op;
  attrs(s) = attrs;
  args(s) = args;
  mark_attr(s) = 0;
  return s;
}


lv *
other(void* x)
{
  lv *s = lalloc();
  s->type = L_OTHER;
  oth(s) = x;
  mark_attr(s) = 0;
  return s;
}


void
node_set_op(lv* s, lv* op)
{
  op(s) = op;
}


/* Marking nodes.
 */

int mark_number = 0;

void
new_node_marker()
{
  mark_number %= INT_MAX;	/* Being paranoid. Marco Antoniotti 19970106 */
  mark_number++;
}

void
mark_node(lv *node)
{
  mark_attr(node) = mark_number;
}

int
node_marked_p(lv *node)
{
  return mark_attr(node) == mark_number;
}


/*
 * Intern
 */

lv *
intern(char* pname)
{
  lv *s, **sp;
  
  /* The next cast is needed for some C compilers */

  for (sp = (lv**) &shtab[shash(pname)]; (s = *sp); sp = &shlink(s))
    if (strcmp(pname(s), pname) == 0)
      return s;
  return *sp = symbol(pname);
}


int
shash(char* p)
{
  unsigned h = 0;
    
  for (; *p; p++)
    {
      h ^= *(unsigned char *)p;
      h = h >> (L_HBITS - L_HSHIFT) | h << L_HSHIFT;
      h &= L_HMASK;
    }
  return h;
}


/*
 * Standard Lisp-like functions
 */

lv *
list(lv *a, ...)
{
  va_list ap;
  lv *s, *e;
    
  if (a == 0)
    return 0;
  e = s = cons(a, 0);
  va_start(ap, a);
  while (a = va_arg(ap, lv *))
    e = tl(e) = cons(a, 0);
  va_end(ap);
  return s;
}


int
length(lv* s)
{
  int n = 0;
    
  if (s != 0)
    {
      assert(consp(s));
      for(; s; s = tl(s))
	n++;
    }
  return n;
}


lv *
nth(int n, lv* l)
{
  int i = 0;
  for(; l; l = tl(l), i++)
    if (i == n) return hd(l);
  return nil;
}


lv *
copy_tree(lv* s)
{
  if (s == 0)
    return s;
  switch (s->type)
    {
    case L_CONS:
      return cons(copy_tree(hd(s)), copy_tree(tl(s)));
    case L_STRING:
    case L_SYMBOL:
      return s;
    case L_NODE:
      return node(op(s), copy_tree(attrs(s)), copy_tree(args(s)));
    default:
      assert(0);
    }
}


lv *
copy_list(lv* s)
{
  lv *x = 0;
  lv **p = &x;
    
  for (; s; s = tl(s))
    {
      assert(s->type == L_CONS);
      *p = cons(hd(s), 0);
      p = &tl(*p);
    }
  return x;
}


lv *
assoc(lv* k, lv* al)
{
  if (al)
    {
      if (eq(k, hd(hd(al))))
	{
	  return hd(al);
	}
      else
	return assoc(k, tl(al));
    }
  else
    return nil;
}


void
reassoc_(lv* k, lv* v, lv** lp)
{
  lv *x = assoc(k, *lp);

  if (x)
    {
      tl(x) = v;
    }
  else
    {
      apush(k, v, *lp);
    }
}


lv *
attr(lv* symbol, lv* node)
{
  lv *c = assoc(symbol, attrs(node));
  return c? tl(c) : nil;
}


void
set_attr(lv* symbol, lv* node, lv* value)
{
  reassoc(symbol, value, attrs(node));
}


/* Equivalent to set_attr(SYMBOL, NODE, cons(VALUE, attr(SYMBOL, NODE)))
 */
void
push_attr(lv* symbol, lv* node, lv* value)
{
  lv *c = assoc(symbol, attrs(node));

  if (c)
    push(value, tl(c));
  else
    apush(symbol, list1(value), attrs(node));
}


lv *
nreverse(lv* s)
{
  lv *x = 0;
    
  while (s)
    {
      lv *y;

      assert(s->type == L_CONS);
      y = tl(s);
      tl(s) = x;
      x = s;
      s = y;
    }
  return x;
}


lv *
nreverse2(lv* s, lv* e)
{
  lv *x = 0;
    
  while (s != e)
    {
      lv *y;

      assert(s->type == L_CONS);
      y = tl(s);
      tl(s) = x;
      x = s;
      s = y;
    }
  return x;
}


lv *
nconc(lv* a, lv* b)
{
  lv *x, *y;
    
  if (a == 0)
    return b;
  if (b == 0)
    return a;
  for (x = a; y = tl(x); x = y)
    ;
  tl(x) = b;
  return a;
}


lv *
append(lv* a, lv* b)
{
  lv *x, **p = &x;

  if (a == 0) return b;
  if (b == 0) return a;
  for (; a; a = tl(a))
    {
      *p = cons(hd(a), nil);
      p = &tl(*p);
    }
  *p = b;
  return x;
}


lv *
del_el(lv* a, lv* b)
{
  if (b == nil)
    return b;
  else if (a == first(b))
    return rest(b);
  else
    return cons(first(b), del_el(a, rest(b)));
}


lv *
mapcar(lv* (*f)(lv *), lv* s)
/* lv *(*f)(lv *);
   lv *s;
 */
{
  lv *x = 0;
  lv **p = &x;
    
  for (; s; s = tl(s))
    {
      assert(s->type == L_CONS);
      *p = cons((*f)(hd(s)), 0);
      p = &tl(*p);
    }
  return x;
}


lv *
mapcarx(lv* (*f)(lv *, lv *), lv* s, lv* a)
/* lv *(*f)(lv *, lv *);
   lv *s;
   lv *a;
 */
{
  lv *x = 0;
  lv **p = &x;
    
  for (; s; s = tl(s))
    {
      assert(s->type == L_CONS);
      *p = cons((*f)(hd(s), a), 0);
      p = &tl(*p);
    }
  return x;
}


static lv*
mapcan_tr(lv* (*f)(lv *), lv* s, lv * acc)
{
  if (s == nil)
    return acc;
  else
    return mapcan_tr(f, tl(s), nconc(acc, (*f)(hd(s))));
}


lv *
mapcan(lv* (*f)(lv *), lv* s)
{
  return mapcan_tr(f, s, nil);
}


void
mapc(void (*f)(lv *), lv* s)
/* void (*f)(lv *);
   lv *s;
 */
{
  for (; s; s = tl(s))
    (*f)(hd(s));
}


void
mapcx(void (*f)(lv *, lv*), lv* s, lv* a)
/* void (*f)(lv *, lv*);
   lv *s, *a;
 */
{
  for (; s; s = tl(s))
    (*f)(hd(s), a);
}


lv *
member_op(lv* s, lv* a)
{
  for (; s; s = tl(s))
    {
      if (nodep(hd(s)) && eq(op(hd(s)), a))
	break;
    }
  return s;
}


lv*
memq(lv* el, lv* l)
{
  if (l == nil)
    return nil;
  else if (hd(l) == el)
    return l;
  else
    return memq(el, tl(l));
}


/*
 * Allocation and collection
 */

#define CHUNK 1024
lv *freelist;
lv *alloclist;


lv *
lalloc()
{
  lv *p;
    
again:
  if ((p = freelist) == 0)
    {
      int i;

      p = malloc(sizeof *p * CHUNK);
      if (p == 0)
	{
	  fputs("lisp.c: unable to allocate heap memory.\n", stderr);
	  fflush(stderr);
	  exit(1);
	}
      for (i = CHUNK; --i >= 0; p++)
	{
	  p->type = L_FREE;
	  p->link = freelist;
	  freelist = p;
	}
      goto again;
    }
  freelist = p->link;
  p->mark = 0;
  p->flags = 0;
  p->link = alloclist;
  alloclist = p;
  return p;
}


lv *rootlist;

void
gc_set_root(lv* s)
{
  rootlist = cons(s, rootlist);
}


void
gc_clear_root()
{
  rootlist = 0;
}

static void mark(lv *p);
static void sweep();
static int mark_value = 1;

void
gc()
{
  mark(rootlist);
  sweep();
}


static void
mark(lv* p)
{
again:
  if (p == 0 || p->mark == mark_value) return;
  p->mark = mark_value;
  switch (p->type)
    {
    case L_STRING:
    case L_OTHER:
    case L_FIXNUM:
      break;
    case L_SYMBOL:
      p = plist(p);
      goto again;
    case L_CONS:
      mark(hd(p));
      p = tl(p);
      goto again;
    case L_NODE:
      if (op(p) != 0 && op(p)->type != L_SYMBOL)
	mark(op(p));
      mark(attrs(p));
      p = args(p);
      goto again;
    default:
      assert(0);
    }
}


static void
markv(lv* p, int v)
{
  int old = mark_value;

  mark_value = v;
  mark(p);
  mark_value = old;
}


static void
sweep()
{
  lv *p, *q;
  lv *a = 0;
  lv *f = freelist;
    
  for (p = alloclist; p; p = q)
    {
      q = p->link;
      if (p->flags & L_STATIC)
	;
      else if (p->mark)
	{
	  p->mark = 0;
	  p->link = a;
	  a = p;
	}
      else
	{
	  switch (p->type)
	    {
	    case L_SYMBOL:
	      free(pname(p));
	      break;
	    case L_STRING:
	      free(str(p));
	      break;
	    }
	  p->type = L_FREE;
	  p->link = f;
	  f = p;
	}
    }
  alloclist = a;
  freelist = f;
}

#endif /* SHIFTC_LISP_I */

/* end of file -- lisp.c -- */
