/* -*- Mode: C -*- */

/* transform.c -- */

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
/* Transformations to simplify code generation and/or to optimize
 * emitted code.
 */

#include <stdio.h>
#include <assert.h>
#include "lisp.h"
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

  if (o == @setcons || o == @arraycons)
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
    return o == @semis
           || o == @id && op(attr(@type, e)) == @state_type
           || o == @int
           || o == @float
           || o == @string
           || o == @symbol;
}


lv *
compute_state_set_op(lv *o, lv *l, lv *r)
{
  lv *result_list;

  assert(op(l) == @setcons);
  assert(op(r) == @setcons);
  if (o == @set_union)
    {
      result_list = args(l);
      dolist (re, args(r))
	{
	  lv *rentity = attr(@entity, re);
	  int found = 0;

	  dolist (le, args(l))
	    {
	      lv *lentity = attr(@entity, le);

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
  else if (o == @intersection)
    {
      result_list = nil;
      dolist (re, args(r))
	{
	  lv *rentity = attr(@entity, re);
	  int found = 0;

	  dolist (le, args(l))
	    {
	      lv *lentity = attr(@entity, le);

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
  else if (o == @difference)
    {
      result_list = nil;
      dolist (le, args(l))
	{
	  lv *lentity = attr(@entity, le);
	  int found = 0;

	  dolist (re, args(r))
	    {
	      lv *rentity = attr(@entity, re);

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
  return node(@setcons, result_list, alist1(@type, stateset_type));
}


lv *
fold(lv *e)
{
  if (literalp(e))
    return e;
  else if (op(e) == @id)
    {
      return meaning(e);
    }
  else
    {
      lv *o = op(e);
      lv *t = attr(@type, e);

      if ((o == @set_union || o == @set_difference || o == @set_intersection)
	  && op(t) == @set
	  && op(arg1(t)) == @state_type)
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
  /* Move all initializations into the @initialize attribute of the
   * @setup node.  Initialize all variables to some default.
   */
  lv *setup = attr(@setup, td);
  lv *init = nil;

  if (! setup)
    {
      setup = node(@setup, nil, nil);
      set_attr(@setup, td, setup);
    }

  dolist (d, attr(@input, td))
    {
      if (tl(args(d)))
	push(node(@assign, list2(attr(@id, d), arg2(d)), nil), init);
    }
  tsilod;

  dolist (d, attr(@output, td))
    {
      if (tl(args(d)))
	push(node(@assign, list2(attr(@id, d), arg2(d)), nil), init);
    }
  tsilod;

  dolist (d, attr(@state, td))
    {
      if (tl(args(d)))
	push(node(@assign, list2(attr(@id, d), arg2(d)), nil), init);
    }
  tsilod;

  dolist (d, attr(@global, td))
    {
      if (tl(args(d)))
	push(node(@assign, list2(attr(@id, d), arg2(d)), nil), init);
    }
  tsilod;

  set_attr(@initialize, setup, nreverse(init));

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

  dolist (tr, attr(@transition, td))
    {
      dolist (e, attr(@events, tr))
	{
	  if (op(e) == @external_event)
	    {
	      lv *sync_type = attr(@sync_type, e);

	      if (sync_type && sync_type == @one)
		{
		  lv *etype = arg1(attr(@type, arg1(e)));
		  lv *y = node(@entity, nil, alist1(@type, etype));
		  lv *a = alist3(@type, etype, @name, @SY, @entity, y);
		  lv *id = node(@id, nil, a);

		  set_attr(@sync_type, e, id);
		}
	    }
	} tsilod;
    } tsilod;

}


void
initialize_transform()
{

  /* zero = node(@int, nil, alist2(@value, intern("0"), @type, number_type));
   * Useless
   * Marco Antoniotti 19971023
   */
  null_symbol = node(@null_symbol, nil, alist1(@type, symbol_type));
}
	

void
transform(lv *program)
{
  dolist (n, program)
    {
      if (op(n) == @typedef)
	transform_typedef(n);
    }
  tsilod;
}

/* end of file -- transform.c -- */
