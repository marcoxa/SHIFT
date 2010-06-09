/* -*- Mode: C -*- */

/* linklist.hs -- */

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
/* This example illustrates how a link list can be created in SHIFT
   It also illustrates the use the define and do statements.
   The new ListNode id is stored in the temporary variable temp in the
   define statement and used on the rhs in the do statement.

   You can use the online debugger or TkShift to get lists of
   components and see the connections

*/


global Creator myC := create(Creator);

type Creator {

   state continuous number a := 0;
         ListNode firstNode;
         ListNode currentNode; 

   setup do { currentNode := firstNode; }

   flow default {a' = 1};

   discrete createfirst, loop;

   transition
   createfirst -> loop {} 
     when a > 1 
     define { ListNode temp := create(ListNode);}
     do { firstNode := temp; 
          a:=0;
          currentNode:= temp;};

   loop -> loop {} 
     when a > 1 
     define { ListNode temp := create(ListNode);}
     do { nextN(currentNode) := temp; 
          a:=0;
          currentNode:= temp;};

}


type ListNode {

    input ListNode nextN;
    output X listElem;

}

type X {}
