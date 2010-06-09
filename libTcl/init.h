/* -*- Mode: C -*- */

/* init.h -- */

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

#ifndef libtcl_init_h
#define libtcl_init_h

#include <sys/types.h>
#include <sys/socket.h>

#ifdef OS_SOLARIS
#include <sys/filio.h>
#endif

#include <tcl.h>

extern int InitializeSocket(int reset);
extern int InitializeServer();
extern int initRun(int clientSock);
extern void closeServer ();

/****** Global definitions of socket IDs *******/
#define MAXSIZE 512

extern int clientSock1, clientSock2;
extern int serverSock1, serverSock2;
extern int tempSock1, tempSock2;

void process_data(ClientData clientData, int mask);

int GetChunkCmd(ClientData clientData,
		Tcl_Interp *interp,
		int argc,
		char **argv);

#endif /* libtcl_init_h */

/* end of file -- init.h -- */
