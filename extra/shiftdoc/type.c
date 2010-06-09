/* -*- Mode: C -*- */

/* type.c -- */

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
 * Copyright 1995-96 Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */

/* Type machinery, and auxiliary functions.
 */

#ifndef SHIFT_COMP_TYPE_I
#define SHIFT_COMP_TYPE_I

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <limits.h>

#include "lisp.h"
#include "shic.h"

#define LINE_LENGTH 80

void
internal_error(char *message)
{
  fprintf(stderr, "shic: internal compiler error: ");
  if (strlen(message) < LINE_LENGTH - 31) /* 31 is the length of the header. */
    fprintf(stderr, "%s.\n", message);
  else
    fprintf(stderr, "\n      %s\n", message);
  fflush(stderr);
  exit(1);
}


lv*
decl_type(lv* decl)
{
  if (op(decl) != @declare)
    internal_error("'decl_type' called on a non declaration.");
  else
    return arg1(decl);
}

int
equal_type(lv *x, lv *y)
{
  lv *ox = op(x);
  lv *oy = op(y);

  
  /* identical type */
  if (x == y) return 1;

  /* either one is error type */
  if (x == error_type || y == error_type) return 1;

  /* numbers */
  if (ox == @number_type && oy == @number_type) return 1;

  /* sets */
  if (ox == @set && oy == @set && equal_type(arg1(x), arg1(y))) return 1;

  /* arrays */
  if (ox == @array && oy == @array && equal_type(arg1(x), arg1(y))) return 1;

  if ((ox == @array && oy == @empty_array) || (ox == @set && oy == @null_set))
    {
      /* This is an interesting case.  It might be that the node
       * representing the empty set or array is not "typed" yet.  We
       * take the chance to mark it now with a possibly correct type.
       */
      if (args(y) == nil) args(y) = args(x);
      return 1;
    }

  if ((oy == @array && ox == @empty_array) || (oy == @set && ox == @null_set))
    {
      /* Same as above, but with 'ox' and 'oy' switched. */
      if (args(x) == nil) args(x) = args(y);
      return 1;
    }


  if ((ox == @array || ox == @set) && oy == @"nil"
      || (oy == @array || oy == @set) && ox == @"nil")
    return 1;

  /* component types */
  if (ox == @id && oy == @id)
    {
      return attr(@entity, x) == attr(@entity, y);
    }

  return 0;
}

int
set_type_p(lv *type)
{
  return type == error_type || op(type) == @set || op(type) == @null_set;
}


int
null_set_type_p(lv* type)
{
  return type == error_type || op(type) == @null_set;
}


int
array_type_p(lv *type)
{
  return type == error_type || op(type) == @array || op(type) == @empty_array;
}


int
empty_array_type_p(lv *type)
{
  return type == error_type || op(type) == @empty_array;
}


int
range_type_p(lv* type)
{
  return type == error_type || op(type) == @range;
}

int
type_type_p(lv *type)
{
    return type == error_type || op(type) == @type_or_set;
}

int
number_type_p(lv *type)
{
  return type == error_type || op(type) == @number_type;
}

int
discrete_number_type_p(lv *type)
{
  return type == error_type || (op(type) == @number_type &&
				attr(@continuous, type) == nil);
}

int 
continuous_number_type_p(lv *type)
{
  return type == error_type || (op(type) == @number_type &&
				attr(@continuous, type) == @true);
}

int
symbol_type_p(lv* type)
{
  return type == error_type || op(type) == @symbol_type;
}


int
logical_type_p(lv* type)
{
  return type == error_type || op(type) == @logical_type;
}


int
nil_type_p(lv* type) 
{
  return type == error_type || op(type) == @"nil";
}


int
function_type_p(lv *type)
{
  return type == error_type || op(type) == @function;
}


int
out_formal_p(lv* formal_par)
{
  lv* io_qual = attr(@io_qual, formal_par);

  return io_qual == @out || io_qual == @i_o;
}


int
in_formal_p(lv* formal_par)
{
  lv* io_qual = attr(@io_qual, formal_par);

  return io_qual == @in || io_qual == @i_o;
}


int
in_out_formal_p(lv* formal_par)
{
  lv* io_qual = attr(@io_qual, formal_par);

  return io_qual == @i_o;
}


/* L_value_p -- Is the expression assignable? */
int
L_value_p(lv* expr)
{
  lv* expr_op = op(expr);

  if (expr_op == @index || expr_op == @id || expr_op == @access)
    return 1;
  else
    return 0;
}


/* descendant -- Is X a descendant of Y?
 *
 * Note: Added test for special 'nil' case.  'nil' is a subtype of any
 * type defined by the user.
 *
 * Marco Antoniotti 19970707
 */
int
descendant(lv *x, lv *y)
{
  lv *p;

  if (equal_type(x, y))
    return 1;
  else
    {
      lv *mymean;

      /* Check if it is a user type - Note that
       * user types are id nodes.
       */
      if (op(x) == @id && op(y) == @id)
	{
          mymean = meaning(x);
          if (mymean)
	    p = attr(@parent, meaning(x));
          else
	    p = nil;
	  if (p) 
	    return descendant(p, y);
	}
      else if (op(x) == @"nil" && op(y) == @id
	       || op(x) == @"nil" && op(y) == @"nil")
	{
	  return 1;
	}
    }
  return 0;
}


/* type_compatible_with_p -- Returns true if type 't1' is equal or a
 * descendant of type 't2'.
 * Usually called as
 *
 * type_compatible_p(type_of(t1), type_of(t2));
 *
 * Note: the tests used in the case of sets and arrays, call
 * 'equal_type' out of efficiency and unfortunate consequences
 * stemming from the problems dealing with null sets and arrays.
 * 'equal_type' performs a key side effect in determining the base
 * type of the set.  Hence, the call.
 *
 * Marco Antoniotti 19970724
 */
int
type_compatible_with_p(lv* t1, lv* t2)
{
  if (nil_type_p(t1))
    return (op(t2) == @id	/* Assuming a SHIFT 'type' */
	    || symbol_type_p(t2)
	    || set_type_p(t2)
	    || array_type_p(t2));
  else if (number_type_p(t1))
    return number_type_p(t2);
  else if (logical_type_p(t1))
    return logical_type_p(t2);
  else if (symbol_type_p(t1))
    return symbol_type_p(t2);
  else if (set_type_p(t1))
    return (set_type_p(t2)
	    && (equal_type(t1, t2)
		|| type_compatible_with_p(arg1(t1), arg1(t2))));
  else if (array_type_p(t1))
    return (array_type_p(t2)
	    && (equal_type(t1, t2)
		|| type_compatible_with_p(arg1(t1), arg1(t2))));
  else if (op(t2) == @id && op(t1) == @id)
    return descendant(t1, t2);
  else
    return 0;
}

/* super_type_p --- Definition.
 *
 * This function is similar to type_compatible_with_p
 * but it distinguishes between continuous number and
 * numbers: a number is a supertype of continuous number
 *
 * Tunc Simsek 24th Sept., 1997
 *
 */
super_type_p(lv *t1, /* The supertype that is being tested */
	     lv *t2) /* The subtype */
{
  if (nil_type_p(t1))
    return (op(t2) == @id	
	    || symbol_type_p(t2)
	    || set_type_p(t2)
	    || array_type_p(t2));
  else if (discrete_number_type_p(t1))
    return (discrete_number_type_p(t2) || 
	    continuous_number_type_p(t2));
  else if (continuous_number_type_p(t1))
    return continuous_number_type_p(t2);
  else if (logical_type_p(t1))
    return logical_type_p(t2);
  else if (symbol_type_p(t1))
    return symbol_type_p(t2);
  else if (set_type_p(t1))
    return (set_type_p(t2)
	    && (equal_type(t1, t2)
		|| type_compatible_with_p(arg1(t1), arg1(t2))));
  else if (array_type_p(t1))
    return (array_type_p(t2)
	    && (equal_type(t1, t2)
		|| type_compatible_with_p(arg1(t1), arg1(t2))));
  else if (op(t2) == @id && op(t1) == @id)
    return descendant(t2, t1);
  else
    return 0;

} /* int super_type_p */

/* find_types_ancestor -- This function assumes single inheritance.
 *
 * Marco Antoniotti 19970723
 */

lv*
find_types_ancestor(lv* t1, lv* t2)
{
  lv* parent;
  lv* mark;

  if (op(t1) != @id || op(t2) != @id)
    return nil;
  else if (equal_type(t1, t2))
    return t1;
  else
    {
      new_node_marker();
      mark_node(meaning(t1));
      for (parent = attr(@parent, meaning(t1));
	   parent != nil;
	   parent = attr(@parent, meaning(parent)))
	mark_node(meaning(parent));

      for (parent = t2; parent != nil; parent = attr(@parent, meaning(parent)))
	{
	 
	  if (node_marked_p(meaning(parent)))
	    {
	      /* if we have a marked parent and the mark is equal to
	       * the current search mark, then we have a winner!
	       */
	      return parent;
	    }
	}
    }
  return nil;
}


/* This should be called only after rewriting.
 */
lv *
compute_type(lv *expr)
{
  lv *x = op(expr);

  if (x == @call)
    return arg1(type_of(arg1(expr)));
  else if (x == @exists)
    return logical_type;
  else if (x == @int || x == @float)
    return number_type;
  else if (x == @symbolic)
    return symbol_type;
  else if (x == @assign
	   || x == @opassign
	   || x == @sync
	   || x == @donothing
	   || x == @dnth)
    return void_type;
  else if (x == @create)
    return arg1(expr);
  else if (x == @access)
    return attr(@type, attr(@accessor, expr));
  else if (x == @minel || x == @maxel)
    return attr(@type, attr(@id, expr));
  else
    internal_error("requesting type of unrewritten expression");
}


/* Lazily compute the type.
 */
lv *
type_of(lv *expr)
{
  lv *type = attr(@type, expr);
  lv* aggregate_type;

  if (type)
    {
      if (op(type) == @type_of)
	{
	  if (node_marked_p(expr))
	    {
	      user_error(find_leaf(expr),
			 "unresolvable type for expression");
	      type = error_type;
	    }
	  else
	    {
	      mark_node(expr);
	      type = type_of(rewrite_expression(arg1(type), arg2(type)));
	    }
	}
      else if (op(type) == @element_type_of)
	{
	  aggregate_type = type_of(node(@x, nil, alist1(@type, arg1(type))));
	  if (aggregate_type == error_type)
	    {
	      type = error_type;
	    }
	  else if (set_type_p(aggregate_type) || range_type_p(aggregate_type))
	    {
	      type = arg1(aggregate_type);
	    }
	  else
	    {
	      user_error(find_leaf(expr), "expression must have set type");
	      type = error_type;
	    }
	}
    }
  else
    {
      type = compute_type(expr);
    }
  set_attr(@type, expr, type);
  if (op(expr) == @id)
    {
      set_attr(@type, attr(@entity, expr), type);
    }
  return type;
}

lv *
find_leaf(lv *n)
{
    if (op(n) == @id || op(n) == @numliteral) return n;
    if (op(n) == @error) return nil;
    dolist (x, args(n)) {
	lv *y = find_leaf(x);
	if (y) return y;
    } tsilod;
    return nil;
}

/* Error messages.
 */
int n_errors   = 0;
int n_warnings = 0;

void
user_error(lv *id, char *format, ...)
{
  va_list ap;
  lv *a;
  char *s = format;

  n_errors++;
  if (id != nil)
    {
      lv* file = attr(@file, id);
      lv* line = attr(@line, id);

      fprintf(stderr, "%s:",
	      file == nil ? "unknown location" : str(file));
      if (line != nil)
	fprintf(stderr, "%d: ", num(line));
    }
  else
    {
      fprintf(stderr, "unknown location: ");
    }

  va_start(ap, format);	/* Varargs start. */
  while (*s)
    {
      switch (*s)
	{
	case '~':
	  s++;
	  switch (*s)
	    {
	    case 'a':
	      {
		lv *a = va_arg(ap, lv *);
		if (symbolp(a))
		  {
		    fprintf(stderr, "%s", pname(a));
		  }
		else if (stringp(a))
		  {
		    fprintf(stderr, "%s", str(a));
		  }
		else {
		  write_sexpr1(a, stderr);
		}
		s++;
		break;
	      }
	    case 's':
	      {
		lv *a = va_arg(ap, lv *);
		write_sexpr1(a, stderr);
		s++;
		break;
	      }
	    }
	default:
	  putc(*s, stderr);
	  s++;
	  break;
	}
    }
  putc('\n', stderr);
  va_end(ap);			/* Varargs end. */

  if (n_errors > 100)
    {
      fprintf(stderr, "Too many errors!\n");
      exit(1);
    }
}



void
user_warning(lv *id, char *format, ...)
{
  va_list ap;
  lv *a;
  char *s = format;

  n_warnings++;
  if (id)
    {
      char *file = str(attr(@file, id));
      int line = num(attr(@line, id));
      fprintf(stderr, "%s:%d: ", file, line);
    }
  else
    {
      fprintf(stderr, "unknown location: ");
    }

  va_start(ap, format);	/* Varargs start. */
  while (*s)
    {
      switch (*s)
	{
	case '~':
	  s++;
	  switch (*s)
	    {
	    case 'a':
	      {
		lv *a = va_arg(ap, lv *);
		if (symbolp(a))
		  {
		    fprintf(stderr, "%s", pname(a));
		  }
		else if (stringp(a))
		  {
		    fprintf(stderr, "%s", str(a));
		  }
		else {
		  write_sexpr1(a, stderr);
		}
		s++;
		break;
	      }
	    case 's':
	      {
		lv *a = va_arg(ap, lv *);
		write_sexpr1(a, stderr);
		s++;
		break;
	      }
	    }
	default:
	  putc(*s, stderr);
	  s++;
	  break;
	}
    }
  putc('\n', stderr);
  va_end(ap);			/* Varargs end. */
}


/* For debugging.
 */
static lv *tree;

void
summarize(lv *n)
{
  if (consp(n))
    {
      printf("list(%d) at 0x%x\n", length(n), n);
    }
  else if (nodep(n))
    {
      fputs("node ", stdout);
      print(op(n));
      if (op(n) == @id) printf(" [%s]", pname(attr(@name, n)));
      if (args(n))
	{
	  int l = length(args(n));
	  printf(" (%d arg%s)", l, l > 1? "s" : "");
	}
      else
	fputs(" ()", stdout);

      if (attrs(n))
	{
	  printf(" (attrs:");
	  dolist(a, attrs(n)) {
	    if (tl(a)) {
	      printf(" %s", pname(hd(a)));
	    }
	  } tsilod;
	  printf(")");
	}
      else
	fputs(" (no attributes)", stdout);

      printf("\n");
    }
  else if (symbolp(n) || fixnump(n))
    {
      print(n);
      printf("\n");
    }
  else
    {
      printf("type %d at 0x%x\n", n->type, n);
    }
}

int
browse(lv *n)
{
  char buf[100];
  int consp_index = 0;

  while (1)
    {
      if (n)
	summarize(n);
      else
	printf("NIL\n");
      gets(buf); 
      if (!strcmp(buf, ""))
	{
	  ;
#define X(s) } else if (!strcmp(buf, #s)) {
  X(first)	if (consp(n)) browse(hd(n)); else printf("Non cons element.\n");
  X(car)	if (consp(n)) browse(hd(n)); else printf("Non cons element.\n");
  X(rest)	if (consp(n)) browse(tl(n)); else printf("Non cons element.\n");
  X(cdr)	if (consp(n)) browse(tl(n)); else printf("Non cons element.\n");
  X(up)		return 0;
  X(quit)	return 1;
  X(top)	return browse(tree);
  X(hex)	printf("0x%x ", n);
  X(?) printf("commands: up, top, quit, hex,\n\
<argument #>, <[! | ^]attribute> (for nodes)\n\
<element #> first, car, rest, cdr (for lists)\n");
#undef X
        }
      else if (buf[0] >= '0' && buf[0] <= '9')
	{
	  consp_index = atoi(buf);

	  if (consp(n))
	    {
	      if (browse(nth(consp_index, n)))
		return 1;
	    }
	  else
	    {
	      if (browse(nth(consp_index, args(n))))
		return 1;
	    }
	}
      else
	{
	  if (buf[0] == '!')
	    {
	      /* The exclamation mark is for use with
	       * browsing the environments of a typedef
	       *
	       * Tunc Simsek 18th Sept., 1997
	       *
	       */
	      print_env(attr(intern(buf+1), n));
	    }
	  else if (buf[0] == '^')
	    {
	      /* The @ option is for use with
	       * namespaces - similar to the ! option
	       */
	      print_ns(attr(intern(buf+1), n));
	    }
	  else if (browse(attr(intern(buf), n)))
	    return 1;
	}
    }
}

void
b(lv *n)
{
  tree = n;
  (void) browse(n);
}

#endif /* SHIFT_COMP_TYPE_I */

/* end of file -- type.c -- */
