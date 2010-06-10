/* -*- Mode: C -*- */

/* gen-special-forms.c -- */

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

/* Code generation.
 *
 * The strategy is brain-dead: use PRINTF statements directly from the
 * IR.
 *
 * Since most of what we generate is structure-typed variable
 * declarations and initialitions, one might extend the IR to include
 * them, and construct more stuff before emitting it.  The execution
 * overhead would probably be small.  However, it's not clear that the
 * extra machinery would pay off.
 *
 * No other obvious abstractions come to mind.
 *
 * Currently, name conventions are not done nicely.  They are
 * reproduced typically in at least two places (declaration and use),
 * so they should be expressed by #DEFINEs.  But maybe not.  Why name
 * names?
 */

#ifndef GEN_SPECIAL_FORMS_I
#define GEN_SPECIAL_FORMS_I

#include <shift_config.h>

#include <assert.h>
#include <stdio.h>
#include "lisp.h"
#include <stdarg.h>
#include "shic.h"
#include "utilities.h"

extern void
generate_expression(lv*);


/* Special form call generation */
void
special_form_generator(lv* sf_form)
{
  fprintf(cfile, "sf_%s_F%d(_self)",
	  pname(attr(intern("sfid"), sf_form)),
	  num(attr(intern("unique"), sf_form)));
}


/* Special form code generation.
 * It looks that a lot of the code will be very similar to the
 * set/array former one.  I could probably fix it and share it, but
 * that would require probably more work.
 *
 * Marco Antoniotti 19970620
 */

void
generate_find_sf_function(lv* sf_find_form)
{
  /* The lengths are arbitrary and chose just for convenience.  They
   * are used only in this function and others directly called from
   * this one.
   */

  char indent[3] = "  ";
  char cursor_var_name[80];
  int indent_level = 1;

  lv* find_form = arg1(sf_find_form);
  lv* default_form
    = length(args(sf_find_form)) == 2 ? arg2(sf_find_form) : nil;

  int u = num(attr(intern("unique"), sf_find_form));
  lv* element_expr   = arg1(find_form);
  lv* iterator_exprs = arg2(find_form);
  lv* filter_expr    = arg3(find_form);

  lv* element_type   = arg1(attr(intern("type"), find_form));
  lv* hashed_element_type = attr(intern("hashed_type"), find_form);

  int n_iterator_exprs = length(iterator_exprs);


  /* Generate H file function header */
  fputs("extern ", hfile);
  generate_type(element_type, hfile);
  fprintf(hfile, "\nsf_%s_F%d(Component*);\n",
	  pname(attr(intern("sfid"), sf_find_form)),
	  u);

  /* Generate C file function header */
  generate_type(element_type, cfile);
  fprintf(cfile, "\nsf_%s_F%d(Component* _self)\n{\n",
	  pname(attr(intern("sfid"), sf_find_form)),
	  u);

  /* Generate the nested loops */
  generate_loop_headers(iterator_exprs, indent_level);

  /* Now generate the filter expression */
  indent_to(indent_level + n_iterator_exprs, indent, cfile);
  fputs("if (", cfile);
  generate_expression(filter_expr);
  fputs(")\n", cfile);

  /* Generate return expression if test true. */
  indent_to(indent_level + n_iterator_exprs + 1, indent, cfile);
  fputs("{\n", cfile);

  indent_to(indent_level + n_iterator_exprs + 2, indent, cfile);
  fputs("return ", cfile);
  generate_expression(element_expr);
  fputs(";\n", cfile);

  indent_to(indent_level + n_iterator_exprs + 1, indent, cfile);
  fputs("}\n", cfile);

  generate_loop_closing(nreverse(copy_list(iterator_exprs)),
			indent_level + length(iterator_exprs) - 1);

  /* Generate code to handle case when no elements are found.
   * If a default is specified, then we just return it, otherwise, we
   * generate an error.
   *
   * Marco Antoniotti 19970707
   */
  
  if (default_form != nil)
    {
      indent_to(indent_level, indent, cfile);
      fputs("return ", cfile);
      generate_expression(default_form);
      fputs(";\n", cfile);
    }
  else
    {
      indent_to(indent_level, indent, cfile);
      fprintf(cfile,
	      "runtime_error(\"sf_%s_F%d: FIND expression did not succeed.\");\n",
	      pname(attr(intern("sfid"), sf_find_form)),
	      u);
    }

  /* Generate C file function closing */
  fputs("}\n\n", cfile);
}


void
generate_count_sf_function(lv* sf_count_form)
{
  /* The lengths are arbitrary and chose just for convenience.  They
   * are used only in this function and others directly called from
   * this one.
   */

  char indent[3] = "  ";
  char cursor_var_name[80];
  int indent_level = 1;

  lv* count_form = arg1(sf_count_form);

  int u = num(attr(intern("unique"), sf_count_form));
  lv* element_expr   = arg1(count_form);
  lv* iterator_exprs = arg2(count_form);
  lv* filter_expr    = arg3(count_form);

  lv* element_type   = arg1(attr(intern("type"), count_form));
  lv* hashed_element_type = attr(intern("hashed_type"), count_form);

  int n_iterator_exprs = length(iterator_exprs);


  /* Generate H file function header */
  fputs("extern double", hfile);
  fprintf(hfile, "\nsf_%s_F%d(Component*);\n",
	  pname(attr(intern("sfid"), sf_count_form)),
	  u);

  /* Generate C file function header */
  fprintf(cfile, "double\nsf_%s_F%d(Component* _self)\n{\n",
	  pname(attr(intern("sfid"), sf_count_form)),
	  u);

  /* Generate the count variable name */
  sprintf(cursor_var_name, "_sf_count_V%d", u);
  indent_to(indent_level, indent, cfile);
  fprintf(cfile, "int %s = 0;\n\n", cursor_var_name);

  /* Generate the nested loops */
  generate_loop_headers(iterator_exprs, indent_level);

  /* Now generate the filter expression */
  indent_to(indent_level + n_iterator_exprs, indent, cfile);
  fputs("if (", cfile);
  generate_expression(filter_expr);
  fputs(")\n", cfile);

  /* Generate accumulation expression if test true. */
  indent_to(indent_level + n_iterator_exprs + 1, indent, cfile);
  fputs("{\n", cfile);

  indent_to(indent_level + n_iterator_exprs + 2, indent, cfile);
  fprintf(cfile, "%s++;\n", cursor_var_name);

  indent_to(indent_level + n_iterator_exprs + 1, indent, cfile);
  fputs("}\n", cfile);

  generate_loop_closing(nreverse(copy_list(iterator_exprs)),
			indent_level + length(iterator_exprs) - 1);

  /* Generate return value */

  indent_to(indent_level, indent, cfile);
  fprintf(cfile, "return (double) %s;\n", cursor_var_name);


  /* Generate C file function closing */
  fputs("}\n\n", cfile);
}


void
generate_choose_sf_function(lv* sf_choose_form)
{
  /* The lengths are arbitrary and chose just for convenience.  They
   * are used only in this function and others directly called from
   * this one.
   */

  char indent[3] = "  ";
  char choose_set_name[80];
  char choose_set_size_name[80];
  char choose_var_name[80];
  char chosen_el_var_name[80];
  char cursor_var_name[80];
  int indent_level = 1;

  lv* choose_form = arg1(sf_choose_form);
  lv* default_form
    = length(args(sf_choose_form)) == 2 ? arg2(sf_choose_form) : nil;

  int u = num(attr(intern("unique"), sf_choose_form));
  lv* element_expr   = arg1(choose_form);
  lv* iterator_exprs = arg2(choose_form);
  lv* filter_expr    = arg3(choose_form);

  lv* element_type   = arg1(attr(intern("type"), choose_form));
  lv* hashed_element_type = attr(intern("hashed_type"), choose_form);

  int n_iterator_exprs = length(iterator_exprs);


  /* Generate H file function header */
  fputs("extern ", hfile);
  generate_type(element_type, hfile);
  fprintf(hfile, "\nsf_%s_F%d(Component*);\n",
	  pname(attr(intern("sfid"), sf_choose_form)),
	  u);

  /* Generate C file function header */
  generate_type(element_type, cfile);
  fprintf(cfile, "\nsf_%s_F%d(Component* _self)\n{\n",
	  pname(attr(intern("sfid"), sf_choose_form)),
	  u);

  /* Generate auxiliary variables and various declarations. */
  sprintf(choose_var_name, "_sf_choose_V%d", u);
  sprintf(cursor_var_name, "_sf_choose_cursor_V%d", u);
  sprintf(choose_set_name, "_sf_choose_temp_set_V%d", u);
  sprintf(choose_set_size_name, "_sf_choose_temp_set_size_V%d", u);
  sprintf(chosen_el_var_name, "_sf_chosen_el_V%d", u);

  indent_to(indent_level, indent, cfile);
  generate_type(element_type, cfile);
  fprintf(cfile, " %s;\n", chosen_el_var_name);

  indent_to(indent_level, indent, cfile);
  fprintf(cfile, "GenericSet* %s = ", choose_set_name);
  generate_expression(choose_form);
  fputs(";\n", cfile);

  indent_to(indent_level, indent, cfile);
  fprintf(cfile, "int %s = set_size(%s);\n",
	  choose_set_size_name,
	  choose_set_name);

  indent_to(indent_level, indent, cfile);
  fprintf(cfile, "long %s = 0;\n", cursor_var_name);

  indent_to(indent_level, indent, cfile);
  fprintf(cfile, "long %s = (%s == 0) ? 0 : random() %% %s;\n",
	  choose_var_name,
	  choose_set_size_name,
	  choose_set_size_name);

  /* Generate the loop */
  fputc('\n', cfile);
  indent_to(indent_level, indent, cfile);
  fprintf (cfile, "if (%s > 0)\n", choose_set_size_name);

  indent_to(indent_level + 1, indent, cfile);
  fprintf(cfile, "FOR_ALL_ELEMENTS(__chosen, %s) {\n", choose_set_name);

  indent_to(indent_level + 2, indent, cfile);
  fprintf(cfile, "if (%s == %s)\n",
	  cursor_var_name,
	  choose_var_name);

  indent_to(indent_level + 3, indent, cfile);
  fputs("{\n", cfile);

  indent_to(indent_level + 4, indent, cfile);
  fprintf(cfile, "%s = ", chosen_el_var_name);
  if (op(element_type) == intern("number_type"))
    fputs("__chosen.d;\n", cfile);
  else if (op(element_type) == intern("symbol_type")
	   || op(element_type) == intern("logical_type"))
    fputs("__chosen.i;\n", cfile);
  else
    {
      fputc('(', cfile);
      generate_type(element_type);
      fputc(')', cfile);
      fputs(" __chosen.v;\n", cfile);
    }

  indent_to(indent_level + 4, indent, cfile);
  fputs("break;\n", cfile);

  indent_to(indent_level + 3, indent, cfile);
  fputs("}\n", cfile);
  
  indent_to(indent_level + 2, indent, cfile);
  fputs("else\n", cfile);

  indent_to(indent_level + 3, indent, cfile);
  fprintf(cfile, "%s++;\n", cursor_var_name);

  indent_to(indent_level + 1, indent, cfile);
  fputs("} END_FAE;\n", cfile);

  indent_to(indent_level, indent, cfile);
  fputs("else\n", cfile);

  /* Generate code to handle case when no elements are found.
   * If a default is specified, then we just return it, otherwise, we
   * generate an error.
   *
   * Marco Antoniotti 19970707
   */
  if (default_form != nil)
    {
      indent_to(indent_level + 1, indent, cfile);
      fputs("return ", cfile);
      generate_expression(default_form);
      fputs(";\n", cfile);
    }
  else
    {
      indent_to(indent_level + 1, indent, cfile);
      fprintf(cfile,
	      "runtime_error(\"sf_%s_F%d: CHOOSE expression did not succeed.\");\n",
	      pname(attr(intern("sfid"), sf_choose_form)),
	      u);
    }
  fputs("\n\n", cfile);

  /* Generate cleanup and the return expression. */
  indent_to(indent_level, indent, cfile);
  fprintf(cfile, "clear_set(%s);\n", choose_set_name);
  
  indent_to(indent_level, indent, cfile);
  fprintf(cfile, "return %s;\n", chosen_el_var_name);

  /* Generate C file function closing */
  fputs("}\n\n", cfile);
}


void
generate_special_form_function(lv* sf_form)
{
  lv* sf_op = attr(intern("sfid"), sf_form);

  if (sf_op == intern("find"))
    generate_find_sf_function(sf_form);
  else if (sf_op == intern("count"))
    generate_count_sf_function(sf_form);
  else if (sf_op == intern("choose"))
    generate_choose_sf_function(sf_form);
}

#endif /* GEN_SPECIAL_FORMS_I */

/* end of file -- gen-special-forms.c -- */
