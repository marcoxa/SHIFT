/* -*- Mode: C -*- */

/* check.c -- */

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

/* Type checking and other static checks.
 */

#ifndef COMPILER_CHECK_I
#define COMPILER_CHECK_I

#include <stdlib.h>		/* For 'malloc'. */
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>		/* For 'memset'. */

/* #include "lisp.h" */
#include "crscl.h"

#include "shic.h"

extern int line;
extern lv *current_file;

int glob_or_td = 0;

void
check_typedef(lv *td);

/* Global name space. */
lv *global_env, *global_ns;

/* The type definition for the internal global type (a type which
 * contains all global variables).
 */
lv *global_typedef, *global_typedef_ns, *global_type;

/* Predefined types. */
lv *error_type, *event_type, *open_event_type,
   *flow_type, *number_type, *discrete_number_type, *symbol_type,
   *logical_type, *state_type, *stateset_type,
   *void_type, *integer_type, *flow_function_type,
   *simple_number_function_type,
   *simple_number_function_type_1_arg,
   *simple_number_function_type_2_args,
   *simple_number_function_type_3_args,
   *differential_state_type, *mode_type,
   *component_type, *nil_type, *null_set_type;

/* Other predefined nodes. */
lv *error_entity, *all_entity, *nil_entity,
   *false_entity, *true_entity,
   *error_expression;

/* Internal compiler structures */
lv* predefined_ids = nil;


/* predefined_id_p -- */
int
predefined_id_p(lv* expr)
{
  return memq(expr, predefined_ids) != nil;
}


/*
 * SHIFT does not have a declare-before-use rule, so semantic checks
 * are an inherently multiple-traversal process.
 */

/*
 * A NAMESPACE (ns) maps symbols to entities (variables, types, flows,
 * events, etc.).  An ENVIRONMENT (env) is a list of namespaces
 * accessible from a program point.  Program constructs that define a
 * scope are given an environment as their @env attribute.
 */

/* build_name -- create internal names in the form: <what>~<i>  **/
lv *
build_name(int i, char *what /*, lv *tfile, lv *tline */ )
{
  char nm[50];

  memset(nm, 0, 50);
  sprintf(nm, "%s~%d", what, i);
  return identifier(intern(nm));
}


lv *
make_ns()
{
  return list1(nil);
}

/* ns_define --
 * Define symbol NAME in namespace NS to entity ENTITY.
 */
void
ns_define(lv *name, lv *ns, lv *entity)
{
  apush(name, entity, hd(ns));
}


/* ns_find --
 * Find symbol NAME in namespace NS.  Return its entity if found, else
 * NIL.
 */
lv *
ns_find(lv *name, lv *ns)
{
  lv *x = assoc(name, hd(ns));
  return x? tl(x) : nil;
}

/* env_find --
 * Return the entity of symbol NAME in environment ENV, or nil if not
 * found.
 */
lv *
env_find(lv *name, lv *env)
{
  if (env)
    {
      lv *e = ns_find(name, hd(env));
      return e? e : env_find(name, tl(env));
    }
  else
    {
      return nil;
    }
}


/* print_name_space, print_env -- Inspection utilities mainly for
 * debugging.
 *
 * Note: name space is the equivalent of 'frame' in other compiler
 * lingos
 * Marco Antoniotti 19970305
 */
void
print_ns(lv* name_space)
{
  extern void summarize(lv*);	/* Defined in 'type.c' */

  if (hd(name_space) != 0)
    {
      dolist (association, hd(name_space))
	{
	  fputs("(", stdout);
	  print(hd(association));
	  fputs(" . ", stdout);
	  if (tl(association))
	    {
	      summarize(tl(association));
	    }
	  else
	    fputs("nil", stdout);
	  fputs(")\n", stdout);
	}
      tsilod;
    }
  else
    fputs("Empty name space\n", stdout);
}


void
print_env(lv* env)
{
  int level = length(env);

  if (env != 0)
    {
      dolist (ns, env)
	{
	  fprintf(stdout, "\nName Space (frame) %d\n", level--);
	  print_ns(ns);
	}
      tsilod;
    }
  else
    {
      fputs("\nEmpty environment\n", stdout);
    }
}

/*
 * Define identifier node ID in namespace NS, creating an entity for
 * it with TYPE, MEANING, and KIND.  If ID already has an entity, use
 * that one instead.  This happens when defining an ID in more than a
 * namespace.
 *
 * Eventually, each identifier gets an entity.  Each entity has an
 * optional MEANING, representing the `value' of the identifier, when
 * such value is a compile-time constant.  An entity also has a TYPE,
 * which is the same as for the identifier, and for the meaning.  The
 * KIND of the entity is a type modifier, and conveys additional
 * information.
 */
void
define_id2(lv *id,
	   lv *ns,
	   lv *type,
	   lv *meaning,
	   lv *kind,
	   int allow_redefinition)
{
  lv *name = attr(intern("name"), id);
  lv *entity = ns_find(name, ns);
  lv *old_entity = attr(intern("entity"), id);

  assert(type || old_entity);
  if (entity && ! allow_redefinition)
    {
      user_error(id, "redefinition of \"~a\"", name);
      entity = error_entity;
    }
  else
    {
      if (old_entity)
	{
	  entity = old_entity;
	  type = attr(intern("type"), old_entity);
	}
      else if (! entity)
	{
	  entity = node(intern("entity"), nil,
			alist3(intern("type"), type,
			       intern("meaning"), meaning,
			       intern("kind"), kind));
	}
    }
  /* Changed add_attr to set_attr: It seems that eventually
   * define_id2 is called for the same id more than once, but
   * only the first intern("entity") and intern("type") attributes of an id are
   * meaningful.  So why build up the attribute list of the id?
   * 
   * Tunc Simsek 17th Sept., 1997
   *
   * Old code:
   * ---------
   * add_attr(intern("entity"), id, entity);
   * add_attr(intern("type"), id, type);
   */
  set_attr(intern("entity"), id, entity);
  set_attr(intern("type"), id, type);
  ns_define(name, ns, entity);
}


void
define_id(lv *id, lv *ns, lv *type, lv *meaning, lv *kind)
{
  define_id2(id, ns, type, meaning, kind, 0);
}


void
define_or_redefine_id(lv *id, lv *ns, lv *type, lv *meaning, lv *kind)
{
  define_id2(id, ns, type, meaning, kind, 1);
}


/*
 * Assign to ID its type and entity.  Signal an error if ID is not
 * defined in ENV.
 */
void
resolve_id(lv *id, lv *env)
{
  lv *name = attr(intern("name"), id);
  lv *entity = 0;

  dolist (ns, env)
    {
      entity = ns_find(name, ns);
      if (entity) break;
    }
  tsilod;
  if (! entity)
    {
      user_error(id, "\"~a\" is not defined", name);
      set_attr(intern("entity"), id, error_entity);
      set_attr(intern("type"), id, error_type);
      define_id(id, first(env), nil, nil, nil);
    }
  else
    {
      set_attr(intern("type"), id, attr(intern("type"), entity));
      set_attr(intern("entity"), id, entity);
    }
}

lv *
meaning(lv *id)
{
/*
 * MAK: check for non-nil attr(intern("entity"), id). If nil, then wrong use of var
 * and/or internal error  (unrewritten var).
 */
  if (!attr(intern("entity"), id))
    {
#if 0
        user_error(find_leaf(id),"bad variable name");
#endif
        return nil;
    }
  else
        return attr(intern("meaning"), attr(intern("entity"), id));
}


/* Expression rewriting.
 */
lv *rewrite_expression(lv *expr, lv *env);


/* rewrite_exists --
 * Rewriter for 'exists expressions'.
 *
 * Notes:
 * 1 - There is one problem with this function:  If the set is the empty
 *     one (i.e. the constant '{}') there is no way - save to force a
 *     declaration beforehand - to know the  type of the identifier.  In
 *     these cases it is better to just fold the expression into a intern("false").
 *
 * Marco Antoniotti 19970106
 */
lv *
rewrite_exists(lv *expr, lv *env)
{
  lv *v = attr(intern("id"), expr);            /* identifier to be bound */
  lv *s = rewrite_expression(arg1(expr), env); /* set expression */
  lv *t = type_of(s);
  lv *eltype;
  lv *e;

  if (! (set_type_p(t) || range_type_p(t)))
    {
      user_error(find_leaf(s), "'in' expression is neither a set nor a range");
      eltype = error_type;
    }
  else if (t == error_type)
    {
      eltype = error_type;
    }
  else
    {
      eltype = arg1(t);
    }

  e = rewrite_expression(arg2(expr), attr(intern("env"), arg2(expr)));
  arg1(expr) = s;
  arg2(expr) = e;
  set_attr(intern("type"), v, eltype);
  return expr;
}


lv*
rewrite_special_form(lv* sf_form, lv* env)
{
  lv* sf_body = arg1(sf_form);
  lv* sf_default = length(args(sf_form)) == 2 ? arg2(sf_form) : nil;
  lv* sf_return_type;

  lv* rewrite_setcons_iter(lv*, lv*);

  if (op(sf_body) != intern("setcons2"))
    {
      user_error(find_leaf(sf_body),
		 "special form '~s' incorrectly specified.",
		 attr(intern("sfid"), sf_form));
      return error_expression;
    }
  else
    {
      arg1(sf_form) = rewrite_setcons_iter(sf_body, env);
      
      /* We need the type of the iterator expression.  Note that its
       * type is actually going to be a 'node(intern("set"), (...))', hence we
       * need to do some data extraction to get to the right base type
       * of the set.
       *
       * Marco Antoniotti 19970819
       */
      sf_return_type = arg1(attr(intern("type"), arg1(sf_form)));

      if (sf_default != nil)
	{
	  arg2(sf_form) = rewrite_expression(sf_default, env);
	  if (arg2(sf_form) == error_expression
	      || ! (descendant(sf_return_type, type_of(arg2(sf_form)))
		    || descendant(type_of(arg2(sf_form)), sf_return_type)))
	    {
	      user_error(find_leaf(sf_body),
			 "'default' form and main expression of special form have incompatible types.");
	      return error_expression;
	    }
	}
      apush(intern("type"), sf_return_type, attrs(sf_form));
      return sf_form;
    }
}


/* Check that a function call matches the function type.
 */
void
check_formal_actual(lv *f, lv *e, lv *a, lv *call)
{
  lv *type = attr(intern("type"), e);
  lv *argtypes = tl(args(type));
  lv *form, *formc, *act, *actc;

  /* First mark the call with the appropriate signature. */
  set_attr(intern("signature"), call, attr(intern("signature"), type));

  for (formc = argtypes, actc = a;
       formc && actc;
       formc = tl(formc), actc = tl(actc))
    {
      form = hd(formc);
      act = hd(actc);

      /* Check the types of the expressions. */
      if (! equal_type(type_of(act), form))
	{
	  user_error(find_leaf(act), "wrong parameter type");
	  set_attr(intern("type"), call, error_type);
	  return;
	}

      /* Check restriction on input and output parameters. I.e. all
       * OUT parameters must be 'accessors' to some
       * instance value.
       */
      if (out_formal_p(form) && ! L_value_p(act))
	{
	  user_error(find_leaf(act),
		     "expression passed to external function is not assignable");
	  set_attr(intern("type"), call, error_type);
	  return;

	}
    }

  /* If the length of formals and actual is different, then signal an
   * error.
   */
  if (formc || actc)
    {
      user_error(find_leaf(f), "wrong number of arguments");
      set_attr(intern("type"), call, error_type);
      return;
    }

  /* If everything is ok then just set the 'type' of the call and
   * return.
   */
  set_attr(intern("type"), call, arg1(type));
}


lv *
rewrite_if(lv *expr, lv *env)
{
  /* Ignore ENTITY. */
  lv *a = mapcarx(rewrite_expression, args(expr), env);
  lv *clauset = type_of(first(a));
  lv *thent = type_of(second(a));
  lv *elset = type_of(third(a));
  lv *type = thent;

  args(expr) = a;
  if (! equal_type(clauset, logical_type))
    {
      user_error(find_leaf(first(a)), "IF clause is not logical");
      type = error_type;
    }

  if (! (descendant(thent, elset) || descendant(elset, thent))
      && ! (thent == nil_type || elset == nil_type))
    {
      user_error(find_leaf(second(a)),
		 "incompatible types of THEN and ELSE clauses");
      type = error_type;
    }

  if (type == error_type)
    set_attr(intern("type"), expr, type);
  else if (descendant(elset, thent))
    set_attr(intern("type"), expr, thent);
  else
    set_attr(intern("type"), expr, elset);

  return expr;
}


lv*
make_ff_wrapper_name(lv* name)
{
  char* wrapper_name = (char*)malloc(strlen(pname(name)) + 5);

  sprintf(wrapper_name, "FFIW_%s", pname(name));
  return intern(wrapper_name);
}


/* rewrite_call --
 * This is a tricky one, because external accessors have the same
 * syntax as function calls.
 */
lv *
rewrite_call(lv *call, lv *env)
{
  lv *mvr;
  lv *f = arg1(call);
  lv *a = mapcarx(rewrite_expression, tl(args(call)), env);

  tl(args(call)) = a;
  if (op(f) == intern("id"))
    {
      lv *name = attr(intern("name"), f);
      lv *e = env_find(name, env), *r = nil;
      if (e) r = attr(intern("rewriter"), e);
      if (e && (e == error_entity || attr(intern("type"), e) == error_type))
	{
	  set_attr(intern("type"), call, error_type);
	  return call;
	}
      else if (r)
	{
	  /* Predefined function. */
	  /* return ((lv *(*)(lv *, lv *, lv *)) (oth(r)))(f, e, a); */
	  mvr = ((lv *(*)(lv *, lv *, lv *)) (oth(r)))(f, e, a);
	  return mvr;
	}
      else if (e && function_type_p(attr(intern("type"), e)))
	{
	  check_formal_actual(f, e, a, call);
	  return call;
	}
      else
	{
	  /* Not a regular function.  Check for exported variable
	   * accessor.
	   */
	  lv *at;

	  if (length(a) == 1
	      && (at = type_of(hd(a)), op(at) == intern("id")) /* type name */
	      && attr(intern("type"), at) != error_type
	      && (e = ns_find(name, attr(intern("export_ns"), meaning(at)))))
	    {
	      return node(intern("access"), a, alist1(intern("accessor"), e));
	    }
	}

      /* Couldn't find the function anywhere. */
      if (e)
	{
	  user_error(f, "\"~a\" is not a function", name);
	  return error_expression;
	}
      else
	{
	  user_error(f, "undefined function \"~a\"", name);
	  define_id(f, first(env), error_type, nil, nil);
	  set_attr(intern("entity"), f, error_entity);
	  set_attr(intern("type"), f, error_type);
	  set_attr(intern("type"), call, error_type);
	  return call;
	}
    }
  else
    {
      user_error(find_leaf(f), "functional operators are not supported");
      return error_expression;
    }
}


/* rewrite_array_range -- Rewrite an array range expression.
 * The IR node representing this construct no arguments and
 * the following non empty attributes:
 * 1 - bound1: the first bound of the expression (an expression node)
 * 2 - bound2: the second bound (an expression node)
 * 3 - step_expr: how the variable should move between the bounds (an
 *                'by_expr' node)
 */
lv*
rewrite_array_range(lv* ar_expr, lv* env)
{
  lv* rewritten_bound_1;
  lv* rewritten_bound_2;
  void rewrite_step_expression(lv*, lv*); /* Forward declaration. */
  void fix_range_bounds_types2discrete(lv*, lv*);

  rewritten_bound_1 = rewrite_expression(attr(intern("bound1"), ar_expr), env);
  if (!equal_type(type_of(rewritten_bound_1), number_type))
    {
      user_error(find_leaf(rewritten_bound_1),
		 "first bound of range expression is not numeric");
      set_attr(intern("type"), rewritten_bound_1, error_type);
    }
  set_attr(intern("bound1"), ar_expr, rewritten_bound_1);

  rewritten_bound_2 = rewrite_expression(attr(intern("bound2"), ar_expr), env);
  if (!equal_type(type_of(rewritten_bound_2), number_type))
    {
      user_error(find_leaf(rewritten_bound_2),
		 "second bound of range expression is not numeric");
      set_attr(intern("type"), rewritten_bound_2, error_type);
    }
  set_attr(intern("bound2"), ar_expr, rewritten_bound_2);

  rewrite_step_expression(attr(intern("step_expr"), ar_expr), env);

  if (attr(intern("discrete"), attr(intern("step_expr"), ar_expr)) == intern("true"))
    {
      set_attr(intern("discrete"), ar_expr, intern("true"));
      fix_range_bounds_types2discrete(attr(intern("bound1"), ar_expr),
				      attr(intern("bound2"), ar_expr));
    }

  /* set_attr(intern("step")_expr, ar_expr, ); .... we'll see... */
  set_attr(intern("type"), ar_expr,
           node(intern("range"), list1(type_of(attr(intern("bound1"), ar_expr))), nil));

  return ar_expr;
}

/* Version before 'step_options' were introduced'
lv*
rewrite_array_range(lv* ar_expr, lv* env)
{
  lv* rewritten_bound_1;
  lv* rewritten_bound_2;
  lv* rewritten_step_expr;

  rewritten_bound_1 = rewrite_expression(attr(intern("bound1"), ar_expr), env);
  if (!equal_type(type_of(rewritten_bound_1), number_type))
    user_error(find_leaf(rewritten_bound_1),
	       "first bound of range expression is not numeric");
  set_attr(intern("bound1"), ar_expr, rewritten_bound_1);

  rewritten_bound_2 = rewrite_expression(attr(intern("bound2"), ar_expr), env);
  if (!equal_type(type_of(rewritten_bound_2), number_type))
    user_error(find_leaf(rewritten_bound_2),
	       "second bound of range expression is not numeric");
  set_attr(intern("bound2"), ar_expr, rewritten_bound_2);


  if (attr(intern("step")_expr, ar_expr))
    {
      rewritten_step_expr = rewrite_expression(arg1(attr(intern("step")_expr, ar_expr)),
					       env);
      if (!equal_type(type_of(rewritten_step_expr), number_type))
	user_error(find_leaf(rewritten_step_expr),
		   "step of range expression is not numeric");
      / * Make the 'by_expr' disappear for the time being.
       * Subsequently we may decide to keep t around if the node will
       * get more attributes (like 'intern("ascending")' or 'intern("descending")'.)
       *
       * Marco Antoniotti 19970104
       * /
      set_attr(intern("step")_expr, ar_expr, rewritten_step_expr);
    }

  set_attr(intern("type"), ar_expr,
           node(intern("range"), list1(type_of(attr(intern("bound1"), ar_expr))), nil));

  return ar_expr;
}
*/


/* fix_range_bounds_types2discrete -- Name says it all.  To be called
 * only in one specific case.
 */
void
fix_range_bounds_types2discrete(lv* bound1, lv* bound2)
{
  if (op(bound1) == intern("float"))
     set_attr(intern("convert"), bound1, nil);

  if (op(bound2) == intern("float"))
     set_attr(intern("convert"), bound2, nil);
}


/* rewrite_step_expression -- Rewrite the step expression for the
 * node.
 * The step expression is  'by_expr' node with the following
 * attributes which may be empty:
 * 1 - direction: whether we want an 'ascending' or 'descending'
 *                index; the default is 'ascending'
 * 2 - step_expr: the actual expression node for the step; the default
 *                is the number 1
 * 3 - discrete:  a boolean stating whether the index should be
 *                implemented as a double or as an integer; default is
 *                true.
 */
void
rewrite_step_expression(lv* by_expr, lv* env)
{
  lv* step_expr;
  lv* rewritten_step_expr;
  lv* rewritten_step_exp_type;

  /* Rewrite the step expr only if the node argument is null. The node
   * argument will contain the rewritten expression.
   */
  if (args(by_expr) == nil)
    {
      step_expr = attr(intern("step_expr"), by_expr);
      if (step_expr != nil)
	{
	  rewritten_step_expr = rewrite_expression(step_expr, env);
	  if (!equal_type(type_of(rewritten_step_expr), number_type))
	    {
	      user_error(find_leaf(rewritten_step_expr),
			 "step of range expression is not numeric");
	      set_attr(intern("type"), rewritten_step_expr, error_type);
	    }
	}
      else
	{
	  /* Make up the default. Same as in 'parser.y' for empty
	   * 'step_options' rule.
	   */
	  rewritten_step_expr = node(intern("int"), nil,
	                             alist2(intern("value"), fixnum(1),
				            intern("type"), discrete_number_type));
	}
      args(by_expr) = list1(rewritten_step_expr);
    }
  else
    arg1(by_expr) = rewrite_expression(arg1(by_expr), env);

  /* Fill in possibly missing attributes */

  if (!attr(intern("direction"), by_expr))
    set_attr(intern("direction"), by_expr, intern("ascending"));

  /* Note that we do not do much more type checking here.  This is
   * because the type of the expression would most likely come out to
   * be 'double'
   */
  if (!attr(intern("discrete"), by_expr))
    {
      set_attr(intern("discrete"), by_expr, intern("true"));
    }
}


/* Rewite a floating 'in_expr' used as a cursor. */

/* This is the old version, which does not work since the intern("env") of
 * aggregate_expr may be empty.
lv*
rewrite_in_expr(lv* in_expr, lv* env)
{
  lv* aggregate_expr = arg2(in_expr);

  if (op(aggregate_expr) == intern("arrayrange"))
    arg2(in_expr) = rewrite_array_range(aggregate_expr,
					attr(intern("env"), aggregate_expr));
  else
    arg2(in_expr) = rewrite_expression(aggregate_expr,
					attr(intern("env"), aggregate_expr));

  return in_expr;
}
*/

/* The new version instead assumes that the 'env' parameters contains
 * the correct environment.
 */
lv*
rewrite_in_expr(lv* in_expr, lv* env)
{
  lv* aggregate_expr = arg2(in_expr);

  if (op(aggregate_expr) == intern("arrayrange"))
    arg2(in_expr) = rewrite_array_range(aggregate_expr, env);
  else
    arg2(in_expr) = rewrite_expression(aggregate_expr, env);

  /* I believe we need this.
   * Marco Antoniotti 19970717
   */
  set_attr(intern("type"), arg1(in_expr), arg1(type_of(arg2(in_expr))));

  return in_expr;
}


/* Forward declaration.
 */
void resolve_type(lv *, lv *);

/* Rewrite a set constructor.
 */
lv *
rewrite_setcons(lv *expr, lv *env)
{
  /* Adding support for the internally generated
   * sets (those that are accessed with the DOT 
   * notation).
   *
   * Tunc Simsek 19980528
   */
  lv *a = args(expr);
  lv *intrinsic = attr(intern("components"), expr);

  if ( intrinsic )
    {
      lv *op = op(intrinsic);

      /* Check that 'intrinsic' is a valid type.
       */
      if (op && op == intern("id"))
	{
	  /*
	   * intrinsic = rewrite_expression(intrinsic, global_env);
	   */
	  resolve_id(intrinsic, env);
	  if (! type_type_p(attr(intern("type"), intrinsic)))
	    {
	      user_error(intrinsic, 
			 "Argument to \"~a\"\
 must be a valid type. \"~a\" is not a type.", 
			 intern("components"),
			 attr(intern("name"), intrinsic));
	      exit(1);
	    }
	}
      else
	{
	  user_error(intrinsic, "Must be a valid user defined type.");
	  /* It is very difficult to continue after this type of
	   * error so I make sure to exit as not to give a seg. fault
	   * later on.
	   */
	  exit(1);
	}

      apush(intern("type"), node(intern("set"), list1(intrinsic), nil), attrs(expr));
    }
  else if (a)
    {
      lv *x, *c, *t, *ancestor;
      for (c = a; c; c = tl(c))
	{
	  hd(c) = rewrite_expression(hd(c), env);
	}
      /* Check that all elements have the same type.
       */
      x = hd(a);
      t = type_of(x);
      for (c = tl(a); c; c = tl(c))
	{
	  if (type_compatible_with_p(type_of(first(c)), t))
	    continue;
	  else if (type_compatible_with_p(t, type_of(first(c))))
	    {
	      t = type_of(first(c));
	      continue;
	    }
	  else
	    {
	      ancestor = find_types_ancestor(t, type_of(first(c)));

	      if (ancestor != nil)
		{
		  t = ancestor;
		  continue;
		}
	    }

	  /* If we have not continued yet, then we have different
	   * types.  The test is left around just for historical
	   * reasons and for safety as well.
	   */

	  if (! equal_type(type_of(hd(c)), t))
	    {
	      user_error(find_leaf(hd(c)), "mixed types in set constructor");
	      return error_expression;
	    }
	}
      apush(intern("type"), node(intern("set"), list1(t), nil), attrs(expr));
    }
  else
    {
      /* Making different leaves for null sets; then, during
       * "check_assignment", we will add an lv to the second param
       * which will contain the type of RHS.  That will be used during
       * the code generation to create a proper overhead for an empty
       * set.
       *
       * MAK
       */
      apush(intern("type"), node(intern("null_set"), nil, nil), attrs(expr));
    }
  return expr;
}


lv *
rewrite_setcons_iter(lv* setcons_expr, lv* env)
{
  lv* iter_expr    = arg1(setcons_expr);
  lv* iter_cursors = arg2(setcons_expr);
  lv* iter_cond    = arg3(setcons_expr);
  lv* set_computed_type;
  lv* rewritten_iter_cursors = nil;

  if (! iter_expr)
    {
      user_error(nil, "empty expression in set contructor iterator");
      return error_expression;
    }

  /* Rewrite each of the iterator expressions... */
  dolist (iter, iter_cursors)
    {
      rewritten_iter_cursors = cons(rewrite_in_expr(iter, attr(intern("env"), iter)),
				    rewritten_iter_cursors);
    }
  tsilod;

  /* ...flag their identifiers as "C variables" and remember what kind
   * of iterator it is (either 'set' or 'range'...
   */
  dolist (iter, rewritten_iter_cursors)
    {
      set_attr(intern("c_stack_id"), attr(intern("entity"), arg1(iter)), intern("true"));
      if (op(arg2(iter)) == intern("arrayrange"))
	set_attr(intern("range_element"), attr(intern("entity"), arg1(iter)), intern("true"));
      else
	set_attr(intern("set_element"), attr(intern("entity"), arg1(iter)), intern("true"));
    }
  tsilod;

  /* ... and plug them back in the 'setcons_expr'. */
  arg2(setcons_expr) = nreverse(rewritten_iter_cursors);

  /* Rewrite the iterator and the filter expressions using their
   * 'private' environment (and plug them back in just for pure
   * paranoia.)
   * Also check that the condition is a boolean expression.
   */
  iter_expr = rewrite_expression(iter_expr, attr(intern("env"), iter_expr));
  if (iter_cond != nil)		/* 'iter_cond' is optional */
    {
      iter_cond = rewrite_expression(iter_cond, attr(intern("env"), iter_cond));
      if (! equal_type(type_of(iter_cond), logical_type))
	user_error(find_leaf(iter_cond),
		   "condition on set or array former is not a boolean expression");
    }

  arg1(setcons_expr) = iter_expr;
  arg3(setcons_expr) = iter_cond;

  /* Compute the complete type of the resulting aggregate */
  set_computed_type = type_of(iter_expr);
  apush(intern("type"), node(intern("set"), list1(set_computed_type), nil), attrs(setcons_expr));

  return setcons_expr;
}


lv *
rewrite_arraycons(lv *expr, lv *env)
{
  if (args(expr))
    {
      lv* a  = mapcarx(rewrite_expression, args(expr), env);
      lv* t1 = type_of(first(a));
      lv* ancestor;

      args(expr) = a;
      dolist (x, rest(a))
	{
	  if (type_compatible_with_p(type_of(x), t1))
	    continue;
	  else if (type_compatible_with_p(t1, type_of(x)))
	    {
	      t1 = type_of(x);
	      continue;
	    }
	  else
	    {
	      ancestor = find_types_ancestor(t1, type_of(x));

	      if (ancestor != nil)
		{
		  t1 = ancestor;
		  continue;
		}
	    }

	  /* If we have not continued yet, then we have different
	   * types.  The test is left around just for historical
	   * reasons and for safety as well.
	   */

	  if (! equal_type(type_of(x), t1))
	    {
	      user_error(find_leaf(x), "mixed types in array constructor");
	      return error_expression;
	    }
	}
      tsilod;
      set_attr(intern("type"), expr, node(intern("array"), list1(t1), nil));
      return expr;
    }
  else
    {
      /************ MAK: create an overhead for empty array 
	return rewrite_expression(identifier(@"nil"), env);
	*****************************************************/
      set_attr(intern("type"), expr, node(intern("empty_array"), nil, nil));
      return expr;
    }
}


lv*
rewrite_arraycons_iter(lv* arraycons_expr, lv* env)
{
  lv* rewritten_array_expr;
  lv* component_type;

  /* This is really the same... */
  rewritten_array_expr = rewrite_setcons_iter(arraycons_expr, env);

  /* ... with the exception that we need to change the type of the
   * returned expression to 'array'.
   */
  component_type = arg1(attr(intern("type"), rewritten_array_expr));
  set_attr(intern("type"),
           rewritten_array_expr,
           node(intern("array"), list1(component_type), nil));

  return rewritten_array_expr;
}


lv *
rewrite_index(lv *expr, lv *env)
{
  lv *a = rewrite_expression(arg1(expr), env);
  lv *i = rewrite_expression(arg2(expr), env);
  lv *ta = type_of(a);

  arg1(expr) = a;
  arg2(expr) = i;
  if (! equal_type(type_of(i), number_type))
    {
      user_error(find_leaf(i), "index to array is not a number");
    }
  if (op(ta) == intern("array"))
    {
      set_attr(intern("type"), expr, arg1(ta));
    }
  else if (op(ta) != intern("error_type"))
    {
      user_error(find_leaf(a), "indexing a non-array");
      return error_expression;
    }
  return expr;
}



#ifndef OLD_CHECK_ASSIGN
void
check_assignment(lv *l, lv *r)
{
  lv *tl = type_of(l);
  lv *tr = type_of(r);

#if 0				/* Left here for possible future reference */
  lv *eg = attr(intern("entity"), l);
  lv *kg = nil;

  if (eg) 
    kg = attr(intern("kind"), eg);

  if (kg)
    if ((!glob_or_td) && (!strcmp("GLOBAL", str(kg))))
      {
	user_error(find_leaf(l), "global not allowed at LHS");
      }
#endif

  /* First of all we need to check that the LHS is an L-value */

  if (! L_value_p(l))
    {
      user_error(find_leaf(l), "LHS is not an L-value");
    }

  /* If the set is null, then we attach args from LHS to use the type
   * during the code generation! This is the only purpose for the empty
   * set to have arguments!!
   */

  if ((op(tl) == intern("set") && op(tr) == intern("null_set"))
      || (op(tl) == intern("array") && op(tr) == intern("empty_array")))
    {
      args(tr) = args(tl);
    }

  if (! type_compatible_with_p(tr, tl))
    {
      user_error(find_leaf(l), 
               "incompatible types in assignment or improper variable");

      /* MAK: do not use 2 lines below! type info in tl, tr is not
       * always available!
       */ 
#if 0
      user_error(find_leaf(l), "LHS type is ~s\n", tl);
      user_error(find_leaf(l), "RHS type is ~s\n", tr);
#endif
    }
}

#else /* OLD_CHECK_ASSIGN */

void
check_assignment(lv *l, lv *r)
{
  lv *tl = type_of(l);
  lv *tr = type_of(r);

#if 0				/* Left here for possible future reference */
  lv *eg = attr(intern("entity"), l);
  lv *kg = nil;

  if (eg) 
    kg = attr(intern("kind"), eg);

  if (kg)
    if ((!glob_or_td) && (!strcmp("GLOBAL", str(kg))))
      {
	user_error(find_leaf(l), "global not allowed at LHS");
      }
#endif

  /* First of all we need to check that the LHS is an L-value */

  if (! L_value_p(l))
    {
      user_error(find_leaf(l), "LHS is not an L-value");
    }

  /* If the set is null, then we attach args from LHS to use the type
   * during the code generation! This is the only purpose for the empty
   * set to have arguments!!
   */

  if ((op(tl) == intern("set") && op(tr) == intern("null_set"))
      || (op(tl) == intern("array") && op(tr) == intern("empty_array")))
    {
      args(tr) = args(tl);
    }

  if (! equal_type(tl, tr)
      && ! (op(tl) == intern("id") && tr == nil_type)
      && ! (op(tl) == intern("set") && op(tr) == intern("null_set"))
      && ! (op(tl) == intern("array") && op(tr) == intern("empty_array"))
      && ! descendant(tr, tl)  )
    {
      user_error(find_leaf(l), 
               "incompatible types in assignment or improper variable");

      /* MAK: do not use 2 lines below! type info in tl, tr is not
       * always available!
       */ 
#if 0
      user_error(find_leaf(l), "LHS type is ~s\n", tl);
      user_error(find_leaf(l), "RHS type is ~s\n", tr);
#endif
    }
}
#endif /* OLD_CHECK_ASSIGN */


void
check_op_assignment(lv* opassign, lv* op, lv *l, lv *r)
{
  lv *tl = type_of(l);
  lv *tr = type_of(r);

  /* First of all we need to check that the LHS is an L-value */

  if (! L_value_p(l))
    {
      user_error(find_leaf(l), "LHS is not an L-value");
    }

  if (number_type_p(tl))
    {
      if (! number_type_p(tr))
	{
	  user_error(find_leaf(l),
		     "incompatible types in operator form assignment (~s).",
		     op);
	  user_error(find_leaf(l),
		     "LHS is a number, while RHS is not.");
	}
    }
  else if (set_type_p(tl))
    {
      /* We overload '+:=' and '-:='.  They can be used to add a
       * single element to the set or to perform a shorthand set union
       * or difference.
       *
       * Marco Antoniotti 19970601
       */
      if (op == intern("+") || op == intern("-"))
	{
	  if (type_compatible_with_p(tr, arg1(tl)))
	    {
	      /* This is a add or a remove from a set. */
	      set_attr(intern("opassign_type"), opassign, intern("set_single_modify"));
	    }
	  else if (set_type_p(tr) && type_compatible_with_p(tr, tl))
	    {
	      /* This is a 'multiple' set assignment. I.e. all the
	       * elements of the RHS will be inserted in the LHS via a
	       * destructive operation.
	       */
	      set_attr(intern("opassign_type"), opassign, intern("set_multi_modify"));
	    }
	  else
	    {
	      /* No good */
	      user_error(find_leaf(l),
			 "incompatible types in operator form assignment (~s).",
			 op);
	      user_error(find_leaf(l),
			 "LHS is a set and the RHS is not of the correct type.");
	  
	    }
	}
      else
	{
	  /* ...for completeness */
	  user_error(find_leaf(l),
		     "operator ~s cannot be used with a set valued LHS.",
		     op);
	}
    }
  else
    {
      user_error(find_leaf(l),
		 "operator form assignment is not defined for LHS of type ~s",
		 op(tl));
    }
}


lv *
rewrite_create(lv *expr, lv *env)
{
  lv *type = rewrite_expression(arg1(expr), env);

  if (op(attr(intern("type"), type)) != intern("type_or_set"))
    {
      user_error(find_leaf(type),
		 "first argument to CREATE must be a component type");
      return error_expression;
    }
  else
    {
      lv *local_ns = first(attr(intern("env"), meaning(type)));
      dolist (i, tl(args(expr))) {
	lv *id = attr(intern("id"), i);
	lv *entity = ns_find(attr(intern("name"), id), local_ns);
	arg1(i) = rewrite_expression(arg1(i), env);
	if (! entity)
	  {
	    user_error(id, "\"~a\" is not defined in \"~a\"",
		       attr(intern("name"), id),
		       attr(intern("name"), type));
	    set_attr(intern("entity"), id, error_entity);
	    set_attr(intern("type"), id, error_type);
	  }
	else
	  {
	    set_attr(intern("entity"), id, entity);
	    set_attr(intern("type"), id, attr(intern("type"), entity));
	    check_assignment(id, arg1(i));
	  }
      } tsilod;
      /* The next line was missing until today.  I believe that this
       * is what causes problems on SGI and AIX.
       * Marco Antoniotti 19961021
       */
      return expr;
    }
}


lv *
rewrite_sync(lv *expr, lv *env)
{
  lv *a = mapcarx(rewrite_expression, args(expr), env);

  dolist (x, a) {
    if (op(type_of(x)) != intern("event_type")
	|| op(type_of(x)) != intern("open_event_type"))
      {
	user_error(find_leaf(x), "not an event");
      }
    if (op(x) != intern("external_event"))
      {
	user_error(find_leaf(x),
		   "synchronization must refer to external events");
	return error_expression;
      }
    if (op(x) == intern("external_event") && attr(intern("sync_type"), x))
      {
	user_error(find_leaf(x),
		   "only single-valued synchronization is allowed");
	return error_expression;
      }
  } tsilod;

  args(expr) = a;
  return expr;
}


/* MAK: workaround for <->  09/13/96 */
lv *
rewrite_donothing(lv *expr, lv *env)
{
  return rewrite_sync(expr, env);
}


lv *
rewrite_dnth(lv *expr, lv *env)
{
  return expr;
}


lv *
rewrite_external_event(lv *e, lv *env)
{
  lv *link = rewrite_expression(arg1(e), env);
  lv *event = arg2(e);
  lv *export_ns, *entity;
  lv *link_type = type_of(link);

  if (link_type == error_type)
    return error_expression;
  if (op(link_type) != intern("id"))
    {
      user_error(link, "\"~a\" is not a component", attr(intern("name"), link));
      return error_expression;
    }
  export_ns = attr(intern("export_ns"), meaning(link_type));
  entity = ns_find(attr(intern("name"), event), export_ns);
  if (entity)
    {
      set_attr(intern("entity"), event, entity);
    }
  else
    {
      user_error(event, "event \"~a\" is not exported by type \"~a\"",
		 attr(intern("name"), event), attr(intern("name"), link_type));
      return error_expression;
    }
  arg1(e) = link;
  /* Modifying --
   * Note now that the event_type can be specified.
   *
   * set_attr(intern("type"), e, event_type);
   * 
   * Tunc Simsek 15th December, 1997
   */
  if (entity)
    {
      set_attr(intern("type"), e, attr(intern("type"), entity));
    }
  else
    {
      set_attr(intern("type"), e, error_type);
    }
  return e;
}   


/* rewrite_op_assign --
 * no op for the time being.
 */
lv*
rewrite_op_assign(lv* assignment)
{
  return assignment;
}


/* Rewrite EXPR in environment ENV.
 */
lv *
rewrite_expression(lv *expr, lv *env)
{
  lv *x = op(expr);

  if (x == intern("call"))
    {
      return rewrite_call(expr, env);
    }
  else if (x == intern("id"))
    {
      resolve_id(expr, env);
      if (attr2(intern("rewriter"), intern("entity"), expr))
	{
	  user_error(expr, "invalid use of built-in function");
	  return error_expression;
	}
      else
	{
	  return expr;
	}
    }
  else if (x == intern("exists") || x == intern("minel") || x == intern("maxel"))
    {
      return rewrite_exists(expr, env);
    }
  else if (x == intern("setcons"))
    {
      return rewrite_setcons(expr, env);
    }
  else if (x == intern("setcons2"))
    {
      return rewrite_setcons_iter(expr, env);
    }
  else if (x == intern("arraycons"))
    {
      return rewrite_arraycons(expr, env);
    }
  else if (x == intern("arraycons2"))
    {
      return rewrite_arraycons_iter(expr, env);
    }
  else if (x == intern("arrayrange"))
    {
      /* This is here only because of the way 'exists', 'minel' and
       * 'maxel' are currently defined.
       */
      return rewrite_array_range(expr, env);
    }
  else if (x == intern("special_form"))
    {
      return rewrite_special_form(expr, env);
    }
  else if (x == intern("create"))
    {
      return rewrite_create(expr, env);
    }
  else if (x == intern("if"))
    {
      return rewrite_if (expr, env);
    }
  else if (x == intern("index"))
    {
      return rewrite_index(expr, env);
    }
  else if (x == intern("assign"))
    {
      lv *l = rewrite_expression(arg1(expr), env);
      lv *r = rewrite_expression(arg2(expr), env);

      arg1(expr) = l;
      arg2(expr) = r;
      check_assignment(l, r);
      return expr;
    }
  else if (x == intern("opassign"))
    {
      lv *l = rewrite_expression(arg1(expr), env);
      lv *r = rewrite_expression(arg2(expr), env);

      arg1(expr) = l;
      arg2(expr) = r;
      check_op_assignment(expr, attr(intern("op"), expr), l, r);
      return rewrite_op_assign(expr);
    }
  else if (x == intern("sync"))
    {
      return rewrite_sync(expr, env);
    }
  else if (x == intern("dlink"))
    {
      /*
	lv *ll = rewrite_expression(arg1(expr), env);
	lv *rr = rewrite_expression(arg2(expr), env);
	*/
      arg1(expr) = rewrite_expression(arg1(expr), env);
      arg2(expr) = rewrite_expression(arg2(expr), env);
      return expr;

      /************** SYNTAX CHANGE !!!  insert for dlink instead of '=' ****/

    }
  else if (x == intern("donothing"))
    {
      return rewrite_donothing(expr, env);
    }
  else if (x == intern("dnth"))
    {
      return rewrite_dnth(expr, env);
    }
  else if (x == intern("external_event"))
    {
      return rewrite_external_event(expr, env);
    }
  else if (x == intern("error"))
    {
      return expr;
    }
  else
    {
      lv *c;
      for (c = args(expr); c; c = tl(c))
	{
	  hd(c) = rewrite_expression(hd(c), env);
	}
      return expr;
    }
}


/* An equation can be a single ID, which must be a flow name;
 * a differential equation; or an algebraic definition.
 */
lv *
rewrite_equation(lv *e, lv *env)
{
  lv *req = rewrite_expression(e, env);

  if (op(req) == intern("error"))
    return req;

  if (op(req) == intern("id"))
    {
      if (! equal_type(type_of(req), flow_type))
	{
	  user_error(req, "\"~a\" is not a flow", attr(intern("name"), req));
	  req = error_expression;
	}
    }
  else if (op(req) != intern("=")
	   && req != error_expression
	   && op(req) != intern("link-=")
	   && op(req) != intern("set-="))
    {
      user_error(find_leaf(e), "illegal equation");
      req = error_expression;
    }
  else
    {
      lv *lhs = arg1(req);
      if (op(lhs) == intern("id"))
	{
	  op(req) = intern("define");
	  set_attr(intern("algebraic"), attr(intern("entity"), lhs), intern("true"));
	}
      else if (op(lhs) == intern("derivative"))
	{
	  lv *v = arg1(lhs);
	  if (op(v) != intern("id"))
	    {
	      user_error(find_leaf(v), "illegal derivative");
	      req = error_expression;
	    }
	  else
	    {
	      op(req) = intern("equate");
	      arg1(req) = v;
	      set_attr(intern("differential"), attr(intern("entity"), v), intern("true"));
	    }
	}
      else
	{
	  user_error(find_leaf(lhs), "illegal left-hand side of equation");
	  req = error_expression;
	}
      if (req != error_expression &&
	  attr(intern("kind"), attr(intern("entity"), arg1(req))) == intern("INPUT"))
	{
	  user_error(arg1(req), "illegal input definition");
	}
    }
  /* Check for continuous number */
#if 0
  if (op(req) != intern("error")
      && op(req) != intern("id")
      && ! continuous_p(arg1(req)))
    {
      user_error(arg1(req), "discrete number may not be defined by a flow.");
    }
#endif

  if (op(req) != intern("id"))
    {
      /* Changed the order of the logical expressions:
       * It should first be checked whether op(req) is intern("error")
       * or not.  Because if it is then continuous_p will give
       * a seg. fault.  But in the case below if op(req) is intern("error")
       * then continuous_p will never be called anyway.
       *
       * Tunc Simsek 4th November, 1997
       */
      if (op(req) != intern("error") 
	  && !continuous_p(arg1(req)))
	{
	  if (arg2(req))
	    {
	      if (continuous_p(arg2(req)))
		user_error(arg1(req), 
			   "discrete number may not be defined by a flow.");
	    }
	}
    }
  return req;
}


int
continuous_p(lv *e)
{
  if (op(e) == intern("id"))
    return continuous_p(attr(intern("entity"), e));
  else if (op(e) == intern("access"))
    return continuous_p(arg1(e)) || continuous_p(attr(intern("accessor"), e));
  else if (op(e) == intern("entity"))
    return attr(intern("continuous"), attr(intern("type"), e)) == intern("true");
  else dolist (a, args(e)) {
    if (continuous_p(a))
      return 1;
  } tsilod;

  return 0;
}


lv *
rewrite_action(lv *a, lv *env)
{
  lv *l, *r, *e, *erp;
  lv *na = rewrite_expression(a, env);

  /********** SYNTAX CHANGE !!!!!  insert for '<-'  *********************/

  if (op(na) == intern("=")
      || op(na) == intern("link-=")
      || op(na) == intern("dlink")
      || op(na) == intern("set-="))
    {
      /* Connection */
      l = arg1(na);
      r = arg2(na);
      op(na) = intern("define");

      if (op(l) == intern("id"))
	{
	  user_error(find_leaf(na),
		     " connection: LHS is not an input of another component");
	  return error_expression;

	  e = attr(intern("entity"), l);

	  if (attr(intern("kind"), e) == intern("INPUT"))
	    {
	      user_error(l, "cannot define own input (open to debate)");
	      return error_expression;
	    }
	  else
	    {
	      set_attr(intern("algebraic"), e, intern("true"));
	    }
	}
      else if (op(l) != intern("access"))
	{
	  user_error(find_leaf(l), "malformed connection");
	  return error_expression;
	}
      else
	{
	  lv *accessor = attr(intern("accessor"), l);

	  if (attr(intern("kind"), accessor) != intern("INPUT"))
	    {
	      user_error(find_leaf(l), "not an input");
	      return error_expression;
	    }
	  else
	    {

	      /***********MAK some checks for (=) *************/

#if 0
	      if (op(r) == intern("float"))
		{
		  user_error(r, "improper use of \"=\"; try \":=\" "); 
		  return error_expression;
		}

####if 0
	      if (! (op(r) == intern("id") || op(r) == intern("access")))
		{
		  user_error(find_leaf(r), "connection: bad RHS ");
		  return error_expression;
		}
####endif

	      if (op(r) == intern("id"))
		if (!(attr(intern("kind"), attr(intern("entity"), r))))
		  {
		    user_error(find_leaf(r), "temporary variable at the RHS "); 
		    return error_expression;
		  }
#endif

	      set_attr(intern("algebraic"), accessor, intern("true"));
	    }
	}

      /* This first condition is the original case.
       * I will add another to rule out flows with no continuous
       * left end side only.
       * This second condition can be removed once the _AA_ and
       * _AAL_ accessors are changed to reflect the possibility of
       * having different kinds of flows.
       * Marco Antoniotti 19960912
       */

#if 0
      if (! continuous_p(l) && continuous_p(r))
	{
	  user_error(find_leaf(l),
		     "illegal assignment of time-evolving expression:");
	  user_error(find_leaf(l),
		     "left hand side of equation is not continuous.");
	  user_error(find_leaf(l),
		     "while right hand side is.");
	  return error_expression;
	}
      else if (! continuous_p(l) && ! continuous_p(r)) /* Extra Check! */
	{
	  /* Both sides are not continuous.
	   * I signal an error if their types are not numeric.
	   * This is the extra check that might be removed in the
	   * future.
	   * Marco Antoniotti 19960912
	   */
	  if (!(number_type_p(type_of(l)) && number_type_p(type_of(r))))
	    /* Added 'type' computation (otherwise 'number_type_p'
	     * wouldn't work.
	     * Marco Antoniotti 19960916
	     */
	    {
	      user_error(find_leaf(l),
			 "illegal assignment of time-evolving expression:");
	      user_error(find_leaf(l),
			 "either left or right hand side is not");
	      user_error(find_leaf(l),
			 "numeric expression");

	      return error_expression;
	    }
	  else
	    return na;
	}
      else
#endif
	return na;

    }
  else if (! equal_type(type_of(na), void_type) && op(na) != intern("create"))
    {
      user_error(find_leaf(a), "illegal or malformed action");
      return error_expression;
    }
  else if (op(na) == intern("let") && op(arg1(na)) != intern("create"))
    {
      user_error(find_leaf(a), "LET expression may only be CREATE");
      return error_expression;
    }
  else
    return na;
}


/* collect_ff_calls_in_expr -- */

lv*
collect_ff_calls_in_expr(lv* expr)
{
  lv* f;
  lv* ff_signature;

  if (op(expr) == intern("id") || op(expr) == intern("numliteral"))
    return nil;
  else if (op(expr) == intern("error"))
    return nil;
  else if (op(expr) == intern("call"))
    {
      f = arg1(expr);
      if (op(f) == intern("id"))
	{
	  ff_signature = attr(intern("signature"), expr); /* FF calls are
						  * tagged with the
						  * signature.
						  */
	  if (ff_signature != nil)
	    return cons(ff_signature,
			mapcan(collect_ff_calls_in_expr, tl(args(expr))));
	  else
	    return mapcan(collect_ff_calls_in_expr, tl(args(expr)));
	}
      else
	{
	  return append(collect_ff_calls_in_expr(f),
			mapcan(collect_ff_calls_in_expr, tl(args(expr))));
	}
    }
  else if (predefined_id_p(op(expr)))
    {
      return mapcan(collect_ff_calls_in_expr, args(expr));
    }
  else
    return nil;
}


/* rewrite_action_tr --
 * 
 * It looks like this function contains a bunch of historical baggage.
 * The intern("let") tag is not really used anywhere.
 *
 * Marco Antoniotti
 * 19970506
 *
 * Added check for foreign functions causing side effects.
 *
 * Marco Antoniotti
 * 19970506
 */
lv *
rewrite_action_tr(lv *a, lv *env)
{
  lv *na = rewrite_expression(a, env);
  lv *ff_call_list = nil;

  /** SYNTAX CHANGE !!!!! probably this one is OK (parser takes care) **/

  /* It should always be the case */
  if (op(na) == intern("assign") || op(na) == intern("opassign"))
    {
      /* Collect all the calls to FF's in the RHS */
      ff_call_list = collect_ff_calls_in_expr(arg2(na));
      dolist (ffcall, ff_call_list)
	{
	  if (ff_has_side_effects_p(ffcall))
	    {
	      user_warning(find_leaf(a),
			   "Foreign Function '~s' causes side effects",
			   attr2(intern("name"), intern("id"), ffcall));
	      user_warning(find_leaf(a),
			   " and its use in a transition action");
	      user_warning(find_leaf(a),
			   "may cause possible race conditions");
	    }
	}
      tsilod;
    }


  if (op(na) == intern("=") || op(na) == intern("link-=") || op(na) == intern("dlink"))
    {
      /* connection */
      user_error(find_leaf(a), 
		 "Setup-only action is attempted during transition");
      return error_expression;

    }
  else if (! equal_type(type_of(na), void_type) && op(na) != intern("create"))
    {
      user_error(find_leaf(a), "illegal or malformed action");
      return error_expression;
    }
  else if (op(na) == intern("let") && op(arg1(na)) != intern("create"))
    {
      user_error(find_leaf(a), "LET expression may only be CREATE");
      return error_expression;
    }
  else return na;
}


/* Specialized rewriters.
 */

lv *
rewrite_ptm(lv *f, lv *entity, lv *a)
{
  switch (length(a))
    {
    case 1:
      if (number_type_p(type_of(first(a))))
	{
	  return node(intern("negate"), a, alist1(intern("type"), number_type));
	}
      else
	{
	  user_error(f, "argument to unary minus is not a number");
	  return error_expression;
	}
      break;

    case 2:
      {
	lv *t1 = type_of(first(a));
	lv *t2 = type_of(second(a));
	if (t1 == error_type || t2 == error_type)
	  {
	    return error_expression;
	  }
	else if (number_type_p(t1) && number_type_p(t2))
	  {
	    return node(attr(intern("operator"), entity), a,
			alist1(intern("type"), number_type));
	  }
	else if (set_type_p(t1) && equal_type(t1, t2))
	  {
	    lv *o = attr(intern("operator"), entity);
	    lv *newop;
	    if (o == intern("+")) newop = intern("set_union");
	    else if (o == intern("*")) newop = intern("set_intersection");
	    else if (o == intern("-")) newop = intern("set_difference");
	    return node(newop, a, alist1(intern("type"), type_of(first(a))));
	  }
	else
	  {
	    user_error(f, "illegal argument types for \"~a\"", attr(intern("name"), f));
	    return error_expression;
	  }
	break;
      }

    default:
      abort();
    }
}


lv *
rewrite_binmath_(lv *f, lv *entity, lv *a, lv *result_type)
{
  if (length(a) != 2)
    {
      user_error(f, "wrong number of arguments for \"~a\"", attr(intern("name"), f));
      return error_expression;
    }
  else if (number_type_p(type_of(first(a)))
	   && number_type_p(type_of(second(a))))
    {
      return node(attr(intern("operator"), entity), a, alist1(intern("type"), result_type));
    }
  else
    {
      user_error(f, "invalid argument types for \"~a\"", attr(intern("name"), f));
      return error_expression;
    }
}


lv *
rewrite_binmath(lv *f, lv *entity, lv *a)
{
  return rewrite_binmath_(f, entity, a, number_type);
}


lv *
rewrite_relational(lv *f, lv *entity, lv *a)
{
  return rewrite_binmath_(f, entity, a, logical_type);
}


lv *
rewrite_logical(lv *f, lv *entity, lv *a)
{
  switch (length(a))
    {
    case 1:
      {
	lv *lt = type_of(first(a));
	if (! equal_type(lt, logical_type))
	  {
	    user_error(f, "invalid argument type for \"~a\"", attr(intern("name"), f));
	  }
	return node(intern("not"), a, alist1(intern("type"), logical_type));
      }

    default: 
      {
	lv *lt = type_of(first(a));
	lv *rt = type_of(second(a));

	if (!equal_type(lt, logical_type) || !equal_type(rt, logical_type)) 
	  {
	    user_error(f, "invalid argument types for \"~a\"", attr(intern("name"), f));
	    return error_expression;
	  }
	else
	  {
	    return node(attr(intern("name"), f), a, alist1(intern("type"), logical_type));
	  }
      }
    }
}


lv *
rewrite_size(lv *f, lv *entity, lv *a)
{
  if (length(a) != 1)
    {
      /* Check the number of arguments of SIZE. */
      user_error(find_leaf(a), "wrong number of arguments for COPY");
      return error_expression;
    }
  else
    {
      /* Ignore ENTITY. */
      lv *x = first(a);
      lv *xt = type_of(x);

      if (op(xt) == intern("array"))
	{
	  return node(intern("array_size"), a, alist1(intern("type"), number_type));
	}
      else if (op(xt) == intern("set"))
	{
	  return node(intern("set_size"), a, alist1(intern("type"), number_type));
	}
      else
	{
	  user_error(f, "argument of SIZE must be array or set");
	  return error_expression;
	}
    }
}


lv *
rewrite_narrow(lv *f, lv *entity, lv *a)
{
  lv *narrow_type;
  lv *wide_type;
  lv *wide_component;
  lv *wide_entity;
  lv *wide_kind;

  /* Check the number of arguments of NARROW. */
  if (length(a) != 2)
    {
      user_error(find_leaf(a), "wrong number of arguments for NARROW");
      return error_expression;
    }

  /* Check the first argument of NARROW. */
  narrow_type = first(a);
  if (op(type_of(narrow_type)) != intern("type_or_set"))
    {
      user_error(find_leaf(narrow_type),
		 "first argument of NARROW must be a component type");
      return error_expression;
    }

  /* Check the second argument of NARROW. */
  wide_component = second(a);
  wide_entity = attr(intern("entity"), wide_component);
  if (wide_entity != NULL)
    wide_kind = attr(intern("kind"), wide_entity);
  else
    {
      user_error(find_leaf(narrow_type),
		 "second argument of NARROW must be a component");
      return error_expression;
    }

  /* I do not understand the meaning of this error here.
   * Besides, it breaks some seemingly correct programs, so I disable
   * it for the time being.
   *
   * Marco Antoniotti 19970825
   */
  if (0 && strcmp(str(wide_kind), "STATE") && strcmp(str(wide_kind), "GLOBAL"))
    {
      user_error(find_leaf(narrow_type),
		 "second argument of NARROW must be a non-state component");
      return error_expression;
    }

  /* Check the type of the second argument of NARROW. */
  wide_type = type_of(wide_component);
  if (! descendant(narrow_type, wide_type))
    {
      user_error(f, "type of second arg of NARROW is not a supertype of \"~a\"",
		 attr(intern("name"), narrow_type));
      return error_expression;
    }

  /* If everything is correct set the type of the NARROW expression. */
  return node(intern("narrow"), a, alist1(intern("type"), narrow_type));
}


lv *
rewrite_copy(lv *f, lv *entity, lv *a)
{
  if (length(a) != 1)
    {
      /* Check the number of arguments of COPY. */
      user_error(find_leaf(a), "wrong number of arguments for COPY");
      return error_expression;
    }
  else
    {
      /* Ignore ENTITY. */
      lv *x = first(a);
      lv *xt = type_of(x);

      if (op(xt) == intern("array"))
	{
	  return node(intern("copy_array"), a, alist1(intern("type"), xt));
	}
      else if (op(xt) == intern("set"))
	{
	  return node(intern("copy_set"), a, alist1(intern("type"), xt));
	}
      else
	{
	  user_error(f, "argument of COPY must be array or set");
	  return error_expression;
	}
    }
}


/******** MAK: added max, min, atan2 **************/

#define max_function 1001
#define min_function 1002
#define atan2_function 1003

lv *
rewrite_func2args(int funame, lv *f, lv *entity, lv *a)
{
  if (length(a) != 2)
    {
      user_error(f, "wrong number of arguments for \"~a\"", attr(intern("name"), f));
      return error_expression;
    }
  else if (number_type_p(type_of(first(a))) &&
	   number_type_p(type_of(second(a))))
    {
      if (funame == max_function)
	return node(intern("max"), a, alist1(intern("type"), number_type));
      else if (funame == min_function)
	return node(intern("min"), a, alist1(intern("type"), number_type));
      else if (funame == atan2_function)
	return node(intern("atan2"), a, alist1(intern("type"), number_type));
    }
  else
    {
      user_error(f, "invalid argument types for \"~a\"", attr(intern("name"), f));
      return error_expression;
    }
}


lv *
rewrite_max(lv *f, lv *entity, lv *a)
{
  return rewrite_func2args(max_function, f, entity, a);
}


lv *
rewrite_min(lv *f, lv *entity, lv *a)
{
  return rewrite_func2args(min_function, f, entity, a);
}


lv *
rewrite_atan2(lv *f, lv *entity, lv *a)
{
  return rewrite_func2args(atan2_function, f, entity, a);
}


lv *
rewrite_random(lv *f, lv *entity, lv *a)
{
  if (length(a) != 0)
    {
      user_error(f, "wrong number of arguments for \"~a\"", attr(intern("name"), f));
      return error_expression;
    }
  else 
    return node(intern("random"), a, alist1(intern("type"), number_type));

}


lv*
rewrite_ln(lv* f, lv* entity, lv* a)
{
  if (length(a) != 1)
    {
      user_error(f, "wrong number of arguments for \"~a\"", attr(intern("name"), f));
      return error_expression;
    }
  else
    return node(intern("log"), a, alist1(intern("type"), number_type));
}


lv *
rewrite_equality(lv *f, lv *entity, lv *a)
{

  /* Ignore ENTITY. */
  lv *lt = type_of(first(a));
  lv *rt = type_of(second(a));

  /* Modifying equality mechanism to account
   * for the new inheritance mechanism:
   *
   * equal_type --> super_type_p
   *
   * Tunc Simsek 21st October, 1997
   */
  if (!  type_compatible_with_p(rt, lt) 
      && ! (lt == nil_type && op(rt) == intern("id")
	    || rt == nil_type && op(lt) == intern("id")))
    {
      if ( ! type_compatible_with_p(lt, rt)
	   && ! (lt == nil_type && op(rt) == intern("id")
		 || rt == nil_type && op(lt) == intern("id")))
	{
	  user_error(f, "invalid argument types for \"~a\"", attr(intern("name"), f));
	  return error_expression;
	}
      else 
	{
	  user_error(f, "left hand side of \"~a\" must be supertype of\n\
right hand side", attr(intern("name"), f));
	  return error_expression;
	}
    }
  else
    {
      lv *t = type_of(first(a));
      lv *e = node((number_type_p(t) ? intern("=") :
		    set_type_p(t) ? intern("set-=") :
		    intern("link-=")),
		   a,
		   alist1(intern("type"), logical_type));
		   if (attr(intern("name"), f) == intern("=") || attr(intern("name"), f) == intern("dlink")) 
	{
	  return e;
	}
      else
	{
	  return node(intern("not"), list1(e), alist1(intern("type"), logical_type));
	}
    }
}


lv *
rewrite_derivative(lv *f, lv *entity, lv *a)
{
  /* Ignore F and ENTITY. */
  lv *x = first(a);
  if (op(x) != intern("id") || ! number_type_p(type_of(x)))
    {
      user_error(find_leaf(f),
		 "operand of derivative is not a numeric variable");
      return error_expression;
    }
  else
    {
      return node(intern("derivative"), a, alist1(intern("type"), number_type));
    }
}


lv *
rewrite_in(lv *f, lv *entity, lv *a)
{
  /* Ignore F and ENTITY. */
  lv *x = first(a);
  lv *tx = type_of(x);
  lv *s = second(a);
  lv *ts = type_of(s);

  if (! set_type_p(ts))
    {
      user_error(find_leaf(s), "second argument to IN is not a set type");
      return error_expression;
    }

  if (! descendant(tx, arg1(ts)))
    {
      user_error(find_leaf(x), "wrong type of first argument to IN");
      return error_expression;
    }
  else
    {
      return node(intern("in"), a, alist1(intern("type"), logical_type));
    }
}


/* Static semantics.
 */

lv *
element_type_of(lv *expr, lv *env)
{
    return node(intern("element_type_of"),
		list1(node(intern("type_of"), list2(expr, env), nil)),
		nil);
}


/* First pass for constructing the symbol table.  The types
 * ELEMENT_TYPE_OF and TYPE_OF must go away by the end of typechecks.
 */

/* set_env_for_special_form -- forward declaration */

void set_env_for_special_form(lv*, lv*, lv*);


/* set_env_for_iter_constructor -- forward declaration */

void set_env_for_iter_constructor(lv*, lv*, lv*);
void set_env_for_range(lv*, lv*, lv*);


/* set_env_for_expression --
 *
 * Log:
 * Marco Antoniotti 19961230
 * Modified to take care of new iterators.
 */
void
set_env_for_expression(lv *expr, lv *env, lv *also_env)
{
  lv *s = op(expr);

  if (s == intern("exists") || s == intern("minel") || s == intern("maxel"))
    {
      lv *new_ns = make_ns();
      lv *new_env = cons(new_ns, env);
      lv *type = element_type_of(arg1(expr), env);
      define_id(attr(intern("id"), expr), new_ns, type, nil, nil);
      if (also_env)
	{
	  define_id(attr(intern("id"), expr), hd(also_env), nil, nil, nil);
	}
      add_attr(intern("env"), arg2(expr), new_env);
      set_env_for_expression(arg1(expr), env, also_env);
      set_env_for_expression(arg2(expr), new_env, also_env);
    }
  else if (s == intern("setcons2") || s == intern("arraycons2"))
    {
      set_env_for_iter_constructor(expr, env, also_env);
    }
  else if (s == intern("arrayrange"))
    {
      set_env_for_range(expr, env, also_env);
    }
  else if (s == intern("special_form"))
    {
      set_env_for_special_form(expr, env, also_env);
    }
  else
    {
      dolist (c, args(expr))
	{
	  set_env_for_expression(c, env, also_env);
	}
      tsilod;
    }
}


/* set_env_for_iter_constructor -- Definition.
 *
 * Log:
 * Marco Antoniotti 19961230
 * Added
 */
void
set_env_for_iter_constructor(lv* expr, lv* env, lv* also_env)
{
  /* The set and array constructors provide a series of 'iterator'
   * variables to be unfolded into a series of nested loops.
   * Therefore we need to build up a new env from this sequence of
   * 'in' expressions.
   */
  lv* new_ns;			/* Namespace containing a single id
				 * for each 'in' expression.
				 */
  lv* constructor_env = env;    /* The Environment where the
				 * constructor expression and the
				 * filter condition will be evaluated.
				 */
  lv* in_exprs = arg2(expr);	/* The list of 'in' expressions. */

  lv* iter_env = env;		/* A temporary Environment used during
				 * the construction of 'constructor_env'.
				 */
  lv* in_expr;			/* Temporary cursor. */
  lv* type;			/* Computed type of iterator variable. */

  /* Processing the 'in' expressions in sequence.
   * This ensures that the (i-th + 1) expression will see the
   * identifier defined by the i-th one.
   */
  dolist (in_expr, in_exprs)
    {
      /* Just being paranoid... */
      if (op(in_expr) != intern("in_expr"))
	internal_error("expected an 'in_expr' while building environment.");

      new_ns = make_ns();
      type = element_type_of(arg2(in_expr), constructor_env);
      constructor_env = cons(new_ns, constructor_env);

      /* Now we define the identifier.
       * Note that this might not work with the new 'define' scheme,
       * but let's see what happens first.
       */
      define_id(arg1(in_expr), new_ns, type, nil, nil);

      if (also_env)
	{
	  /* This is here only for historical reasons.  It should
	   * never happen to call this function with 'also_env'
	   * different from nil.
	   */
	  define_id(arg1(in_expr), hd(also_env), type, nil, nil);
	}

      /* Now make sure that the iterator expression has the necessary
       * environment and update the 'iter_env'.
       */
      set_env_for_expression(arg2(in_expr), iter_env, also_env);
      set_attr(intern("env"), in_expr, iter_env); /* This was missing, and made
					  * 'rewrite_setcons_iter
					  * choke while rewriting the
					  * iter expressions.
					  * Marco Antoniotti 19970305
					  */
      iter_env = constructor_env;
    }
  tsilod;

  /* Now we can set the environment for the iterator constructor and
   * for the filter expression.
   */
  add_attr(intern("env"), arg1(expr), constructor_env);
  set_env_for_expression(arg1(expr), constructor_env, also_env);

  if (arg3(expr) != nil)
    {
      add_attr(intern("env"), arg3(expr), constructor_env);
      set_env_for_expression(arg3(expr), constructor_env, also_env);
    }
}


void
set_env_for_range(lv* range_expr, lv* env, lv* also_env)
{
  set_env_for_expression(attr(intern("bound1"), range_expr), env, also_env);
  set_env_for_expression(attr(intern("bound2"), range_expr), env, also_env);
  if (attr(intern("step_expr"), range_expr))
    set_env_for_expression(attr(intern("step_expr"), range_expr), env, also_env);
}


void
set_env_for_special_form(lv* sf_expr, lv* env, lv* also_env)
{
  set_env_for_expression(arg1(sf_expr), env, also_env);
  if (length(args(sf_expr)) > 1 && arg2(sf_expr) != nil)
    set_env_for_expression(arg2(sf_expr), env, also_env);
}


/*********** SYNTAX CHANGE !!!!!!!!!**************************/
void
set_env_for_definitions(lv *deflist, lv *ns, lv *env)
{
  dolist (a, deflist)
    {
      if (op(a) != intern("declare"))
	user_error(find_leaf(a), "malformed local definition");
      else
	{
	  define_id(attr(intern("id"), a), ns, arg1(a), nil, nil);
	  if (tl(args(a)))
	    set_env_for_expression(arg2(a), env, nil);
	}
    }
  tsilod;
}


/*
 * Initialize the environments for a type definition.
 */
void
set_env_for_typedef(lv *type_def)
{
  lv *c, *trans, *e, *a;
  lv *export_ns = make_ns();
  int g = type_def == global_typedef;
  lv *local_ns = g ? global_typedef_ns : make_ns();
  lv *local_env = g ? list2(local_ns, global_ns) : cons(local_ns, global_env);
  lv *setup = attr(intern("setup"), type_def);
  lv *id = attr(intern("id"), type_def);
  lv *exttypedef = attr(intern("exttypedef"), type_def);

  lv *setup_ns = make_ns();
  lv *setup_env = cons(setup_ns, local_env);

  add_attr(intern("env"), type_def, local_env);
  add_attr(intern("export_ns"), type_def, export_ns);
  add_attr(intern("setup_env"), type_def, setup_env);

  /* Declare type in global namespace.  The type of the type is
   * TYPE_OR_SET_TYPE, meaning a type or a set depending on context.
   * The child is the element type, that is the id itself.
   */
  
  /* Adding support for external typedefs.
   *
   * Tunc Simsek 15th April, 1998
   */
  if(exttypedef && exttypedef == intern("true"))
    {
      define_id(id,
		global_ns,
		node(intern("type_or_set"), list1(id), alist1(intern("exttypedef"), intern("true"))),
		type_def,
		intern("TYPE"));
    }
  else
    {
      define_id(id,
		global_ns,
		node(intern("type_or_set"), list1(id), nil),
		type_def,
		intern("TYPE"));
    }

  /* Declare flows. */
  dolist (c, attr(intern("flow"), type_def))
    {
      lv *id = attr(intern("id"), c);
      define_id(id, local_ns, flow_type, c, nil);

      /* Set local environment for equations */
      dolist (eq, args(c))
	{
	  set_env_for_expression(eq, local_env, nil);
	}
      tsilod;
    }
  tsilod;

  /* Declare self variable. */
  define_id(node(intern("id"), nil, alist1(intern("name"), intern("self"))),
	    local_ns, id, nil, intern("SELF"));

  /* Declare state variables. */
  dolist (c, attr(intern("state"), type_def))
    {
      lv *id = attr(intern("id"), c);
      lv *type = arg1(c);
      define_id(id, local_ns, type, nil, intern("STATE"));
      if (tl(args(c)))
	set_env_for_expression(arg2(c), local_env, nil);
    }
  tsilod;

  /* Declare inputs. */
  dolist (c, attr(intern("input"), type_def))
    {
      lv *id = attr(intern("id"), c);
      lv *type = arg1(c);
      define_id(id, local_ns, type, nil, intern("INPUT"));
      define_id(id, export_ns, nil, nil, nil);
      if (tl(args(c)))
	set_env_for_expression(arg2(c), local_env, nil);
    }
  tsilod;

  /* Declare outputs. */
  dolist (c, attr(intern("output"), type_def))
    {
      lv *id = attr(intern("id"), c);
      lv *type = arg1(c);
      define_id(id, local_ns, type, nil, intern("OUTPUT")); /* Make this
						      non-capital
						      and it does
						      not work */
      define_id(id, export_ns, nil, nil, nil);
      if (tl(args(c))) 
	set_env_for_expression(arg2(c), local_env, nil);
    }
  tsilod;

  /* Declare global variable. */
  dolist (c, attr(intern("global"), type_def))
    {
      lv *id = attr(intern("id"), c);
      lv *type = arg1(c);
      define_id(id, local_ns, type, nil, intern("GLOBAL"));
      if (tl(args(c)))
	set_env_for_expression(arg2(c), local_env, nil);
    }
  tsilod;

  /* Changed way in which the 'setup_env' is built and used.
   * In the previous versions, the 'set_env_for_expression' used
   * 'local_env', which is not necessarily correct.
   * the function.
   *
   * Marco Antoniotti 19970710
   */
  if (setup != nil)
    {
      set_env_for_definitions(attr(intern("define"), setup), setup_ns, setup_env);
      dolist (a, attr(intern("do"), setup)) {
	set_env_for_expression(a, setup_env, nil);
      } tsilod;

      dolist (conn, attr(intern("connections"), setup)) {
	set_env_for_expression(conn, setup_env, nil);
      } tsilod;
    }

  /* Declare discrete states names. */
  dolist (c, attr(intern("discrete"), type_def)) {
    lv *id = attr(intern("id"), c);
    lv *inv = attr(intern("invariant"), c);

    if (inv)
      set_env_for_expression(inv, local_env, nil);
    define_id(id, local_ns, state_type, c, nil);
    /* Set local environment for equations. */
    dolist (eq, attr(intern("equations"), c)) {
      set_env_for_expression(eq, local_env, nil);
    } tsilod;
  } tsilod;

  /* Declare events in export list.  WEIRD: NEED TO CHANGE LANGUAGE.
   */
  /* Modifying --
   * The modified export syntax requires that the intern("export")
   * attribute be a list of intern("declare") node.  Now it is time
   * to:
   * 1) Map the intern("declare") nodes to intern("id") nodes
   * 2) Declare the events in the newly mapped export list.
   *
   * Tunc Simsek 12th December, 1997
   */
  /*
   * dolist (id, attr(intern("export"), type_def)) {
   *  define_id(id, local_ns, event_type, nil, nil);
   *  define_id(id, export_ns, nil, nil, nil);
   * } tsilod;
   */
  /* First we define a new scope.
   */
  if(1)
    {
      lv *export = nil;

      dolist (declare, attr(intern("export"), type_def)) {
	lv *id = attr(intern("id"), declare);
	lv *type = arg1(declare);

	export = nconc(export, list1(id));
	define_id(id, local_ns, type, nil, nil);
	define_id(id, export_ns, nil, nil, nil);
      } tsilod;
      /* This next move will cause a small memory leak, but it 
       * is fairly finite so I'm not going to worry about it:
       * 
       * i.e. the old intern("export") attr. is lost into empty space.
       */
      set_attr(intern("export"), type_def, export);
    }

  /* Declare events, ONE variables, and local variables in actions
   * (DEFINEs and quantifiers).
   */
  dolist (trans, attr(intern("transition"), type_def)) {
    lv *action_ns = make_ns();
    lv *action_env = cons(action_ns, local_env);
    lv *guard = attr(intern("guard"), trans);
    add_attr(intern("action_env"), trans, action_env);

    dolist (e, attr(intern("events"), trans)) {
      if (op(e) == intern("id"))
	{
#if 0
	  define_or_redefine_id(e, local_ns, event_type, nil, nil);
#endif
	}
      else
	{
	  /* External event: declare variable in ONE clause, if
	   * present.
	   */
	  lv *s = attr(intern("sync_type"), e);
	  if (nodep(s)) {
	    define_id(s, action_ns,
		      element_type_of(arg1(e), local_env),
		      nil, nil);
	  }
	}
    } tsilod;

    set_env_for_definitions(attr(intern("define"), trans), action_ns, action_env);

    /* Additional variables are declared by EXISTS (MINEL, MAXEL)
     * expressions in guards.  The scope of the free variables in
     * EXISTS clauses in a guard for a transition is all the
     * actions for that transition.
     */
    if (guard)
      {
	set_env_for_expression(guard, local_env, action_env);
      }
    dolist (a, attr(intern("do"), trans)) {
      set_env_for_expression(a, action_env, nil);
    } tsilod;
  } tsilod;

#if 0
  /* Construct exported environment.  Check that exported IDs
   * are defined locally.
   */
  dolist (id, attr(intern("export"), type_def)) {
    lv *name = attr(intern("name"), id);
    lv *entity = ns_find(name, local_ns);

    if (entity)
      {
	if (! equal_type(event_type, attr(intern("type"), entity)))
	  {
	    user_error(id, "exporting non-event \"~a\"", name);
	  }
      }
    else
      {
	user_error(id, "exporting undefined identifier \"~a\"", name);
	entity = error_entity;
      }
    ns_define(name, export_ns, entity);
    add_attr(intern("entity"), id, entity);
  } tsilod;
#endif
}

void
declare_external_function(lv *fd)
{
  lv *id = attr(intern("id"), fd);
  lv* ff_call_wrapper_name = make_ff_wrapper_name(attr(intern("name"), id));
  lv *ftype = node(intern("function"), nil, nil);
  lv *tl = 0;

  set_attr(intern("signature"), ftype, fd);
  set_attr(intern("ffi_wrapper_name"), attr(intern("signature"), ftype), ff_call_wrapper_name);

  dolist (f, attr(intern("formals"), fd))
    {
      push(arg1(f), tl);
    }
  tsilod;
  tl = nreverse(tl);
  push(attr(intern("return_type"), fd), tl);
  args(ftype) = tl;

  /* At this point the 'args(ftype)' will contain the list of types
   * formatted as follows:
   * (return_type . args_types)
   */

  define_id(id, global_ns, ftype, nil, nil);
}


int
ff_has_side_effects_p(lv* signature)
{
  lv* formals = attr(intern("formals"), signature);

  dolist (f, formals)
    {
      if (out_formal_p(f))
	return 1;
    }
  tsilod;
  return 0;
}


void
resolve_type(lv *type, lv *env)
{
  lv *s = op(type);

  if (s == intern("id"))
    {
      resolve_id(type, env);
      if (! type_type_p(attr(intern("type"), type)))
	{
	  user_error(type, "\"~a\" is not a type", attr(intern("id"), type));
	}
    }
  else if (s == intern("array") || s == intern("set"))
    {
      resolve_type(arg1(type), env);
    }
  else if (s == intern("number_type") || s == intern("logical_type") || s == intern("symbol_type"))
    {
      ; /* Do nothing */
    }
  else
    {
      internal_error("invalid type");
    }
}



/*************** SYNTAX CHANGE !!! - this one may be used/touched ***/
void
resolve_type_in_decl(lv *decl, lv *env)
{
  lv *id;
  assert(op(decl) == intern("declare"));
  resolve_type(arg1(decl), env);
}


void
check_type_use(lv *td)
{
  lv *env = attr(intern("env"), td);
  lv *parent = attr(intern("parent"), td);

  /* Check that parent type is defined. */
  if (parent)
    {
      resolve_id(parent, global_env);
    }

  /* Check that types which appear in declarations are defined. */
  mapcx(resolve_type_in_decl, attr(intern("input"), td), env);
  mapcx(resolve_type_in_decl, attr(intern("output"), td), env);
  mapcx(resolve_type_in_decl, attr(intern("state"), td), env);
  mapcx(resolve_type_in_decl, attr(intern("global"), td), env);

  /* Note that we do not resolve the type in the declaration of
   * event variables because this is already done at parsing time.
   * Basically this means that we do not have user defined event 
   * types (i.e. an event type IDENTIFIER will cause a syntax error.)
   *
   * Tunc Simsek 14th December, 1997
   */

  /******************************* SYNTAX CHANGE !!!! ******************/

#if 0
  mapcx(resolve_type_in_decl, attr(intern("define"), attr(intern("setup"), td)), env);
  dolist (mm, attr(intern("transition"), td))
    {
      mapcx(resolve_type_in_decl, attr(intern("define"), mm), env);
    }
  tsilod;
#endif
}


void
check_header_type_use(lv *fd)
{
  mapcx(resolve_type_in_decl, attr(intern("formals"), fd), global_env);
  resolve_type(attr(intern("return_type"), fd), global_env);

  /* Checking the limits of the FF Interface
   * (It is incomplete for the time being)
   *
   * Marco Antoniotti 19970428
   *
   * Restriction lifted.
   *
   * Marco Antoniotti 19970506
   */
  /*
  if (!number_type_p(attr(intern("return_type"), fd)))
    {
      user_error(find_leaf(attr(intern("id"), fd)),
		 "Foreign Function return types are limited to 'number' ('double') type.");
    }
    */
}


/* check_loop --- Definition.
 * 
 * Modified:
 * A new parameter is passed to check_loop: a temporary namespace.
 * This namespace keeps track of the nodes visited in the 
 * current chain.  Node marking can also be used for this purpose
 * but it is already being used to mark the checked chains to
 * optimize compilation time.
 *
 * Tunc Simsek 17th Sept., 1997
 *
 */
static void
check_loop(lv *type, lv *ancestor_id, lv *loop_ns)
{
  if (ancestor_id)
    {
      lv *ancestor_type = meaning(ancestor_id);
      lv *ancestor_name = attr(intern("name"), ancestor_id);

      if (! node_marked_p(ancestor_type))
	{
	  mark_node(ancestor_type);
	  /* We define this id node in the temporary name space of the
	   * current chain we are tracking.  Note that we do not
	   * specify any type, meaning or kind because the ancestor_id
	   * should have already had these attributes set during the
	   * set_env phase of the compiler.
	   */
	  define_id(ancestor_id, loop_ns,
		    nil,
		    nil,
		    nil); 
	  if (type == ancestor_type)
	    {
	      user_error(ancestor_id, "loop in parent chain of type \"~a\"",
			 ancestor_name);
	    }
	  else
	    {
	      check_loop(type, attr(intern("parent"), ancestor_type), loop_ns);
	    }
	} /* if (! node_marked ... ) ... */
      else
	{
	  if (ns_find(ancestor_name, loop_ns))
	      {
		user_error(ancestor_id, "loop in parent chain of type \"~a\"",
			   ancestor_name);
	      }
	} /* if (! node_marked ... ) ... else ... */	
    } /* if (ancestor_id) ... */
} /* static void check_loop */ 


/* check_hierarchy --- Definition.
 *
 * Description:
 * Check that there are no loops in inheritance chain.
 *
 * Log:
 * Modified the check_hierarchy and check_loop algo.
 * Previously, SHIC would go into an infinite loop if
 * there was a loop in the inheritance chain and a 
 * child was declared before the parent.
 * 
 * Tunc Simsek 17th Sept., 1997
 *
 */
void
check_hierarchy(lv *program)
{
    lv *n;
    new_node_marker();
    dolist (n, program) {
      lv *loop_ns = make_ns();

      if (op(n) == intern("typedef")) {
	check_loop(n, attr(intern("parent"), n), loop_ns);
	mark_node(n);
      }
    } tsilod;
}

/* check_inheritance --
 * Check that the inheritance rules are observed.
 */
#define VERBOSE_INHERITANCE
#ifndef VERBOSE_INHERITANCE
void
check_inheritance(lv *td)
{
  if (! node_marked_p(td))
    {
      lv *child_id = attr(intern("id"), td);
      lv *child_entity = attr(intern("entity"), child_id);
      lv *parent_id = attr(intern("parent"), td);
      lv *child_ns = attr(intern("export_ns"), td);

      mark_node(td);
      if (parent_id)
	{
	  lv *pid, *pdecl;
	  lv *parent = meaning(parent_id);

	  check_inheritance(parent);
	  /* Check that every exported event in the parent is also
	   * exported in the child.
	   */
	  dolist (pid, attr(intern("export"), parent))
	    {
	      lv *parent_event_name = attr(intern("name"), pid);
	      lv *entity = ns_find(parent_event_name, child_ns);

	      /* EXPM */
	      if (! entity || ! equal_type(attr(intern("type"), entity), event_type))
		{
		  user_error(child_id, "type \"~a\" is a subtype of \"~a\"\n\
but does not export event \"~a\"",
			     attr(intern("name"), child_id),
			     attr(intern("name"), parent_id),
			     attr(intern("name"), pid));
		}
	    }
	  tsilod;
	  /* Check that every output in the parent is also an output
	   * in the child.  Same for inputs.
	   */
	  dolist (pdecl, attr(intern("output"), parent))
	    {
	      lv *pid = attr(intern("id"), pdecl);
	      lv *pentity = attr(intern("entity"), pid);
	      lv *parent_output_name = attr(intern("name"), pid);
	      lv *entity = ns_find(parent_output_name, child_ns);

	      if (! entity
		  || ! (attr(intern("kind"), entity) == intern("OUTPUT")
			|| attr(intern("kind"), entity) == intern("error"))
		  || ! equal_type (attr(intern("type"), entity), attr(intern("type"), pentity)))
		{
		  user_error(child_id, "type \"~a\" is a subtype of \"~a\"\n\
but does not have output \"~a\" with the same type",
			     attr(intern("name"), child_id),
			     attr(intern("name"), parent_id),
			     attr(intern("name"), pid));
		}
	    }
	  tsilod;
	  dolist (pdecl, attr(intern("input"), parent))
	    {
	      lv *pid = attr(intern("id"), pdecl);
	      lv *pentity = attr(intern("entity"), pid);
	      lv *parent_input_name = attr(intern("name"), pid);
	      lv *entity = ns_find(parent_input_name, child_ns);

	      if (! entity
		  || ! (attr(intern("kind"), entity) == intern("INPUT")
			|| attr(intern("kind"), entity) == intern("error"))
		  || ! equal_type (attr(intern("type"), entity), attr(intern("type"), pentity)))
		{
		  user_error(child_id, "type \"~a\" is a subtype of \"~a\"\n\
but does not have input \"~a\" with the same type",
			     attr(intern("name"), child_id),
			     attr(intern("name"), parent_id),
			     attr(intern("name"), pid));
		}
	    }
	  tsilod;
	}
    }
}
#else  /* VERBOSE_INHERITANCE */

/* inherit_event_decl_list -- definition.
 * 
 * Description: This function works with lists
 * of intern("id") nodes.  The "child" inherits the event 
 * identifiers from the "parent".  The intern("id") node 
 * list is obtained by getting the value of 
 * the "symbol" attribute:
 * 
 * The "symbol" attribute is intended to be intern("export").
 *
 * Tunc Simsek  9th Sept., 1997
 *
 */
void
inherit_event_decl_list(lv *child,
			lv *parent,
			lv *symbol)
{
  lv *child_id = attr(intern("id"), child);
  lv *child_export_ns = attr(intern("export_ns"), child);
  lv *child_local_ns = first(attr(intern("env"), child));
  lv *pid; /* Variable used to loop through the intern("id") nodes
	    * in the list
	    */

  dolist (pid, attr(symbol, parent))
    {
      lv *pentity = attr(intern("entity"), pid);
      lv *pid_name = attr(intern("name"), pid);
      lv *entity = ns_find(pid_name, child_export_ns);
	
      /* Check to see if an entity was found.
       */
      if (! entity)
	{
	  lv *child_declaration_list;
	  lv *cid;
	  /* The entity is not found in the child's 
	   * name space so we have to inherit it...
	   */
	  /* First create a new instance, i.e. a new
	   * intern("id") node, as to create a personal copy
	   * of the inherited event
	   */
	  cid = node(intern("id"),
		     nil,
		     alist3(intern("name"), pid_name,
		            intern("file"), attr(intern("file"),pid),
		            intern("line"), attr(intern("line"),pid)));
	  /* Then update the namespaces of the child
	   */
	  define_id(cid, child_local_ns, attr(intern("type"), pentity),
		    nil,
		    nil);
	  define_id(cid, child_export_ns, nil,
		    nil,
		    nil);
	  
	  /* Now we have to also expand the identifier list
	   * of the child so as to mimic user code...
	   */
	  child_declaration_list = attr(symbol, child);
	  if (child_declaration_list)
	    {
	      push(cid, child_declaration_list);
	    }
	  else
	    {
	      child_declaration_list = list1(cid);
	    }
	  set_attr(symbol, child, child_declaration_list);

	} /* if (! entity) { ...  */
      else
	{
	  /* The basic concern here is if the user redeclares an
	   * event as a different type.  This would cause semantic
	   * errors which is basically why we do not allow depth
	   * redeclaration in state, input and output variables, 
	   * thus I will not allow depth subtyping here either.
	   *
	   * Tunc Simsek 14th December, 1997
	   */
	  lv *type = attr(intern("type"), entity);
	  lv *ptype = attr(intern("type"), pentity);

	  if (! equal_type(type, ptype))
	    {
	      user_error(child_id, "type \"~a\" is a subtype of \"~a\"\n\
but does not export event \"~a\" of type \"~a\".\n\
Event is redeclared as \"~a\".\n",
			 attr(intern("name"), child_id),
			 attr(intern("name"), attr(intern("parent"), child)),
			 attr(intern("name"), pid),
			 /* These next statements are not really
			  * generic, but it should hold out
			  * for a while.
			  * Tunc Simsek 14th December, 1997
			  */
			 equal_type(ptype, event_type) ?
			      intern("closed_event_type") : op(ptype),
			 equal_type(type, event_type) ?
			      intern("closed_event_type") : op(type));
	    } /* if (! equal_type ... */
	} /* if (! entity) { .. } else { ... */
    } tsilod; /* dolist (pid,...) { ... */
} /* void inherit_event_decl_list */


/* inherit_var_decl_list -- definition.
 * 
 * Description: This function works with lists
 * of intern("declaration") nodes.  The "child" inherits 
 * the declarations in the list from the "parent".  
 * The intern("declaration") node list is obtained by getting 
 * the value of the "symbol" attribute:
 * 
 * The "symbol" attribute is intended to be one of 
 * intern("state"), intern("input") or intern("output").
 *
 * Tunc Simsek  9th Sept., 1997
 *
 */
void
inherit_var_decl_list(lv *child, 
		      lv *parent,
		      lv *symbol)
{
  lv *child_id = attr(intern("id"), child);
  lv *child_export_ns = attr(intern("export_ns"), child);
  lv *child_local_ns = first(attr(intern("env"),child));
  lv *pdecl; /* Variable used to loop through the intern("declare")
	      * node in the list 
	      */
  
  dolist (pdecl, attr(symbol, parent))
    {
      lv *pid = attr(intern("id"), pdecl);
      lv *pentity = attr(intern("entity"), pid);
      lv *pid_name = attr(intern("name"), pid);
      lv *entity_local = ns_find(pid_name, first(attr(intern("env"), child)));
      lv *entity_export = ns_find(pid_name, child_export_ns);
      lv *entity;

      /* First we get the entity from one of the namespaces.
       */
      entity = entity_export ? entity_export : entity_local;
	
      /* Then we check to see if any entity was found at all.
       */
      if (! entity )
	{
	  lv *child_declaration_list;
	  lv *cdecl;
	  lv *cid;
	  
	  /* The entity is not found in the child namespace
	   * so we have to inherit it...
	   */
	  /* First create a new instance, i.e. a new
	   * intern("id") node and a new intern("declare") node, as to create 
	   * a personal copy of the inherited variable
	   * Note that the personal copy does not have a
	   * personal copy of the expression - this may 
	   * cause a problem!
	   */
	  cid = node(intern("id"),
	             nil,
                     alist3(intern("name"), pid_name,
		            intern("file"), attr(intern("file"),pid),
		            intern("line"), attr(intern("line"),pid)));
	  cdecl = node(intern("declare"),
	               args(pdecl),
	               alist1(intern("id"), cid));
	
	  /* Then update the namespaces of the child:
	   * Nore that the intern("state") variable are not defined
	   * in the "export namespaces".
	   */
	  if (symbol == intern("input") 
	      || symbol == intern("output"))
	    {
	      define_id(cid, child_local_ns, attr(intern("type"), pentity),
			nil,
			attr(intern("kind"), pentity));
	      define_id(cid, child_export_ns, nil,
			nil,
			nil);
	    }
	  else if (symbol == intern("state"))
	    {
	      define_id(cid, child_local_ns, attr(intern("type"), pentity),
			nil,
			attr(intern("kind"), pentity));
	    }
	  /* Now we have to also expand the declaration list
	   * of the child so as to mimic user code...
	   */
	  child_declaration_list = attr(symbol, child);
	  if (child_declaration_list)
	    {
	      push(cdecl, child_declaration_list);
	    }
	  else
	    {
	      child_declaration_list = list1(cdecl);
	    }
	  set_attr(symbol, child, child_declaration_list);
	}
      else /* if (! entity) { ... */
	{
	  /* An entity is found in the child namespace:
	   * we have to check if it is of the same kind
	   * and type, because "depth" subtyping is not
	   * allowed in SHIC - SHIC is meant to be a 
	   * "sound" compiler.
	   */

	  /* First we check if it is of the same intern("kind").
	   */
	  if (! (attr(intern("kind"), entity) == attr(intern("kind"), pentity) 
		 || attr(intern("kind"), entity) == intern("error")))      
	    { 
	      user_error(child_id, "type \"~a\" is a subtype of \"~a\"\n\
but \"~a\" is not a \"~s\"",
			 attr(intern("name"), child_id),
			 attr(intern("name"), attr(intern("parent"), child)),
			 attr(intern("name"), pid),
			 symbol);
	    }
	  /* Now we check the supertype rule
	   */
	  if (! super_type_p(attr(intern("type"), entity), 
			   attr(intern("type"), pentity)))
	    {
	      user_error(child_id, "type \"~a\" is a subtype of \"~a\"\n\
but ~s \"~a\" is not redeclared as a supertype", 
			 attr(intern("name"), child_id), 
			 attr(intern("name"), attr(intern("parent"), child)),
			 symbol,
			 attr(intern("name"), pid)); 
	    } /* if (! equal_type ... */
	} /* if (! entity) { ... } else { ... */
    } tsilod;      
} /* void inherit_var_decl_list */


/* check_inheritance -- definition.
 *
 * Description: This function recursively traverses 
 * the inheritance graph and calls the inheritance routines 
 * to perform the inheritance maneouvers.  It is important
 * that each node be visited exactly and only once.
 * For this nodes in the inheritance graph are marked.
 *
 * Tunc Simsek 9th Sept., 1997
 *
 */
void
check_inheritance(lv *child)
{
  if (! node_marked_p(child))
    {
      lv *parent_id = attr(intern("parent"), child);

      mark_node(child);
      if (parent_id)
	{
	  lv *parent = meaning(parent_id);

	  check_inheritance(parent);

	  /* Inheritance in SHIFT is an implementation 
	   * technique where intern("state"), intern("input"), intern("output")
	   * variables and intern("export") events are inherited
	   * from the parent (i.e. the supertype).
	   */
	  inherit_var_decl_list(child, parent, intern("output"));
	  inherit_var_decl_list(child, parent, intern("input"));

	  /* Modification:  It is decided that STATE
	   * variables need not be inherited or typechecked
	   * for the contravariance (depth subtyping) rule.
	   *
	   * Tunc Simsek 27th October, 1997
	   *	  
	   * inherit_var_decl_list(child, parent, intern("state"));
	   */

	  /* Inheritance of the intern("export") list is treated
	   * differently than the inheritance of intern("state"),
	   * intern("output") and intern("input") lists.  This is due to
	   * the fact that intern("export") lists is a list of
	   * intern("id") nodes where as the latter are a list
	   * of intern("declare") nodes.
	   */
	  inherit_event_decl_list(child, parent, intern("export"));
	}
    }
}
#endif /* VERBOSE_INHERITANCE */


void
check_legal_operators(lv *eq)
{
  lv *x = op(eq);

  if (x == intern("error")) return;
  if (x == intern("minel") || x == intern("maxel") || x == intern("exists"))
    {
      user_error(find_leaf(eq), "bad operator \"~a\" in flow equation", x);
    }
  mapc(check_legal_operators, args(eq));
}

/* This function checks whether expression 'e' contains
 * a create statement
 *
 * Tunc Simsek 4th November, 1997
 */
int
create_in_expression_p(lv *e)
{
  int p = 0;

  if (e == nil)
    return 0; /* No create was found in this branch */
  switch (e->type)
    {
    case L_CONS:
       p = create_in_expression_p(hd(e));  
       return create_in_expression_p(tl(e)) || p;
    case L_NODE:
      if (op(e) == intern("create"))
	return 1;
      else if (op(e) == intern("error"))
	return 0;
      else
	dolist(a, args(e))
	  {
	    p = p || create_in_expression_p(a);
	  }
      tsilod;
      return p;
    case L_STRING:
    case L_SYMBOL:
    default:
      /* These cases are not of concern to us (infact they should
       * not occur in an expression).
       */
      return 0;
    }
}

/* This function checks if an expression contains 
 * a create statement, and if it does then it issues
 * an error message. Note that 'e' should be an expression
 * node that contains the file and line information of 
 * the expression.
 *
 * Tunc Simsek 4th November, 1997
 */
void
check_uncountable_create(lv *e)
{
  if(create_in_expression_p(e))
    user_error(find_leaf(e),"Dynamic creation not allowed in expression.");
}

void
check_flow(lv *f, lv *env)
{
  args(f) = mapcarx(rewrite_equation, args(f), env);
  /* Create_statements should not be allowed in flows
   * because there will be uncountably many components
   * created.
   *
   * Tunc Simsek 4th November, 1997
   */
  dolist(a, args(f))
    {
      check_uncountable_create(a);
    }
  tsilod;
/*****
  mapc(check_legal_operators, args(f));
***** MAK: allow minel, etc in rhs of flow defs */
}

void
check_transition(lv *tr, lv *env, lv *export_ns)
{
  /* Check FROM and TO attributes of the transition.  The FROM
   * attribute must be constant-folded (set operations are
   * allowed) and an empty set is an error.
   */
  lv *from = rewrite_expression(attr(intern("from"), tr), env);
  lv *to = rewrite_expression(attr(intern("to"), tr), env);
  lv *t, *guard, *action_env;
  void check_transition_event_spec(lv* tr, lv* event, lv* env, lv* export_ns);

  t = type_of(from);
  if (op(t) != intern("error_type") &&
      op(t) != intern("state_type") &&
      (op(t) != intern("set") || op(arg1(t)) != intern("state_type")))
    {
      /* Can you have a transition from a state s to a set of 
       * states S'?  I don't think the syntax (parser) allows 
       * this.
       *
       * Tunc Simsek 15th December, 1997
       */
      user_error(find_leaf(from),
		 "not a discrete state or a set of discrete states");
    }
  else if (op(t) != intern("error_type"))
    {
      if (op(from) == intern("id") && attr(intern("entity"), from) != all_entity)
	{
	  from = node(intern("setcons"), list1(from), alist1(intern("type"), stateset_type));
	}
      else if (op(from) != intern("setcons"))
	{
	  lv *x = fold(from);
	  if (x == nil)
	    {
	      internal_error("cannot fold set of states");
	    }
	  else if (args(x) == nil)
	    {
	      user_error(find_leaf(from), "FROM set is the empty set");
	    }
	  from = x;
	}
    }
  set_attr(intern("from"), tr, from);

  if (! equal_type(type_of(to), state_type))
    {
      user_error(find_leaf(to), "not a discrete state");
    }
  set_attr(intern("to"), tr, to);

  dolist (e, attr(intern("events"), tr))
    {
      check_transition_event_spec(tr, e, env, export_ns);
    }
  tsilod;

  guard = attr(intern("guard"), tr);
  if (guard)
    {
      lv *nguard = rewrite_expression(guard, env);

      if (! equal_type(logical_type, type_of(nguard)))
	{
	  user_error(find_leaf(guard), "guard expression is not logical");
	}
      set_attr(intern("guard"), tr, nguard);
      /* Check that the guard expression does not contain any creates,
       * since the guard (just like the invariant) is evaluated
       * infinitely many times.
       * 
       * Tunc Simsek 4th November, 1997
       */       
      check_uncountable_create(guard);
    }
  action_env = attr(intern("action_env"), tr);
  dolist (d, attr(intern("define"), tr))
    {

      /****************** SYNTAX CHANGE !!!!!!!!! ***************************/
      if (op(d) != intern("declare")) continue;		/* error recovery */
      arg1(d) = rewrite_expression(arg1(d), action_env);
      if (tl(args(d)))
	{
	  arg2(d) = rewrite_expression(arg2(d), action_env);
	  check_assignment(attr(intern("id"), d), arg2(d));
	}

      /***************************************************************
	arg1(d) = rewrite_expression(arg1(d), action_env);
	arg2(d) = rewrite_expression(arg2(d), action_env);
	(void) type_of(arg1(d));
	(void) type_of(arg2(d));
	*****************************************************************/
    }
  tsilod;

  /******MAK: call rewrite_action_tr to discard setup-only!! *******/

  set_attr(intern("do"), tr, mapcarx(rewrite_action_tr, attr(intern("do"), tr), action_env));
}


void
check_transition_event_spec(lv* tr, lv* e, lv* env, lv* export_ns)
{
  lv *link;
  lv *event;
  lv *rule;
  lv *base_type_export_ns, *link_type, *base_type, *entity;

  if (op(e) == intern("external_event"))
    {
      /* First we rewrite the list2 of intern("id") nodes:
       *
       * i.e. T:alpha
       *
       * Tunc Simsek 15th December, 1997
       */
      link  = rewrite_expression(arg1(e), env);
      event = arg2(e);
      rule  = attr(intern("sync_type"), e);

      /* resolve_id(link, env); */
      link_type = attr(intern("type"), link);

      if (link_type == error_type) return;
      if (attr(intern("entity"), link_type) == error_entity) return;

      if (op(link_type) == intern("set"))
	base_type = arg1(link_type);
      else if (op(link_type) == intern("id"))
	base_type = link_type;
      else
	{
	  /* Basically, this means that under the current
	   * mechanism you cannot sync. with a set(set(T))
	   *
	   * I suppose that this should be allowed.
	   *
	   * Tunc Simsek 15th December, 1997
	   */
	  user_error(link, "\"~a\" must be a link or a set of links",
		     attr(intern("name"), link));
	  return;
	}
      base_type = meaning(base_type);
      if (base_type == nil) return;	/* undefined base type */
      base_type_export_ns = attr(intern("export_ns"), base_type);
      entity = ns_find(attr(intern("name"), event), base_type_export_ns);
      if (entity)
	{
	  set_attr(intern("entity"), event, entity);
	  /* This next line should have been here,
	   * I don't know how the whole thing worked
	   * so far without it. (Compare it to 
	   * rewrite_external_event which is 
	   * apparently not used).
	   *
	   * Tunc Simsek 15th December, 1997
	   */
	  set_attr(intern("type"), event, attr(intern("type"), entity));
	}
      else
	{
	  user_error(event,
		     "event \"~a\" is not exported by type \"~a\"",
		     attr(intern("name"), event),
		     attr(intern("name"), base_type));
	  return;
	}
      if (rule && op(link_type) != intern("set"))
	{
	  user_error(event, "\
set synchronization rule does not apply to single-valued links");
	}

      /* Begin Modification (Marco Antoniotti 19960528)
       *
       * Signal an error if the external event was of the form
       * S:a, with S a set variable and no sync_rule was
       * specified.
       */

      if (!rule && op(link_type) == intern("set"))
	{
	  user_error(event, "\
missing set synchronization rule for set valued link");
	}
      /* End Modification */
	     
      /* Fix type of id rule */
      if (rule && op(rule) == intern("id")) (void) type_of(rule);
    }
  else
    {
      /* Local event.  The meaning of the local event name is
       * all the transitions with this name.
       */
      /* resolve_id(e, env); */
      rewrite_expression(e, env);
      entity = attr(intern("entity"), e);
      /* Verify that the use of the entity matches its 
       * declaration.
       *
       * Tunc Simsek 19th Sept., 1997
       *
       */
      if (entity)
	{
	if (equal_type(attr(intern("type"), entity), event_type)
	    || equal_type(attr(intern("type"), entity), open_event_type))
	  {
	    push_attr(intern("meaning"), entity, tr);
	  }
	else
	  {
	    user_error(e, "Event is a \"~s\"", attr(intern("kind"), entity));
	  }
	} /* if (entity) ... */
    } /* else ... local event */
} /* void check_transition_event_spec */



/* Verify that use of entities matches their declaration, and rewrite
 * the program.
 *
 * Several things happen in this routine.  Type checks are done in
 * parallel with a transformation whose primary purpose is to change
 * CALL nodes into primitive operators.  For instance: a call to
 * function "+" on numbers becomes a node with operator @+.  We do
 * this to simplify type computation, optimizing transformations (if
 * we ever get to that ;-), and code generation.
 *
 * Rewriting and computing types cannot be separated because of
 * overloading (e.g., `+' means both set union and addition).  In
 * general we need to rewrite before we can compute the type, because
 * (by design decision) we only know the type rules in the rewritten
 * program.  (They are simple rules: the type depends solely on the
 * operator.)  But sometimes to rewrite an expression we must first
 * compute the type of its arguments.
 *
 * To further confuse the issues, previous phases have introduced the
 * intern("type_of") operator, which refers to the type of an arbitrary
 * expression (and, similarly, the intern("element_type_of")).  intern("type_of") is
 * used when a variable is declared without explicit type, which is
 * derived from its initializing expression or other context (e.g.,
 * the EXISTS construct).  In this phase the intern("type_of") nodes are
 * changed into fully-resolved types.
 */
void
check_typedef(lv *td)
{
  lv *env = attr(intern("env"), td);
  lv *export_ns;
  lv *setup = attr(intern("setup"), td);
  lv *discrete_ids = nil;

  /* For each intern("flow") node in the attr(intern("flow"),td) list, call the 
   * check_flow(f, env) function, where f is a intern("flow") node in
   * the above mentioned list.
   */
  mapcx(check_flow, attr(intern("flow"), td), env);

  dolist (s, attr(intern("discrete"), td))
    {
      lv *equations = attr(intern("equations"), s);
      lv *invariant = attr(intern("invariant"), s);
      set_attr(intern("equations"), s, mapcarx(rewrite_equation, equations, env));
      mapc(check_legal_operators, attr(intern("equations"), s));
      /* Check that there are no create statements in the
       * flows, otherwise uncountably many components will 
       * be created.
       *
       * Tunc Simsek 4th November, 1997
       */ 
      dolist(eq, attr(intern("equations"), s))
	{
	  check_uncountable_create(eq);
	}
      tsilod;
      if (invariant)
	{
	  invariant = rewrite_expression(invariant, env);
	  set_attr(intern("invariant"), s, invariant);
	  if (! equal_type(logical_type, type_of(invariant)))
	    {
	      user_error(find_leaf(invariant),
			 "invariant expression is not logical");
	    }
	  /* Check that there is no create statement in an invariant,
	   * since an invariant is by nature evaluated infinitely many
	   * times.
	   * Tunc Simsek 4th November, 1997
	   */
	  check_uncountable_create(invariant);
	}
    }
  tsilod;

  /****************** SYNTAX CHANGE !!!!!!!!!!!!!!!!!!****************/
  if (setup)
    {
      lv *setup_env = attr(intern("setup_env"), td);
#if 0
      dolist (d, attr(intern("define"), setup))
	{
	  if (op(d) != intern("assign")) continue; /* error recovery */
	  arg1(d) = rewrite_expression(arg1(d), setup_env);
	  arg2(d) = rewrite_expression(arg2(d), setup_env);
	  (void) type_of(arg1(d));
	  (void) type_of(arg2(d));
	} tsilod;
#endif /************ ^^^ old syntax *******/

      dolist (d, attr(intern("define"), setup))
	{
	  if (op(d) != intern("declare"))
	    continue; /* error recovery */

	  arg1(d) = rewrite_expression(arg1(d), setup_env);
	  if (tl(args(d)))
	    {
	      arg2(d) = rewrite_expression(arg2(d), setup_env);
	      check_assignment(attr(intern("id"), d), arg2(d));
	    }
	}
      tsilod;
      set_attr(intern("do"), setup, mapcarx(rewrite_action,
				   attr(intern("do"), setup),
				   setup_env));
      set_attr(intern("connections"), setup, mapcarx(rewrite_action,
					    attr(intern("connections"), setup),
					    setup_env));
      /* Check that the connecions which are akin to flow equals do
       * not contain any create's else infinitely many components
       * will be created.
       * 
       * Tunc Simsek 4th November, 1997
       */
      dolist(ca, attr(intern("connections"), setup))
	{
	  check_uncountable_create(ca);
	}
      tsilod;
    } /* end setup */

  dolist (d, attr(intern("output"), td))
    {
      if (tl(args(d)))
	{
	  arg2(d) = rewrite_expression(arg2(d), env);
	  check_assignment(attr(intern("id"), d), arg2(d));
	}
    }
  tsilod;
	
  dolist (d, attr(intern("input"), td))
    {
      if (tl(args(d)))
	{
	  arg2(d) = rewrite_expression(arg2(d), env);
	  check_assignment(attr(intern("id"), d), arg2(d));
	}
    }
  tsilod;
	
  dolist (d, attr(intern("state"), td))
    {
      if (tl(args(d)))
	{
	  arg2(d) = rewrite_expression(arg2(d), env);
	  check_assignment(attr(intern("id"), d), arg2(d));
	}
    }
  tsilod;
	
  dolist (d, attr(intern("global"), td))
    {
     glob_or_td=1;
      if (args(d)) {
       if (arg1(d) == number_type) {
	      user_error(find_leaf(attr(intern("id"),d)),
			 "global variable can not be continuous number");

       }
      }

      if (tl(args(d)))
	{
	  arg2(d) = rewrite_expression(arg2(d), env);
	  check_assignment(attr(intern("id"), d), arg2(d));
	}
     glob_or_td=0;
    }
  tsilod;
	
  /* Set the current meaning of `all' for the constant folder. */
  dolist (d, attr(intern("discrete"), td))
    {
      push(attr(intern("id"), d), discrete_ids);
    }
  tsilod;
  set_attr(intern("meaning"), all_entity,
	   node(intern("setcons"), discrete_ids, stateset_type));

  export_ns = attr(intern("export_ns"), td);
  dolist (tr, attr(intern("transition"), td))
    {
      check_transition(tr, env, export_ns);
    }
  tsilod;
}


/* Check use restrictions of link variables in external events.
 */
void
check_algebraic_event(lv *td)
{
  dolist (t, attr(intern("transition"), td))
    {
      dolist (e, attr(intern("events"), t))
	{
	  if (op(e) == intern("external_event") && attr2(intern("algebraic"), intern("entity"), arg1(e)))
	    {
	      user_error(arg1(e),
			 "variable in external event may not be algebraically defined.");
	    }
	}
      tsilod;
    }
  tsilod;
}


void
initialize_check()
{
  init_scanner();

  global_ns = make_ns();
  global_typedef_ns = make_ns();
  global_env = list2(global_typedef_ns, global_ns);

  /* Predefined types. */
#define constype(name) name ## _type = node(intern(#name "_type"), nil, nil)
  constype(error);
  /* Adding open_event_type 
   * Tunc Simsek 12th December, 1997
   */
  constype(open_event); 
  constype(event);
  constype(flow);
  constype(state);
  constype(symbol);
  constype(logical);
  constype(void);
  constype(integer);
  constype(mode);
#undef constype

  number_type = node(intern("number_type"), nil, alist1(intern("continuous"), intern("true")));
  discrete_number_type = node(intern("number_type"), nil, nil);
  component_type = node(intern("component"), nil, nil);
  nil_type = node(intern("nil"), nil, nil);

  /* Allocation for differential type (for runge kutta step). The 13
   * refers to the variable step requirements.  Fixed step requires
   * only 9.
   */
  differential_state_type = node(intern("vector"), list1(fixnum(13)), nil);

  flow_function_type = node(intern("function"), list3(void_type,
					     component_type,
					     integer_type),
                                       nil);

  /* Predefine the following types for some built in math functions.
   * Functions with more than 3 args will have to be defined using
   * rewriters. (Some of the functions alread are even if of single
   * arity.)
   *
   * Marco Antoniotti 19970115
   */

  simple_number_function_type
    = simple_number_function_type_1_arg
    = node(intern("function"), list2(number_type, number_type), nil);

  simple_number_function_type_2_args
    = node(intern("function"),
           list3(number_type,
	         number_type,
                 number_type),
            nil);

  simple_number_function_type_3_args
    = node(intern("function"),
           nconc(list3(number_type,
	               number_type,
		       number_type),
	               list1(number_type)),
                 nil);

  stateset_type = node(intern("set"), list1(state_type), nil);

  /* Built-in functions. */
#define predefine(name, f)						\
  push(intern(#name), predefined_ids);					\
  ns_define(intern(#name), global_ns,					\
            node(intern("entity"), nil,						\
                 alist2(intern("rewriter"), other((void *) rewrite_ ## f),	\
                        intern("operator"), intern(#name))))

  predefine(+, ptm);
  predefine(*, ptm);
  predefine(-, ptm);
  predefine(/, binmath);
  predefine(rem, binmath);
  predefine(mod, binmath);
  predefine(>, relational);
  predefine(>=, relational);
  predefine(<, relational);
  predefine(<=, relational);
  predefine(=, equality);
  predefine(dlink, equality);
  predefine(/=, equality);
  /* predefine("'", derivative); */
  predefine(in, in);
  predefine(and, logical);
  predefine(or, logical);
  predefine(not, logical);
  predefine(size, size);
  predefine(narrow, narrow);
  predefine(copy, copy);
  predefine(max, max);
  predefine(min, min);
  predefine(atan2, atan2);
  predefine(random, random);
  predefine(ln, ln);

  push(intern("'"), predefined_ids);
  ns_define(intern("'"), global_ns,
            node(intern("entity"), nil,
                 alist2(intern("rewriter"), other((void *) rewrite_derivative),
                        intern("operator"), intern("'"))));
#undef predefine


  /* Predefined external functions. */
#define predefine_simple(f)						\
  push(intern(#f), predefined_ids);					\
  define_id(node(intern("id"), nil, alist1(intern("name"), intern(#f))), global_ns, \
	    simple_number_function_type, nil, nil)

#define predefine_simple_1(f) predefine_simple(f)

#define predefine_simple_2(f)						\
  push(intern(#f), predefined_ids);					\
  define_id(node(intern("id"), nil, alist1(intern("name"), intern(#f))), global_ns, \
	      simple_number_function_type_2_args, nil, nil)

#define predefine_simple_3(f)						\
  push(intern(#f), predefined_ids);					\
  define_id(node(intern("id"), nil, alist1(intern("name"), intern(#f))), global_ns, \
	    simple_number_function_type_3_args, nil, nil)


  predefine_simple(exp);
  predefine_simple(log);  /* MAK  "ln" does not exist */
  predefine_simple(log10);
  predefine_simple_2(pow);
  predefine_simple(sin);
  predefine_simple(cos);
  predefine_simple(tan);
  predefine_simple(sqrt);
  predefine_simple(atan);
  predefine_simple(asin);
  predefine_simple(acos);
  predefine_simple(abs);
  predefine_simple(floor);
  predefine_simple(ceil);
  predefine_simple(frac);

  /* Note: 'rint' seems to be defined only in true UNIX systems.
   * 'rint' does not seem to be part of the ANSI/IEEE suite, so MS
   * does not care to implement it.
   * Since 'round' is implemented on top of 'rint' we must disable it
   * as well.
   *
   * Marco Antoniotti 19970826
   */
#ifndef OS_MSWINDOWS
  predefine_simple(rint);
  predefine_simple(round);
#endif

  predefine_simple(signum);

#undef predefine_simple

  /* Predefine other things. */
  error_expression = node(intern("error"), nil, alist1(intern("type"), error_type));
  args(error_expression) = list1(error_expression);
  error_entity = node(intern("error"), nil, alist1(intern("type"), error_type));

  define_id(identifier(intern("all")), global_ns, stateset_type, nil, nil);
  all_entity = ns_find(intern("all"), global_ns);

  define_id(identifier(intern("nil")), global_ns, nil_type, nil, nil);
  nil_entity = ns_find(intern("nil"), global_ns);

  define_id(identifier(intern("false")), global_ns, logical_type, nil, nil);
  false_entity = ns_find(intern("false"), global_ns);

  define_id(identifier(intern("true")), global_ns, logical_type, nil, nil);
  true_entity = ns_find(intern("true"), global_ns);

  null_set_type = node(intern("null_set"), nil, nil);
}


void
check()
{
  lv *decls = nil;

#if 0   /* done in collect_typedef_sections() */
  /* Concatenate declaration lists.
   */
  dolist (n, Program) {
    if (op(n) == intern("typedef"))
      {
	lv *states = nil;
	lv *outputs = nil;
	lv *inputs = nil;
	
	dolist (p, attrs(n)) {
	  if (hd(p) == intern("state"))
	    states = nconc(states, tl(p));
	  else if (hd(p) == intern("input"))
	    inputs = nconc(inputs, tl(p));
	  else if (hd(p) == intern("output"))
	    outputs = nconc(outputs, tl(p));
	} tsilod;
	set_attr(intern("state"), n, states);
	set_attr(intern("input"), n, inputs);
	set_attr(intern("output"), n, outputs);
      }
  } tsilod;
#endif

  /* Put all global declarations in the global list of the `global
   * component.'
   */
  global_type = node(intern("id"), nil, alist1(intern("name"), intern("global")));
  global_typedef = node(intern("typedef"), nil, alist1(intern("id"), global_type));
  dolist (n, Program)
    {
      if (op(n) == intern("glob_var_decl"))
	decls = nconc(decls, args(n));


      /* Adding the 'setup' clause to the global component.
       *
       * Note that the way this is done assumes that there is 
       * a single global 'setup' clause.  It will definitely
       * not work for SHIFT programs with more than one global
       * 'setup' clause.  But currently the syntax is such that
       * there can be only one 'setup' clause in any 'typedef'
       * so this is not a big concern.
       *
       * Tunc Simsek 19980527
       */
      if (op(n) == intern("setup"))
	set_attr(intern("setup"), global_typedef, n);
    }
  tsilod;

  set_attr(intern("global"), global_typedef, decls);
  
  push(global_typedef, Program);

  /* Define everything. */
  dolist (n, Program)
    {
      if (op(n) == intern("typedef"))
	set_env_for_typedef(n);
      else if (op(n) == intern("ext_fun_decl"))
	declare_external_function(n);
    } tsilod;

  /* Check use of component type names. */
  dolist (n, Program)
    {
      if (op(n) == intern("typedef"))
	check_type_use(n);
      else if (op(n) == intern("ext_fun_decl"))
	check_header_type_use(n);
    }
  tsilod;

  /* Detect inheritance loops. */
  check_hierarchy(Program);

  /* Check inheritance rules. */
  new_node_marker();
  dolist (n, Program)
    {
      if (op(n) == intern("typedef"))
	check_inheritance(n);
    }
  tsilod;

  /* Check all expressions. */
  dolist (n, Program)
    {
      if (op(n) == intern("typedef"))
	check_typedef(n);
    }
  tsilod;


#if 0
  /************************* MAK commenting out for SYNTAX CHANGE **/
  /* Check variables on edges. */
  dolist (n, Program) {
    if (op(n) == intern("typedef")) {
      check_algebraic_event(n);
    }
  } tsilod;
#endif
}





/***************************** unfolding the '<->' : 3 funcs below ********/

lv *
trav_def(char *thename, lv *thedef)
{
  lv *clv1, *clv2;
  if (thedef)
    dolist (tn, thedef)
      {
	if (!strcmp( str(attr(intern("name"), attr(intern("id"), tn))), thename))
	  {
	    clv1 = arg1(tn);
	    clv2 = attr(intern("name"), clv1);
	      if (clv2)
		{
		  return identifier(intern( str(clv2) ));
		}
	      else
		{
		  user_error(find_leaf(clv1), "not a component type");
		}
	  }
      }
  tsilod;
  return nil;
}


lv *
m_shift_tools_find_type(lv *vrn, lv *nprg)
{
  lv *lvl1; 
  if (lvl1 = trav_def( str(attr(intern("name"), vrn)), attr(intern("state"), nprg)))
    return lvl1; 

  if (lvl1 = trav_def( str(attr(intern("name"), vrn)), attr(intern("input"), nprg)))
    return lvl1; 

  if (lvl1 = trav_def( str(attr(intern("name"), vrn)), attr(intern("output"), nprg) ) )
    return lvl1; 

  return nil;
}


void
patch_sync()
{
  int numsy = 0;
  int varno = 0;
  lv *ssetup;
  lv *dopart;
  lv *l1, *l2, *l3, *l4, *l5;
  lv *lProg = Program;

  dolist (n, lProg)
    {
      if (op(n) == intern("typedef"))
	{
	  ssetup = attr(intern("setup"), n);
	  if (ssetup)
	    {
	      /* now it should be connections   dopart = attr(intern("do"), ssetup); */
	      dopart = attr(intern("connections"), ssetup);
	      dolist (sn, dopart)
		{
		  if (op(sn) == intern("sync") )
		    {
		      line = num(attr(intern("line"), find_leaf(sn)));
		      current_file = attr(intern("file"), find_leaf(sn));
		      numsy++;
		      push_attr(intern("define"), 
		                ssetup, 
		                node(intern("declare"), 
          		             cons(build_name(numsy, "Ityp"), 
			                  list1(node(intern("create"), 
				                     cons(build_name(numsy,
                                                                     "Ityp"),
                                                          nil), 
				                     nil))),
		                     alist1(intern("id"), build_name(numsy, "Itmp"))));

		      l3 = nil; 
		      l2 = nil;
		      varno = 0;
		      dolist (exv, args(sn))
			{
			  if (op(exv) == intern("external_event"))
			    {
			      l1 = m_shift_tools_find_type(arg1(exv), n);
			      if (l1)
				{
				  l3 = cons(node(intern("declare"),
				                 cons(l1, nil),
				                 alist1(intern("id"), build_name(varno,
                                                                        "lvr"))
                                                 ),
					    l3);
				  l5 = tl(args(exv));
				  if (l5)
				    {
				      l5 = identifier(intern(str(attr(intern("name"),
				                                      hd(l5)))
							     ));
				      l2 = cons(node(intern("external_event"),
				                     list2(build_name(varno,
								      "lvr"),
							   l5),
				                     alist1(intern("sync_type"), nil)),
  						l2);
				    }
				  else
				    {
				    user_error(find_leaf(exv),
					       "Event not specified");
				    }
				}
			      else
				{
				user_error(find_leaf(exv),
					   "Variable not defined");
				}

#if 0   /* do not uncomment!!!! may be used to return to ':=' instead of '<-'*/
			      push_attr(intern("do"), ssetup,
			      /* '<-' instead of ':='   node(intern("assign"), list2(node(intern("call"), *****/
			                node(intern("dlink"),
					     list2(node(intern("call"),
					                cons(build_name(varno,
									"lvr"),
							     cons(build_name(numsy, "Itmp"),
								  nil)),
					                nil),
						   identifier(intern(str(attr(intern("name"), arg1(exv)))))), nil));
#endif
			      push_attr(intern("connections"), ssetup,
			                node(intern("call"),
					     list3(identifier(intern("dlink")), 
						   node(intern("call"),
						        cons(build_name(varno,
									"lvr"),
							     cons(build_name(numsy, "Itmp"), nil) ),
						        nil),
						   identifier(intern(str(attr(intern("name"), arg1(exv))))) ), nil
				            ));

			      varno++;
			    }
			  else
			    {
			      user_error(find_leaf(exv),
					 "Not an external event");
			    }
			}
		      tsilod;

		      /* obliterate sync subtree */
		      op(sn) = intern("donothing");
		      /* create internal type and append it to the Program */
		      l4 = cons(cons(intern("transition"),
				     list1(node(intern("transition"),
				                nil,
				                acons(intern("from"), build_name(0, "loop"),
				acons(intern("to"), build_name(0, "loop"),
				acons(intern("events"), l2, nil)))))),
				cons(
				     cons(intern("discrete"),   
				     list1(node(intern("discrete"),
				     nil,
				     alist3(intern("id"),
				     build_name(0, "loop"),
				     intern("equations"), nil,
				     intern("invariant"), nil)))),
				     cons(cons(intern("input"), l3), nil)  )  /* l3 contains var list */
				);
		      Program = cons(node(intern("typedef"), 
		      nil, 
		      acons(intern("id"),
		      build_name(numsy, "Ityp"),
		      acons(intern("parent"), nil, l4))),
				     Program);
		    }
		}
	      tsilod;
	    }
	}
    }
 tsilod;
}


/***************** MAK  inserting a dummy state if none *********/
void 
insert_dummy_state()
{
  lv *discr;

  dolist (n, Program) {
    if (op(n) == intern("typedef")) {
      if (!(attr(intern("discrete"), n))) {
	       line = num(attr(intern("line"), find_leaf(attr(intern("id"), n))));
	       current_file = attr(intern("file"), find_leaf(attr(intern("id"), n)));
	       push_attr(intern("discrete"), 
			 n, 
		         node(intern("discrete"),
			      nil,
			      alist3(intern("id"), build_name(0, "loop"),
				     intern("equations"), nil,
				     intern("invariant"), nil)));
      }
    }
  } tsilod;
}

/**************** MAK concatenating clauses ******************/
void
collect_typedef_sections()
{
  lv *global_setups = nil;

  dolist (n, Program) {

    if (op(n) == intern("typedef"))
      {
	lv *states = nil;
	lv *outputs = nil;
	lv *inputs = nil;
	lv *transitions = nil;
	lv *discretes = nil;
	lv *exports = nil;
	lv *flows = nil;
	lv *setups = nil;
	lv *sdefs = nil;
	lv *sdos = nil;
	lv *sconn = nil;

	dolist (p, attrs(n)) {
	  if (hd(p) == intern("state")) {
	    states = nconc(states, tl(p));
	  } else if (hd(p) == intern("input")) {
	    inputs = nconc(inputs, tl(p));
	  } else if (hd(p) == intern("output")) {
	    outputs = nconc(outputs, tl(p));
	  } else if (hd(p) == intern("transition")) {
	    transitions = nconc(transitions, tl(p));
	  } else if (hd(p) == intern("export")) {
	    exports = nconc(exports, tl(p));
	  } else if (hd(p) == intern("discrete")) {
	    discretes = nconc(discretes, tl(p));
	  } else if (hd(p) == intern("flow")) {
	    flows = nconc(flows, tl(p));
	  } else if (hd(p) == intern("setup")) {
	    setups = nconc(setups, tl(p));
	  } 
	} tsilod;
	set_attr(intern("state"), n, states);
	set_attr(intern("input"), n, inputs);
	set_attr(intern("output"), n, outputs);
	set_attr(intern("transition"), n, transitions);
	set_attr(intern("export"), n, exports);
	set_attr(intern("discrete"), n, discretes);
	set_attr(intern("flow"), n, flows);
	set_attr(intern("setup"), n, setups);
	if (setups)
	  {
	    sdefs = nil;
	    sdos = nil;
	    sconn = nil;
	    dolist (ppn, setups) {
	      if (hd(ppn) == intern("define")) {
		sdefs = nconc(sdefs, tl(ppn));
	      } else if (hd(ppn) == intern("do")) {
		sdos = nconc(sdos, tl(ppn));
	      } else if (hd(ppn) == intern("connections")) {
		sconn = nconc(sconn, tl(ppn));
	      }
	    } tsilod;
	    set_attr(intern("define"), setups, sdefs);
	    set_attr(intern("do"), setups, sdos);
	    set_attr(intern("connections"), setups, sconn);
	    set_attr(intern("setup"), n, setups);
	  }
      }

    /* Adding support for the global setup
     * clauses which are organized as top level intern("setup")
     * nodes.
     */
    if (op(n) == intern("setup"))
      {

	global_setups = nconc(global_setups, list1(n));
      }	
  } tsilod;

  if (global_setups)
    {
      lv *sdefs = nil;
      lv *sdos = nil;
      lv *sconn = nil;
      lv *global_setup = node(op(first(global_setups)),
			      args(first(global_setups)),
			      attrs(first(global_setups)));

      dolist(s, global_setups)
	{	    
	  /* 1) Record the intern("define"), intern("do") and
	   *    intern("connectitons") field of the 
	   *    global setup clause.
	   * 
	   * 2) Remove the setup clause from 
	   *    the top-level list.
	   */
	  sdefs = nconc(sdefs, copy_list(attr(intern("define"), s)));
	  sdos = nconc(sdos, copy_list(attr(intern("do"), s)));
	  sconn = nconc(sconn, copy_list(attr(intern("connections"), s)));
	  
	  Program = del_el(s, Program);
	}
      tsilod;
      
      set_attr(intern("define"), global_setup, sdefs);
      set_attr(intern("do"), global_setup, sdos);
      set_attr(intern("connections"), global_setup, sconn);

      Program = nconc(Program, list1(global_setup));
    }
}

/* patch_differential_variables -- Definition.
 *
 * A function for the temporary relief of inheritance features.
 * The problem that is being fixed has to do with the accessing of 
 * a differentially defined variable 'x' in a child that is constantly
 * or algebraically defined in the parent.  
 * The temporary medicine is to let all 'continuous' variables be
 * differential.
 *
 * Tunc Simsek 6th November, 1997
 */
void
patch_differential_variables(lv *p)
{
  void _patch_differential_variables(lv*);
  /* We loop through the program 'p' and for each element in
   * this list we operate on intern("typedef") nodes.
   */
  dolist(td, p)
    {
      if (op(td) == intern("typedef"))
	_patch_differential_variables(td);
    } 
  tsilod;
} /* void patch_differential_variables */

void
_patch_differential_variables(lv *td)
{
  lv *output_var_list = attr(intern("output"), td);
  /* We loop through the variables of type 'td' and for each
   * variable in the three lists we patch the entities of the 
   * variables of type intern("number") to have an additional attribute 
   * (key, value) pair in the form of (intern("differential"), TRUE).
   */
  /* Note that we do not mess around with intern("input") variables,
   * this is because they are always intern("algebraic") 'ally defined.
   * Note also that we do not have to mess around with intern("state")
   * variables either since they are not externally accessible.
   */
  dolist(dcl, output_var_list)
    {
      lv *id = attr(intern("id"), dcl);
      lv *type = attr(intern("type"), id);
      lv *continuous = attr(intern("continuous"), type);

      if (continuous && continuous == intern("true"))
	{
  	  lv *entity = attr(intern("entity"), id);
          lv *differential = attr(intern("differential"), entity);
	  lv *algebraic = attr(intern("algebraic"), entity);

	  if (! differential ||  differential != intern("true"))
	    {
	      if(algebraic && algebraic == intern("true"))
		{
		  user_warning(find_leaf(id),
			       "Overriding attribute \"~s\" of variable \"~a\" as \"~s\"",
			       intern("algebraic"),
			       attr(intern("name"), id),
			       intern("differential"));
		}
	      else
		{
		  user_warning(find_leaf(id),
			       "Declaring variable \"~a\" as \"~s\"",
			       attr(intern("name"), id),
			       intern("differential"));
		}
	      set_attr(intern("differential"), entity, intern("true"));
	    }
	}
    }
  tsilod;
} /* void _patch_differential_variables */
#endif /* COMPILER_CHECK_I */

/* end of file -- check.c -- */
