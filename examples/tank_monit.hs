/* -*- Mode: C -*- */

/* tank_monit.hs -- */

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
/*  This propgram does not work in current version of SHIFT.
    the event connections
       myT:turn_off <-> myM:turn_off;
       myM:turn_on <-> myT:turn_on; 
    are not properly implemented.

    Here is a tank and a monitor
    The tank is either on (filling) or off (emptying). 
    The monitor controls the tank and turns it on or off
    The Compose type puts the two systems together.
    Note that a tank component by itself would fall into an infitie
    loop of transitions, the monitor would not have a meaningful w.
    This is a simple example of hierarchical design. 
    tank and monitor are "subsystem"s that can be comined to create
    more complesx strutures.
    The Compose could have easily used monitor2, and the tank would
    care less.

    Use TkShift to plot w in tank.

*/


global Compose i := create(Compose);

type Compose {

	state tank myT := create(tank);
	      monitor myM := create(monitor);
	      setup connect { w(myM) <- w(myT); 
		myT:turn_off <-> myM:turn_off;
		myM:turn_on <-> myT:turn_on; 
	}
	discrete a;
}

type tank {
  output continuous number w := 1;
  export turn_on, turn_off;
  discrete on {w' = 1;},
           off {w' = -2;};

  transition on -> off {turn_off},
             off -> on {turn_on};
}

type monitor {
  export turn_on, turn_off;
  input continuous number w;

  discrete on,
           off;

  transition on -> off {turn_off} when w > 12,
             off -> on {turn_on} when w < 1;
}

type monitor2 {
  export turn_on, turn_off;
  input continuous number w;

  discrete on,
           off;

  transition on -> off {turn_off} when w > 9,
             off -> on {turn_on} when w < 1;
} 
