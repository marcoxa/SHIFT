/* -*- Mode: C -*- */

/* utilities.c -- */

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

/* Utilities.
 * Some utility functions to be used during code generation.
 *
 * Marco Antoniotti 19970105
 */

#ifndef COMPILER_UTILITIES_I
#define COMPILER_UTILITIES_I

#include <shift_config.h>

#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

/* indent_to --
 * Puts 'indent_level' 'indent_string' on the file stream 'stream'.
 */
void
indent_to(int indent_level, char* indent_string, FILE* stream)
{
  int i;

  for (i = 0; i < indent_level; i++)
    fputs(indent_string, stream);
}


/* start and end C comments -- useful mostly for avoiding messing
 * around with the coloring scheme of Emacs, but not only.
 */
void
start_c_comment(FILE* stream)
{
  putc('/', stream);
  putc('*', stream);
}


void
end_c_comment(FILE* stream)
{
  putc('*', stream);
  putc('/', stream);
}

void*
safe_malloc(size_t size)
{
  void *memory_chunk = malloc(size);

  if (memory_chunk == 0)
    {
      fputs("shic: unable to allocate memory.\n", stderr);
      fflush(stderr);
      exit(1);
    }
  return memory_chunk;
}


/* verbose_list_data_fields -- Added
 *
 * This function traverses the specified data 
 * field lists and prints them on the stdout.
 *
 * Tunc Simsek 10th November, 1997
 */
extern int verbose_list_globals;
extern int verbose_list_outputs;
extern int verbose_list_states;
extern int verbose_list_inputs;
 
void
verbose_list_data_fields(lv *p)
{
  int g = 0;
  void write_declare_list(lv*, FILE*);  
  void write_type(lv*, FILE*);
  /* Traverse the list of top level
   * nodes in the program.
   */
  dolist(t, p)
    {
      if(op(t) == intern("typedef"))
	{
	  if(verbose_list_states 
	     || verbose_list_outputs
	     || verbose_list_inputs)
	    {
	      fprintf(stdout, "#");
	      write_sexpr1(intern("typedef"), stdout);
	      fprintf(stdout, "\n");
	      write_type(attr(intern("id"), t), stdout);
	      fprintf(stdout, "\n");
	      
	      if(verbose_list_states)
		{
		  lv *states = attr(intern("state"), t);
		  
		  if(states)
		    {
		      fprintf(stdout, "#");
		      write_sexpr1(intern("state"), stdout);
		      fprintf(stdout, "\n");
		      write_declare_list(states, stdout);
		    }
		}
	      if(verbose_list_outputs)
		{
		  lv *outputs = attr(intern("output"), t);
		  
		  if(outputs)
		    {
		      fprintf(stdout, "#");
		      write_sexpr1(intern("output"), stdout);
		      fprintf(stdout, "\n");
		      write_declare_list(outputs, stdout);
		    }
		}
	      
	      if(verbose_list_inputs)
		{
		  lv *inputs = attr(intern("input"), t);

		  if(inputs)
		    {
		      fprintf(stdout, "#");
		      write_sexpr1(intern("input"), stdout);
		      fprintf(stdout, "\n");
		      write_declare_list(inputs, stdout);
		    }
		}
	    }
	}
      else if(op(t) == intern("glob_var_decl") && ! g)
	{
	  /* The flag g is used so that only the first
	   * node intern("glob_var_decl") is visited.
	   * The rest is redundant because the typechecker
	   * collects these lists under the first one.
	   */
	  g = 1;
	  if(verbose_list_globals)
	    {
	      fprintf(stdout, "#");
	      write_sexpr1(intern("glob_var_decl"), stdout);
	      fprintf(stdout,"\n");
	      write_declare_list(args(t), stdout);
	    }
	}
    }
  tsilod;
}

void
write_declare_list(lv *l, FILE *fp)
{
  void write_type(lv*, FILE*);

  dolist(d, l)
    {
      if(op(d) == intern("declare"))
	{
	  lv *id = attr(intern("id"), d);
	  lv *type = id ? attr(intern("type"), id) : nil;
	  /* The intern("id") attribute provides the name of the
	   * global variable that is declared.
	   */
	  if(id)
	    {
	      write_type(id, fp);
	      fprintf(fp, " ");
	    }
	  
	  /* The first argument is the type of that
	   * global variable.
	   */
	  if(type)
	    {
	      write_type(type, fp);
	      fprintf(fp, "\n");
	    }
	}
    }
  tsilod;
}

void
write_type(lv *type, FILE *fp)
{
  if(op(type) == intern("id"))
    {
      lv *name = attr(intern("name"), type);
      
      if(name)
	{
	  fprintf(fp, pname(name));
	}
    }
  else
    {
	  write_sexpr1(op(type), fp);
	  if (args(type))
	    {
	      fprintf(fp, "(");
	      write_type(arg1(type), fp);
	      fprintf(fp, ")");
	    }
    }  
}
#endif /* COMPILER_UTILITIES_I */

/* end of file -- utilities.c -- */
