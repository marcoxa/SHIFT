/* -*- Mode: C -*- */

/* generate.c -- */

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


#ifndef COMPILER_GEN_I
#define COMPILER_GEN_I

#include <shift_config.h>

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include "lisp.h"
#include "shic.h"
#include "utilities.h"
#include "gen-iterators.h"
#include "gen-special-forms.h"
#include "ffi-generators.h"

lv *self_variable, *global_variable, *new_variable;
lv *component_set_type;
/* lv *zero;
 * Useless
 * Marco Antoniotti 19971023
 */
lv* new_component_set;

/* LHS -- Are we generating a left-hand side? */
int LHS = 0;


/* Construct a variable with type TYPE and name NAME.  intern("VARIABLE") nodes
 * are akin to intern("ENTITY") nodes, but they are used for target variables.
 * They should probably go away.
 */
lv *
make_variable(lv *type, lv *name, lv *kind)
{
  if (! type) internal_error("make_variable: missing type");
  return node(intern("variable"), nil, alist3(intern("type"), type, intern("name"), name, intern("kind"), kind));
}


/* Compute the variables that implement ENTITY.  NAME is passed for
 * debugging convenience.
 *
 * Most of the attributes of ENTITY are also attached to the main
 * target variable.  This is exclusively for convenience in the
 * emission of debugging information.
 */
lv *
implementation_variables(lv *entity, lv *name)
{
  lv *type = attr(intern("type"), entity);
  lv *kind = attr(intern("kind"), entity);
  lv *differential = attr(intern("differential"), entity);

#ifdef no_global_analysis
  /* Input variables are potentially (without global analysis)
   * algebraically defined.
   */
  lv *algebraic = attr(intern("algebraic"), entity) || kind == intern("INPUT") ? intern("true") : 0;
  /* The above code is not sane anyhow, but Misha wants me to save it 
   * here. 
   */
#else
  lv *algebraic = attr(intern("algebraic"), entity);
#endif

  lv *variables = nil;

  /* The main target variable.  Everybody has one. */
  lv *target_variable = make_variable(type, name, kind);

  /* mode, flow, context, differential_state, differential_temporary.
   * Auxiliary variables for algebraic and differential variables.
   */
  lv *mode_v, *flow_function_v, *context_v;
  char *mode_v_name, *flow_function_v_name, *context_v_name;

  lv *differential_state_v, *differential_temporary_v;
  char *differential_state_v_name, *differential_temporary_v_name;

  char *_v_name_scratch = 0;	/* Reusable scratch variable */

  if (kind == intern("INPUT"))
    {
      algebraic = intern("true");
      set_attr(intern("algebraic"), entity, intern("true"));
    }

  set_attr(intern("target_variable"), entity, target_variable);
  set_attr(intern("algebraic"), target_variable, algebraic);
  set_attr(intern("differential"), target_variable, differential);

  /* Luigi, Marco, Michael 19960614
   * Added the necessary lines to generate the proper 'context'
   * information. (Check lines involving 'context' here below.)
   */
  if (algebraic || differential)
    {

      _v_name_scratch = (char*)safe_malloc((strlen(pname(name)) + 10)
					   * sizeof(char));

      sprintf(_v_name_scratch, "M_V_%s", pname(name));
      mode_v = make_variable(mode_type,
			     intern(_v_name_scratch),
			     intern("MODE"));
      set_attr(intern("mode"), entity, mode_v);
      set_attr(intern("mode"), target_variable, mode_v);
      push(mode_v, variables);

      sprintf(_v_name_scratch, "FF_V_%s", pname(name));
      flow_function_v = make_variable(flow_function_type,
				      intern(_v_name_scratch),
				      intern("FLOW_FUNCTION"));
      set_attr(intern("flow_function"), entity, flow_function_v);
      set_attr(intern("flow_function"), target_variable, flow_function_v);
      push(flow_function_v, variables);

      sprintf(_v_name_scratch, "C_V_%s", pname(name));
      context_v = make_variable(component_type,
				intern(_v_name_scratch),
				intern("CONTEXT"));
      set_attr(intern("context"), entity, context_v);
      set_attr(intern("context"), target_variable, context_v);
      push(context_v, variables);
    }

  if (differential)
    {
      _v_name_scratch = (char*)safe_malloc((strlen(pname(name)) + 10)
					   * sizeof(char));

      differential_state_v =
	make_variable(differential_state_type, name, intern("DIFFERENTIAL"));
      set_attr(intern("state"), entity, differential_state_v);
      set_attr(intern("state"), target_variable, differential_state_v);
      push(differential_state_v, variables);

      sprintf(_v_name_scratch, "D_T_%s", pname(name));
      differential_temporary_v = make_variable(number_type,
					       intern(_v_name_scratch),
					       intern("TEMPORARY"));
      set_attr(intern("temporary"), entity, differential_temporary_v);
      push(differential_temporary_v, variables);
    }

  push(target_variable, variables);
  free(_v_name_scratch);
  return variables;
}


lv *
make_id(lv *variable)
{
  lv *name = attr(intern("name"), variable);
  lv *type = attr(intern("type"), variable);
  lv *a = alist2(intern("type"), type, intern("target_variable"), variable);
  lv *entity = node(intern("entity"), nil, a);

  if (! name) name = intern("t");
  return node(intern("id"), nil, alist3(intern("entity"), entity, intern("type"), type, intern("name"), name));
}


/* Allocate variables to implement quantifiers in EXPR.  Put them in
 * *LISTP.
 */
void
push_auxiliary_variables(lv *expr, lv **listp)
{
  lv* x = op(expr);

  void push_sf_auxiliary_vars(lv* sf_expr,
			      lv* iter_forms,
			      lv* cond,
			      lv** listp);

  if (x == intern("exists") || x == intern("minel") || x == intern("maxel"))
    {
      lv* id = attr(intern("id"), expr);
      lv* y  = make_variable(attr(intern("type"), id), attr(intern("name"), id), intern("TEMPORARY"));

      set_attr(intern("target_variable"), attr(intern("entity"), id), y);
      push(y, *listp);

      dolist (a, args(expr))
	{
	  push_auxiliary_variables(a, listp);
	}
      tsilod;
    }
  else if (x == intern("setcons2") || x == intern("arraycons2"))
    {
      push_sf_auxiliary_vars(arg1(expr), arg2(expr), arg3(expr), listp);
    }
  else if (x == intern("special_form"))
    {
      lv* sf_spec = arg1(expr);

      push_sf_auxiliary_vars(arg1(sf_spec),
			     arg2(sf_spec),
			     arg3(sf_spec),
			     listp);
    }
  else
    {
      dolist (a, args(expr))
	{
	  push_auxiliary_variables(a, listp);
	}
      tsilod;
    }
}


void
push_sf_auxiliary_vars(lv* sf_expr, lv* iter_forms, lv* sf_cond, lv** listp)
{
  lv* iter_var;
  lv* shift_iter_var;

  dolist (in_expr, iter_forms)
    {
      iter_var = arg1(in_expr);

      push_auxiliary_variables(arg2(in_expr), listp);

      shift_iter_var = make_variable(attr(intern("type"), iter_var),
				     attr(intern("name"), iter_var),
				     intern("TEMPORARY"));
      set_attr(intern("target_variable"), attr(intern("entity"), iter_var), shift_iter_var);
      push(shift_iter_var, *listp);
    }
  tsilod;

  push_auxiliary_variables(sf_expr, listp);
  push_auxiliary_variables(sf_cond, listp);
}


/* push_action_variables --
 * Allocate variable for temporary values in ACTION and put it in
 * *LISTP.  Also store variable as intern("temp") attribute in various parts of
 * ACTION.
 *
 * Note:
 * Changed to create meaningful names for extra variables.  However,
 * the scheme is not coarse enough.  The names now encode at least the
 * type context (which is quite an improvement and which seems to save
 * the day when inheritance is in the picture).  However, it would be
 * better to actually tag the statement with a unique number and
 * generate the variable name from it.
 * Marco Antoniotti 19970904
 *
 * Changed to handle op assignments.
 * Marco Antoniotti 19970601
 */
void
push_action_variables(lv *td, lv *a, lv **listp)
{
  lv *x = op(a);
  lv *v = (lv *) 0;
  lv* rhs;
  lv* lhs;
  lv *ext_function_call_out_vars = nil;
  lv* temp_var_type;
  char *_v_temp_scratch = 0;	/* Reusable scratch */

  void push_formal_out_vars(lv* rhs_expr, lv** listp);


  _v_temp_scratch =
    (char*)safe_malloc((strlen(pname(attr2(intern("name"), intern("id"), td))) + 20)
		       * sizeof(char));

  if (x == intern("create"))
    {
      sprintf(_v_temp_scratch, "ACT_CREATE_%s", pname(attr2(intern("name"), intern("id"), td)));
      v = make_variable(arg1(a), intern(_v_temp_scratch), intern("TEMPORARY"));
    }
  else if (x == intern("assign") || x == intern("opassign"))
    {
      lhs = arg1(a);
      rhs = arg2(a);

      /* The type of the temporary variable needs to be the one of the
       * RHS.  The generation of the assignment code will decide what
       * to do with op assignments.
       * However, there are "constants" which might not really have an
       * associated type yet.  These cases must be treated specially.
       *
       * Marco Antoniotti 19970612
       */
      /* Original code:
       * v = make_variable(attr(intern("type"), rhs), intern("t"), intern("TEMPORARY"));
       */

      temp_var_type = type_of(rhs);
      if (op(temp_var_type) == intern("nil")
	  || op(temp_var_type) == intern("empty_array")
	  || op(temp_var_type) == intern("null_set"))
	{
	  temp_var_type = type_of(lhs);
	}

      sprintf(_v_temp_scratch, "ACT_T_%s", pname(attr2(intern("name"), intern("id"), td)));
      v = make_variable(temp_var_type, intern(_v_temp_scratch) , intern("TEMPORARY"));
      if (op(lhs) != intern("id"))
	{
	  /* Need additional temporary for accessor destination */

	  /* I do not think that this is needed at all.  Why it is
	   * here is a mistery to me.
	   *
	   * Marco Antoniotti 19970220
	   */
/* Joel VanderWerf 19991109 */
#if 0
	  lv *c = arg1(lhs);
	  lv *v;

	  sprintf(_v_temp_scratch, "ACT_C_%s", attr2(intern("name"), intern("id"), td));
	  v = make_variable(attr(intern("type"), c),
			    intern(_v_temp_scratch),
			    intern("TEMPORARY"));
	  set_attr(intern("temp"), lhs, make_id(v));
	  push(v, *listp);
#endif
	}

      /* Now push all the necessary extra variables for the foreign
       * function calls embedded in the RHS.
       * Save the extra list of variables (which may turn out to be
       * empty) in the action expression for later use in the generation.
       *
       * Marco Antoniotti 19970423
       */
      /*
	push_formal_out_vars(rhs, &ext_function_call_out_vars);
	set_attr(intern("ffi_out_variables"), a, copy_list(ext_function_call_out_vars));
	*listp = nconc(ext_function_call_out_vars, *listp);
	*/
      /* print(ext_function_call_out_vars); */
    }
  else
    {
      free(_v_temp_scratch);
      return;
    }

  if (v != (lv *) 0)		/* Being paranoid */
    {
      set_attr(intern("temp"), a, make_id(v));
      push(v, *listp);
    }
  free(_v_temp_scratch);
}


/* push_formal_out_vars -- This function collects all the "extra"
 * variables for the expression (a RHS one) and tags the expression
 * it with them.
 * Later on, this list can be used to produce the necessary glue code
 * for the FF Calls.
 */
void
push_formal_out_vars(lv* rhs_expr, lv** listp)
{
  void push_formal_out_vars_traversing(lv* rhs_expr, lv** listp);

  push_formal_out_vars_traversing(rhs_expr, listp);
  if (*list != nil) set_attr(intern("ffi_out_variables"), rhs_expr, copy_list(*listp));
}


void
push_formal_out_vars_traversing(lv* rhs_expr, lv** listp)
{
  lv* rhs_args;
  lv* signature;
  lv* formals;
  lv* actuals;
  lv* v;
  lv* rhs_op;

  if (rhs_expr != 0)
    {
      rhs_op = op(rhs_expr);

      if (rhs_op == intern("call"))
	{
	  signature = attr(intern("signature"), rhs_expr);
	  if (signature != nil)	/* We have a Foreign Function */
	    {
	      for (formals = attr(intern("formals"), signature),
		   actuals = tl(args(rhs_expr));
		   formals != 0 && actuals != 0;
		   formals = rest(formals), actuals = rest(actuals))
		{
		  /* We know by now that the number of actuals matches
		   * that of formals.
		   */
		  if (out_formal_p(first(formals)))
		    {
		      char* t_name = (char *)malloc(80 * sizeof(char));

		      sprintf(t_name, "__%s__out_temp",
			      pname(attr2(intern("name"), intern("id"), first(formals))));

		      v = make_variable(arg1(first(formals)),
					intern(t_name),
					intern("TEMPORARY"));
		      set_attr(intern("temp"), first(actuals), make_id(v));
		      push(v, *listp);
		    }
		  else if (in_formal_p(first(formals))
			   && array_type_p(decl_type(first(formals))))
		    {
		      char* t_name = (char *)malloc(80 * sizeof(char));

		      sprintf(t_name, "__%s__in_temp",
			      pname(attr2(intern("name"), intern("id"), first(formals))));

		      v = make_variable(arg1(first(formals)),
					intern(t_name),
					intern("TEMPORARY"));
		      set_attr(intern("temp"), first(actuals), make_id(v));
		      push(v, *listp);
		    }
		}
	    }
	}
      /* Other special nodes go here. E.g. intern("setcons") and friends */
      else if (rhs_op == intern("setcons") || rhs_op == intern("arraycons"))
	{
	  internal_error("'push_formal_out_vars' missing for set and array cons.");
	}
      else if (rhs_op == intern("setcons2") || rhs_op == intern("arraycons2"))
	{
	  internal_error("'push_formal_out_vars' missing for set and array formers.");
	}
      else if (rhs_op == intern("exists") || rhs_op == intern("minel") || rhs_op == intern("maxel"))
	{
	  internal_error("'push_formal_out_vars' missing for exists-like queries.");
	}
      else if (rhs_op == intern("special_form"))
	{
	  internal_error("'push_formal_out_vars' missing for 'special forms'.");
	}
      else
	{
	  rhs_args = args(rhs_expr);
	  dolist (rhs_arg, rhs_args)
	    {
	      push_formal_out_vars_traversing(rhs_arg, listp);
	    }
	  tsilod;
	}
    }
}


/* compute_member_lists -- 
 * Compute public and private member lists for type TD.
 *
 * The offset of public members shared with the supertype must be the
 * same, so compute the lists in topological order (parent first).
 *
 * The algebraic/differential attributes of a variable must be the
 * same in parent and child (and therefore recursively in the entire
 * inheritance chain).  So if any component type in that chain uses
 * that variable in differential (algebraic) mode, we must treat all
 * component types in the chain as if they also used it that way.
 */
#define VERBOSE_INHERITANCE
#ifdef VERBOSE_INHERITANCE
void
compute_member_lists(lv *td)
{
  lv *mtvr;
  lv *pid;
  lv *parent;
  lv *parent_ns;
  lv *local_ns;
  int differential = 0;
  lv *setup;
	
  lv *private_list = nil;
  lv *public_list = nil;

  if (node_marked_p(td)) return;

  pid = attr(intern("parent"), td);
  parent = pid ? meaning(pid) : nil;
  parent_ns = parent ? attr(intern("export_ns"), parent) : nil;
  local_ns = first(attr(intern("env"), td));
  setup = attr(intern("setup"), td);

  if (parent)
    {
      compute_member_lists(parent);
    }
  mark_node(td);

  /* PUBLIC LIST.  Do parent variables first, so the order is
   * preserved.
   */
  if (parent)
    {
      dolist (decl, attr(intern("input"), parent)) {
	lv *id = attr(intern("id"), decl);
	lv *name = attr(intern("name"), id);
	lv *entity = ns_find(name, local_ns);
	public_list = nconc(implementation_variables(entity, name),
			    public_list);
      } tsilod;

      dolist (decl, attr(intern("output"), parent)) {
	lv *id = attr(intern("id"), decl);
	lv *name = attr(intern("name"), id);
	lv *entity = ns_find(name, local_ns);
	public_list = nconc(implementation_variables(entity, name),
			    public_list);
      } tsilod;

      dolist (id, attr(intern("export"), parent)) {
	lv *name = attr(intern("name"), id);
	char *sync_set_pname = (char *)malloc(strlen(pname(name)) + 10);
	lv *sync_set_name;
	lv *entity = ns_find(name, local_ns);
	lv *ss;

	if (sync_set_pname == NULL)
	  {
	    user_error(id, "malloc could not allocate memory");
	    exit(1);
	  }
	sprintf(sync_set_pname, "sync_set_%s", pname(name));
	sync_set_name = intern(sync_set_pname);
	ss = make_variable(component_set_type, sync_set_name, intern("BP_SET"));
	set_attr(intern("sync_set"), entity, ss);
	push(ss, public_list);
      } tsilod;
    }

  /* Now do the child. */
  dolist (decl, attr(intern("input"), td))
    {
      lv *id = attr(intern("id"), decl);
      lv *name = attr(intern("name"), id);
      lv *entity = attr(intern("entity"), id);

      if (! attr(intern("target_variable"), entity))
	{
	  public_list = nconc(implementation_variables(entity, name),
			      public_list);
	}
    }
  tsilod;
  dolist (decl, attr(intern("output"), td))
    {
      lv *id = attr(intern("id"), decl);
      lv *name = attr(intern("name"), id);
      lv *entity = attr(intern("entity"), id);

      if (! attr(intern("target_variable"), entity))
	{
	  public_list = nconc(implementation_variables(entity, name),
			      public_list);
	}
    }
  tsilod;

  /* Every exported event generates one local variable, which
   * holds the set of all components that synchronize with the
   * event.
   *
   * This variable is set from outside the component.  With
   * separate compilation, the event-variable mapping
   * information must still be globally available.
   */
  dolist (id, attr(intern("export"), td))
    {
      lv *entity = attr(intern("entity"), id);
      if (! attr(intern("sync_set"), entity))
	{
	  lv *name = attr(intern("name"), id);
	  char *sync_set_pname = (char *)malloc(strlen(pname(name)) + 10);
	  lv *sync_set_name;
	  lv *ss;

	  if (sync_set_pname == NULL)
	    {
	      user_error(id, "malloc could not allocate memory");
	      exit(1);
	    }
	  sprintf(sync_set_pname, "sync_set_%s", pname(name));
	  sync_set_name = intern(sync_set_pname);
	  ss = make_variable(component_set_type, sync_set_name, intern("BP_SET"));
	  set_attr(intern("sync_set"), entity, ss);
	  push(ss, public_list);
	}
    }
  tsilod;

  set_attr(intern("public_list"), td, nreverse(public_list));

  /* PRIVATE LIST
   */
  dolist (decl, attr(intern("state"), td))
    {
      lv *id = attr(intern("id"), decl);
      lv *name = attr(intern("name"), id);
      lv *entity = attr(intern("entity"), id);

      private_list = nconc(implementation_variables(entity, name),
			   private_list);
    }
  tsilod;

  dolist (decl, attr(intern("global"), td))
    {
      lv *id = attr(intern("id"), decl);
      lv *name = attr(intern("name"), id);
      lv *entity = attr(intern("entity"), id);

      private_list = nconc(implementation_variables(entity, name),
			   private_list);
    }
  tsilod;

  /* Allocate additional variables for 'exists', 'minel', etc. */
  do_all_expressions(e, td, push_auxiliary_variables(e, &private_list));

  /* Add temporaries for actions and set sync clauses. */

  /* SETUP */
  if (setup != 0)
    {
      /* define clause */
      dolist (define_act, attr(intern("define"), setup))
	{
	  lv *t = arg1(define_act);
	  lv *v = make_variable(t,
				attr2(intern("name"), intern("id"), define_act),
				intern("TEMPORARY"));

	  set_attr(intern("target_variable"), attr(intern("entity"), attr(intern("id"), define_act)), v);
	  push(v, private_list);
	}
      tsilod;

      /* do clause */
      dolist (a, attr(intern("do"), setup))
	{
	  push_action_variables(td, a, &private_list);
	}
      tsilod;

      /* connect clause */
      dolist (a, attr(intern("connections"), setup))
	{
	  push_action_variables(td, a, &private_list);
	}
      tsilod;
    }

  /* TRANSITIONS */
  dolist (tr, attr(intern("transition"), td))
    {
      /* sync vars for events */
      dolist (e, attr(intern("events"), tr))
	{
	  lv *sync_id;

	  if (op(e) == intern("external_event")
	      && (sync_id = attr(intern("sync_type"), e))
	      && op(sync_id) == intern("id"))
	    {
	      lv *v = make_variable(attr(intern("type"), sync_id),
				    attr(intern("name"), sync_id),
				    intern("SYNC_VAR"));
	      set_attr(intern("target_variable"), attr(intern("entity"), sync_id), v);
	      push(v, private_list);
	    }
	}
      tsilod;

      /* define clause */
      dolist (a, attr(intern("define"), tr))
	{
	  lv *t = arg1(a);
	  lv *v = make_variable(t, attr2(intern("name"), intern("id"), a), intern("LOCAL"));

	  set_attr(intern("target_variable"), attr2(intern("entity"), intern("id"), a), v);
	  push(v, private_list);
	}
      tsilod;

      /* do clause */
      dolist (a, attr(intern("do"), tr))
	{
	  push_action_variables(td, a, &private_list);
	}
      tsilod;

      /* connect clause */
      dolist (a, attr(intern("connections"), tr))
	{
	  push_action_variables(td, a, &private_list);
	}
      tsilod;
    }
  tsilod;

  set_attr(intern("private_list"), td, private_list);
}
#else  /* VERBOSE_INHERITANCE */
void
compute_member_lists(lv *td)
{

  lv* private_list = nil;
  lv* public_list  = nil;
  void compute_member_lists_1(lv *, lv**, lv**);
  
  compute_member_lists_1(td, &private_list, &public_list);
  set_attr(intern("private_list"), td, private_list);
  set_attr(intern("public_list"), td, private_list);
}


void
compute_member_lists_1(lv *td, lv** private_list, lv** public_list)
{
  lv *mtvr;
  lv *pid;
  lv *parent;
  lv *parent_exported_evs_ns;
  lv *local_ns;
  int differential_p = 0;
  lv *setup;
	
  
  void cml_process_local_variable_decl(lv*, lv*, lv**);
  void cml_process_setup_clause(lv*, lv*, lv**, lv**);
  void cml_process_transition_definitions(lv*, lv*, lv**, lv**);

  if (node_marked_p(td))
    {
      *private_list = attr(intern("private_list"), td);
      *public_list = attr(intern("public_list"), td);
      return;
    }

  pid = attr(intern("parent"), td);
  parent = pid ? meaning(pid) : nil;
  setup = attr(intern("setup"), td);

  mark_node(td);

  if (parent != nil)
    {
      if (!node_marked_p(parent))
	compute_member_lists(parent);

      *private_list = attr(intern("private_list"), parent);
      *public_list = attr(intern("public_list"), parent);
    }

  /* Process INPUT variables */      
  cml_process_local_variable_decl(td,
			      attr(intern("input"), td),
			      public_list);

  /* Process OUTPUT variable */
  cml_process_local_variable_decl(td,
			      attr(intern("output"), td),
			      public_list);

  /* Process STATE variable */
  cml_process_local_variable_decl(td,
			      attr(intern("state"), td),
			      private_list);

  
  /* Process GLOBAL variable */
  cml_process_local_variable_decl(td,
			      attr(intern("global"), td),
			      private_list);

  /* Process the EXPORTED events */
  /* Every exported event generates one local variable, which
   * holds the set of all components that synchronize with the
   * event.
   *
   * This variable is set from outside the component.  With
   * separate compilation, the event-variable mapping
   * information must still be globally available.
   */
  dolist (id, attr(intern("export"), td))
    {
      lv *entity = attr(intern("entity"), id);
      if (! attr(intern("sync_set"), entity))
	{
	  lv *name = attr(intern("name"), id);
	  char *sync_set_pname = (char *)malloc(strlen(pname(name)) + 10);
	  lv *sync_set_name;
	  lv *ss;

	  if (sync_set_pname == NULL)
	    {
	      user_error(id, "malloc could not allocate memory");
	      exit(1);
	    }
	  sprintf(sync_set_pname, "sync_set_%s", pname(name));
	  sync_set_name = intern(sync_set_pname);
	  ss = make_variable(component_set_type, sync_set_name, intern("BP_SET"));
	  set_attr(intern("sync_set"), entity, ss);
	  push(ss, *public_list);
	}
    }
  tsilod;

  /* Allocate additional variables for 'exists', 'minel', etc. */
  do_all_expressions(e, td, push_auxiliary_variables(e, private_list));

  /* Process SETUP clause */
  cml_process_setup_clause(td, setup, private_list, public_list);

  
  /* Process TRANSITION definitions */
  cml_process_transition_definitions(td, setup, private_list, public_list);
}


void
cml_process_transition_definitions(lv *td, 
				   lv *setup, 
				   lv *private_list, 
				   lv *public_list)
{
  dolist (tr, attr(intern("transition"), td))
    {
      /* sync vars for events */
      dolist (e, attr(intern("events"), tr))
	{
	  lv *sync_id;

	  if (op(e) == intern("external_event")
	      && (sync_id = attr(intern("sync_type"), e))
	      && op(sync_id) == intern("id"))
	    {
	      lv *v = make_variable(attr(intern("type"), sync_id),
				    attr(intern("name"), sync_id),
				    intern("SYNC_VAR"));
	      set_attr(intern("target_variable"), attr(intern("entity"), sync_id), v);
	      push(v, private_list);
	    }
	}
      tsilod;

      /* define clause */
      dolist (a, attr(intern("define"), tr))
	{
	  lv *t = arg1(a);
	  lv *v = make_variable(t, attr(intern("name"), attr(intern("id"),a)), intern("LOCAL"));

	  set_attr(intern("target_variable"), attr(intern("entity"), attr(intern("id"),a)), v);
	  push(v, private_list);
	}
      tsilod;

      /* do clause */
      dolist (a, attr(intern("do"), tr))
	{
	  push_action_variables(td, a, &private_list);
	}
      tsilod;

      /* connect clause */
      dolist (a, attr(intern("connections"), tr))
	{
	  push_action_variables(td, a, &private_list);
	}
      tsilod;
    }
  tsilod;
}


void
cml_process_setup_clause(lv *td, lv *setup, lv** private_list, lv** public_list)
{
  if (setup != 0)
    {
      /* define clause */
      dolist (define_act, attr(intern("define"), setup))
	{
	  lv *t = arg1(define_act);
	  lv *v = make_variable(t,
				attr(intern("name"), attr(intern("id"), define_act)),
				intern("TEMPORARY"));

	  set_attr(intern("target_variable"), attr(intern("entity"), attr(intern("id"), define_act)), v);
	  push(v, *private_list);
	}
      tsilod;

      /* do clause */
      dolist (a, attr(intern("do"), setup))
	{
	  push_action_variables(td, a, private_list);
	}
      tsilod;

      /* connect clause */
      dolist (a, attr(intern("connections"), setup))
	{
	  push_action_variables(td, a, private_list);
	}
      tsilod;
    }
}


void
cml_process_local_variable_decl(lv* td,
			    lv* decls,
			    lv** collected_vars_list)
{
  lv *id;
  lv *name;
  lv *entity;
  lv *ns_entity;
  lv *local_ns = first(attr(intern("env"), td));

  dolist (decl, decls)
    {
      id = attr(intern("id"), decl);
      name = attr(intern("name"), id);
      entity = attr(intern("entity"), id);
      ns_entity = ns_find(name, local_ns);
	  
      /* This is a debugging check.
       * It looks like the association between names and entities
       * is done in two ways: via the 'ns/env' and via the intern("entity")
       * slot.
       * I want to be sure.
       *
       * Marco Antoniotti 19970829
       */
      assert(ns_entity == entity);

      if (! attr(intern("target_variable"), entity))
	{
	  *collected_vars_list = nconc(implementation_variables(entity, name),
				       *collected_vars_list);
	}
    }
  tsilod;
}
#endif /* VERBOSE_INHERITANCE */


/* reassoc_variables -- Alter association list *VARIABLESP to reflect
 * the new variable/equation associations as given by EQUATIONS within
 * namespace NS.
 */
void
reassoc_variables(lv **variablesp, lv *equations, lv *ns)
{
  dolist (eq, equations) {
    if (op(eq) == intern("id"))
      {
	lv *flow = ns_find(attr(intern("name"), eq), ns);
	reassoc_variables(variablesp, args(attr(intern("meaning"), flow)), ns);
      }
    else
      {
	reassoc(attr(intern("entity"), arg1(eq)), eq, *variablesp);
      }
  } tsilod;
}


/* substitute_flows -- Replace flow names in discrete states with
 * their equation sets, and remove redundant definitions.  The LAST
 * definition of a variable is the valid one.  Set the attribute
 * intern("defined_variables") of each discrete state to the variables that are
 * defined (either differentially or algebraically) in that state,
 * together with their definitions.
 */
void
substitute_flows(lv *td)
{
  lv *local_ns = hd(attr(intern("env"), td));
  lv *default_flow = ns_find(intern("default"), local_ns); 

  dolist (s, attr(intern("discrete"), td)) {
    lv *variables = nil;
    if (default_flow)
      {
	lv *eqs = args(attr(intern("meaning"), default_flow));
	reassoc_variables(&variables, eqs, local_ns);
      }
    reassoc_variables(&variables, attr(intern("equations"), s), local_ns);
    add_attr(intern("defined_variables"), s, variables);
  } tsilod;
}


void
set_id_mode(lv *name, lv *entity, lv *mode, lv *parent)
{
  lv *parent_ns = attr(intern("export_ns"), parent);
  lv *pentity = ns_find(name, parent_ns);

  if (! pentity)
    return;
  else
    {
      lv *modep = attr(mode, entity);
      lv *pmodep = attr(mode, pentity);

      /* Here it is assumed that if a mode is set then it is
       * set as intern("true").
       */
      if (modep && ! pmodep)
	{
	  lv *grandparent_id = attr(intern("parent"), parent);
	  set_attr(mode, pentity, intern("true"));
	  if (grandparent_id)
	    set_id_mode(name, pentity, mode, meaning(grandparent_id));
	}
    }
}


void
uniformize_modes_up(lv *td)
{
  lv *pid = attr(intern("parent"), td);

  if (! pid)
    return;
  else
    {
      lv *parent = meaning(pid);
      lv *parent_ns = attr(intern("export_ns"), parent);

      dolist (decl, attr(intern("output"), td)) {
	lv *id = attr(intern("id"), decl);
	lv *name = attr(intern("name"), id);
	lv *entity = attr(intern("entity"), id);

	set_id_mode(name, entity, intern("differential"), parent);
	set_id_mode(name, entity, intern("algebraic"), parent);
      } tsilod;
    }
}


void
uniformize_modes_down(lv *td)
{
  lv *pid = attr(intern("parent"), td);

  if (! pid)
    return;
  else
    {
      lv *parent = meaning(pid);
      lv *parent_ns = attr(intern("export_ns"), parent);
    
      dolist (decl, attr(intern("output"), td)) {
	lv *id = attr(intern("id"), decl);
	lv *entity = attr(intern("entity"), id);
	lv *pentity = ns_find(attr(intern("name"), id), parent_ns);

	if (pentity)
	  {
	    lv *diff = attr(intern("differential"), entity);
	    lv *pdiff = attr(intern("differential"), pentity);
	    lv *alg = attr(intern("algebraic"), entity);
	    lv *palg = attr(intern("algebraic"), pentity);

	    if (pdiff && ! diff) set_attr(intern("differential"), entity, intern("true"));
	    if (palg && ! alg) set_attr(intern("algebraic"), entity, intern("true"));
	  }
      } tsilod;
    }
}


typedef struct size_align
{
  short size, align;
} size_align;


/* Portability problems here.
 */
size_align
type_size(lv *v)
{
  size_align sa;
  lv *o = op(v);
#define C(typeop, x) if (o == intern(#typeop)) {			\
    sa.size = x;							\
    sa.align = x;							\
    return sa;								\
  }

  C(number_type, 8);
  C(symbol_type, 4);
  C(logical_type, 4);
  C(mode_type, 4);
  C(function, 4);
  C(typedef, 4);
  C(id, 4);
  C(component, 4);
  C(set, 4);
  C(array, 4);
  C(integer_type, 4);
#undef C
  if (o == intern("vector"))
    {
      sa.size = 8 * num(arg1(v));
      sa.align = 8;
      return sa;
    }
  internal_error("type_size");
  return sa;
}


/* generate_type -- */

void
generate_type(lv *v, FILE *f)
{
  lv *x = op(v);

  if ( x == intern("id") )
    {
      /* Added support for external type descriptors.
       *
       * Tunc Simsek 15th April, 1998
       */
      lv *type = attr(intern("type"), v);
      lv *exttypedef = type ? attr(intern("exttypedef"), type) : nil;

      if ( exttypedef && exttypedef == intern("true"))
	{
	  fprintf(f, "FOREIGN_TYPE *");
	  return;
	}
      else
	{
	  fprintf(f, "Component *");
	  return;
	}
    }

#define C(typeop, impltype) \
  if (x == intern(#typeop)) { fprintf(f, #impltype); return; }

    C(number_type, double);
    C(symbol_type, int);
    C(logical_type, int);
    C(mode_type, int);
    C(typedef, Component *);
    /*    C(id, Component *); */
    C(component, Component *);
    C(set, Set *);
    C(array, Array *);
#undef C
    internal_error("generate_type");
}


/* generate_flow_function_type -- This function prints out the proper
 * type for an algebraically defined variable.  It is very general and
 * it might be an overkill.  Moreover it might have nasty side effects
 * in the generated code.  However it is used only in
 * 'generate_flow_info' and 'generate_fm' in 'generate_setup_function'
 * Marco Antoniotti 19960911
 */
void
generate_flow_function_type(lv* v, FILE *f)
{
  lv *x = op(v);

  /* Added support for external data types.
   *
   * Tunc Simsek 15th April, 1998
   */
  if ( x == intern("id") )
    {
      lv *type = attr(intern("type"), v);
      lv *exttypedef = type ? attr(intern("exttypedef"), type) : nil;

      if ( exttypedef && exttypedef == intern("true"))
	{
	  user_error(v,"Illegal use of variable.\n\
Hint: You cannot use external variables in flow equalities.\n");
	  exit(1);
	}
      else
	{
	  fprintf(f, "ComponentFlowFunction");
	  return;
	}
    }

#define C(typeop, impltype) \
  if (x == intern(#typeop)) { fprintf(f, #impltype); return; }

    C(number_type, DoubleFlowFunction);
    C(symbol_type, IntFlowFunction);
    C(logical_type, IntFlowFunction);
    C(mode_type, IntFlowFunction);
    C(typedef, ComponentFlowFunction);
    C(id, ComponentFlowFunction);
    C(component, ComponentFlowFunction);
    C(set, SetFlowFunction);
    C(array, ArrayFlowFunction);
#undef C
    internal_error("generate_flow_function_type");
}


/* initialize_member_offsets -- This function sets to -1 the offset
 * of each variable member of the type descriptor td. 
 * Alain Girault 19961126 */
void
initialize_member_offsets(lv *td)
{
  dolist (v, attr(intern("public_list"), td)) {
    set_attr(intern("offset"), v, fixnum(-1));
  } tsilod;

  dolist (v, attr(intern("private_list"), td)) {
    set_attr(intern("offset"), v, fixnum(-1));
  } tsilod;
}


/* compute_member_offsets -- This function computes the offset of each
 * variable member of the type descriptor td. Variable members that are 
 * inherited have the same offset as in their parent type. 
 * Alain Girault 19961126
 */ 
void
compute_member_offsets(lv *td)
{
  lv *child_id = attr(intern("id"), td);
  char *child_name = pname(attr(intern("name"), child_id));
  int max_offset = 0;
  int offset = 0;
  int old_offset = 0;
  char *name;
  char *kind;
  int parent_offset = 0;
  char *parent_name;
  char *parent_kind;
  lv *parent_id = attr(intern("parent"), td);
  lv *parent;

  if (parent_id)
    /* get the type descriptor of the parent if there is one */
    parent = meaning(parent_id);
#ifdef DEBUG_INHERITANCE
  printf("type %s\n", child_name);
#endif

  /* 1. retrieve the offset of public inherited members */
#ifdef DEBUG_INHERITANCE
  printf("  public: ");
#endif
  dolist (v, attr(intern("public_list"), td))
    {
      name = pname(attr(intern("name"), v));
      offset = num(attr(intern("offset"), v));
      kind = pname(attr(intern("kind"), v));
#ifdef DEBUG_INHERITANCE
      printf("(%s,%d) ", name, offset);
#endif
      if (parent_id)
	{
	  /* if there is a parent, get the offset of all the common members */
	  dolist (w, attr(intern("public_list"), parent))
	    {
	      parent_name = pname(attr(intern("name"), w));
	      parent_kind = pname(attr(intern("kind"), w));
	      if (!strcmp(parent_name, name) && !strcmp(parent_kind, kind))
		{
		  /* if the member also belongs to the parent, then its offset
		     has already been computed by the parent */
		  parent_offset = num(attr(intern("offset"), w));
#ifdef DEBUG_INHERITANCE
		  printf("(%s,%d) ", parent_name, parent_offset);
#endif
		  if (parent_offset != -1)
		    {
		      /* an offset different than -1 is correct: we copy it
			 into the offset field of the child member */
		      set_attr(intern("offset"), v, fixnum(parent_offset));
		    }
		  else
		    {
		      /* an offset equal to -1 is an error: it means that 
		         the offset was not generated for the parent */
		      user_error(v, "offset should be defined");
		      exit(1);
		    }
		}
	    } tsilod;
	}
      offset = num(attr(intern("offset"), v));
      if (offset >= max_offset)
	{
	  /* we increment the max_offset for future alignment */
	  size_align sa;

	  sa = type_size(attr(intern("type"), v));
	  max_offset = offset + sa.size;
	}
    } tsilod;
#ifdef DEBUG_INHERITANCE
    printf("\n  total offset = %d", max_offset);
#endif

    /* 2. retrieve the offset of private inherited members */
#ifdef DEBUG_INHERITANCE
    printf("\n  private: ");
#endif
    dolist (v, attr(intern("private_list"), td))
      {
	name = pname(attr(intern("name"), v));
	offset = num(attr(intern("offset"), v));
#ifdef DEBUG_INHERITANCE
	printf("(%s,%d) ", name, offset);
#endif
	if (parent_id)
	  {
	    /* if there is a parent, get the offset of 
	       all the common members */
	    dolist (w, attr(intern("public_list"), parent))
	      {
		parent_name = pname(attr(intern("name"), w));
		if (!strcmp(parent_name, name))
		  {
		    /* if the member also belongs to the parent, 
		       its offset has already been computed by the parent */
		    parent_offset = num(attr(intern("offset"), w));
#ifdef DEBUG_INHERITANCE
		    printf("(%s,%d) ", parent_name, parent_offset);
#endif
		    if (parent_offset != -1)
		      {
			/* an offset different than -1 is correct: we copy it
			   into the offset field of the child member */
			set_attr(intern("offset"), v, fixnum(parent_offset));
		      }
		    else
		      {
			/* an offset equal to -1 is an error: it means that 
			   the offset was not generated for the parent */
			user_error(v, "offset should be defined");
			exit(1);
		      }
		  }
	      } tsilod;
	  }
	offset = num(attr(intern("offset"), v));
	if (offset >= max_offset)
	  {
	    /* we increment the max_offset for future alignment */
	    size_align sa;

	    sa = type_size(attr(intern("type"), v));
	    max_offset = offset + sa.size;
	  }
      } tsilod;
#ifdef DEBUG_INHERITANCE
      printf("\n  total offset = %d\n", max_offset);
#endif
      offset = max_offset;

      /* 3. compute the offset of public own members */
      dolist (v, attr(intern("public_list"), td))
	{
	  old_offset = num(attr(intern("offset"), v));
	  /* the member is not inherited if its offset is equal to -1 */
	  if (old_offset == -1)
	    {
	      size_align sa;
	      int m;

	      sa = type_size(attr(intern("type"), v));

	      /* members must be aligned according to their size */
	      m = offset % sa.align;

	      /* if m is zero, then the current alignment is correct */
	      if (m != 0)
		/* if m is not zero, then we must re-align the member */
		offset += (sa.align - m);
	      set_attr(intern("offset"), v, fixnum(offset));
	      offset += sa.size;
	    }
	} tsilod;
#ifdef DEBUG_INHERITANCE
	printf("  total offset = %d\n", offset);
#endif

	/* 4. compute the offset of private own members */
	dolist (v, attr(intern("private_list"), td))
	  {
	    old_offset = num(attr(intern("offset"), v));
	    /* the member is not inherited if its offset is equal to -1 */
	    if (old_offset == -1)
	      {
		size_align sa;
		int m;

		sa = type_size(attr(intern("type"), v));
		/* members must be aligned according to their size */
		m = offset % sa.align;
		/* if m is zero, then the current alignment is correct */
		if (m != 0)
		  /* if m is not zero, then we must re-align the member */
		  offset += (sa.align - m);
		set_attr(intern("offset"), v, fixnum(offset));
		offset += sa.size;
	      }
	  } tsilod;
#ifdef DEBUG_INHERITANCE
	  printf("  total offset = %d\n", offset);
#endif

	  /* 5. set the total size of the type descriptor */
	  set_attr(intern("data_size"), td, fixnum(offset));
}


lv *
attr_do_setup(lv *td)
{
  lv *setup = attr(intern("setup"), td);
  return setup ? attr(intern("do"), setup) : nil;
}


lv *
attr_connections_setup(lv *td)
{
  lv *setup = attr(intern("setup"), td);
  return setup ? attr(intern("connections"), setup) : nil;
}


static int unique_counter = 1;

/* Assign unique numbers to various parts of the program.  The numbers
 * are used later to construct unique names.
 */
void
assign_unique_numbers(lv *td)
{
#define U(n) set_attr(intern("unique"), n, fixnum(unique_counter++))
  U(td);

  dolist (f, attr(intern("flow"), td)) {
    dolist (e, args(f)) {
      if (op(e) != intern("id"))
	U(e);
    } tsilod;
  } tsilod;

  dolist (s, attr(intern("discrete"), td)) {
    U(s);
    dolist (e, attr(intern("equations"), s)) {
      if (op(e) != intern("id"))
	U(e);
    } tsilod;
  } tsilod;

  dolist (e, attr(intern("export"), td)) {
    U(attr(intern("entity"), e));
  } tsilod;

  dolist (tr, attr(intern("transition"), td)) {
    U(tr);
    dolist (e, attr(intern("events"), tr)) {
      if (op(e) == intern("external_event"))
	U(e);
    } tsilod;
  } tsilod;

  dolist (e, attr(intern("sync_variables"), td)) {
    U(e);
  } tsilod;

  dolist (a, attr_do_setup(td)) {
    if (op(a) == intern("sync") || op(a) == intern("define"))
      U(a);
  } tsilod;

  /******************* SYNTAX CHANGE !!! ********************/
  dolist (a, attr_connections_setup(td)) {
    if (op(a) == intern("sync") || op(a) == intern("define"))
      U(a);
  } tsilod;
#undef U
}


/* Forward declaration for 'generate_expression' */
void generate_expression(lv *);

void
binary_generator(char *operator, lv *a1, lv *a2)
{
  fprintf(cfile, "(");
  generate_expression(a1);
  fprintf(cfile, " %s ", operator);
  generate_expression(a2);
  fprintf(cfile, ")");
}


/*********** MAK: adding a generator for 2-args functions ******/

void
func2args_generator(char *funame, lv *e)
{
  fprintf(cfile, "%s(", funame  );
  generate_expression(arg1(e));
  fprintf(cfile, "," );
  generate_expression(arg2(e));
  fprintf(cfile,")");
}


void
max_generator(lv *e)
{
  func2args_generator("max", e);
}

void
min_generator(lv *e)
{
  func2args_generator("min", e);
}

void
atan2_generator(lv *e)
{
  func2args_generator("atan2", e);
}


void
random_generator(lv *e)
{
  fprintf(cfile, "s_rand()" );
}


/****************************************************************/

void
binmath_generator(lv *e)
{
  binary_generator(pname(op(e)), arg1(e), arg2(e));
}


void
equality_generator(lv *e)
{
  binary_generator("==", arg1(e), arg2(e));
}


void
negate_generator(lv *e)
{
  fprintf(cfile, " -");
  generate_expression(arg1(e));
}


void
or_generator(lv *e)
{
  binary_generator("||", arg1(e), arg2(e));
}


void
and_generator(lv *e)
{
  binary_generator("&&", arg1(e), arg2(e));
}


void
not_generator(lv *e)
{
  fprintf(cfile, " !");
  generate_expression(arg1(e));
}


/* generate_actual_parm_expr -- This function is actually redundant.
 * It would become useful iff we did some flow analysis and ordered
 * the executions of assignments.
 */
void
generate_actual_parm_expr(lv* actual_f_arg)
{
  /* lv* temp_loc = attr(intern("temp"), actual_f_arg); */
  
  generate_expression(actual_f_arg); /* For the time being. */
}


void
call_generator2(char *f, lv *arguments, lv* call_signature)
{
  lv* formals;
  lv* actuals;
  int saved_lhs_flag;

  /* We must ensure that we evaluate the arguments as they are and
   * not as L-values.
   *
   * Marco Antoniotti 19970806
   */

  if (call_signature != 0)
    {
      /* We are generating a FF call */
      formals = attr(intern("formals"), call_signature);
      actuals = arguments;
      fprintf(cfile, "%s(", f);

      if (actuals)
	{
	  saved_lhs_flag = LHS;
	  LHS = 0;

	  if (out_formal_p(first(formals))) fputc('&', cfile);
	  generate_actual_parm_expr(first(actuals));
	  for (formals = rest(formals), actuals = rest(actuals);
	       formals != 0 && actuals != 0;
	       formals = rest(formals), actuals = rest(actuals))
	    {
	      fputs(", ", cfile);
	      if (out_formal_p(first(formals))) fputc('&', cfile);
	      generate_actual_parm_expr(first(actuals));
	    }
	  LHS = saved_lhs_flag;
	}
      fputc(')', cfile);
    }
  else
    {
      fprintf(cfile, "%s(", f);
      saved_lhs_flag = LHS;
      LHS = 0;
      if (arguments)
	{
	  generate_actual_parm_expr(first(arguments));
	  dolist (a, rest(arguments))
	    {
	      fputs(", ", cfile);
	      generate_actual_parm_expr(a);
	    }
	  tsilod;
	}
      LHS = saved_lhs_flag;
      fputc(')', cfile);
    }
}


void
library_call_generator(lv *e)
{
  call_generator2(pname(op(e)), args(e), 0);
}


void
set_membership_generator(lv* e)
{
  lv* el_type = type_of(arg1(e));

  if (number_type_p(el_type))
    call_generator2("set_membership_D", args(e), 0);
  else if (symbol_type_p(el_type))
    call_generator2("set_membership_I", args(e), 0);
  else
    call_generator2("set_membership_A_S_C", args(e), 0);
}


void
set_equality_generator(lv *e)
{
  call_generator2("set_equal", args(e), 0);
}


void
self_variable_generator(lv *e)
{
  fprintf(cfile, "_self");
}


void
global_variable_generator(lv *e)
{
  fprintf(cfile, "_global");
}


void
new_variable_generator(lv *e)
{
  fprintf(cfile, "_new");
}


/* Generate a differential accessor (_DA_), algebraic accessor (_AA_),
 * or a plain variable accessor (_VA_) depending on the member
 * attributes.  COMPONENT is an expression that accesses a component
 * instance.  MEMBER is the entity node for a member variable in that
 * instance.
 */
void
simple_variable_generator(lv *component, lv *variable)
{
  int offset = num(attr(intern("offset"), variable));
  int saved_lhs_flag = 0;

  fprintf(cfile, "_VA%s_(", LHS? "L" : "");

  saved_lhs_flag = LHS;
  LHS = 0;
  generate_expression(component);
  LHS = saved_lhs_flag;

  fprintf(cfile, ", ");
  generate_type(attr(intern("type"), variable), cfile);
  fprintf(cfile, ", %d)", offset);
}


/* member_generator --
 * Changed in order to accommodate the new 'typed' algebraic
 * accessors.
 *
 * Marco Antoniotti 19961211
 */
void
member_generator(lv *component, lv *member)
{
  int saved_lhs_flag = 0;

  if (attr(intern("differential"), member))
    {
      int mode_offset = num(attr(intern("offset"), attr(intern("mode"), member)));
      int state_offset = num(attr(intern("offset"), attr(intern("state"), member)));
      int flow_offset = num(attr(intern("offset"), attr(intern("flow_function"), member)));
      int context_offset = num(attr(intern("offset"), attr(intern("context"), member)));

      fprintf(cfile, "_DA%s_(", LHS? "L" : "");
      
      saved_lhs_flag = 0;
      LHS = 0;
      generate_expression(component);
      fprintf(cfile, ", %d, %d, %d, %d, _step)",
	      mode_offset,
	      state_offset,
	      flow_offset,
	      context_offset);
      LHS = saved_lhs_flag;
    }
  else if (attr(intern("algebraic"), member))
    {
      /* Generating a typed algebraic accessor.  The macros have the
       * following arguments:
       *  _AATL_(c, mode, v, vtype, ff, fftype, context)
       *  _AAT_(c, mode, v, vtype, ff, fftype, context, step)
       */
      
      int mode_offset = num(attr(intern("offset"), attr(intern("mode"), member)));
      int v_offset = num(attr(intern("offset"), attr(intern("target_variable"), member)));
      int flow_offset = num(attr(intern("offset"), attr(intern("flow_function"), member)));
      int context_offset = num(attr(intern("offset"), attr(intern("context"), member)));

#ifndef OLD_AA_MEMBER_GENERATOR

      if (LHS)
	{
	  fprintf(cfile, "_AATL_(");

	  saved_lhs_flag = LHS;
	  LHS = 0;

	  generate_expression(component);        /* c - Component accessor */
	  fprintf(cfile, ", %d,", mode_offset);	 /* mode - Var Mode Offset */
	  fprintf(cfile, " %d, ", v_offset);     /* v - Variable Offset */
	  generate_type(type_of(member), cfile); /* vtype - Variable Type */
	  fprintf(cfile, ", %d, ", flow_offset); /* ff - Flow Func Offset */
	  generate_flow_function_type(type_of(member), cfile);
	  /* fftype - Flow Func Type */
	  fprintf(cfile, ", %d)", context_offset);
	  /* context - Context Offset */

	  LHS = saved_lhs_flag;
	}
      else			/* not LHS; i.e. RHS */
	{
	  /* Yeah, yeah!  I could be smarter and make the code
	   * shorter.  Just think how more difficult it would be to
	   * read.
	   *
	   * Marco Antoniotti 19961211
	   */

	  fprintf(cfile, "_AAT_(");

	  saved_lhs_flag = LHS;
	  LHS = 0;

	  generate_expression(component);        /* c - Component accessor */
	  fprintf(cfile, ", %d,", mode_offset);	 /* mode - Var Mode Offset */
	  fprintf(cfile, " %d, ", v_offset);     /* v - Variable Offset */
	  generate_type(type_of(member), cfile); /* vtype - Variable Type */
	  fprintf(cfile, ", %d, ", flow_offset); /* ff - Flow Func Offset */
	  generate_flow_function_type(type_of(member), cfile);
	  /* fftype - Flow Func Type */
	  fprintf(cfile, ", %d,", context_offset);
	  /* context - Context Offset */
	  fprintf(cfile, " _step)");

	  LHS = saved_lhs_flag;
	}

#else  /* OLD_AA_MEMBER_GENERATOR */

      fprintf(cfile, "_AA%s_(", LHS? "L" : "");
      generate_expression(component);
      /* MAKMAK	fprintf(cfile, ", %d, %d, %d, _step)", */


      /******* MAKpatch********/
      if (LHS)
	/* Luigi, Marco, Michael 19960613
	 * Changed generation of _AA_ accessor in order to take into
	 * account the 'context information.
	 */

	fprintf(cfile, ", %d, %d, %d, %d)",
		mode_offset, v_offset, flow_offset, context_offset);
      else 
	fprintf(cfile, ", %d, %d, %d, %d, _step)",
		mode_offset, v_offset, flow_offset, context_offset);

      /***********************/

#endif /* OLD_AA_MEMBER_GENERATOR */

    }
  else
    {
      simple_variable_generator(component, attr(intern("target_variable"), member));
    }
}


void
entity_generator(lv *e)
{
  member_generator(self_variable, e);
}


void
variable_generator(lv *e)
{
  simple_variable_generator(self_variable, e);
}


/* c_stack_variable_generator */
void
c_stack_variable_generator(lv* id, lv* id_entity)
{
  if (attr(intern("set_element"), id_entity) == intern("true"))
    {
      if (equal_type(attr(intern("type"), id_entity), number_type))
	fprintf(cfile, "%s.d", pname(attr(intern("name"), id)));
      else if (equal_type(attr(intern("type"), id_entity), symbol_type)
	       || equal_type(attr(intern("type"), id_entity), logical_type))
	fprintf(cfile, "%s.i", pname(attr(intern("name"), id)));
      else
	{
	  /* Added generation of type cast
	   * Marco Antoniotti 19970305
	   */
	  fputs("(", cfile);
	  generate_type(attr(intern("type"), id), cfile);
	  fputs(")", cfile);
	  fprintf(cfile, "%s.v", pname(attr(intern("name"), id)));
	}
    }
  else				/* range variable as of 19970104 */
    fprintf(cfile, "%s", pname(attr(intern("name"), id)));
}


/* id_generator -- Modified to take care of those identifiers which we
 * want to use as C variables.  Mostly in set and array iterators.
 */
void
id_generator(lv *id)
{
  lv *e = attr(intern("entity"), id);
  lv *kind = attr(intern("kind"), e);

  if (kind == intern("GLOBAL"))
    {
      member_generator(global_variable, e);
    }
  else if (e == nil_entity)
    {
      fprintf(cfile, "NIL");
    }
  else if (e == false_entity)
    {
      fprintf(cfile, "0");
    }
  else if (e == true_entity)
    {
      fprintf(cfile, "1");
    }
  else if (attr(intern("name"), id) == intern("self"))
    {
      self_variable_generator(id);
    }
  else if (e && attr(intern("c_stack_id"), e) == intern("true") && 0) /* Disabled 19970717 */
    {
      /* An identifier that is only to be used as a C
       * variable. (Mostly a set or array former iterator variable.)
       * See 'check.c' at 'rewrite_setcons_iter'.
       *
       * Marco Antoniotti 19970103
       */
      c_stack_variable_generator(id, e);
      /* fprintf(cfile, "%s", pname(attr(intern("name"), id))); */
    }
  else
    {
      entity_generator(e);
    }
}


void
access_generator(lv *e)
{
  lv *component = arg1(e);
  lv *member = attr(intern("accessor"), e);

  member_generator(component, member);
}


void
float_generator(lv *e)
{
  char *s = pname(attr(intern("value"), e));
  fprintf(cfile, attr(intern("convert"), e) == intern("true") ? "((double) %s)" : "%s", s);
}


void
int_generator(lv *e)
{
  fprintf(cfile, attr(intern("convert"), e) == intern("true") ? "((double) %d)" : "%d",
	  num(attr(intern("value"), e)));
}


void
if_generator(lv *e)
{
  fprintf(cfile, "(");
  generate_expression(arg1(e));
  fprintf(cfile, "? ");
  generate_expression(arg2(e));
  fprintf(cfile, " : ");
  generate_expression(arg3(e));
  fprintf(cfile, ")");
}


void
exists_call_generator(lv *e)
{
  if (op(arg1(e)) == intern("arrayrange"))
    fprintf(cfile, "exists_expr_range_F%d(_self)", num(attr(intern("unique"), e)));
  else
    {
      fprintf(cfile, "exists_expr_F%d(_self, ", num(attr(intern("unique"), e)));
      generate_expression(arg1(e));
      fprintf(cfile, ")");
    }
}


void
minmax_call_generator(lv *e)
{
  fprintf(cfile, "minmax_expr_F%d(_self, ", num(attr(intern("unique"), e)));
  generate_expression(arg1(e));
  fprintf(cfile, ")");
}


void
symbol_generator(lv *e)
{
  fprintf(cfile, "_SYMBOL_%s", pname(attr(intern("name"), e)));
}


void
verbatim_generator(lv *e)
{
  fprintf(cfile, "%s", str(attr(intern("code"), e)));
}


/* generate_create_allocation_call --
 * This function is in charge of generating a call to the type
 * instance creation function which does essentially storage
 * allocation and other simple initializations.
 *
 * Note: it used to be called 'create_generator2'
 *
 * Marco Antoniotti 19960910
 */
void
generate_create_allocation_call(lv* e)
{
  lv *td = meaning(arg1(e));
  fprintf(cfile, "createF%d()", num(attr(intern("unique"), td)));
}


/* ALEKS ADDED THESE FUNCTIONS */

/* generate_create_setup_call --
 * This function is in charge of generating a call to the type
 * instance creation function which implements the finalization of the
 * instance creation by calling the appropriate setup actions.
 *
 * Note: it used to be called 'create_generator3'
 *
 * Marco Antoniotti 19960910
 */
void
generate_create_setup_call(lv* e)
{
  lv *td = meaning(arg1(e));
  fprintf(cfile, "create2F%d(_new)", num(attr(intern("unique"), td)));
}


/* create_generator_no_initargs --
 * If the call looks like 'x := create(XType);' then we do not need to
 * introduce the extra initializations.  This functions takes care of
 * this by producing a "bypass".
 */
void
create_generator_no_initargs(lv *e)
{
  lv *td = meaning(arg1(e));

  fprintf(cfile,
	  "create2F%d(createF%d())",
	  num(attr(intern("unique"), td)),
	  num(attr(intern("unique"), td)));
}
/* END ALEKS ADDED FUNCTIONS */


/* create_generator --
 * This function generates a call to the function associated to a
 * SHIFT 'create' expression.
 * If the create expression is without initializers, the call is
 * spliced in (see function 'create_generator_no_initargs'.)
 */
void
create_generator(lv *e)
{
  if (tl(args(e))) {
    fprintf(cfile, "create_expr_X%d(_self)", num(attr(intern("unique"), e)));
  } else {
    /* ALEKS CHANGED THIS FROM create_generator2(e); */
    create_generator_no_initargs(e);
  }
}


void
narrow_generator(lv *e)
{
  lv *ntd = meaning(arg1(e));
  int u = num(attr(intern("unique"), ntd));

  fprintf(cfile, "(Component *)narrow(&c_typed%d, ", u);
  generate_expression(arg2(e));
  fprintf(cfile, ")");
}


void
call_generator(lv *e)
{
  lv *f = arg1(e);
  lv *a = tl(args(e));
  lv *call_signature = attr(intern("signature"), e);

  if (op(f) != intern("id"))
    internal_error("calling non-function");

  if (call_signature != 0 && op(call_signature) == intern("ext_fun_decl"))
    call_generator2(pname(attr(intern("ffi_wrapper_name"), call_signature)),
		    a,
		    call_signature);
  else
    call_generator2(pname(attr(intern("name"), f)), a, 0);
}


void
generate_expression(lv *e)
{
  lv *c = assoc(intern("generator"), plist(op(e)));

  if (! c)
    {
      internal_error("unknown generator");
    }

  ((void (*)(lv *)) (oth(tl(c))))(e);
  /* The previous statement has 16 parens.  To do the same thing in
   * Lisp I would have written (funcall (cdr c) e).  So there.
   */
}


void
generate_logical_function(int u, lv *expression)
{
  fprintf(hfile, "int logicalF%d(Component *_self);\n", u);
  fprintf(cfile, "int\nlogicalF%d(Component *_self)\n{\n  return ", u);
  generate_expression(expression);
  fprintf(cfile, ";\n}\n\n");
}


void
generate_flow_function(lv *unique, lv *equation)
{
  int u = num(unique);

  generate_type(attr(intern("type"), arg2(equation)), hfile);
  fprintf(hfile,
	  "\nflowF%d(Component *_self, int _step, int _recur_depth);\n", u);

  generate_type(attr(intern("type"), arg2(equation)), cfile);
  fprintf(cfile,
	  "\nflowF%d(Component *_self, int _step, int _recur_depth)\n\{\n\
  if (_recur_depth > MAX_RECURSION_ALGEBRAIC_DEPTH)\n\
    runtime_error(\"too many evaluations: circularity in algebraic definitions?\");\n\
  else\n\
    _recur_depth++;\n\
  return ",
	  u);
  generate_expression(arg2(equation));
  fprintf(cfile, ";\n}\n\n");
}


/* generate_fm -- Generate flow or mode or context for entity X in
 * component C. 
 */
/* Original -- Marco Antoniotti 19960911
void
generate_fm(lv *c, lv *x, lv *what)
{
    int n = num(attr(intern("offset"), attr(what, x)));
    fprintf(cfile, "DATA_ITEM(%s, ",
	    what == intern("mode") ? "int" : 
	    what == intern("flow_function") ? "FlowFunctionPointer" :
	    "Component *");
    generate_expression(c);
    fprintf(cfile, ", %d)", n);
}
*/

void
generate_fm(lv *c, lv *x, lv* access_type, lv *what)
{
  int n = num(attr(intern("offset"), attr(what, x)));

  if (what == intern("mode"))
    fprintf(cfile, "DATA_ITEM(int, ");
  else if (what == intern("flow_function"))
    {
      fprintf(cfile, "DATA_ITEM(");
      generate_flow_function_type(access_type, cfile);
      fprintf(cfile, ", ");
    }
  else
    fprintf(cfile, "DATA_ITEM(Component*, ");
  generate_expression(c);
  fprintf(cfile, ", %d)", n);
}


/* generate_flow_info --
 * Constructs the actual reference to the 'followed' variable.
 *
 * The call to 'generate_fm' is now changed in order to pass type
 * information relating to the flow function.
 */
void
generate_flow_info(lv *expr, lv *what)
{
  if (op(expr) == intern("id"))
    {
      generate_fm(self_variable, attr(intern("entity"), expr), (lv *) 0, what);
    }
  else if (op(expr) == intern("access"))
    {
      generate_fm(arg1(expr),
		  attr(intern("accessor"), expr),
		  attr(intern("type"), expr),
		  what);
    }
}


void
generate_mode_definition(lv* id, lv *e, lv *vars)
{
  lv *d = attr(intern("differential"), e);
  lv *a = attr(intern("algebraic"), e);
  int mode_offset;
  int flow_offset;
  int context_offset;
  lv *c = nil;


  if (d || a)
    {
      mode_offset = num(attr(intern("offset"), attr(intern("mode"), e)));
      flow_offset = num(attr(intern("offset"), attr(intern("flow_function"), e)));
      context_offset = num(attr(intern("offset"), attr(intern("context"), e)));
      c = assoc(e, vars);

      /* Print name of variable for documentation. */
      fprintf(cfile, "\n  /* Variable '%s' */\n", pname(attr(intern("name"), id)));
      
      fputs("  /* ", cfile);
      assert(attr(intern("target_variable"), e) != nil);
      assert(attr2(intern("name"), intern("target_variable"), e) != nil);
      fprintf(cfile, "entity at %d id %s offsets <%d %d %d>",
	      e,
	      pname(attr2(intern("name"), intern("target_variable"), e)),
	      mode_offset, flow_offset, context_offset);
      fputs("  */\n", cfile);

      if (! c)
	{
	  /* The instance variable does not have a 'state'
	   * definition. Therefore it can be neither DIFFERENTIAL_MODE
	   * nor ALGEBRAIC_MODE in the current one.
	   * Therefore we set its mode to CONSTANT_MODE.
	   */
	  fprintf(cfile,
		  "  MODE(_self, %d) = CONSTANT_MODE;\n", mode_offset);
	}
      else
	{
	  lv *eq = tl(c);
	  /* fprintf(cfile, "  FLOW(_self, %d) = flowF%d;\n", */
	  fprintf(cfile, "  TFLOW(_self, %d, ", flow_offset);
	  generate_flow_function_type(type_of(e), cfile);  
          fprintf(cfile, ") = flowF%d;\n", num(attr(intern("unique"), eq)));
	  fprintf(cfile, "  MODE(_self, %d) = %s;\n",
		  mode_offset,
		  op(eq) == intern("equate") ? "DIFFERENTIAL_MODE" : "ALGEBRAIC_MODE");
	  fprintf(cfile, "  CONTEXT(_self, %d) = _self;\n",
		  context_offset);
	}
    }
}


/* The entering function for a mode (discrete state) is called when
 * that mode is entered. It sets the variable modes and the mode
 * descriptor in the component instance.
 * If the mode is "exit", then the component instance is removed from
 * the run-time environment.
 */
void
generate_entering_function(lv *m, lv *td)
{
  lv *vars = attr(intern("defined_variables"), m);
  int u = num(attr(intern("unique"), m));
  int has_guards = attr(intern("has_guards"), m) != nil;
  lv *mode_name = attr(intern("name"), attr(intern("id"), m));

  fprintf(hfile, "void enteringF%d(Component *_self);\n", u);

  fprintf(cfile, "/* Entering Function for Mode '%s' in type '%s'.\n */\n",
	  pname(mode_name),
	  pname(attr2(intern("name"), intern("id"), td)));

  fprintf(cfile, "void\nenteringF%d(Component *_self)\n{\n", u);

  /* Warn that the current state is the EXIT state.
   */
  if (!strcmp(pname(mode_name), "exit"))
      {
	fprintf(cfile, "  /* This is the exit state */\n");
      }

  /* Set the new mode for the component.
   */
  fprintf(cfile, "  _self->M = &mode_desc%d;\n", u);

  /* If the new mode is the "exit" mode, destroy the component.
   * Here it is sufficient to remove the component from the various
   * lists of components. The garbage collector will actually 
   * destroy it. One more check that the mode is EXIT is done at
   * run-time.
   * Alain Girault 19970204.
   */
  if (!strcmp(pname(mode_name), "exit"))
      {
	fprintf(cfile, "  if (in_exit_mode(_self))\n");
	fprintf(cfile, "    remove_component(_self);\n");
      }

  /* Go through all local variables in TD, and set the mode of any
   * algebraic or differential variable according to its definition
   * in mode M.
   */
  fprintf(cfile, "\n  /* Output Variables */\n");
  dolist (decl, attr(intern("output"), td))
    {
      generate_mode_definition(attr(intern("id"), decl),
			       attr(intern("entity"), attr(intern("id"), decl)),
			       vars);
    }
  tsilod;

      /* Input Variables are changed only "outside" a component, so they
       * should not be changed in any way when entering a new mode.
       * 
       * The 'if (0)' is here just to keep the code around for a couple
       * of revisions or so.
       *
       * Marco Antoniotti 19960710
       */
#if 0
      fprintf(cfile, "  /* Input Variables */\n");

      dolist (decl, attr(intern("input"), td)) {
	generate_mode_definition(attr(intern("id"), decl),
				 attr(intern("entity"), attr(intern("id"), decl)),
				 vars);
      } tsilod;
#endif

      fprintf(cfile, "\n  /* State Variables */\n");
      dolist (decl, attr(intern("state"), td))
	{
	  generate_mode_definition(attr(intern("id"), decl),
				   attr(intern("entity"), attr(intern("id"), decl)),
				   vars);
	}
      tsilod;
      fprintf(cfile, "}\n\n");
    }


/* Exiting Function -- The exiting function of a mode (i.e. state) is
 * used to update the values of instance variables in case of changes
 * in mode (i.e. from ALGEBRAIC to CONSTANT etc.)
 * The generating function ('generate_value_mode_updates') is simply a
 * big 'if' with all branches just long 'printfs'.
 *
 * Note: changing the 'entering_function' would be more difficult,
 * since we cannot determine the following things:
 * 1 - the 'previous' mode from which we reached the current one.
 *
 * 2 - hence the previous 'mode' (ALGEBRAIC, DIFFERENTIAL or CONSTANT)
 *     of the instance variable itself.
 *
 * 3 - most likely the _AA_ accessors will have to be changed into
 *     _AAT_
 *
 * 4 - I changed the generation scheme to use 'member_generator' (which
 *     also changed.)  I also want to keep the code around for a
 *     release or two before cleaning up the code. This is the reason
 *     for all the '#ifdef NO_MEMBER_GENERATOR' etc.
 *
 * 5 - note however that using 'member_generator' makes me loose some
 *     the 'locality' of information. I.e. if a variable has been
 *     defined to be differential that does mean that I will probably
 *     not be able to set the differential field when the mode in the
 *     current state is - e.g. - ALGEBRAIC.
 *     This calls for the introduction of special operators (macros)
 *     at the same level of _[VAD]AL*_ that allow to actually poke at
 *     the right memory locations.
 *
 * Marco Antoniotti 19961212
 *
 * Note: The exiting function as implemented has a major drawback
 * when dealing with Differential Variables.  It looks like
 * Differential Variables are always accessed through the the DA[L]
 * accessor (which always gets the result of the integration at
 * 'state_offset'.)  Therefore, it seems incorrect to set the CONSTANT
 * part of a possibly differential variable using a VA[L] accessor
 * (which brings up the question of why we need the CONSTANT offset
 * anyway.)
 *
 * Marco Antoniotti 19970204
 */

/* generate_value_mode_updates --
 * This function will be totally rewritten for versions with internal
 * number greater than 3.51.
 * The new rewriting takes into consideration the not very logical
 * difference between the internal setup of instance variables which
 * can be differentially defined and the internal setup of variables
 * which can be only constant or algebraic.
 *
 * The function really needs to update the constant slot for
 * algebraically defined variables which are not differential.
 * All the other cases are really non important. In fact, we have the
 * following cases:
 *
 * a - Differential variable:
 *     both the algebraic and constant value are aways kept and
 *     accessed in the 0th slot of the vector used for integration
 *     (check 'shifttypes.h)
 *
 * b - Algebraic variable which may be Constant but not Differential
 *     case 1: the variable is algebraic in state Q
 *        since we do not know what the value mode might be in the
 *        next state, we must make sure that it the constant value is
 *        updated.
 *     case 2: the variable is constant in state Q
 *        in this case we do not really need to do anything.  If the
 *        variable is algebraic in the next state, then we will be using the
 *        associated flow function, which amounts to a complete
 *        redefinition of the variable.  Note that an algebraic
 *        definition cannot be created by making a reference to
 *        itself. Hence such a definition will not access the value
 *        stored in the current "algebraic" slot.
 *
 * c - Constant variable.
 *     nothing needs to be done.
 *
 * Note: with this change it turns out that the exiting functions may
 * not contain any code.  Therefore they could be "optimized" away and
 * their slot in the Mode defintions be set to 0.  This would also
 * require a change (add a test) in the function 'take_transitions' in
 * 'discrete.c'.
 * Anyway, I will not make these changes right now.
 *
 * Marco Antoniotti 19970205
 */
void
generate_value_mode_updates(lv* var_entity, lv* state_special_defined_vars)
{
  lv* diff_p = attr(intern("differential"), var_entity);
  lv* alge_p = attr(intern("algebraic"), var_entity);
  lv* current_state_def = assoc(var_entity, state_special_defined_vars);
  lv* current_state_var_mode;
  lv* target_variable = attr(intern("target_variable"), var_entity);
  int mode_offset;
  int state_offset;
  int flow_offset;
  int context_offset;
  int v_offset;

   if (current_state_def)
    {
      /* Instance Variable may be either ALGEBRAIC or DIFFERENTIAL in
       * the current state.
       */
      if (op(tl(current_state_def)) == intern("equate"))
	current_state_var_mode = intern("differential");
      else
	current_state_var_mode = intern("algebraic");
    }
  else
    current_state_var_mode = intern("constant");

   /* Case "Algebraic and non Differential" (b) in explanatory comment. */
   if (alge_p && !diff_p)
     {
       /* Subcase 1:
	* generate code to update constant slot.
	*/
       if (current_state_var_mode == intern("algebraic"))
	 {
	   v_offset = num(attr(intern("offset"), attr(intern("target_variable"), var_entity)));
	   mode_offset = num(attr(intern("offset"), attr(intern("mode"), var_entity)));
	   flow_offset = num(attr(intern("offset"), attr(intern("flow_function"), var_entity)));
	   context_offset = num(attr(intern("offset"), attr(intern("context"), var_entity)));
       
	   /* Generating code to update the CONSTANT part. */
	   /* Compare with 'member_generator' output */
	   indent_to(1, "  ", cfile);
	   start_c_comment(cfile);
	   fprintf(cfile, " Updating Constant part ");
	   fprintf(cfile, "for instance variable '%s' ",
		   pname(attr(intern("name"), target_variable)));
	   end_c_comment(cfile);
	   putc('\n', cfile);

	   indent_to(1, "  ", cfile);
	   fprintf(cfile, "_VAL_(_self, ");
	   generate_type(attr(intern("type"), target_variable), cfile);
	   fprintf(cfile, ", %d) = ", num(attr(intern("offset"), target_variable)));

	   member_generator(self_variable, var_entity);
	   fputs(";\n", cfile);
	 }
       /* Subcase 2:
	* nothing to be done.
	*/
     }
   /* No other cases really need anything. */
}

#ifdef OLD_GENVALUPDATE
/* Left here just in case for a couple more rleases.
 *
 * Marco Antoniotti 19970205
 */
void
generate_value_mode_updates(lv* var_entity, lv* state_special_defined_vars)
{
  lv* diff_p = attr(intern("differential"), var_entity);
  lv* alge_p = attr(intern("algebraic"), var_entity);
  lv* current_state_def = assoc(var_entity, state_special_defined_vars);
  lv* current_state_var_mode;
  lv* target_variable = attr(intern("target_variable"), var_entity);
  int mode_offset;
  int state_offset;
  int flow_offset;
  int context_offset;
  int v_offset;

   if (current_state_def)
    {
      /* Instance Variable may be either ALGEBRAIC or DIFFERENTIAL in
       * the current state.
       */
      if (op(tl(current_state_def)) == intern("equate"))
	current_state_var_mode = intern("differential");
      else
	current_state_var_mode = intern("algebraic");
    }
  else
    current_state_var_mode = intern("constant");

  /* Now we kow the current mode of a variable and its possible modes.
   * Hence we proceed to produce code for each of the possible cases.
   */
  if (current_state_var_mode == intern("differential"))
    {
      v_offset = num(attr(intern("offset"), attr(intern("target_variable"), var_entity)));
      mode_offset = num(attr(intern("offset"), attr(intern("mode"), var_entity)));
      flow_offset = num(attr(intern("offset"), attr(intern("flow_function"), var_entity)));
      state_offset = num(attr(intern("offset"), attr(intern("state"), var_entity)));
      context_offset = num(attr(intern("offset"), attr(intern("context"), var_entity)));

      /* Generating code to update the ALGEBRAIC part. */
      fprintf(cfile,
	      "  /* Updating Algebraic part for instance variable '%s' */\n",
	      pname(attr(intern("name"), target_variable)));

      fprintf(cfile, "  ");	/* Indent */

#ifdef NO_MEMBER_GENERATOR
      /* Note: this code may be wrong, given the assumptions made for
       * the treatment of differential variables.
       *
       * Marco Antoniotti 19970204
       */
      fprintf(cfile, "_AAL_(_self, %d, %d, %d, %d) = ",
	      mode_offset,
	      v_offset,
	      flow_offset,
	      context_offset);
#else  /* NO_MEMBER_GENERATOR */
      LHS = 1;
      member_generator(self_variable, var_entity);
      LHS = 0;
      fputs(" = ", cfile);
#endif /* NO_MEMBER_GENERATOR */

      fprintf(cfile, "_DA_(_self, %d, %d, %d, %d, _step);\n",
	      mode_offset,
	      state_offset,
	      flow_offset,
	      context_offset);
#if 0
      /* Generating code to update the CONSTANT part. */
      /* We do not need this, since the value accessed for this
       * variable will always be stored in the 0th position of the
       * "integration" vector, which is always accessed by _DA_.
       * (Also the algebraic part before here could be removed).
       *
       * Marco Antoniotti 19970204
       */
      fprintf(cfile,
	      "  /* Updating Algebraic part for instance variable '%s' */\n",
	      pname(attr(intern("name"), target_variable)));

      fprintf(cfile, "  ");
      fprintf(cfile, "_VAL_(_self, ");
      generate_type(attr(intern("type"), target_variable), cfile);
      fprintf(cfile, ", %d) = ", num(attr(intern("offset"), target_variable)));
      fprintf(cfile, "_DA_(_self, %d, %d, %d, %d, _step);\n",
	      mode_offset,
	      state_offset,
	      flow_offset,
	      context_offset);

#endif /* if 0 */
    }
  else if (current_state_var_mode == intern("algebraic"))
    {
      /* Purely algebraic variables (non differential) do not have the
       * 'state' attribute.
       */
      v_offset = num(attr(intern("offset"), attr(intern("target_variable"), var_entity)));
      mode_offset = num(attr(intern("offset"), attr(intern("mode"), var_entity)));
      flow_offset = num(attr(intern("offset"), attr(intern("flow_function"), var_entity)));
      /* state_offset = num(attr(intern("offset"), attr(intern("state"), var_entity))); */
      context_offset = num(attr(intern("offset"), attr(intern("context"), var_entity)));

      /* If variable may turn into DIFFERENTIAL then generate the
       * assignement.
       */
      if (diff_p)
	{
	  /* Generating code to update the DIFFERENTIAL part. */
	  fprintf(cfile,
	      "  /* Updating Differential part for instance variable '%s' */\n",
	      pname(attr(intern("name"), target_variable)));

	  state_offset = num(attr(intern("offset"), attr(intern("state"), var_entity)));
	  fprintf(cfile, "  ");
	  fprintf(cfile, "_DAL_(_self, %d, %d, %d, %d, _step) = ",
		  mode_offset,
		  state_offset,
		  flow_offset,
		  context_offset);
#ifdef NO_MEMBER_GENERATOR
	  fprintf(cfile, "_AA_(_self, %d, %d, %d, %d, _step);\n",
		  mode_offset,
		  state_offset,
		  flow_offset,
		  context_offset);
#else  /* NO_MEMBER_GENERATOR */
	  member_generator(self_variable, var_entity);
	  fputs(";\n", cfile);
#endif /* NO_MEMBER_GENERATOR */
	}

      /* Generating code to update the CONSTANT part. */
      /* Compare with 'member_generator' output */
      fprintf(cfile,
	      "  /* Updating Constant part for instance variable '%s' */\n",
	      pname(attr(intern("name"), target_variable)));
      fprintf(cfile, "  ");
      fprintf(cfile, "_VAL_(_self, ");
      generate_type(attr(intern("type"), target_variable), cfile);
      fprintf(cfile, ", %d) = ", num(attr(intern("offset"), target_variable)));

#ifdef NO_MEMBER_GENERATOR
      fprintf(cfile, "_AA_(_self, %d, %d, %d, %d, _step);\n",
	      mode_offset,
	      state_offset,
	      flow_offset,
	      context_offset);
#else  /* NO_MEMBER_GENERATOR */
      member_generator(self_variable, var_entity);
      fputs(";\n", cfile);
#endif /* NO_MEMBER_GENERATOR */
    }
  else				/* current mode CONSTANT */
    {
      if (diff_p)
	{
	  v_offset = num(attr(intern("offset"), attr(intern("target_variable"), var_entity)));
	  mode_offset = num(attr(intern("offset"), attr(intern("mode"), var_entity)));
	  flow_offset = num(attr(intern("offset"), attr(intern("flow_function"), var_entity)));
	  state_offset = num(attr(intern("offset"), attr(intern("state"), var_entity)));
	  context_offset = num(attr(intern("offset"), attr(intern("context"), var_entity)));

#if 0
	  /* This piece of code is useless (see similar note above),
	   * since the variable is differential.  Hence its 'VA'
	   * accessible value is never used. (At least we hope so -
	   * check 'member_generator' and its uses.
	   *
	   * Marco Antoniotti 19970204
	   */
	  /* Generating code to update the DIFFERENTIAL part. */
	  fprintf(cfile,
	      "  /* Updating Differential part for instance variable '%s' */\n",
	      pname(attr(intern("name"), target_variable)));
	  fprintf(cfile, "  ");
	  fprintf(cfile, "_DAL_(_self, %d, %d, %d, %d, _step) = ",
		  mode_offset,
		  state_offset,
		  flow_offset,
		  context_offset);
	  fprintf(cfile, "_VA_(_self, ");
	  generate_type(attr(intern("type"), target_variable), cfile);
	  fprintf(cfile, ", %d);\n", num(attr(intern("offset"),
	  target_variable)));
#endif /* if 0 */
	}

      if (alge_p || diff_p)
	{
	  v_offset = num(attr(intern("offset"), attr(intern("target_variable"), var_entity)));
	  mode_offset = num(attr(intern("offset"), attr(intern("mode"), var_entity)));
	  flow_offset = num(attr(intern("offset"), attr(intern("flow_function"), var_entity)));
	  context_offset = num(attr(intern("offset"), attr(intern("context"), var_entity)));
	  /* No state offset for algebraic accessor */

	  /* This update needs to be done iff the variable is truly
	   * algebraic.
	   */
	  if (alge_p && !diff_p)
	    {
	      /* Generating code to update the ALGEBRAIC part. */
	      fprintf(cfile,
		      "  /* Updating Algebraic part for instance variable '%s' */\n",
		      pname(attr(intern("name"), target_variable)));
	      fprintf(cfile, "  ");
#ifdef NO_MEMBER_GENERATOR
	      fprintf(cfile, "_AAL_(_self, %d, %d, %d, %d, _step) = ",
		      mode_offset,
		      v_offset,
		      flow_offset,
		      context_offset);
#else  /* NO_MEMBER_GENERATOR */
	      LHS = 1;
	      member_generator(self_variable, var_entity);
	      LHS = 0;
	      fputs(" = ", cfile);
#endif /* NO_MEMBER_GENERATOR */
	      fprintf(cfile, "_VA_(_self, ");
	      generate_type(attr(intern("type"), target_variable), cfile);
	      fprintf(cfile, ", %d);\n", num(attr(intern("offset"), target_variable)));
	    }
	}
    }
}
#endif /* OLD_GENVALUPDATE */


void
generate_exiting_function(lv* m, lv* td)
{
  lv* vars = attr(intern("defined_variables"), m);
  int u = num(attr(intern("unique"), m));
  
  fprintf(hfile, "void exitingF%d(Component *_self);\n", u);

  fprintf(cfile, "/* Exiting Function for Mode '%s' in Type '%s'\n */\n",
	  pname(attr2(intern("name"), intern("id"), m)),
	  pname(attr2(intern("name"), intern("id"), td)));

  fprintf(cfile, "void\nexitingF%d(Component *_self)\n{\n", u);

  fprintf(cfile, "  /* Update Output Variables */\n");
  dolist (decl, attr(intern("output"), td))
    {
      generate_value_mode_updates(attr(intern("entity"), attr(intern("id"), decl)), vars);
    }
  tsilod;

  fprintf(cfile, "  /* Update State Variables */\n");
  dolist (decl, attr(intern("state"), td))
    {
      generate_value_mode_updates(attr(intern("entity"), attr(intern("id"), decl)), vars);
    }
  tsilod;

  /* Input variables need quite a bit of extra work to get right. I
   * will postpone for the time being.
   */

  fprintf(cfile, "}\n\n");
}


/* This generates a no op.  It is here just in case... */ 
void
generate_dummy_exiting_function(lv* m, lv* td)
{ 
  int u = num(attr(intern("unique"), m));

  fprintf(hfile, "void exitingF%d(Component *_self);\n", u);
  fprintf(cfile, "void\nexitingF%d(Component *_self)\n{ ; }\n\n", u);
}


void generate_assignment(lv *l, lv *r);
void generate_plain_assignment(lv *l, lv *r);
void generate_link_assignment(lv *l, lv *r);

void
generate_init(lv *decl)
{
  lv *x = tl(args(decl));

  if (x) generate_link_assignment(attr(intern("id"), decl), hd(x));
}


/* generate_creation_functions --
 * This function generates the allocation and finalization functions
 * for a type 'td'
 *
 * Note: this function is rather crufty due to the strange machinery
 * required for the creation of the 'global' component.  I believe it
 * should be rewritten, since there is no real reason why the global
 * component should not be created as all the other ones.
 * Marco Antoniotti 19960910
 */
void
generate_creation_functions(lv *td)
{
  int u = num(attr(intern("unique"), td));
  lv *id = attr(intern("id"), td);
  lv *name = attr(intern("name"), id);
  int global = name == intern("global");
  int size = num(attr(intern("data_size"), td));
  /* ALEKS USES THIS FOR SURGERY 2 */
  lv *setup = attr(intern("setup"), td);
  lv *actions, *initializations;
  /* END ALEKS SURGERY 2 */

  /* Printing out some useful comments
   * Marco Antoniotti 19960910
   */
  char comment[160];		/* enough for two lines. */

  /* Special case for global component. */
  char *header_format
    = global ? "Component *%screateGlobal()" : "Component *%screateF%d()";

  sprintf(comment,
	  "\n/* Creation Function for type\n * %s\n */\n",
	  pname(name));

  /* Generation of instance layout allocation function (and other
   * initial settings.
   */

  /* Header file stuff */
  fprintf(hfile, comment);
  fprintf(hfile, header_format, "", u);
  fprintf(hfile, ";\n");

  /* Code file stuff */
  fprintf(cfile, comment);
  fprintf(cfile, header_format, "\n", u);
  fprintf(cfile, "\n{\n");
  fprintf(cfile, "  static int _n = 0;\n");
  fprintf(cfile, "  Component *_self = new(Component);\n");
  fprintf(cfile, "  _self->name = _n++;\n");
  if (global)
    fprintf(cfile, "  _global = _self;\n");
  fprintf(cfile, "  _self->data = (char *) safe_malloc(%d);\n", size);
  fprintf(cfile, "  memset(_self->data, 0, %d);\n", size);
  fprintf(cfile, "  _self->desc = &c_typed%d;\n", u);
  fprintf(cfile, "  _self->mark = 0;\n");
  fprintf(cfile, "  _self->break_flag = 0;\n");
  fprintf(cfile, "  _self->trace_flag = 0;\n");
  fprintf(cfile, "  _self->searching = 0;\n");
  fprintf(cfile, "  _self->allEventsTraced = 0;\n");
  fprintf(cfile, "  _self->allEventsBroken = 0;\n");
  fprintf(cfile, "  _self->printVars = (VariableDescriptor**) NULL;\n");
  fprintf(cfile, "  _self->tracedEvents = (char**) NULL;\n");
  fprintf(cfile, "  _self->brokenEvents = (char**) NULL;\n");

  /*
   * MAK 02-26-97
   * If we have inputs within this td we should set them to current 
   * CONSTANT_MODE.
   * We can not do it later since derived vars may set this input.
   */
  if (attr(intern("input"), td))
    {
      dolist(trvi, attr(intern("input"), td))
	{
	  fprintf(cfile,"  ");
	  generate_flow_info(attr(intern("id"), trvi), intern("mode")); 
	  fprintf(cfile, " = CONSTANT_MODE;\n");
	} tsilod;
    }

  /* Initialize event backpointer synchronization sets. */
  dolist (e, attr(intern("export"), td)) {
    generate_assignment(attr(intern("sync_set"), attr(intern("entity"), e)),
			new_component_set);
  } tsilod;

  /* User initializations. */
#if 0
  /* Removed because of changed scheme */
  mapc(generate_init, attr(intern("state"), td));
  mapc(generate_init, attr(intern("output"), td));
  mapc(generate_init, attr(intern("input"), td));
  mapc(generate_init, attr(intern("global"), td));
#endif

  /* ALEKS SURGERY 2 */
  if (setup)
    {
      initializations = attr(intern("initialize"), setup);
      dolist (i, initializations) {
	generate_link_assignment(arg1(i), arg2(i));
      } tsilod;
    }
  /* END ALEKS SURGERY */


  /* Generation of finalization function (call to 'setup' function of
   * type descriptor).
   */

  /* ALEKS CHANGES */
  /* Split create into two functions such that pass-in arguments can
   * be set.
   */
  if (!global)
    {
      fprintf(cfile, "  return _self;\n}\n\n");
      header_format = "Component* %screate2F%d(Component* _self)";
      fprintf(hfile, header_format, "", u);
      fprintf(hfile, ";\n");
      fprintf(cfile, header_format, "\n", u);
      fprintf(cfile, "\n{\n");
    }
  /* END ALEKS CHANGES */

  /*======
   * Switched with generation of call to setup function.
   */
  /* This needs to be fixed.  Should create a discrete state if there
   * is none.
   */
  {
    lv *d = attr(intern("discrete"), td);
    if (d)
      fprintf(cfile, "  enteringF%d(_self);\n",
	      num(attr(intern("unique"), first(d))));
    else
      fprintf(cfile, "  _self->M = 0;\n");
  }

  /* Setup clauses. */
  if (attr(intern("setup"), td))
    fprintf(cfile, "  setupF%d(_self);\n", u);
  /*
   * Marco Antoniotti 19970205
   *======
   */

  fprintf(cfile, "  _self->live_next = live_components;\n");
  fprintf(cfile, "  live_components = _self;\n");
  fprintf(cfile, "  _self->continuous_next = continuous_components;\n");
  fprintf(cfile, "  continuous_components = _self;\n");
  fprintf(cfile, "  _self->component_next = c_typed%d.component_first;\n", u);
  fprintf(cfile, "  c_typed%d.component_first = _self;\n", u);
  fprintf(cfile, "  return _self;\n}\n\n");
  /*    if (global) fprintf(cfile, "  return _self;\n}\n\n");
	else fprintf(cfile, "  return;\n}\n\n"); */
}


void
generate_plain_assignment(lv *l, lv *r)
{
  fprintf(cfile, "  ");
  LHS = 1;
  generate_expression(l);
  LHS = 0;
  fprintf(cfile, " = ");
  generate_expression(r);
  fprintf(cfile, ";\n");
}


void
generate_assignment(lv *lhs, lv *rhs)
{
  generate_plain_assignment(lhs, rhs);
}


/* Treat links differently from other kinds of assignments, because
 * must reconstruct synchronization backpointers.
 */
void
generate_link_assignment(lv *l, lv *r)
{
#if 0
  lv *type = attr(intern("type"), r);
  lv *o = op(type);

  /* Components and sets of components are the only variables which
   * may be used in external synchronizations.
   */
  if (o == intern("id") || o == intern("set") && op(arg1(type)) == intern("id"))
    {
      lv *var;
      int u;
      char *assigner;
      /* The following conditionals could be avoided if a previous
       * rewrite had transformed x into x(self) for each local
       * variable x.
       */

      if (op(l) == intern("id"))
	{
	  var = attr(intern("entity"), l);
	}
      else if (op(l) == intern("access"))
	{
	  var = attr(intern("accessor"), l);
	}
      else
	{
	  var = arg1(l);
	}
      assigner = o == intern("id") ? "link_assign" : "linkset_assign";
      u = attr(intern("external_events"), var) ? num(attr(intern("unique"), var)) : 0;

      fprintf(cfile, "    %s(", assigner);
      LHS = 1;
      generate_expression(l);
      LHS = 0;
      fprintf(cfile, ", ");
      generate_expression(r);
      fprintf(cfile, ", _self, %s%d);\n",
	      u? "svard" : "", u);
    }
  else
    {
      generate_assignment(l, r);
    }
#endif
  generate_assignment(l, r);
}


/* generate_op_assignment --
 * Generates the appropriate code for the short hand assignments.
 * Note that this function is always called within
 * 'generate_actions_assign'.
 */
void
generate_op_assignment(lv* action)
{
  lv* op_kind   = attr(intern("op"), action);
  lv* modify_op = attr(intern("opassign_type"), action);
  lv* temp_expr_node;		/* I should produce this by
				 * rewriting. But what the heck.
				 */

  if (modify_op == intern("set_single_modify"))
    {
      /* Modifying a set.
       * This might turn out to be rather tricky.
       */
      fputs("  ", cfile);
      LHS = 1;
      generate_expression(arg1(action));
      LHS = 0;
      fputs(" = ", cfile);
      if (op_kind == intern("+"))
	fputs("(Set*) insert_element((void*)&", cfile);
      else
	fputs("(Set*) remove_element((void*)&", cfile);
      generate_expression(attr(intern("temp"), action));
      fputs(", ", cfile);
      generate_expression(arg1(action));
      fputs(");\n", cfile);
    }
  else if (modify_op == intern("set_multi_modify"))
    {
      /* Modifying a set by inserting multiple elements from another.
       * This might turn out to be rather tricky.
       */
      fputs("  FOR_ALL_ELEMENTS(__rhs_set_el, ", cfile);
      generate_expression(attr(intern("temp"), action));
      fputs(") {\n", cfile);
      if (op_kind == intern("+"))
	fputs("    add_to_set(__rhs_set_el, ", cfile);
      else
	fputs("    remove_from_set(__rhs_set_el, ", cfile);
      generate_expression(arg1(action));
      fputs(");\n", cfile);
      fputs("  } END_FAE;\n", cfile);
    }
  else
    {
      /* Regular assignment
       * Note: it should never get here.
       *
       * Marco Antoniotti 19970724
       */
      temp_expr_node = node(attr(intern("op"), action),
			    list2(arg1(action), attr(intern("temp"), action)),
			    nil);
      generate_assignment(arg1(action), temp_expr_node);
    }
}


/* First pass: evaluate all right-hand sides and destinations of
 * assignments into temporaries.
 */
void
generate_actions_rhs(lv *action)
{
  lv *o = op(action);
  lv *c = nil;
  lv *lhs = 0;
  lv *rhs = 0;
  lv *destination_temp = 0;

  /* Generate RHS and destination initializers. */
  if (o == intern("assign") || o == intern("opassign"))
    {
      lhs = arg1(action);
      rhs = arg2(action);
      destination_temp = attr(intern("temp"), lhs);

      generate_assignment(attr(intern("temp"), action), rhs);
      if (destination_temp)
	{
	  generate_assignment(destination_temp, arg1(lhs));
	}
    }
  /* Generate creations --
   * This is not strictly necessary anymore, since we do not allow
   * floating 'create' statements.
   */
  else if (o == intern("create"))
    {
      fprintf(cfile, "    (void) ");
      generate_expression(action);
      fprintf(cfile,";\n");
    }
}


/* Second pass: store temporaries into left-hand sides.
 */
void
generate_actions_assign(lv *action)
{
  lv *o = op(action);
  lv *c = nil;

  /* Standard assignments. */
  if (o == intern("assign"))
    {
      generate_link_assignment(arg1(action), attr(intern("temp"), action));
    }
  else if (o == intern("opassign"))
    {
      generate_op_assignment(action);
    }
}


/* The setup function performs the setup actions.
 */
generate_setup_function(lv *td)
{
  char comment[160];		/* enough for two lines. */
  int u = num(attr(intern("unique"), td));
  lv *setup = attr(intern("setup"), td);
  lv *actions, *definitions, *initializations, *connections;
  lv* type_of_define, *mytvr;

  if (! setup) return;
  initializations = attr(intern("initialize"), setup);
  actions = attr(intern("do"), setup);
  connections = attr(intern("connections"), setup);
  definitions = attr(intern("define"), setup);

  /* Printing out some useful comments
   * Marco Antoniotti 19960910
   */
  sprintf(comment,
	  "\n/* Setup Function for type\n * %s\n */\n",
	  pname(attr(intern("name"), attr(intern("id"), td))));

  fprintf(hfile, comment);
  fprintf(hfile, "void setupF%d(Component *_self);\n", u);

  fprintf(cfile, comment);
  fprintf(cfile, "void\nsetupF%d(Component *_self)\n{\n", u);

  /* ALEKS SURGERY 2 REMOVED THE FOLLOWING
     dolist (i, initializations) {
     generate_link_assignment(arg1(i), arg2(i));
     } tsilod; --- note the change of syntax if uncommenting!!!*/

  dolist (d, definitions) {
    /******** SYNTAX CHANGE  generate_assignment(arg1(d), arg2(d)); *****/ 
    if (tl(args(d)))
      {
	generate_assignment(attr(intern("id"), d), arg2(d));
      }
  } tsilod;
  mapc(generate_actions_rhs, actions);
  mapc(generate_actions_assign, actions);
  /********** SYNTAX CHANGE !!! added for connections clause ****************/
  mapc(generate_actions_rhs, connections);
  mapc(generate_actions_assign, connections);
  /* Set modes and flows */
  dolist (a, actions) {
    if (op(a) == intern("define"))
      {
	type_of_define = type_of(arg1(a));

	fprintf(cfile, "  ");
	generate_flow_info(arg1(a), intern("mode"));
	fprintf(cfile, " = ALGEBRAIC_MODE;\n  ");

	/* The intern("flow_function") symbol may not be correct if we are
	 * using a "flow" tied to a component.
	 * I need to produce the proper tag; however, the way
	 * 'generate_flow_info' works we risk to repeat work in a
	 * useless way.  On top of that I foresee the emergency of
	 * "new" flow function pointers not returning 'double' or
	 * 'Component*' as hardcoded
	 * generate_flow_info, generate_fm and
	 * generate_flow_function_type
	 * Marco Antoniotti 19960910
	 */
	
	generate_flow_info(arg1(a), intern("flow_function"));
	fprintf(cfile, " = flowF%d;\n  ", num(attr(intern("unique"), a)));
	generate_flow_info(arg1(a), intern("context"));
	fprintf(cfile, " = _self;\n");

	/* Marco Antoniotti 19960613 <marcoxaintern("fiat").its.berkeley.edu>
	 * At this point I really need to generate assignments to
	 * the CONTEXT of the 'arg1(a)'.  Later (i.e. here below)
	 * I will need to modify the generation of the flow function
	 * in order reference the proper component.
	 * All of this because I do not have proper closures in C.
	 */
      }
  } tsilod;

  /******** SYNTAX CHANGE: Add connections *****/ 
  dolist (a, connections) {
    if (op(a) == intern("define"))
      {
	mytvr = a;
	type_of_define = type_of(arg1(a));
	fprintf(cfile, "  ");
	generate_flow_info(arg1(a), intern("mode"));
	fprintf(cfile, " = ALGEBRAIC_MODE;\n  ");
	generate_flow_info(arg1(a), intern("flow_function"));
	fprintf(cfile, " = flowF%d;\n  ", num(attr(intern("unique"), a)));
	generate_flow_info(arg1(a), intern("context"));
	fprintf(cfile, " = _self;\n");
      }
  } tsilod;

  fprintf(cfile, "}\n\n");

  /* Generate flow functions. */
  dolist (a, actions) {
    if (op(a) == intern("define"))
      {
	generate_flow_function(attr(intern("unique"), a), a);
      }
  } tsilod;

  /************ SYNTAX CHANGE add for connections *******/
  dolist (a, connections) {
    if (op(a) == intern("define"))
      {
	generate_flow_function(attr(intern("unique"), a), a);
      }
  } tsilod;
}


/* The actions functions perform (in phases) the actions of a
 * transition.
 */
void
generate_actions_function(int phase, int unique, lv *actions, void (*f)(lv *))
{
  /* phase is assumed to be 1 for the right hand-side evaluation and 2
     for the left hand-side evaluation */
  assert(phase == 1 || phase == 2);
  fprintf(hfile, "void action_%s_F%d(Component *_self);\n", 
	  (phase == 1)? "RHS" : "LHS",
	  unique);
  fprintf(cfile, 
	  "/* Evaluate the %s hand-side part of the action.\n */\n",
	  (phase == 1)? "right" : "left");
  fprintf(cfile,
	  "void\naction_%s_F%d(Component *_self)\n{\n", 
	  (phase == 1)? "RHS" : "LHS",
	  unique);
  mapc(f, actions);
  fprintf(cfile, "}\n\n");
}

    
/* Generate functions for the actions of transition T.
 */
void
generate_transition_functions(lv *t)
{
  int u = num(attr(intern("unique"), t));
  lv *actions = attr(intern("do"), t);
  lv *definitions = attr(intern("define"), t);

  /********** MAK     if (! actions) return; ****************************/
  if (definitions)
    {
      fprintf(hfile, "void defineF%d(Component *_self);\n", u);
      fprintf(cfile, "void\ndefineF%d(Component *_self)\n{\n", u);
      dolist (d, definitions) {
	/**** SYNTAX CHANGE	generate_assignment(arg1(d), arg2(d)); ****/
	if (tl(args(d)))
	  {
	    generate_assignment(attr(intern("id"), d), arg2(d));
	  }
      } tsilod;
      fprintf(cfile, "}\n\n");
    }

  if (actions)
    {
      generate_actions_function(1, u, actions, generate_actions_rhs);
      generate_actions_function(2, u, actions, generate_actions_assign);
    }
}



void
generate_event_descriptor(lv *event)
{
  lv *entity = attr(intern("entity"), event);
  lv *type = attr(intern("type"), entity);
  char *kind = nil;
  int u = num(attr(intern("unique"), entity));


  if (equal_type(type, event_type))
    {
      kind = "CLOSED_EVENT_T";
    }
  else if (equal_type(type, open_event_type))
    {
      kind = "OPEN_EVENT_T";
    }
  else
    {
      kind = "UNKNOWN_T";
      user_error(event, "Type of event is not known!");
    }

  /* Emit array of transitions descriptor pointers for this event.
   */
  fprintf(hfile, "extern TransitionDescriptor *tr4ev%d[];\n", u);
  fprintf(cfile, "TransitionDescriptor *tr4ev%d[] = {\n", u);
  dolist (t, attr(intern("meaning"), entity)) {
    fprintf(cfile, "  &trans_desc%d,\n", num(attr(intern("unique"), t)));
  } tsilod;
  fprintf(cfile, "  0\n};\n\n");

  /* Emit event descriptor.
   */
  /* Modifying so that the type of the event is also emitted.
   * 
   * Tunc Simsek 22nd December, 1997
   */
  fprintf(hfile, "extern EventDescriptor event_desc%d;\n", u);
  fprintf(cfile,
	  "EventDescriptor event_desc%d = {\n  \"%s\",\n  tr4ev%d,\n\
  %d,\n  0,\n  0,\n  %s\n};\n\n",
	  u, 
	  pname(attr(intern("name"), event)), 
	  u,
	  num(attr(intern("offset"), attr(intern("sync_set"), entity))),
	  kind);
}


void
generate_ext_event_expression(lv* e, int u, int is_set)
{
  char indent_string[3] = "  ";

  /* Header file */
  fprintf(hfile, "extern void* ext_event_%s_expr_F%d(Component* _self);\n",
	  is_set ? "set" : "single",
	  u);

  /* C file */
  fprintf(cfile, "void*\next_event_%s_expr_F%d(Component* _self)\n{\n",
	  is_set ? "set" : "single",
	  u);
  indent_to(1, indent_string, cfile);
  fprintf(cfile,
	  "return (void*) ");
  generate_expression(arg1(e));
  fputs(";\n}\n\n", cfile);
}


void
generate_external_event_descriptor(lv *e)
{
  int u = num(attr(intern("unique"), e));
  lv *link = attr(intern("entity"), arg1(e));
  lv *ee = attr(intern("entity"), arg2(e));
  int is_set = set_type_p(attr(intern("type"), link));
  lv *rule = attr(intern("sync_type"), e);

  if (rule == nil) rule = intern("one");

  generate_ext_event_expression(e, u, is_set);

  fprintf(hfile, "extern ExternalEventDescriptor ext_event%d;\n", u);
  fprintf(cfile, "ExternalEventDescriptor ext_event%d = {\n", u);
  fprintf(cfile, "  \"%s\",\n", pname(attr(intern("name"), arg2(e))));
  fprintf(cfile, "  %s,\n", is_set ? "SET_CONNECTION" : "SINGLE_CONNECTION");
  fprintf(cfile, "  %d,\n", num(attr(intern("offset"), attr(intern("target_variable"), link))));
  fprintf(cfile, "  %d,\n", attr(intern("kind"), link) == intern("GLOBAL"));
  fprintf(cfile, "  %s,\n", rule == intern("all") ? "SYNC_ALL" : "SYNC_ONE");
  fprintf(cfile,
	  "  %d,\n",
	  nodep(rule) ? num(attr3(intern("offset"), intern("target_variable"), intern("entity"), rule)) : -1);
  fprintf(cfile, "  &event_desc%d,\n", num(attr(intern("unique"), ee)));
  fprintf(cfile, "  \"%s\",\n", pname(attr(intern("name"), arg1(e))));
  fprintf(cfile, "  ext_event_%s_expr_F%d\n",
	  is_set ? "set" : "single",
	  u);
  fputs("};\n\n", cfile);

  /*
  fprintf(cfile, "  %s,\n  %d,\n  %d,\n  %s,\n  %d,\n  &event_desc%d\n};\n\n",
	  is_set ? "SET_CONNECTION" : "SINGLE_CONNECTION",
	  num(attr(intern("offset"), attr(intern("target_variable"), link))),
	  attr(intern("kind"), link) == intern("GLOBAL"),
	  rule == intern("all") ? "SYNC_ALL" : "SYNC_ONE",
	  nodep(rule) ? num(attr3(intern("offset"), intern("target_variable"), intern("entity"), rule))
	  : -1,
	  num(attr(intern("unique"), ee)));
	  */
}


/* Generate transition descriptor for transition T.
 */
void
generate_transition_descriptor(lv *t)
{
  int u = num(attr(intern("unique"), t));
  lv *guard = attr(intern("guard"), t);
  int i;

  /* Emit guard function. */
  if (guard)
    {
      generate_logical_function(u, guard);
    }

  /* Emit external event synchronizations. */
  dolist (e, attr(intern("events"), t)) {
    if (op(e) == intern("external_event"))
      {
	generate_external_event_descriptor(e);
      }
  } tsilod;

  /* Emit array of local event descriptor pointers */
  fprintf(hfile, "extern EventDescriptor *edlist%d[];\n", u);
  fprintf(cfile, "EventDescriptor *edlist%d[] = {\n", u);
  dolist (e, attr(intern("events"), t)) {
    if (op(e) == intern("id"))
      {
	fprintf(cfile, "  &event_desc%d,\n",
		num(attr(intern("unique"), attr(intern("entity"), e))));
      }
  } tsilod;
  fprintf(cfile, "  0\n};\n\n");
    
  /* Emit array of event descriptor pointers. */
  fprintf(hfile, "extern ExternalEventDescriptor *eelist%d[];\n", u);
  fprintf(cfile, "ExternalEventDescriptor *eelist%d[] = {\n", u);
  dolist (e, attr(intern("events"), t)) {
    if (op(e) == intern("external_event"))
      {
	fprintf(cfile, "  &ext_event%d,\n", num(attr(intern("unique"), e)));
      }
  } tsilod;
  fprintf(cfile, "  0\n};\n\n");
    
  /* Emit descriptor. */
  fprintf(hfile, "extern TransitionDescriptor trans_desc%d;\n", u);
  fprintf(cfile, "TransitionDescriptor trans_desc%d = {\n", u);
  fprintf(cfile, "  &mode_desc%d,\n", 
	  num(attr(intern("unique"), meaning(attr(intern("to"), t)))));
  fprintf(cfile, "  edlist%d,\n", u);
  fprintf(cfile, "  eelist%d,\n", u);
  fprintf(cfile, "  %s%d,\n",
	  guard ? "logicalF" : "",
	  guard ? u : 0);
  if (attr(intern("define"), t))
    {
      fprintf(cfile, "  defineF%d,\n", u);
    }
  else
    { 
      fprintf(cfile,"  0,\n"); 
    }

  if (attr(intern("do"), t))
    {
      fprintf(cfile, "  action_RHS_F%d,\n", u);
      fprintf(cfile, "  action_LHS_F%d,\n", u);
   }
  else
    {
      fprintf(cfile, "  0,\n  0\n");
    }
  fprintf(cfile, "};\n\n");
}


/* Generate descriptor for discrete state (mode) D.
 */
void
generate_mode_descriptor(lv *d)
{
  int u = num(attr(intern("unique"), d));
  lv *invariant = attr(intern("invariant"), d);
  lv *transitions = attr(intern("outgoing"), d);
  int g = 0;

  /* Emit array of pointers to outgoing transitions. */
  fprintf(hfile, "extern TransitionDescriptor *outgoing_desc%d[];\n", u);
  fprintf(cfile, "TransitionDescriptor *outgoing_desc%d[] = {", u);
  dolist (t, transitions)
    {
      fprintf(cfile, "\n  &trans_desc%d,", num(attr(intern("unique"), t)));
      if (!g && attr(intern("guard"), t))
	{
	  g = 1;
	  set_attr(intern("has_guards"), d, intern("true"));
	}
    }
  tsilod;
  fprintf(cfile, "\n  0\n};\n\n");

  /* Emit descriptor.
   *
   * Changed in order to emit pointer to exiting function too
   * Marco Antoniotti 19961209
   */
  fprintf(hfile, "extern ModeDescriptor mode_desc%d;\n", u);
  fprintf(cfile, "ModeDescriptor mode_desc%d = {\n", u);
  fprintf(cfile, "  \"%s\",\t\t/* Mode name */\n",
	  pname(attr2(intern("name"), intern("id"), d)));
  fprintf(cfile, "  diff_var_desc%d,\t/* Pointer to differential var */\n",
	  u);
  fprintf(cfile, "  %d,\t\t\t/* Guarded? */\n", g);
  fprintf(cfile, "  %s%d,%s\t\t/* Invariant Function Pointer*/\n",
	  invariant ? "logicalF" : "",
	  invariant ? u : 0,
	  invariant ? "" : "\t");
  fprintf(cfile, "  enteringF%d,\t\t/* Entering Function Pointer */\n", u);
  fprintf(cfile, "  exitingF%d,\t\t/* Exiting Function Pointer */\n", u);
  fprintf(cfile, "  outgoing_desc%d\t/* Outgoing Transitions */\n", u);
  fprintf(cfile, "};\n\n");
}


void
generate_sync_variable_descriptor(lv *entity)
{
  int u = num(attr(intern("unique"), entity));

  fprintf(hfile, "extern ExternalEventDescriptor *svard%d[];\n", u);
  fprintf(cfile, "ExternalEventDescriptor *svard%d[] = {\n", u);
  dolist (ee, attr(intern("external_events"), entity)) {
    fprintf(cfile, "  &ext_event%d,\n", num(attr(intern("unique"), ee)));
  } tsilod;
  fprintf(cfile, "  0\n};\n\n");
}


void
generate_vardesc(lv *v)
{
  char *name = pname(attr(intern("name"), v));
  char *kind = pname(attr(intern("kind"), v));
  int offset = num(attr(intern("offset"), v));
  lv *type = attr(intern("type"), v);
  int ut = num(attr(intern("unique"), attr(intern("hashed_type"), type)));
  lv *dif = attr(intern("differential"), v);
  lv *alg = attr(intern("algebraic"), v);
  char *possible_modes =
    dif ? "DIFFERENTIAL" : alg ? "ALGEBRAIC" : "CONSTANT";
  int mo = dif || alg ? num(attr(intern("offset"), attr(intern("mode"), v))) : 0;
  int ffo = dif || alg ? num(attr(intern("offset"), attr(intern("flow_function"), v))) : 0;
  int cxo = dif || alg ? num(attr(intern("offset"), attr(intern("context"), v))) : 0;
  int so = dif? num(attr(intern("offset"), attr(intern("state"), v))) : 0;
    
  fprintf(cfile, 
	  "  {\"%s\", %s_KIND, %d, &typed%d, %s_MODE, %d, %d, %d, %d},\n",
	  name, kind, offset, ut, possible_modes, mo, ffo, cxo, so);
}


int
ctd_has_ext_evs_synchs(lv* td)
{
  dolist (tr, attr(intern("transition"), td))
    {
      dolist (e, attr(intern("events"), tr))
	{
	  if (op(e) ==  intern("external_event"))
	    return 1;
	}
      tsilod;
    }
  tsilod;
  return 0;
}

/* For every SHIFT component type, emit a run-time type descriptor,
 * consisting of structure initializations, and a bunch of functions
 * which implement the computation of derivatives, algebraic
 * definitions, invariants, guards, and actions.
 */
void
generate_component_type_descriptor(lv *td)
{
  int cache_ctd_hash_code(char*);
  int unique = num(attr(intern("unique"), td));
  lv *parent_id;
  lv *parent;
  char *parent_info = (char *)malloc(13);
  lv *children_id;
  lv *child_id;
  lv *child;
  char *children_info = (char *)malloc(13);
  

  /*************************** SYNTAX CHANGE !!!!!! ************************/
  lv *setup_actions = attr_do_setup(td);
  lv *setup_actions1 = attr_do_setup(td);

  /* Emit setup function.
   */
  generate_setup_function(td);

  /* Emit flow functions for the flows.
   */
  dolist (f, attr(intern("flow"), td))
    {
      dolist (eq, args(f))
	{
	  if (op(eq) != intern("id"))
	    {
	      generate_flow_function(attr(intern("unique"), eq), eq);
	    }
	}
      tsilod;
    }
  tsilod;

  /* Emit invariant function, entering_function, flow functions, and
   * array of variable mode descriptors for each discrete state.
   */
  dolist (m, attr(intern("discrete"), td))
    {
      lv *invariant = attr(intern("invariant"), m);
      int mu = num(attr(intern("unique"), m));

      if (invariant)
	{
	  generate_logical_function(mu, invariant);
	}
      dolist (eq, attr(intern("equations"), m))
	{
	  if (op(eq) != intern("id"))
	    {
	      generate_flow_function(attr(intern("unique"), eq), eq);
	    }
	}
      tsilod;
      fprintf(hfile,
	      "extern DifferentialVarDescriptor diff_var_desc%d[];\n", mu);
      fprintf(cfile,
	      "DifferentialVarDescriptor diff_var_desc%d[] = {\n", mu);
      dolist (c, attr(intern("defined_variables"), m))
	{
	  lv *entity = hd(c);
	  lv *eq = tl(c);
	  if (op(eq) == intern("equate"))
	    {
	      fprintf(cfile, "  {%d, flowF%d},\n",
		      num(attr(intern("offset"), attr(intern("state"), entity))),
		      num(attr(intern("unique"), eq)));
	    }
	}
      tsilod;
      fprintf(cfile, "  {-1, 0}\n};\n\n");
    }
  tsilod;

  /* Emit mode descriptors.
   */
  mapc(generate_mode_descriptor, attr(intern("discrete"), td));
  /* Emit list of pointers to mode descriptors.
   */
  fprintf(cfile, "ModeDescriptor *modes%d[] = {\n", unique);
  dolist (m, attr(intern("discrete"), td))
    {
      fprintf(cfile, "  &mode_desc%d,\n", num(attr(intern("unique"), m)));
    }
  tsilod;
  fprintf(cfile, "  0\n};\n\n");

  mapcx(generate_entering_function, attr(intern("discrete"), td), td);

  /* Generating the exiting function.
   * Change to 'generate_dummy_exiting_function' to disable the feature.
   * Marco Antoniotti 19961209
   */
  mapcx(generate_exiting_function, attr(intern("discrete"), td), td);
  /* mapcx(generate_dummy_exiting_function, attr(intern("discrete"), td), td); */

  /* Emit transition functions. */
  mapc(generate_transition_functions, attr(intern("transition"), td));

  /* Emit transition descriptors. */
  mapc(generate_transition_descriptor, attr(intern("transition"), td));

  /* I believe that the following call is useless.  The generated
   * lists are not used anywhere.
   *
   * Marco Antoniotti and Misha Kourjanski 19970519
   */
  /* Emit sync variable descriptors. */
  /* mapc(generate_sync_variable_descriptor, attr(intern("sync_variables"), td)); */

  /* Emit variable descriptors. */
  fprintf(hfile, "extern VariableDescriptor var_desc%d[];\n", unique);
  fprintf(cfile, "VariableDescriptor var_desc%d[] = {\n", unique);
  fprintf(cfile, "  /* Public List */\n");
  dolist (v, attr(intern("public_list"), td))
    {
      generate_vardesc(v);
    }
  tsilod;
  fprintf(cfile, "  /* Private List */\n");
  dolist (v, attr(intern("private_list"), td))
    {
      generate_vardesc(v);
    }
  tsilod;
  fprintf(cfile, "  {0, NO_KIND, -1, 0, 0, 0, 0, 0}\n};\n\n");

  /* Emit **only** exported events. */
  dolist (e, attr(intern("export"), td))
    {
      generate_event_descriptor(e);
    }
  tsilod;

  /* Emit array of event descriptor pointers */
  fprintf(cfile, "EventDescriptor *events%d[] = {\n", unique);
  dolist (e, attr(intern("export"), td))
    {
      fprintf(cfile, "  &event_desc%d,\n",
	      num(attr2(intern("unique"), intern("entity"), e)));
    }
  tsilod;
  fprintf(cfile, "  0\n};\n\n");

  /* Emit sync setups */
  /*********************** SYNTAX CHANGE !!! *****************/
  dolist (a, setup_actions)
    {
      if (op(a) == intern("sync"))
	{
	  int u = num(attr(intern("unique"), a));
	  fprintf(cfile,
		  "ComponentEventPairDescriptor cepd%d[] = {\n", u);
	  dolist (ce, args(a))
	    {
	      lv *c = arg1(ce);
	      lv *e = arg2(ce);
	      fprintf(cfile,
		      "  {%d, &event_desc%d},\n",
		      num(attr3(intern("offset"), intern("target_variable"), intern("entity"), c)),
		      num(attr2(intern("unique"), intern("entity"), e)));
	    }
	  tsilod;
	  fprintf(cfile, "  {-1, 0}};\n\n");
	}
    }
  tsilod;

  fprintf(cfile, "ExternalSyncDescriptor sync_setup%d[] = {\n", unique);
  dolist (a, setup_actions)
    {
      if (op(a) == intern("sync"))
	{
	  int u = num(attr(intern("unique"), a));
	  fprintf(cfile, "  cepd%d,\n", u);
	}
    }
  tsilod;
  fprintf(cfile, "  0\n};\n\n");

  /* Compute parent information (if any) */
  parent_id = attr(intern("parent"), td);
  if (parent_id)
    {
      parent = meaning(parent_id);
      sprintf(parent_info, "&c_typed%d,", num(attr(intern("unique"), parent)));
    }
  else
    strcpy(parent_info, "0,\t");

  /* Compute information and generate children list (if nay) */
  children_id = attr(intern("children"), td);
  sprintf(children_info, "children%d,", num(attr(intern("unique"), td)));
  fprintf(cfile, 
	  "ComponentTypeDescriptor *children%d[] = {\n", 
	  num(attr(intern("unique"), td)));
  if (children_id)
    {
      dolist (child_id, children_id) {
	child = meaning(child_id);
	fprintf(cfile, "  &c_typed%d,\n", num(attr(intern("unique"), child)));
      } tsilod;
    }
  fprintf(cfile, "  0\n};\n\n");

  /* Emit type descriptor itself. */
  fprintf(hfile, "extern ComponentTypeDescriptor c_typed%d;\n",
	  unique);

  fprintf(cfile, "ComponentTypeDescriptor c_typed%d = {\n", unique);
  fprintf(cfile, "  \"%s\",", pname(attr2(intern("name"), intern("id"), td)));
  fprintf(cfile, "\t/* Type name */\n");
  fprintf(cfile, "  %d,", num(attr(intern("data_size"), td)));
  fprintf(cfile, "\t\t/* Data size */\n");
  fprintf(cfile, "  events%d,", unique);
  fprintf(cfile, "\t/* Events list pointer name */\n");
  fprintf(cfile, "  var_desc%d,", unique);
  fprintf(cfile, "\t/* Instance var descriptors list pointer name */\n");
  fprintf(cfile, "  modes%d,", unique);
  fprintf(cfile, "\t/* Mode (state) descriptors pointer name */\n");
  fprintf(cfile, "  sync_setup%d,", unique);
  fprintf(cfile, "\t/* External events list pointer name */\n");
  fprintf(cfile, "  0,");
  fprintf(cfile, "\t\t/* Value of break_flag */\n");
  fprintf(cfile, "  0,");
  fprintf(cfile, "\t\t/* Value of trace_flag */\n");
  fprintf(cfile, "  0,");
  fprintf(cfile, "\t\t/* List of printed instance variable. */\n");
  fprintf(cfile, "  0,");
  fprintf(cfile, "\t\t/* 'All' or 'some' traced events flag */\n");
  fprintf(cfile, "  0,");
  fprintf(cfile, "\t\t/* 'All' or 'some' broken events flag */\n");
  fprintf(cfile, "  %s", parent_info);
  fprintf(cfile, "\t/* Parent type (if any) */\n");
  fprintf(cfile, "  %s", children_info);
  fprintf(cfile, "\t/* Children type list (if any) */\n");
  fprintf(cfile, "  0,");
  fprintf(cfile, "\t\t/* List of components */\n");
#ifdef HAVE_NEW_HASH_P
  fprintf(cfile, "  %d,", cache_ctd_hash_code(pname(attr2(intern("name"), intern("id"), td))));
  fprintf(cfile, "\t\t/* Cached hash code for type */\n");
#endif /* HAVE_NEW_HASH_P */
  fprintf(cfile, "  %d", ctd_has_ext_evs_synchs(td));
  fprintf(cfile, "\t\t/* Does the type synchronizes on external events? */\n");
  fprintf(cfile, "};\n\n");

  generate_creation_functions(td);
}


int
cache_ctd_hash_code(char* ctd_name)
{
  int acc = 0;
  char* cursor = ctd_name;

  while (*cursor != '\0')
    {
      acc += (int)(*cursor);
      cursor++;
    }
  return acc;
}


/* Compute all the transitions exiting from a discrete state.
 */
void
compute_outgoing_transitions(lv *td)
{
  lv *states = attr(intern("discrete"), td);
  lv *transitions = attr(intern("transition"), td);
  lv *x = nil;

  /* Compute the transitions leaving each state. */
  dolist (t, transitions) {
    lv *from_list = args(attr(intern("from"), t));

    dolist (f, from_list) {
      lv *d = meaning(f);

      push_attr(intern("outgoing"), d, t);
    } tsilod;
  } tsilod;
}


/* Compute all places in which a variable is used for synchronization.
 */
void
compute_sync_variable_uses(lv *td)
{
  dolist(tr, attr(intern("transition"), td)) {
    dolist(e, attr(intern("events"), tr)) {
      if (op(e) == intern("external_event"))
	{
	  lv *link_entity = attr(intern("entity"), arg1(e));
	  push_attr(intern("external_events"), link_entity, e);
	  if (! node_marked_p(link_entity))
	    {
	      mark_node(link_entity);
	      push_attr(intern("sync_variables"), td, link_entity);
	    }
	}
    } tsilod;
  } tsilod;
}


void
generate_function_declaration(lv *fd)
{
  lv *formals = attr(intern("formals"), fd);

  generate_type(attr(intern("return_type"), fd), hfile);
  fprintf(hfile, " %s(", pname(attr2(intern("name"), intern("id"), fd)));
  if (formals)
    {
      generate_type(arg1(hd(formals)), hfile);
      dolist (f, tl(formals)) {
	fprintf(hfile, ", ");
	generate_type(arg1(f), hfile);
      } tsilod;
    }
  fprintf(hfile, ");\n");
}


/* Traverse expression E, assign a unique number to subexpressions
 * that are to be implemented as functions.  Save the subexpression
 * nodes in *PLIST.
 *
 * Log:
 * Modified to take into account 'setcons2' and 'arraycons2'.
 * Marco Antoniotti 19970101
 */
void
find_subexpressions(lv *e, lv **plist)
{
  lv *o = op(e);

  /* intern("special_form") does not necessarily need the subexpression to be
   * tagged.  As a matter of fact, this really needs to be a little
   * more sophisticated.  intern("count") and intern("find") do not need the set former
   * function function to be generated, while intern("choose") does.
   *
   * Marco Antoniotti 19970701
   */
  if (o == intern("exists")
      || o == intern("minel")
      || o == intern("maxel")
      || o == intern("setcons2")
      || o == intern("arraycons2")
      || o == intern("special_form")
      || (o == intern("create") && tl(args(e))))
    {
      push(e, *plist);
      set_attr(intern("unique"), e, fixnum(unique_counter++));
    }
  dolist (a, args(e))
    {
      if (nodep(a))
	find_subexpressions(a, plist);
      else if (consp(a))
	{
	  dolist (l_element, a)
	    {
	      find_subexpressions(l_element, plist);
	    }
	  tsilod;
	}
      else
	internal_error("find_subexpressions: node argument is not a list or a node.");
    }
  tsilod;
}


/* generate_create_expr_function --
 * Generate the function which implements a specific 'create' function
 * call.
 */
void
generate_create_expr_function(lv *e)
{
  int u = num(attr(intern("unique"), e));
  fprintf(hfile, "extern Component *create_expr_X%d(Component *);\n", u);
  fprintf(cfile, "/* create expression function\n");
  fprintf(cfile, " * type: %s\n", pname(attr(intern("name"), arg1(e))));
  fprintf(cfile, " * at:   %s:%d\n",
	  str(attr(intern("file"), e)),
	  num(attr(intern("line"), e)));
  fprintf(cfile, " */\n");
  fprintf(cfile, "Component *\ncreate_expr_X%d(Component *_self)\n{\n", u);
  fprintf(cfile, "  Component *_new = ");

  /* This form is really the allocation step of the 'create'
   * expression. Hence the name change.
   * create_generator2(e);
   * Marco Antoniotti 19960910
   */
  generate_create_allocation_call(e);

  fprintf(cfile, ";\n");
  dolist (a, tl(args(e))) {
    lv *e = attr2(intern("entity"), intern("id"), a);
    lv *x = node(intern("access"), list1(new_variable), alist1(intern("accessor"), e));

    /* At this point we really need to figure out what kind of
     * assignment we have.  More specifically, we need to add some
     * type information to 'arg1(a)' in order to have the proper
     * type in case of empty set or empty array assignment.
     * Marco Antoniotti 19960726
     */
    generate_assignment(x, arg1(a));
  } tsilod;
  /* NEXT LINES ADDED BY ALEKS */
  fprintf(cfile, "  ");

  /* This form is really the generation of the call to the function
   * in charge of completenig the setup of the component.  Hence the
   * name change.
   * create_generator3(e);
   * Marco Antoniotti 19960910
   */
  generate_create_setup_call(e);

  fprintf(cfile, ";\n");
  /* END ALEKS LINES */
  fprintf(cfile, "  return _new;\n}\n\n");
}


/* generate_subfunctions --
 * Generates special functions for the special operators and the
 * iterator constructors of SHIFT.
 *
 * Modified
 * Marco Antoniotti 19970101
 */
void
generate_subfunction(lv *q)
{
  if (op(q) == intern("exists"))
    generate_exists_function(q);
  else if (op(q) == intern("minel"))
    generate_minmax_function(q, 0);
  else if (op(q) == intern("maxel"))
    generate_minmax_function(q, 1);
  else if (op(q) == intern("create") && tl(args(q)))
    generate_create_expr_function(q);
  else if (op(q) == intern("setcons2"))
    generate_set_former_function(q);
  else if (op(q) == intern("arraycons2"))
    generate_array_former_function(q);
  else if (op(q) == intern("reduce"))
    generate_reduce_form_function(q);
  else if (op(q) == intern("special_form"))
    generate_special_form_function(q);
  else
    assert(0);
}


/* Create one function for certain subexpressions (exists, minel,
 * maxel, create) in the program.
 */
void
generate_subfunctions(lv *program)
{
  lv *subexpressions = nil;
  dolist (n, program) {
    if (op(n) == intern("typedef"))
      {
	do_all_expressions(e, n, find_subexpressions(e, &subexpressions));
      }
  } tsilod;
  mapc(generate_subfunction, subexpressions);
}


void
find_symbols(lv *e, lv **symbol_list)
{
  lv *o = op(e);

  if (o == intern("symbolic"))
    {
      lv *n = attr(intern("name"), e);
      lv *p = plist(n);
    
      if (! assoc(intern("symbol_seen"), p))
	{
	  push(e, *symbol_list);
	  plist(n) = acons(intern("symbol_seen"), intern("true"), p);
	}
    }
  else
    {
      dolist (a, args(e))
	{
	  if (nodep(a))
	    find_symbols(a, symbol_list);
	  else if (consp(a))
	    {
	      dolist (l_element, a)
		{
		  find_symbols(l_element, symbol_list);
		}
	      tsilod;
	    }
	  else
	    internal_error("find_symbols: expression argument is neither a node nor a list.");
	}
      tsilod;
    }
}


void
generate_symbols(lv *program)
{
  lv *symbol_list = nil;	/* all the symbols of the SHIFT program */
  const int first = 1000;	/* number of the first symbol */
  int i = first;		/* number of the current symbol */
  int size;			/* size of the symbol table */

  /* Record all the symbols in the list SYMBOL_LIST */
  dolist (n, program) {
    if (op(n) == intern("typedef"))
      {
	do_all_expressions(e, n, find_symbols(e, &symbol_list));
      }
  } tsilod;

  /* Print a #define for each symbol in SYMBOL_LIST */
  dolist (s, symbol_list) {
    fprintf(hfile, "#define ");
    fprintf(hfile, "_SYMBOL_%s", pname(attr(intern("name"), s)));
    fprintf(hfile, " %d\n", i++);
  } tsilod;

  /* Print a table entry for each symbol in SYMBOL_LIST */
  size = i - first;
  fprintf(cfile, "const int first_shift_symbol = %d;\n\n", first);
  fprintf(cfile, "const int last_shift_symbol = %d;\n\n", first+size);
  fprintf(cfile, "char *shift_symbol_table[%d] = {\n", 
	  size ? size : 1);


  if (size == 0)
    fprintf(cfile, "0");

  dolist (s, symbol_list)
    {
      fprintf(cfile, "  \"$%s\"", pname(attr(intern("name"), s)));
      if (--size)
	fputs(",\n", cfile);
      else
	fputc('\n', cfile);
    }
  tsilod;
  fprintf(cfile, "};\n\n");
}


char *
string_for_type(lv *type, char *s, char *end)
{
  lv *x = op(type);
  char c;

  if (x == intern("number_type"))
    c = 'n';
  else if (x == intern("symbol_type"))
    c = 'y';
  else if (x == intern("logical_type"))
    c = 'l';
  else if (x == intern("array") || x == intern("empty_array"))
    c = 'a';
  else if (x == intern("set") || x == intern("null_set"))
    c = 's';
  else if (x == intern("id"))
    {
      *s++ = '.';
      strcpy(s, pname(attr2(intern("name"), intern("id"), meaning(type))));
      s += strlen(s);
      c = '.';
    }
  else
    {
      c = 'u';	/* unknown or uninteresting type */
    }
  *s++ = c;
  if (s >= end) internal_error("type too large");
  if (c == 'u') return s;
  dolist (a, args(type)) {
    s = string_for_type(a, s, end);
  } tsilod;
  return s;
}


/* Return a unique symbol for the set of types structurally equivalent
 * to TYPE.
 */
lv *
symbol_for_type(lv *type)
{
  char buf[100];
  char *s = string_for_type(type, buf, buf+100);
  *s = '\0';
  return intern(buf);
}


void
hash_type(lv *type, lv **plist)
{
  lv* sy;
  lv* pr;

  if (! attr(intern("hashed_type"), type))
    {
      sy = symbol_for_type(type);
      pr = assoc(intern("hash"), plist(sy));

      if (pr)
	{
	  set_attr(intern("hashed_type"), type, tl(pr));
	}
      else
	{
	  apush(intern("hash"), type, plist(sy));
	  set_attr(intern("hashed_type"), type, type);
	  push(type, *plist);
	}

      dolist (a, args(type)) {
	hash_type(a, plist);
      } tsilod;
    }
}


/* Hash constructors types.  The descriptors are used in the generic
 * implementation of collections (sets + arrays).
 *
 * Log:
 * Modified to take into account the new set iterators.
 */
void
hash_cons_types(lv *e, lv **ptypes)
{
  if (op(e) == intern("arraycons")
      || op(e) == intern("arraycons2")
      || op(e) == intern("setcons")
      || op(e) == intern("setcons2"))
    {
      hash_type(attr(intern("type"), e), ptypes);
    }
  else if (op(e) == intern("special_form"))
    {
      /* special form nodes may have a second arg equal to 'nil' when
       * the default is not specified.
       *
       * Marco Antoniotti 19970728
       */
      hash_cons_types(arg1(e), ptypes);
      if (attr(intern("sfid"), e) != intern("count")
	  && length(args(e)) > 1
	  && arg2(e) != nil)
	{
	  hash_cons_types(arg2(e), ptypes);
	}
      return;
    }

  dolist (a, args(e))
    {
      if (nodep(a))
	hash_cons_types(a, ptypes);
      else if (consp(a))
	{
	  dolist (l_element, a)
	    {
	      hash_cons_types(l_element, ptypes);
	    }
	  tsilod;
	}
      else
	internal_error("hash_cons_types: node argument is not a list or a node.");
    }
  tsilod;
}


void
generate_type_descriptor(lv *type)
{
  char *kind;
  int td = -1;
  int ctd = -1;
  int u = num(attr(intern("unique"), type));
  lv *x = op(type);

  fprintf(hfile, "extern TypeDescriptor typed%d;\n", u);
  fprintf(cfile, "TypeDescriptor typed%d = {\n", u);

  if (x == intern("number_type")) kind = "NUMBER_T";
  else if (x == intern("symbol_type")) kind = "SYMBOL_T";
  else if (x == intern("logical_type")) kind = "LOGICAL_T";
  else if (x == intern("array") || x == intern("empty_array"))
    {
      kind = "ARRAY_T";
      td = num(attr(intern("unique"), attr(intern("hashed_type"), arg1(type))));
    }
  else if (x == intern("set") || x == intern("null_set"))
    {
      kind = "SET_T";
      td = num(attr(intern("unique"), attr(intern("hashed_type"), arg1(type))));
    }
  else if (x == intern("id"))
    {
      /* Added support for external data types.
       *
       * Tunc Simsek 15th April, 1998
       */
      lv *exttype = attr(intern("exttypedef"), attr(intern("type"), type));

      if(exttype && exttype == intern("true"))
	{
	  kind = "FOREIGN_T";
	}
      else
	{
	  kind = "COMPONENT_T";
	  ctd = num(attr(intern("unique"), meaning(type)));
	}
    }
  else
    {
      kind = "UNKNOWN_T";
    }
  fprintf(cfile, "  %s,\n  %s%d,\n  %s%d\n};\n\n",
	  kind,
	  td == -1 ? "" : "&typed", td == -1 ? 0 : td,
	  ctd == -1 ? "" : "&c_typed", ctd == -1 ? 0 : ctd);
}


/* Emit type descriptors for some type trees:
 *
 * 1. the types of all variables in the program.
 *
 * 2. the types of array and set constructors.
 *
 * Minimize the number of descriptors by avoiding repetitions.
 */
void
generate_type_descriptors(lv *program)
{
  lv *types = nil;
  int i = 0;

  dolist (n, program) {
    if (op(n) == intern("typedef"))
      {

	dolist (v, attr(intern("private_list"), n)) {
	  fflush(stdout);
	  hash_type(attr(intern("type"), v), &types);
	} tsilod;
	dolist (v, attr(intern("public_list"), n)) {
	  fflush(stdout);
	  hash_type(attr(intern("type"), v), &types);
	} tsilod;
      }
    do_all_expressions(e, n, hash_cons_types(e, &types));
  } tsilod;

  dolist (t, types) {
    set_attr(intern("unique"), t, fixnum(i++));
  } tsilod;

  mapc(generate_type_descriptor, types);
}


void
generate_component_type_descriptor_list(lv *program)
{
  fprintf(cfile, "ComponentTypeDescriptor *component_type_list[] = {\n");
  dolist (n, program) {
    if (op(n) == intern("typedef"))
      {
	/* Added support for external data types.
	 * 
	 * Tunc Simsek 15th April, 1998
	 */
	lv *id = attr(intern("id"), n);
	lv *type = id ? attr(intern("type"), id) : nil;
	lv *exttypedef = type ? attr(intern("exttypedef"), type) : nil;
	
	if ( ! exttypedef || exttypedef != intern("true"))
	  {
	  fprintf(cfile, "  &c_typed%d,\n", num(attr(intern("unique"), n)));
	  }
      }
  } tsilod;
  fprintf(cfile, "  0\n};\n\n");
}


/* compute_component_type_descriptor_children -- This function sets
 * the attribute intern("children") of each component type that is a parent type.
 * Alain Girault 19970109 */
void
compute_component_type_descriptor_children(lv *program)
{
  lv *parent_id;
  lv *parent;
  lv *child_id;
  lv *old_children_id;
  lv *new_children_id;

  /* Build the list types of component types, and set the attribute
     intern("children") of each parent (if any). This attribute is a list of
     identifiers that is built incrementally. */
  dolist (child, program) {
    if (op(child) == intern("typedef"))
      {
	parent_id = attr(intern("parent"), child);
	if (parent_id)
	  {
	    parent = meaning(parent_id);
	    child_id = attr(intern("id"), child);
	    old_children_id = attr(intern("children"), parent);
	    new_children_id = cons(child_id, old_children_id);
	    set_attr(intern("children"), parent, new_children_id);
	  }
      }
  } tsilod;
}


/* print_component_type_descriptor_forest -- This function traces the
   hierarchy of component type descriptors of the SHIFT program. Each
   component type descriptor is printed with its parent (if any) and
   the list of its children (if any). This function is only used for
   tracing.
   Alain Girault 19970109 */
void
print_component_type_descriptor_forest(lv *program)
{
  lv *parent_id;
  lv *parent;
  lv *children_id;
  lv *child;

  printf("\nComponent type forest = {\n");
  dolist (n, program) {
    if (op(n) == intern("typedef"))
      {
	printf("  %s (c_typed%d)\n", 
	       pname(attr(intern("name"), attr(intern("id"), n))), 
	       num(attr(intern("unique"), n)));
	parent_id = attr(intern("parent"), n);
	if (parent_id)
	  {
	    parent = meaning(parent_id);
	    printf("    parent is %s (c_typed%d)\n",
		   pname(attr(intern("name"), parent_id)),
		   num(attr(intern("unique"), parent)));
	  }
	children_id = attr(intern("children"), n);
	if (children_id)
	  {
	    printf("    children are");
	    dolist (child_id, children_id) {
	      child = meaning(child_id);
	      printf(" %s (c_typed%d)", 
		     pname(attr(intern("name"), child_id)), 
		     num(attr(intern("unique"), child)));
	    } tsilod;
	    printf("\n");
	  }
      }
  } tsilod;
  printf("}\n\n");
}


void
initialize_generate()
{
#define gen(op, f) apush(intern("generator"), other((void *) f), plist(intern(#op)))
#define gen1(op)	gen(op, #op "_generator")
#define genlib(op)	gen(op, library_call_generator)
  gen(+,		binmath_generator);
  gen(*,		binmath_generator);
  gen(-,		binmath_generator);
  gen(/,		binmath_generator);
  gen(>,		binmath_generator);
  gen(>=,		binmath_generator);
  gen(<,		binmath_generator);
  gen(<=,		binmath_generator);
  gen1(negate);
  gen1(not);
  gen1(and);
  gen1(or);
  genlib(set_union);
  genlib(set_difference);
  genlib(set_intersection);
  gen(=,		equality_generator);
  gen(link-=,		equality_generator);
  gen(set-=,		set_equality_generator);
  gen1(self_variable);
  gen1(global_variable);
  gen1(new_variable);
  gen1(id);
  gen1(entity);
  gen1(variable);
  gen1(access);
  gen1(int);
  gen1(float);
  gen1(if);
  gen(exists,		exists_call_generator);
  gen(minel,		minmax_call_generator);
  gen(maxel,		minmax_call_generator);
  gen(symbolic,  	symbol_generator);
  gen1(create);
  gen1(narrow);
  gen1(max);
  gen1(min);
  gen1(atan2);
  gen1(random);
  gen1(call);

  /* Modified generation of set membership function
   *
   * Marco Antoniotti 19970612
   */
#ifndef HAVE_NEW_HASH_P
  genlib(in);
#else
  gen(in,               set_membership_generator);
#endif /* HAVE_NEW_HASH_P */

  gen(setcons,		cons_generator);
  gen(arraycons,	cons_generator);
  /* Added generators for 'setcons2' and 'arraycons2'.
   * Marco Antoniotti 19970101
   */
  gen(setcons2,		set_former_generator);
  gen(arraycons2,	array_former_generator);
  gen1(index);
  gen1(verbatim);
  genlib(set_size);
  genlib(array_size);

  /* Added generators for 'setcons2' and 'arraycons2'.
   * Marco Antoniotti 19970619
   */
  gen(special_form,	special_form_generator);

  /* Added generators for 'copy_array' and 'copy_set'.
   * Alain Girault 19970116
   */
  gen(copy_array,       copy_array_generator);
  gen(copy_set,         copy_set_generator);

  /* Added generator for 'narrow'.
   * Alain Girault 19970122
   */
  gen(narrow,           narrow_generator);

#undef gen
#undef gen1
#undef genlib

  self_variable = node(intern("self_variable"), nil, nil);
  global_variable = node(intern("global_variable"), nil, nil);
  new_variable = node(intern("new_variable"), nil, nil);
  component_set_type = node(intern("set"), list1(node(intern("component"), nil, nil)), nil);
  /* zero = node(intern("int"), nil, alist1(intern("value"), fixnum(0)));
   * Useless
   * Marco Antoniotti 19971023
   */
  new_component_set =
    node(intern("verbatim"), nil,
    alist1(intern("code"), string("new_set(&generic_component_type)")));
}


char *edit_warning = "\
/* SHIFT compiler output.\n\
 * Read at your own risk.\n\
 * FBI Warning: Do Not Edit Under Any Circumstance.\n\
 * If you edit this file I hope you know what you are doing.\n\
 */\n";


/* Get rid of directory part of filename */
char*
generate_include_file_name(char* h_filename)
{
  char* true_h_file_name = rindex(h_filename, '/');

  if (true_h_file_name != 0)
    {
    if (true_h_file_name[1] != '\0')
      return true_h_file_name + 1 ; /* Very big assumption */
    else
      {
	fprintf(stderr, "shic: malformed file name for H file: %s\n",
		h_filename);
	exit(1);
      }
    }
  else
    return h_filename;
}


void
generate(lv *program)
{
  /* If an output variable in a type declaration has
   * algebraic/differential mode, make all occurrences of that
   * variable in the inheritance chain also have that mode.
   *
   * This is a two pass process.  In the first pass, propagate the
   * mode up to the parents.  In the second, propagate it down to
   * the children.
   */
  dolist (n, program) {
    if (op(n) == intern("typedef")) {
      uniformize_modes_up(n);
    }
  } tsilod;

  /* There is no need to propagate a mode down to 
   * its children.
   *
   * Tunc Simsek 24th November, 1997
   *
   * dolist (n, program) {
   *  if (op(n) == intern("typedef"))
   *    {
   *      uniformize_modes_down(n);
   *    }
   *  } tsilod;
   */

  new_node_marker();
  dolist (n, program) {
    if (op(n) == intern("typedef"))
      {
	compute_member_lists(n);
	compute_sync_variable_uses(n);
	substitute_flows(n);
	assign_unique_numbers(n);
	initialize_member_offsets(n);
      }
  } tsilod;

  dolist (n, program) {
    if (op(n) == intern("typedef"))
      {
	compute_member_offsets(n);
      }
  } tsilod;

  /* Begin spewing code.
   */
  fprintf(cfile, "%s\n\
#include <stdio.h>\n\
#include <math.h>\n\
#include <signal.h>\n\n\
#include \"shifttypes.h\"\n\
#include \"collection.h\"\n\
#include \"built-ins.h\"\n\
#include \"shift_debug.h\"\n\
#include \"ffi-support.h\"\n\
#include \"%s\"\n\
int _step = 0;\n\
\n\
", edit_warning, generate_include_file_name(hfilename));

  fprintf(hfile, "%s\n", edit_warning);

  /******* MAK ****** renaming the abs function to fabs ****/
  fprintf(hfile,"\n#define abs(x) fabs(x)");

  /* Note: 'rint' seems to be defined only in true UNIX systems.
   * 'rint' does not seem to be part of the ANSI/IEEE suite, so MS
   * does not care to implement it.
   *
   * Marco Antoniotti 19970826
   */
#ifndef OS_MSWINDOWS
  fprintf(hfile,"\n#define round(x) rint(x)\n\n");
#endif

  generate_type_descriptors(program);
  generate_subfunctions(program);
  generate_symbols(program);
  compute_component_type_descriptor_children(program);
  /*  print_component_type_descriptor_forest(program);*/

  dolist (n, program) {
    if (op(n) == intern("typedef"))
      {
	/* Added support for external data types.
	 * 
	 * Tunc Simsek 15th April, 1998
	 */
	lv *id = attr(intern("id"), n);
	lv *type = id ? attr(intern("type"), id) : nil;
	lv *exttypedef = type ? attr(intern("exttypedef"), type) : nil;

	if ( ! exttypedef || exttypedef != intern("true"))
	  {
	    compute_outgoing_transitions(n);
	    generate_component_type_descriptor(n);
	  }
      }
    else if (op(n) == intern("ext_fun_decl"))
      {
	/* generate_function_declaration(n); Old FFI */
	generate_foreign_function_code(n);
      }
  } tsilod;

  generate_component_type_descriptor_list(program);

  fprintf(hfile, "\n\n/* end of file -- %s -- */\n", hfilename);
  fprintf(cfile, "\n\n/* end of file -- %s -- */\n", cfilename);
}

#endif /* COMPILER_GEN_I */

/* end of file -- generate.c -- */
