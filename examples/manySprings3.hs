/* -*- Mode: C -*- */

/* manySprings3.hs -- */

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
/* A simple example that illustrates incorrect use of set
   synchronization.
   The s->s transition in observer is possible iff one spring can
   execute its s->s transition. Furthermore, the transition in observer
   must be synchronized with one and only one spring.
   The transition in spring must synchronize with the observer, which
   means that for the observer to take the s->s transition all springs
   must execute the s->s transition.
   As a result NO transitions are possible.

   You can use TkShift to plot the springs' x value
   You can use breaktype to see that there are no transitions

*/

global observer m := create(observer);

global set(spring) S := { create(spring, f := 0.81233),
			  create(spring, f := 1.98937),
			  create(spring, f := 3.67589) };

type observer {
    discrete s;
    export alfa;
    transition s -> s { S:e(one:p), alfa };
} 

type spring {
    output continuous number x := -1, v := 0, f;
        
    discrete s {
	x' =  v;
	v' = -x * f;
    }
    export e;
    transition s -> s { e, m:alfa} when x > .7 do { x := -x; }
}
