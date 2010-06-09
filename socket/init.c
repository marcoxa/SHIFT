/* -*- Mode: C -*- */

/* init.c -- */

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

/* init.c
 * This files contains the functions that initialize the sockets
 * for the client and the server and a function that closes the server
 * sockets
 */

#ifndef socket_init_i
#define socket_init_i

#include "socket.h"
#include "run-time.h"
#include "shift_socket_wrappers.h"

extern sim_argument* simulation_args[];

/* Globals */
int clientSock1, clientSock2;
int serverSock1, serverSock2;
int tempSock1, tempSock2;

/*---------------------------------------------------------------
// int InitializeSocket(reset)
//	Initializes the socket on the client side only on the first
// call of this function.  If reset is 1, then the next call to
// InitializeSocket will result in an initialization of a socket.
//
//	Retuns the socket ID under normal operation.  Returns -1 if
// CONFIG file non-existant.  Returns -2 if server down.
//
// "reset" - if 1, resets setFlag
---------------------------------------------------------------*/  
int
InitializeSocket(int reset)
{
  static int setFlag;
  char *file;
  char serverHost[200];
  int Port1, Port2, i, arg = 1;

  if (setFlag == 0)
    {
      if ((file = getenv("DEBUGGERCONFIGFILE")) == NULL)
	{
	  fprintf(stderr, "DEBUGGERCONFIGFILE is not defined\n");
	  exit(1);
	}
 
      Port1 = simulation_args[GUI_DEBUG_ARG]->long_arg;
      Port2 = Port1 + 1;

      if ((clientSock1 = sockConnect(Port1, serverHost)) < 0)
	{
	  close(clientSock1);
	  /*unlink(clientSock1);*/
	  return -2;
	} 
      else if ((clientSock2 = sockConnect(Port2, serverHost)) < 0)
	{
	  close(clientSock1);
	  /*unlink(clientSock1);*/
	  return -2;
	}

      /* Set client socket to non-blocking IO */
      if (ioctl(clientSock2, FIONBIO, &arg) == -1)
	{
	  perror("ioctl");
	}	
      setFlag = 1;
    }
  if (reset == 1) setFlag = 0;
  return 1; 			/* Return value > 0 (For error checking) */
}


/*---------------------------------------------------------------
// int InitializeServer()
// 	Initializes sockets for the server only on the first call  
// of this function by setting the necessary socket IDs.  Returns 1 
// under normal operation.
---------------------------------------------------------------*/
#define __len_val__ 10

int
InitializeServer()
{
  static int setFlag;
  char *file;
  char hostname[200];
  int Port1, Port2;
  int arg = 1;	/* Argument for ioctl */
  int succ, extra1, extra2;
  int len = __len_val__;
  char valuebuf[__len_val__];

  if (setFlag == 0)
    {
      Port1 = simulation_args[GUI_DEBUG_ARG]->long_arg;
      Port2 = Port1 + 1;
    
      /* Initialize first port */
      fprintf(stderr, "Port1: ");
      tempSock1 = initServerSock (Port1);

      /* Initialize second port */
      fprintf(stderr, "Port2: ");
      tempSock2 = initServerSock (Port2);

      serverSock1 = initRun (tempSock1);
      serverSock2 = initRun (tempSock2);
      /*	printf("Called initRun on both sockets \n");*/

      /* Set server socket to non-blocking IO */
      if (ioctl(serverSock1, FIONBIO, &arg) == -1)
	{
	  perror("ioctl");
	}

      /*
      len = 20000;
      succ = setsockopt(serverSock1, SOL_SOCKET,  SO_SNDBUF, (char*)&len,
		 sizeof(int));

      
      succ = getsockopt(serverSock1, SOL_SOCKET,  SO_SNDBUF, (char*)&extra1,
		 &extra2);
		 */
      /*printf("Succ was %d and valuebuf is %d\n ",succ, extra1);*/
      setFlag = 1;
      return 1;	/* Return value > 0 (For error checking) */
    }

  return 1;		/* Return value > 0 (For error checking) */
}

    
/*---------------------------------------------------------------
// closeServer()
//	Closes the socket connections on the server side.
---------------------------------------------------------------*/

void
closeServer()
{
  /* Close first communication channel */

  if (close(tempSock1) != 0)
    perror("TEMPCLOSE");
  if (close(serverSock1) != 0)
    perror("close");	 

  /* Close second communication channel */

  if (close(tempSock2) != 0)
    perror("TEMPCLOSE");
  if (close(serverSock2) != 0)
    perror("close");	     
}



/*---------------------------------------------------------------
 * char *skipSpace (buffer)
 *	Skips any white space in buffer and returns a pointer to
 * the string after the white space
 *
 * "buffer" - character string
 */
char *
skipSpace (char* buffer)
{
  while (isspace(*buffer))
    {
      buffer++;
    }
  return buffer;
}

#endif /* socket_init_i */

/* end of file -- init.c -- */

