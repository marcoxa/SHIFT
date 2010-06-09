/* -*- Mode: C -*- */

/* ffi-support.c -- */

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

/* Foreign Function Interface Support */

#ifndef FFI_SUPPORT_I
#define FFI_SUPPORT_I

#include <shift_config.h>
#include <limits.h>
#include <signal.h>
#include "shifttypes.h"
#include "collection.h"


/* shift_ffi_signal_filter */

void
shift_ffi_signal_filter(int sig)
{
  char signame[8];

  if (sig == SIGSEGV)
    strcpy(signame, "SIGSEGV");
  else if (sig == SIGBUS)
    strcpy(signame, "SIGBUS");

  runtime_error("FFI: caught signal %s while calling a foreign function",
		signame);
}


/* shift_ffi_cvt_D_array_1D_C -- Converts and copies a number SHIFT
 * vector to a C one.
 */
double*
shift_ffi_cvt_D_array_1D_C(GenericArray* shift_array)
{
  int shift_array_size = array_size(shift_array);
  double* C_array = (double *)safe_malloc(shift_array_size * sizeof(double));
  GenericArray* the_array = (GenericArray*) shift_array;
  int i;

  if (C_array == 0)
    runtime_error("shift_ffi_cvt_array_1D: out of heap memory.");

  for (i = 0; i < shift_array_size; i++)
    C_array[i] = *(double_index((void *) the_array, i));

  return C_array;
}

/*-----------------------------------------------------------------------------
 * SHIFT to C copiers
 */
/* shift_ffi_cvt_I_array_1D_C -- Converts and copies a symbol or logical
 * SHIFT vector to a C one.
 */
int*
shift_ffi_cvt_I_array_1I_C(GenericArray* shift_array)
{
  int shift_array_size = array_size(shift_array);
  int* C_array = (int*)safe_malloc(shift_array_size * sizeof(int));
  GenericArray* the_array = (GenericArray*) shift_array;
  int i;

  if (C_array == 0)
    runtime_error("shift_ffi_cvt_array_1D: out of heap memory.");

  for (i = 0; i < shift_array_size; i++)
    C_array[i] = *(int_index((void *) the_array, i));

  return C_array;
}


/* shift_ffi_cvt_D_array_2D_C -- Converts and copies a number SHIFT
 * vector to a C one.
 */
double**
shift_ffi_cvt_D_array_2D_C(GenericArray* shift_array)
{
  int shift_array_rows = array_size(shift_array);
  int shift_array_cols;
  double** C_array;
  double* C_array_row;
  GenericArray* the_array = (GenericArray*) shift_array;
  int i, j;

  C_array = (double **)safe_malloc(shift_array_rows * sizeof(double *));
  if (C_array == 0)
    runtime_error("shift_ffi_cvt_array_2D: out of heap memory.");

  for (i = 0; i < shift_array_rows; i++)
    {
      /* Modifying the call to voidstar_index:
       *
       * An array(array(.)) is an array of **void's.
       *
       * Tunc Simsek 15th April, 1998
       */
      GenericArray **rowArray =
	   (GenericArray **) voidstar_index((void *)shift_array, i);	
      shift_array_cols = array_size(*rowArray);
      C_array_row = (double *)safe_malloc(shift_array_cols * sizeof(double));
      
      if (C_array == 0)
	runtime_error("shift_ffi_cvt_array_2D: out of heap memory.");

      C_array[i] = C_array_row;

      for (j = 0; j < shift_array_cols; j++)
	{
	  /* Ready for this? :) */
	  C_array_row[j] = *(double_index(*(voidstar_index((void *)the_array,
							   i)),
					  j));
	}
    }

  return C_array;
}


/*-----------------------------------------------------------------------------
 * C to SHIFT copiers
 *
 * Note:
 * In a previous version the copiers were in charge of allocating a
 * SHIFT array if this was empty.  The signature of the copiers was
 *
 * void
 * shift_ffi_cvt_D_array_1D_shift(double* C_array,
 *			          Array** shift_array,
 *			          int dimension)
 * and
 *
 * void
 * shift_ffi_cvt_D_array_2D_shift(double** C_array,
 *			          Array** shift_array,
 *			          int dimension1,
 *			          int dimension2)
 *
 * I.e. they needed to have the proper dimensions passed in.
 * In the new scheme the SHIFT programmer needs to properly allocate
 * the arrays before passing it to the FF in order to fill it.
 * The copiers, will, on their tur rely only on the current array
 * dimensions.
 *
 * Marco Antoniotti 19970513
 */

void
shift_ffi_cvt_D_array_1D_shift(double* C_array, Array** shift_array)
{
  int i;
  GenericArray** the_array = (GenericArray**) shift_array;
  TypeDescriptor* td;

  if (*the_array == 0)
    runtime_warning("FFI: copying back values into a non initialized array.");

  for (i = 0; i < array_size(*the_array); i++)
    array_set((void*) *the_array, i, (void*) &(C_array[i]));
}


void
shift_ffi_cvt_I_array_1I_shift(int* C_array, Array** shift_array)
{
  int i;
  GenericArray** the_array = (GenericArray**) shift_array;
  TypeDescriptor* td;

  if (*the_array == 0)
    runtime_warning("FFI: copying back values into a non initialized array.");

  for (i = 0; i < array_size(*the_array); i++)
    array_set((void*) *the_array, i, (void*) &(C_array[i]));
}


void
shift_ffi_cvt_D_array_2D_shift(double** C_array, Array** shift_array)
{
  int i, j;
  GenericArray** the_array = (GenericArray**) shift_array;
  GenericArray* shift_row;
  TypeDescriptor* td;

  if (*the_array == 0)
    runtime_warning("FFI: copying back values into a non initialized array.");

  for (i = 0; i < array_size(*the_array); i++)
    {
      shift_row = (GenericArray*) *voidstar_index((void *)the_array, i);
      for (j = 0; j < array_size(shift_row); j++)
	array_set((void*) shift_row, j, (void*) &(C_array[i][j]));
    }
}


void
shift_ffi_cvt_I_array_2I_shift(int** C_array, Array** shift_array)
{
  int i, j;
  GenericArray** the_array = (GenericArray**) shift_array;
  GenericArray* shift_row;
  TypeDescriptor* td;

  if (*the_array == 0)
    runtime_warning("FFI: copying back values into a non initialized array.");

  for (i = 0; i < array_size(*the_array); i++)
    {
      shift_row = (GenericArray*) *voidstar_index((void *)the_array, i);
      for (j = 0; j < array_size(shift_row); j++)
	array_set((void*) shift_row, j, (void*) &(C_array[i][j]));
    }
}


/* Old version 19970507
GenericArray*
shift_ffi_cvt_D_array_1D_shift(double* C_array, int dimesion)
{
  int i;
  TypeDescriptor* td =
    (TypeDescriptors*)malloc(sizeof(TypeDescriptor));
  GenericArray* shift_array;

  td->kind = NUMBER_T;
  shift_array = new_array(td, dimension);

  for (i = 0, i < dimension; i++)
    array_set((void*) shift_array, i, (void*) &(C_array[i]));

  return shift_array;
}

GenericArray*
shift_ffi_cvt_D_array_2D_shift(double* C_array, int dimesion1, int dimension2)
{
  int i, j;
  TypeDescriptor* td_array =
    (TypeDescriptors*)malloc(sizeof(TypeDescriptor));
  TypeDescriptor* td_array =
    (TypeDescriptors*)malloc(sizeof(TypeDescriptor));
  GenericArray* shift_array;
  GenericArray* shift_row;

  td_array->kind = ARRAY_T;
  td_base->kind  = NUMBER_T;

  shift_array = new_array(td_array, dimension1);

  for (i = 0, i < dimension1; i++)
    {
      shift_row = new_array(td_base, dimension2);
      array_set((void*) shift_array, i, (void*) shift_row);      
      for (j = 0; j < dimension2; j++)
	array_set((void*) shift_row, j, (void*) &(C_array[i][j]));
    }

  return shift_array;
}
*/

#endif /* FFI_SUPPORT_I */

/* end of file -- ffi-support.c -- */
