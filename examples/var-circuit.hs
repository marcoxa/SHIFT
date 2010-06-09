/* -*- Mode: C -*- */

/* var-circuit.hs -- */

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
/* The RCCircuit has input vin, which changes depending on discrete
   state up or down.
   It has a time constant tau for charging/discharging.
   While it is active the RCCircuit executes the tick event every one
   sencond. 

   In this example the Monitor creates a new RCCircuit every 5 seconds

   You can use TkShift to plot vc of RCCircuits, 
   You can also animate them, use i as the alternate cooridante
   

*/

global Monitor  myM := create(Monitor);


type Monitor {

	input RCCircuit b;
	setup do { b := create(RCCircuit, i:= count); count := count+1;}
        state continuous number x;
		number count;

	export traceon, traceoff;

	discrete active {x' = 1}, idle {x' = 0} ;

	transition
		active -> active {b:dump} when x >= 5
			do {x :=  0; count := count + 1;
			    b := create(RCCircuit, i := count);};

	}

type RCCircuit {

	export dump, tick;
        input number i;
	output continuous number vc:=0, vin, z, tao := 1;
	flow default {z' = 1, vc' = vin  - tao*vc}
 	discrete up {vin = 3}, 
		down {vin = -3}, 
		dead {vc' = 0, z' =0};
	transition up -> down {} when vc >= 2,
		   down -> up {} when vc <= -2,
		   up -> up {tick} when z > 1 do { z:= 0;},
		   down -> down {tick} when z > 1 do { z:= 0;},
		   up -> dead {dump} do {vc := 0;},
		   down -> dead {dump} do {vc := 0;},
}
