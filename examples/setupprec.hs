/* -*- Mode: C -*- */

/* setupprec.hs -- */

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
/* This example illustrates the precedence rules in the setup actions
   You can go into the debugger to see the results
 
   Print the components in the command line debugger to see what has
   happened 

   This program also illustrates incorrect use of SHIFT.
   Component B0 has a dangling input. 

   It is the programmer responsibility to set the link (reference)
   variables properly

*/



global A a0 := create(A, myB := create(B));
global A a1 := create(A);


type A {  
    state B myB, otherB;
          C myC, otherC;
	  number b := 1;
	  number c := 2;
    setup define
	{
	// Define statements are executed sequentially
	C temp := create(C);
	C temp2 := temp;  // This gets value of temp
	}


    do {
	// First all create actions are executed, i.e the B and C 
        // Components are created
	myB := create(B);
	myC := temp; // This gets value of temp

	otherB := myB; // This gets old value of myB
        otherC := temp2; // This gets the value of temp2

	// Then all other right hand sides are computed
	b := 5;
	c := b; // This gets old value of b

	// Then all left hand sides are assigned values

    }
    connect {
        // Finally all connections are established
	xIn(myB) <- xOut(myC);      
    }

}


type B {
    input continuous number xIn;
}


type C {
    output continuous number xOut;
    discrete d {xOut' = 1}
}
