/* -*- Mode: C -*- */
  
/* tkAppInit.c -- */

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
/* 
 * tkAppInit.c --
 *
 *	Provides a default version of the Tcl_AppInit procedure for
 *	use in wish and similar Tk-based applications.
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#ifndef lint
static char sccsid[] = "@(#) tkAppInit.c 1.15 95/06/28 13:14:28";
#endif /* not lint */

#include <stdlib.h>
#include <strings.h>
#include <tcl.h> 
#include <tk.h>

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

extern int matherr();
static int (*dummyMathPtr)() = matherr;

extern int 
C_ProcessData(ClientData clientData,
	      Tcl_Interp *interp, 
	      int argc,
	      const char *argv[]); 

extern int 
C_stripSpaces(ClientData clientData,
	      Tcl_Interp *interp, 
	      int argc,
	      const char *argv[]); 

extern int 
C_UpdateItem(ClientData clientData,
	     Tcl_Interp *interp, 
	     int argc,
	     const char *argv[]); 

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None: Tk_Main never returns here, so this procedure never
 *	returns either.
 *
 * Side effects:
 *	Whatever the application does.
 *
 *----------------------------------------------------------------------
 */

int
main(int argc, char** argv)
/* argc -- Number of command-line arguments. */
/* argv -- Values of command-line arguments. */
{
  Tk_Main(argc, argv, Tcl_AppInit);
}



/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

static const char * shift_tk_dirname = "SHIFT_TKDIR";

int
Tcl_AppInit(Tcl_Interp* interp)
     /* interp -- Interpreter for application. */
{
  char buffer[250];
  char *g;

  /* few pointers to test my theory about insuffiency of interp->result */
  int max_result_size = 500; 
  interp->result = Tcl_Alloc(max_result_size); 

  if (Tcl_Init(interp) == TCL_ERROR) {
    fprintf(stderr,"TCL is not loading properly.");
    return TCL_ERROR;
  }
  if (Tk_Init(interp) == TCL_ERROR) {
    fprintf(stderr,"TK is not loading properly.");
    return TCL_ERROR;
  }

  /*
   * Call the init procedures for included packages.  Each call should
   * look like this:
   *
   * if (Mod_Init(interp) == TCL_ERROR) {
   *     return TCL_ERROR;
   * }
   *
   * where "Mod" is the name of the module.
   */

  if (Blt_Init(interp) == TCL_ERROR) {
    fprintf(stderr,"BLT is not loading properly.");
    return TCL_ERROR;
  }


  /*
   * Specify a user-specific startup file to invoke if the application
   * is run interactively.  Typically the startup file is "~/.apprc"
   * where "app" is the name of the application.  If this line is deleted
   * then no user-specific startup file will be run under any conditions.
   */
  
  /*Tcl_Eval(interp, "import add blt");*/
  
  g = getenv(shift_tk_dirname);
  
  if (g == NULL) {
    fprintf(stderr,
	    "There is no %s environment variable set.\n",
	    shift_tk_dirname);
    fprintf(stderr, "Exiting...\n");
    exit(0);
  }

  strcpy(buffer, "source ");
  strcat(buffer, g);
  strcat(buffer, "/initialize.tcl\0");
      
  Tcl_Eval(interp, buffer);

  /*
   * Call Tcl_CreateCommand for application-specific commands, if
   * they weren't already created by the init procedures called above.
   */ 

  Tcl_CreateCommand(interp, "ProcessData",  C_ProcessData, 
		    (ClientData)NULL, (Tcl_CmdDeleteProc *)Tcl_Free); 
  
  Tcl_CreateCommand(interp, "stripSpaces",  C_stripSpaces, 
		    (ClientData)NULL, (Tcl_CmdDeleteProc *)Tcl_Free);
  
  Tcl_CreateCommand(interp, "UpdateItem",  C_UpdateItem, 
		    (ClientData)NULL, (Tcl_CmdDeleteProc *)Tcl_Free);
  
  Tcl_Eval(interp, "MainPanel");
  
  /*
    Tcl_Eval(interp, "source test.tcl");
  */	 
  return  TCL_OK;
}


/* strerror -- The following monstrosity is here because of a wrong
 * assumption made by the BLT developers about the availability of
 * 'strerror'.  The following is a tentative patch which may or may
 * not work on systems other than SunOS4.  Of course if your system
 * has 'strerror' in 'libc', you should not have any problems.
 *
 * Marco Antoniotti <marcoxa@icsi.berkeley.edu> 19960926
 */

#ifndef HAVE_STRERROR

#ifdef OS_SUNOS4

#include <errno.h>

char *
strerror(int errnum)
{
  if (errnum >= sys_nerr)
    return (char *) 0;
  else
    return sys_errlist[errnum];
}

#else

char *
strerror(int errnum)
{
  fprintf(stderr, "SHIFT runtime: error %d passed to patched 'strerror'.\n",
	  errnum);
  perror("SHIFT runtime: patched 'strerror'");
  return (char *) 0;
}

#endif /* OS_SUNOS4 */

#endif /* HAVE_STRERROR */


/* end of file -- tkAppInit.c -- */


