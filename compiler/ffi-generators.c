/* -*- Mode: C -*- */

/* ffi-generators.c -- */

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

#ifndef COMPILER_FFI_GEN_I
#define COMPILER_FFI_GEN_I

#include <shift_config.h>

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

/* #include "lisp.h" */
#include "crscl.h"

#include "shic.h"
#include "utilities.h"

static void generate_ffi_wrapper_code(lv*, lv*, lv*, lv*);
static void generate_ffi_type(lv*, FILE*);
static void generate_ffi_type_from_formal_decl(lv*, FILE*);
static void generate_shift_type_from_formal_decl(lv*, FILE*);

void
generate_foreign_function_code(lv* fd)
{
  lv *formals = attr(intern("formals"), fd);

  /* Header file */
  generate_type(attr(intern("return_type"), fd), hfile);
  fprintf(hfile, " %s(", pname(attr(intern("ffi_wrapper_name"), fd)));
  if (formals)
    {
      generate_shift_type_from_formal_decl(hd(formals), hfile);
      dolist (f, tl(formals))
	{
	  fprintf(hfile, ", ");
	  generate_shift_type_from_formal_decl(f, hfile);
	}
      tsilod;
    }
  fprintf(hfile, ");\n");

  /* C file */
  generate_ffi_wrapper_code(attr(intern("ffi_wrapper_name"), fd),
			    formals,
			    attr(intern("return_type"), fd),
			    fd);
}


/* generate_formal_type_indirection --
 * We must generate an appropriate set of '*' C indirections according
 * with the I/O qualifier of the formal parameter.
 */
static void
generate_ffi_formal_type_indirection(lv* formal_decl, FILE* file)
{
  if (number_type_p(arg1(formal_decl)))
    {
      if (out_formal_p(formal_decl))
	fputc('*', file);
    }
  /* Arrays are already passed by indirection */
}


static void
generate_shift_formal_type_indirection(lv* formal_decl, FILE* file)
{
  if (out_formal_p(formal_decl))
    fputc('*', file);
}


static void generate_ffi_type(lv*, FILE*);

static void
generate_ffi_type_from_formal_decl(lv* formal_decl, FILE* file)
{
  generate_ffi_type(arg1(formal_decl), file);
  generate_ffi_formal_type_indirection(formal_decl, file);
}


static void generate_ffi_type(lv*, FILE*);

static void
generate_shift_type_from_formal_decl(lv* formal_decl, FILE* file)
{
  generate_type(arg1(formal_decl), file);
  generate_shift_formal_type_indirection(formal_decl, file);
}


static void generate_ffi_pre_glue_code(lv*, lv*, lv*);
static void generate_ffi_call(lv*, lv*);
static void generate_ffi_post_glue_code(lv*, lv*);
static void generate_ffi_type(lv*, FILE*);
static void generate_shift_type_from_formal_decl(lv*, FILE*);


/* generate_ffi_wrapper_code -- The scheme is really braindead.  I
 * just allocate extra variables and copy the values into them.  I
 * could do it in a 'as needed' basis, but it would complicate the
 * code without really speeding things up that much.
 *
 * Marco Antoniotti 19970430
 */
void
generate_ffi_wrapper_code(lv* ffi_wrapper_name,
			  lv* formals,
			  lv* return_type,
			  lv* function_desc)
{
  /* Function Header */
  fprintf(cfile, "/* Foreign Function Wrapper for '%s' */\n\n",
	  pname(attr2(intern("name"), intern("id"), function_desc)));

  generate_ffi_type(attr(intern("return_type"), function_desc), cfile);
  fprintf(cfile, "\n%s(", pname(ffi_wrapper_name));
  if (formals)
    {
      generate_shift_type_from_formal_decl(hd(formals), cfile);
      fprintf(cfile, " %s", pname(attr2(intern("name"), intern("id"), hd(formals))));
      dolist (f, tl(formals))
	{
	  fprintf(cfile, ", ");
	  generate_shift_type_from_formal_decl(f, cfile);
	  fprintf(cfile, " %s", pname(attr2(intern("name"), intern("id"), f)));
	}
      tsilod;
    }
  fprintf(cfile, ")\n");

  /* Function Body */
  fputs("{\n", cfile);

  generate_ffi_pre_glue_code(formals, return_type, function_desc);
  generate_ffi_call(formals, function_desc);
  generate_ffi_post_glue_code(formals, function_desc);
  
  fputs("}\n\n", cfile);
}


static void generate_ffi_type(lv*, FILE*);
static void generate_ffi_pre_glue_expr(lv*, FILE*);
static void generate_ffi_signal_catching();

/* generate_ffi_pre_glue_code -- */

static void
generate_ffi_pre_glue_code(lv* formals, lv* return_type, lv* function_desc)
{
  char indent_string[3] = "  ";
  int indent_level = 1;

  /* First declare the necessary variables. */

  /* Function Arguments */
  dolist (f, formals)
    {
      if (array_type_p(arg1(f)))
	{
	  indent_to(1, indent_string, cfile);
	  generate_ffi_type(arg1(f), cfile);
	  fprintf(cfile, " _ffi_%s;\n", pname(attr2(intern("name"), intern("id"), f)));
	}
    }
  tsilod;

  /* Return Value */
  indent_to(1, indent_string, cfile);
  generate_ffi_type(return_type, cfile);
  fputs(" _ffi__return_value;\n", cfile);
  indent_to(1, indent_string, cfile);
  generate_type(return_type, cfile);
  fputs(" _ffi_shift__return_value;\n", cfile);

  /* Foreign Function Signature */
  indent_to(1, indent_string, cfile);
  fputs("extern ", cfile);
  generate_ffi_type(return_type, cfile);
  fprintf(cfile, " %s(", pname(attr2(intern("name"), intern("id"), function_desc)));
  if (formals != 0)
    {
      generate_ffi_type_from_formal_decl(hd(formals), cfile);
      dolist (f, tl(formals))
	{
	  fputs(", ", cfile);
	  generate_ffi_type_from_formal_decl(f, cfile);
	}
      tsilod;
    }
  fputs(");\n\n", cfile);

  /* Generate the signal catcher code */
  generate_ffi_signal_catching();

  /* Then initialize them appropriately. */
  dolist (f, formals)
    {
      if (array_type_p(arg1(f)))
	{
	  /* If the array is an out variable, we need to ensure that
	   * it is properly allocated.
	   * How much to allocate is taken from the declaration of the
	   * parameter.  This is the best that we can do and it is a
	   * consequence of the possibility to set array slots to nil.
	   *
	   * Marco Antoniotti 19970512
	   */
	  /* indent_to(1, indent_string, cfile);
	  fprintf(cfile, "ensure_array_allocation(%s, NUMBER_T,
	  %d);");
	  */
	  indent_to(1, indent_string, cfile);
	  fprintf(cfile, "_ffi_%s = ", pname(attr2(intern("name"), intern("id"), f)));
	  generate_ffi_pre_glue_expr(f, cfile);
	  fputs(";\n", cfile);
	}
    }
  tsilod;
}


static void
generate_ffi_signal_catching()
{
  char indent_string[3] = "  ";
  int indent_level = 1;

  indent_to(indent_level, indent_string, cfile);
  fputs("void (* old_sigsegv_handler)(int);\n", cfile);
  indent_to(indent_level, indent_string, cfile);
  fputs("void (* old_sigbus_handler)(int);\n\n", cfile);

  indent_to(indent_level, indent_string, cfile);
  fputs("old_sigsegv_handler = signal(SIGSEGV, shift_ffi_signal_filter);\n",
	cfile);
  indent_to(indent_level, indent_string, cfile);
  fputs("old_sigbus_handler  = signal(SIGBUS, shift_ffi_signal_filter);\n",
	cfile);
}


/* Adding support for external types.
 *
 * Tunc Simsek 15th April, 1998
 */
static void generate_ffi_array_type(lv*, FILE*);
static void generate_ffi_type_type(lv*, FILE*);
/* generate_ffi_type -- Almost like 'generate_type', but with some
 * different behavior.
 */
static void
generate_ffi_type(lv* type_decl, FILE* file)
{
  if (number_type_p(type_decl))
    fputs("double", file);
  else if (symbol_type_p(type_decl))
    fputs("int", file);
  else if (logical_type_p(type_decl))
    fputs("int", file);
  else if (array_type_p(type_decl))
    generate_ffi_array_type(type_decl, file);
  else if (op(type_decl) == intern("id"))
    generate_ffi_type(attr(intern("type"), type_decl), file);
  else if (type_type_p(type_decl))
    generate_ffi_type_type(type_decl, file);
  else
    {
      user_error(nil,
		 "FFI: cannot generate a foreign representation for type ~s\n",
		 op(type_decl));
    }
}


static void
generate_ffi_type_type(lv* type_decl, FILE* file)
{
  lv *exttypedef = attr(intern("exttypedef"), type_decl);
  
  /* Adding support for external data types.
   *
   * Tunc Simsek 15th April, 1998
   */
  if ( exttypedef && exttypedef == intern("true") )
    {
      fputs("FOREIGN_TYPE *", file);
    }
  else
    {
      user_error(nil,
		 "FFI: cannot generate a foreign representation for type ~s\n",
		 op(type_decl));
    }
}

static int
count_array_indirections(lv* array_decl)
{
  lv* array_type = arg1(array_decl);

  if (! array_type_p(array_type))
    return 1;
  else
    return 1 + count_array_indirections(array_type);
}


static lv*
array_element_type(lv* array_decl)
{
  lv* array_type = arg1(array_decl);

  if (! array_type_p(array_type))
    return array_type;
  else
    return array_element_type(array_type);
}


static void
generate_ffi_array_type(lv* array_decl, FILE* file)
{
  int array_indirections = count_array_indirections(array_decl);
  lv* array_el_type = array_element_type(array_decl);
  int i;
  
  generate_type(array_el_type, file);
  for (i = 0; i < array_indirections; i++)
    fputs("*", file);
}


static void generate_ffi_pre_glue_array_expr(lv*, FILE*);

static void
generate_ffi_pre_glue_expr(lv* formal_arg, FILE* file)
{
  lv* type_decl = arg1(formal_arg);

  if (number_type_p(type_decl))
    fprintf(file, "%s", pname(attr2(intern("name"), intern("id"), formal_arg)));
  else if (array_type_p(type_decl))
    generate_ffi_pre_glue_array_expr(formal_arg, file);
  else
    {
      user_error(attr(intern("id"), formal_arg),
		 "FFI: cannot generate a foreign expression for argument ~s\n",
		 attr(intern("id"), formal_arg));
    }
}


/* The following function names are defined in the SHIFT runtime. */
static char*
ffi_shift_D_array_copiers[] = {
  "shift_ffi_cvt_D_array_0D_C",
  "shift_ffi_cvt_D_array_1D_C",
  "shift_ffi_cvt_D_array_2D_C",
  "shift_ffi_cvt_D_array_3D_C",
  "shift_ffi_cvt_D_array_4D_C",
};

static char*
ffi_C_D_array_copiers[] = {
  "shift_ffi_cvt_D_array_0D_shift",
  "shift_ffi_cvt_D_array_1D_shift",
  "shift_ffi_cvt_D_array_2D_shift",
  "shift_ffi_cvt_D_array_3D_shift",
  "shift_ffi_cvt_D_array_4D_shift",
};

static char*
ffi_shift_I_array_copiers[] = {
  "shift_ffi_cvt_I_array_0I_C",
  "shift_ffi_cvt_I_array_1I_C",
  "shift_ffi_cvt_I_array_2I_C",
  "shift_ffi_cvt_I_array_3I_C",
  "shift_ffi_cvt_I_array_4I_C",
};

static char*
ffi_C_I_array_copiers[] = {
  "shift_ffi_cvt_I_array_0I_shift",
  "shift_ffi_cvt_I_array_1I_shift",
  "shift_ffi_cvt_I_array_2I_shift",
  "shift_ffi_cvt_I_array_3I_shift",
  "shift_ffi_cvt_I_array_4I_shift",
};

typedef enum _lang_dir { C, SHIFT } lang_direction;

char*
fetch_array_copier_function(lv* array_base_type,
			    int array_dims,
			    lang_direction target_lang)
{
  if (number_type_p(array_base_type))
    {
      switch (target_lang)
	{
	case C:
	  return ffi_shift_D_array_copiers[array_dims];
	case SHIFT:
	  return ffi_C_D_array_copiers[array_dims];
	}
    }
  else				/* Assume SHIFT Symbols and Logicals */
    {
      /* ToDo:
       * It seems that you might have to add another test here
       * for FOREIGN_TYPE's, or else they will be treated as
       * integer/symbol arrays.
       *
       * Tunc Simsek 21st April, 1998
       */
      switch (target_lang)
	{
	case C:
	  return ffi_shift_I_array_copiers[array_dims];
	case SHIFT:
	  return ffi_C_I_array_copiers[array_dims];
	}
    }
}


static void
generate_ffi_pre_glue_array_expr(lv* array_formal_arg, FILE* file)
{
  lv* array_decl = arg1(array_formal_arg);
  int array_indirections = count_array_indirections(array_decl);
  lv* array_el_type = array_element_type(array_decl);
  int i;

  fprintf(cfile, "%s(%s%s)",
	  fetch_array_copier_function(array_el_type,
				      array_indirections,
				      C),
	  out_formal_p(array_formal_arg) ? "*" : "",
	  pname(attr2(intern("name"), intern("id"), array_formal_arg)));
}


static void
generate_ffi_call(lv* formals, lv* function_desc)
{
  char indent_string[3] = "  ";
  int indent_level = 1;

  indent_to(1, indent_string, cfile);
  fputs("_ffi__return_value = ", cfile);
  fprintf(cfile, "%s(", pname(attr2(intern("name"), intern("id"), function_desc)));

  if (formals != 0)
    {
      if (array_type_p(arg1(hd(formals))))
	fputs("_ffi_", cfile);
      fprintf(cfile, "%s", pname(attr2(intern("name"), intern("id"), hd(formals))));
      dolist (f, tl(formals))
	{
	  fputs(", ", cfile);
	  if (array_type_p(arg1(f)))
	    fputs("_ffi_", cfile);

	  fprintf(cfile, "%s", pname(attr2(intern("name"), intern("id"), f)));
	}
      tsilod;
    }
  fputs(");\n", cfile);
}


static void generate_ffi_post_glue_expr(lv*, FILE*);
static void generate_ffi_copyback_code(lv*, lv*, FILE*);
static void generate_ffi_return_value_expr(lv*, FILE*);
static void generate_signal_restore();
static void generate_ffi_return_value_expr(lv*, FILE*);

static void
generate_ffi_post_glue_code(lv* formals, lv* function_desc)
{
  char indent_string[3] = "  ";
  int indent_level = 1;

  dolist(f, formals)
    {
      generate_ffi_copyback_code(f, function_desc, cfile);
    }
  tsilod;
  generate_ffi_return_value_expr(attr(intern("return_type"), function_desc), cfile);
  generate_signal_restore();

  indent_to(1, indent_string, cfile);
  fputs("return _ffi_shift__return_value;\n", cfile);
}


static void
generate_signal_restore()
{
  char indent_string[3] = "  ";
  int indent_level = 1;

  indent_to(indent_level, indent_string, cfile);
  fputs("(void) signal(SIGSEGV, old_sigsegv_handler);\n", cfile);
  indent_to(indent_level, indent_string, cfile);
  fputs("(void) signal(SIGBUS, old_sigbus_handler);\n", cfile);
}


static void generate_ffi_post_glue_expr(lv*, FILE*);
static void generate_ffi_C_D_array_dimensions(lv*, lv*, FILE*);

static void
generate_ffi_copyback_code(lv* formal_arg, lv* function_desc, FILE* file)
{
  char indent_string[3] = "  ";
  int indent_level = 1;

  if (array_type_p(arg1(formal_arg)) && out_formal_p(formal_arg))
    {
      /* I need to restore the proper HASH stuff for the array.
       * (Or maybe I should change the scheme and not overwrite the
       * toplevel variable)
       */
      
      /* Old version
	 indent_to(1, indent_string, file);
	 fprintf(file, "%s = ", pname(attr2(intern("name"), intern("id"), formal_arg)));
	 generate_ffi_post_glue_expr(formal_arg, cfile);
	 fputs(";\n", file);
	 */

      /* New version 19970507 */
      int array_indirections = count_array_indirections(arg1(formal_arg));
      lv* array_el_type = array_element_type(arg1(formal_arg));

      indent_to(1, indent_string, file);
      /* Note that the second parameter is, in this case, an 'out'
       * parameter, declared as 'Array**'.
       * The run time support routine must have the proper signature.
       */
      fprintf(file, "%s(_ffi_%s, %s",
	      fetch_array_copier_function(array_el_type,
					  array_indirections,
					  SHIFT),
	      /* ffi_C_D_array_copiers[array_indirections], */
	      pname(attr2(intern("name"), intern("id"), formal_arg)),
	      pname(attr2(intern("name"), intern("id"), formal_arg)));

      /* generate_ffi_C_D_array_dimensions(arg1(formal_arg),
					attr(intern("id"), formal_arg),
					file);
					*/
      fputs(");\n", file);
      /* New version */
    }
}


static void generate_ffi_post_glue_array_expr(lv*, FILE*);

static void
generate_ffi_post_glue_expr(lv* formal_arg, FILE* file)
{
  lv* type_decl = arg1(formal_arg);

  if (number_type_p(type_decl))
    fprintf(file, "_ffi_%s", pname(attr2(intern("name"), intern("id"), formal_arg)));
  else if (array_type_p(type_decl))
    generate_ffi_post_glue_array_expr(formal_arg, file);
  else
    {
      user_error(attr(intern("id"), formal_arg),
		 "FFI: cannot generate a foreign expression for argument ~s\n",
		 attr(intern("id"), formal_arg));
    }
}


static void generate_ffi_C_D_array_dimensions(lv*, lv*, FILE*);

static void
generate_ffi_post_glue_array_expr(lv* array_formal_arg, FILE* file)
{
  lv* array_decl = arg1(array_formal_arg);
  int array_indirections = count_array_indirections(array_decl);
  lv* array_el_type = array_element_type(array_decl);
  int i;

  fprintf(file, "%s(_ffi_%s",
	  fetch_array_copier_function(array_el_type,
				      array_indirections,
				      C),
	  /* ffi_C_D_array_copiers[array_indirections], */
	  pname(attr2(intern("name"), intern("id"), array_formal_arg)));
  generate_ffi_C_D_array_dimensions(arg1(array_formal_arg),
				    attr(intern("id"), array_formal_arg),
				    file);
  fputs(")", file);
}


static void
generate_ffi_C_D_array_dimensions(lv* array_formal_type,
				  lv* array_formal_id,
				  FILE* file)
{
  lv* dimension_form;
  int dimension;

  if (array_type_p(array_formal_type))
    {
      dimension_form = attr(intern("dimensions"), array_formal_type);
      if (dimension_form == 0)
	user_error(find_leaf(array_formal_id),
		   "FFI: cannot generate C array copy back code (no dimensions in array type).");
      else
	{
	  dimension = num(attr(intern("value"), dimension_form));
	  fprintf(file, ", %d", dimension);
	  generate_ffi_C_D_array_dimensions(arg1(array_formal_type),
					    array_formal_id,
					    file);
	}
    }
}


/* generate_ffi_return_value_expr --
 * This function is rather redundant, given that, for the time being,
 * we do not have any way to know what could be the size of an array
 * returned by a C function (and given that the SHIFT compiler catches
 * attempts to declare functions returning arrays).
 *
 * Marco Antoniotti 19970501
 */
static void
generate_ffi_return_value_expr(lv* return_type, FILE* file)
{
  int array_indirections;
  char indent_string[3] = "  ";
  int indent_level = 1;

  indent_to(1, indent_string, file);
  fputs("_ffi_shift__return_value = ", file);
  if (logical_type_p(return_type) || number_type_p(return_type))
    {
      fputs("_ffi__return_value;\n", file);
    }
  else if (array_type_p(return_type))
    {
      array_indirections = count_array_indirections(return_type);
      fprintf(file, "%s(_ffi__return_value",
	      fetch_array_copier_function(array_element_type(arg1(return_type)),
					  array_indirections,
					  SHIFT)
	      /* ffi_C_D_array_copiers[array_indirections] */
	      );
      generate_ffi_C_D_array_dimensions(return_type,
					nil,
					file);
      fputs(");\n", file);
    }
  else if (op(return_type) == intern("id"))
    {
      fputs("_ffi__return_value;\n", file);
    }
  else
    {
      internal_error("generate_ffi_return_value_expr: type not yet handled.");
    }
}


#endif /* COMPILER_FFI_GEN_I */

/* end of file -- ffi-generators.c -- */
