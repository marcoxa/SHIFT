/* -*- Mode: C -*- */

/* integrate.c -- */

/* 
 * Copyright (c)1996, 1997, 1998 The Regents of the University of
 * California (Regents). All Rights Reserved. 
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for educational, research, and not-for-profit
 * purposes, without fee and without a signed licensing agreement, is
 * hereby granted, provided that the above copyright notice, this
 * paragraph and the following two paragraphs appear in all copies,
 * modifications, and distributions. 
 * 
 * Contact The Office of Technology Licensing, UC Berkeley, 2150
 * Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510)
 * 643-7201, for commercial licensing opportunities. 
 * 
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE. 
 *   
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
 * DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
 * REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS. 
 */

/* -*- Mode: C -*- */

/* integrate.c -- ODE integration. */

#ifndef INTEGRATE_I
#define INTEGRATE_I

#include "shift_config.h"

#include <math.h>

#include "shifttypes.h"
#include "collection.h"
#include "shiftlib.h"

#define   TINY      1.0e-25
#define   PGROW    -0.2
#define   PSHRINK  -0.25
#define   FCOR      0.066666666666
#define   SAFETY    0.9

#define   ERRCON    6.0e-4
#define   VS_EPS    1.0e-6
#define   SCALMIN   1.0e-2


#define MAXISTEP    0.1
#define MINISTEP    1.0e-4


extern double h;
extern int ISVARSTEP;
extern long nsteps;
double CurrentTime=0; 

/* It would be more efficient to maintain a set of all components
 * that have some differentiable variables in their current state.
 * FOR_ALL_COMPONENTS(c, differential_components) {
 */

#define iterate								     \
    for (c = continuous_components; c; c = c->continuous_next) {	     \
	if (c->M) FOR_ALL_DIFFERENTIAL_VARIABLES(v, c) 

#define end								     \
	END_FADV;							     \
    }

/***************************************************************************/
void
rungekutta_var()
{
  double hh, htry, errmax, temp, xx0, xx1;
  long i;

  Component *c;
  if (before_advance_function) before_advance_function();

  /* X(c,v,0) - init. point */
  /* X(c,v,1) - init. point derivative */

  iterate {
    X(c,v,1) = F(c,v,0);
  } end;


  /* X(c,v,12) - scaling factor */
  iterate {
    xx0=(X(c,v,0)<0.0)?-X(c,v,0):X(c,v,0);
    xx1=(X(c,v,1)<0.0)?-X(c,v,1):X(c,v,1);
    if((xx0<SCALMIN) &&(xx1<SCALMIN)) 
      X(c,v,12)=SCALMIN;
    else
      X(c, v, 12) =  xx0 + xx1*h + TINY; 
  } end;

  htry=(h<MINISTEP)?MINISTEP:h;

  for(;;){

    hh=htry/2.0;

    /* RK4_(hh)_1 */
    iterate {
      X(c,v,2) = X(c,v,0) + hh/2.0 * X(c,v,1);
    } end;
    iterate {
      X(c,v,5) = F(c, v, 2);
      X(c, v, 3) = X(c,v,0) + hh/2.0 * X(c,v,5);
    } end;
    iterate {
      X(c,v,6) = F(c,v,3);
      X(c,v,4) = X(c,v,0) + hh*X(c,v,6);
      X(c,v,7) = X(c,v,5) + X(c,v,6);
    } end;
    iterate {
      X(c,v,8) =
	X(c,v,0) + hh/6.0 * (X(c,v,1) + F(c,v,4) + 2 * X(c,v,7));
    } end;

    /* RK4_(hh)_2 */
    iterate {
      X(c,v,9) = F(c,v,8);
      X(c,v,2) = X(c,v,8) + hh/2.0 * X(c,v,9);
    } end;
    iterate {
      X(c,v,5) = F(c, v, 2);
      X(c, v, 3) = X(c,v,8) + hh/2.0 * X(c,v,5);
    } end;
    iterate {
      X(c,v,6) = F(c,v,3);
      X(c,v,4) = X(c,v,8) + hh*X(c,v,6);
      X(c,v,7) = X(c,v,5) + X(c,v,6);
    } end;
    iterate {
      X(c,v,10) =
	X(c,v,8) + hh/6.0 * (X(c,v,9) + F(c,v,4) + 2 * X(c,v,7));
    } end;

    /* RK4_(h) */
    iterate {
      X(c,v,2) = X(c,v,0) + htry/2.0 * X(c,v,1);
    } end;
    iterate {
      X(c,v,5) = F(c, v, 2);
      X(c, v, 3) = X(c,v,0) + htry/2.0 * X(c,v,5);
    } end;
    iterate {
      X(c,v,6) = F(c,v,3);
      X(c,v,4) = X(c,v,0) + htry*X(c,v,6);
      X(c,v,7) = X(c,v,5) + X(c,v,6);
    } end;
    iterate {
      X(c,v,11) =
	X(c,v,0) + htry/6.0 * (X(c,v,1) + F(c,v,4) + 2 * X(c,v,7));
    } end;

    /*  Now X(c,v,10) contains result of 2 half-steps.
     *      X(c,v,11) contains result of 1 step.
     *      Comparing these we eval. the step to be taken.
     */
    errmax=0.0;

    iterate {
      X(c,v,11) = X(c,v,10) - X(c,v,11);
      temp=X(c,v,11)/X(c,v,12);
      temp=(temp<0)?-temp:temp;
      if (errmax<temp) errmax=temp;
    } end;

    errmax = errmax / VS_EPS;

    if(errmax <=1.0){
      /*
       * htry=(errmax>ERRCON)?SAFETY*htry*exp(PGROW*log(errmax)):4.0*htry;
       */
      htry=(errmax>ERRCON)?SAFETY*htry*pow(errmax,PGROW):4.0*htry;
      h=(htry>MAXISTEP)?MAXISTEP:htry;
      CurrentTime=CurrentTime+h;
      break;
    }

    htry=SAFETY*htry*pow(errmax,PSHRINK);

    if(htry<MINISTEP) {
      h=MINISTEP;
      CurrentTime=CurrentTime+h;
      /** should be run-time warning! **/
      break;
    }

  } /** end for(;;) **/
 
  iterate {
    X(c,v,0)=X(c,v,10)+X(c,v,11)*FCOR;
  } end;

  if (dump_continuous_state) printf("%d %e %e",tclick,h, CurrentTime);
  tclick++;
  iterate {
    if (dump_continuous_state) printf(" %e", X(c,v,0));
  } end;

  if (dump_continuous_state) printf("\n");
  if (tclick == nsteps) exit(0);

  if (after_advance_function) after_advance_function();
}


/***************************************************************************/
void
rungekutta_fix()
{

  Component *c;
  if (before_advance_function) before_advance_function();


  iterate {
    F0(c, v) = F(c, v, 0);
    X(c, v, 1) = X(c, v, 0) + h/2 * F0(c, v);
  } end;

  iterate {
    F1(c, v) = F(c, v, 1);
    X(c, v, 2) = X(c, v, 0) + h/2 * F1(c, v);
  } end;

  iterate {
    F2(c, v) = F(c, v, 2);
    X(c, v, 3) = X(c, v, 0) + h * F2(c, v);
    DYM(c, v) = F1(c, v) + F2(c, v);
  } end;

  iterate {
    XNEW(c, v) =
      X(c, v, 0) + h/6 * (F0(c, v) + F(c, v, 3) + 2 * DYM(c, v));
  } end;
     
  CurrentTime=CurrentTime+h;

  if (dump_continuous_state) printf("%d %e %e",tclick,h, CurrentTime);
  tclick++;
  iterate {
    X(c, v, 0) = XNEW(c, v);
    if (dump_continuous_state) printf(" %e", XNEW(c, v));
  } end;

  if (dump_continuous_state) printf("\n");
  if (tclick == nsteps) exit(0);

  if (after_advance_function) after_advance_function();
}

/***************************************************************************/
void
continuous_step()
{
  if (ISVARSTEP)
    rungekutta_var();
  else
    rungekutta_fix();
}


#endif /* INTEGRATE_I */


/* end of file -- integrate.c */
