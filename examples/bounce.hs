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

The program simulates 3 balls bouncing in a box of a size 50x50.
To compile the program use command "gmake bounce.sim".
To run an animation the following settings should be chosen:
rectangular canvas 500x500 and the diameter of the balls 15.
The animation engine takes x,y coordinates of the balls
from the x,y variables in the type Ball.

*/
 

global SV sv := create(SV);   // global monitor; creates balls

global set(Ball) balls := {}; //  global set of balls (initially empty)




type Ball                     // type definition
{
 output 
  continuous number 
		   x, y,      // coordinates
		   vxo, vyo,  // velocities before collision
		   vx, vy;    // current velocities

 state 
  Ball btmp;                  // pointer to the ball colliding with us
  continuous number tm, spr;  // internal timeouts

 discrete 
	   fly {flying;},      // regular motion
           depart {flying;},   // after collision
           flyx {flying;},     // beyond x-boundary of the box
           flyy {flying;},     // beyond y-boundary of the box
           flyxy {flying;},    // beyond corners of the box
           flyc {tmr;};        // collision in progress

 flow                         // ODE for regular motion and timers with rate=1
  flying {
   x' = vx; 
   y' = vy;
   tm'=1;
   spr'=1;
  },

  tmr {
   tm'=1;
  };

 transition
                              // the following two - restrict the velocity
  fly -> fly {}
   when vx > 40 
   do {vx := vx -1;},

  fly -> fly {}
   when vy > 40
   do {vy := vy -1;},

                              // the following two - crossing box' boundary
 fly -> flyy {}
  when 
   (y <=0 or y >=50) and 
   ( x >0 and x <50)
  do 
   { vy := -vy + (vy/8.0) * (random()-0.5); },  // reflect from the boundary

 fly -> flyx {}
  when 
   (x <=0 or x >= 50) and 
   y >0 and y <50
  do 
   { vx := -vx + (vx/8.0) * (random()-0.5); },  // reflect from the boundary

			      // the following three - going beyond box' corners
 fly -> flyxy{}
  when (x <0 or x >50) and (y < 0 or y > 50)
  do { vx := -vx; vy := -vy;},

 flyy -> flyxy {}
  when x <0 or x > 50
  do {vx := -vx ; },

 flyx -> flyxy {}
  when y <0 or y > 50
  do {vy := -vy ; },


			      // the following three -  back to normal
 flyy -> fly {}
  when y >0.5 and y < 49.5,

 flyx -> fly {}
  when x >0.5 and x <49.5,

 flyxy -> fly {}
  when x >0.5 and x <49.5 and y >0.5 and y <49.5,

			      // collision detected
 fly -> flyc {}
  when 
   exists bl in balls :       // if the following ball exists
   (spr > 0.1) and            // time separating collisions
   (not (bl = self)) and      // not ourselves
   ((x(bl) -x)*(x(bl)-x) + 
   (y(bl) -y)*(y(bl)-y) ) <8  // touching us (collision distance)
  do {
	 vxo := vx;           // remember current speeds
	 vyo := vy;
	 btmp:=bl;            // remember that other ball
	 tm :=0;              // reset timers
	 spr := 0;
   },
            
			      //  colliding pair matched; let them separate
  flyc -> depart {}
   when tm > 0.01             // pair matched 
			      // (small time step allows balls 
			      // to detect each other)
   do {
     vx := vxo(btmp);       // reassign velocities(point unit masses collision)
     vy := vyo(btmp); 
     tm :=0;                  // reset timer
   },

			      //  back to normal
 depart -> fly {}
  when tm > 0.01;             // let everybody recalculate velocities
   

}

/////////////////////////////////////////////////

                              // type definition for ball generator
type SV {

 state 
   continuous number x:=0;

 discrete 
    evol {x'=1;};

 transition
    evol -> evol {}
    when 
     x>1 and                 // balls generation is separated in time
     size(balls) <3          // do not exceed 3 balls!
    do { x:=0; 
	 balls := balls + 
		  { create(Ball, 
			   x:=1+48*random(),            // random position
			   y:=1+48*random(),
			   vx:=20+10*(random()-0.5),    // random speed
			   vy:=20+10*(random()-0.5))
		  } ; 
        };
}






