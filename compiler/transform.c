/* -*- Mode: C -*- */

/* transform.c -- */

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

/* Transformations to simplify code generation and/or to optimize
 * emitted code.
 */

#include <stdio.h>
#include <assert.h>

/* #include "lisp.h" */
#include "crscl.h"

#include "shic.h"

/* lv *zero;
 * Useless
 * Marco Antoniotti 19971023
 */
lv *null_symbol;

/* Constant folding.
 */

/* Is E a literal?
 */
int
literalp(lv *e)
{
  lv *o = op(e);

  if (o == intern("setcons") || o == intern("arraycons"))
    {
      dolist (a, args(e))
	{
	  if (!literalp(a)) {
	    return 0;
	  }
	}
      tsilod;
      return 1;
    }
  else
    return o == intern("semis")
           || o == intern("id") && op(attr(intern("type"), e)) == intern("state_type")
           || o == intern("int")
           || o == intern("float")
           || o == intern("string")
           || o == intern("symbol");
}


lv *
compute_state_set_op(lv *o, lv *l, lv *r)
{
  lv *result_list;

  assert(op(l) == intern("setcons"));
  assert(op(r) == intern("setcons"));
  if (o == intern("set_union"))
    {
      result_list = args(l);
      dolist (re, args(r))
	{
	  lv *rentity = attr(intern("entity"), re);
	  int found = 0;

	  dolist (le, args(l))
	    {
	      lv *lentity = attr(intern("entity"), le);

	      if (rentity == lentity)
		{
		  found = 1;
		  break;
		}
	    }
	  tsilod;
	  if (! found) push(re, result_list);
	}
      tsilod;
    }
  else if (o == intern("intersection"))
    {
      result_list = nil;
      dolist (re, args(r))
	{
	  lv *rentity = attr(intern("entity"), re);
	  int found = 0;

	  dolist (le, args(l))
	    {
	      lv *lentity = attr(intern("entity"), le);

	      if (rentity == lentity)
		{
		  found = 1;
		  break;
		}
	    }
	  tsilod;
	  if (found) push(re, result_list);
	}
      tsilod;
    }
  else if (o == intern("difference"))
    {
      result_list = nil;
      dolist (le, args(l))
	{
	  lv *lentity = attr(intern("entity"), le);
	  int found = 0;

	  dolist (re, args(r))
	    {
	      lv *rentity = attr(intern("entity"), re);

	      if (rentity == lentity)
		{
		  found = 1;
		  break;
		}
	    }
	  tsilod;
	  if (! found) push(le, result_list);
	}
      tsilod;
    }
  return node(intern("setcons"), result_list, alist1(intern("type"), stateset_type));
}


lv *
fold(lv *e)
{
  if (literalp(e))
    return e;
  else if (op(e) == intern("id"))
    {
      return meaning(e);
    }
  else
    {
      lv *o = op(e);
      lv *t = attr(intern("type"), e);

      if ((o == intern("set_union") || o == intern("set_difference") || o == intern("set_intersection"))
	  && op(t) == intern("set")
	  && op(arg1(t)) == intern("state_type"))
	{
	  lv *l = fold(arg1(e));
	  lv *r = fold(arg2(e));

	  if (l == nil || r == nil)
	    {
	      return nil;
	    }
	  else return compute_state_set_op(o, l, r);
	}
      else
	{
	  /* Add here other folding code as needed.
	   */
	  return nil;
	}
    }
}


void
transform_typedef(lv *td)
{
  /* Move all initializations into the intern("initialize") attribute of the
   * intern("setup") node.  Initialize all variables to some default.
   */
  lv *setup = attr(intern("setup"), td);
  lv *init = nil;

  if (! setup)
    {
      setup = node(intern("setup"), nil, nil);
      set_attr(intern("setup"), td, setup);
    }

  dolist (d, attr(intern("input"), td))
    {
      if (tl(args(d)))
	push(node(intern("assign"), list2(attr(intern("id"), d), arg2(d)), nil), init);
    }
  tsilod;

  dolist (d, attr(intern("output"), td))
    {
      if (tl(args(d)))
	push(node(intern("assign"), list2(attr(intern("id"), d), arg2(d)), nil), init);
    }
  tsilod;

  dolist (d, attr(intern("state"), td))
    {
      if (tl(args(d)))
	push(node(intern("assign"), list2(attr(intern("id"), d), arg2(d)), nil), init);
    }
  tsilod;

  dolist (d, attr(intern("global"), td))
    {
      if (tl(args(d)))
	push(node(intern("assign"), list2(attr(intern("id"), d), arg2(d)), nil), init);
    }
  tsilod;

  set_attr(intern("initialize"), setup, nreverse(init));

  /* Original (Marco Antoniotti 19960528)
   * This does not quite work.  It might be the case that the
   * external event is specified as "S:alpha" with S a set variable.
   * In this case we find 'nil' in 'sync_type' and the system breaks
   * downstream because no variable is generated.
   *
   * Possible fixes:
   * 1 - fix it here by always forcing the extra variable.
   * 2 - signal a 'type error' when no variable or no 'ONE' or 'ALL'
   *     specifier is present.
   *
   * Choice 2 seems better. The change is done in 'check.c'.
   */
    
  /* Make all external synchronizations of the type S : alpha (one)
   * have a sync variable.
   */

  dolist (tr, attr(intern("transition"), td))
    {
      dolist (e, attr(intern("events"), tr))
	{
	  if (op(e) == intern("external_event"))
	    {
	      lv *sync_type = attr(intern("sync_type"), e);

	      if (sync_type && sync_type == intern("one"))
		{
		  lv *etype = arg1(attr(intern("type"), arg1(e)));
		  lv *y = node(intern("entity"), nil, alist1(intern("type"), etype));
		  lv *a = alist3(intern("type"), etype, intern("name"), intern("SY"), intern("entity"), y);
		  lv *id = node(intern("id"), nil, a);

		  set_attr(intern("sync_type"), e, id);
		}
	    }
	} tsilod;
    } tsilod;

}


void
initialize_transform()
{

  /* zero = node(intern("int"), nil, alist2(intern("value"), intern("0"), intern("type"), number_type));
   * Useless
   * Marco Antoniotti 19971023
   */
  null_symbol = node(intern("null_symbol"), nil, alist1(intern("type"), symbol_type));
}
	

void
transform(lv *program)
{
  dolist (n, program)
    {
      if (op(n) == intern("typedef"))
	transform_typedef(n);
    }
  tsilod;
}

/* end of file -- transform.c -- */
