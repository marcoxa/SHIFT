/* -*- Mode: C -*- */

/* socket.h -- */

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

#ifndef shift_socket_h
#define shift_socket_h


#include <shift_config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#if defined OS_LINUX
#include <asm/ioctls.h>
#endif

#if defined OS_SOLARIS || defined OS_SUNOS4 || defined OS_IRIX

#include <stropts.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/conf.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/filio.h>

#endif

#if defined WIN32
#include <winsock.h>
#endif

#include "shift_socket_wrappers.h"


/****** Global definitions of socket IDs *******/
#define MAXSIZE 512

extern int clientSock1, clientSock2;
extern int serverSock1, serverSock2;
extern int tempSock1, tempSock2;


/**** Socket function definitions *****/

extern int sockConnect (int port, char *host);
extern int initServerSock (int portno);
extern int transmit (int sockID, char *msg,int ID);
extern int receive_command (int sockID, char *msg);
extern int receive_data(int, char*,int);
extern void closeServer ();
extern int InitializeSocket (int reset);
extern int InitializeServer ();
extern int initRun(int clientSock);
extern void closeServer ();

#endif /* socket_h */

/* end of file -- socket.h -- */
