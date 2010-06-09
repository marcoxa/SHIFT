/* -*- Mode: C -*- */

/* particle.hs -- */

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
#define nextBroadcastTime 5

// For simplicity we hardcode the value of nextBroadcastTime
// Alternatively a C function can calculate and reurn random number

global Monitor monitor
	:= create(Monitor, speed := 100, duration := nextBroadcastTime);
global Source source := create(Source, monitor := monitor);
global set(Particle) secondHalfParticles := {};

type Particle
{
  state continuous number x;
  number y := 1;
  output number speed;

  discrete
    firstHalf { x' = speed },
    secondHalf { x' = speed };

  export raiseSpeed, notifyExit;

  transition

    firstHalf -> secondHalf {}
      when x >= 500
      do {
        secondHalfParticles := secondHalfParticles + {self};
      };

    secondHalf -> secondHalf {raiseSpeed}
      do {
        speed := 1.1*speed;
      };

    secondHalf -> exit {notifyExit}
      when x >= 1000
      do {
        secondHalfParticles := secondHalfParticles - {self};
      };
}

type Source
{
  state continuous number x;
	Monitor monitor;
        Particle new_part;

  discrete start { x' = 1 };

  transition

    start -> start {}
      when x >= 1
      do {
        new_part := create(Particle, speed := speed(monitor));
        x := 0;
      };
}

type Monitor
{
  output number speed;

  state continuous number x;
        number duration;

  discrete start { x' = 1 }

  transition

    start -> start {secondHalfParticles:notifyExit(one:p)}
      do {
        speed := 0.5*(speed + speed(p));
      };

    start -> start {secondHalfParticles:raiseSpeed(all)}
      when x >= duration
      do {
        x := 0;
        duration := nextBroadcastTime;
      };
}

