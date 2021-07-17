/* -*- Mode: C -*- */

/* socket.c -- */

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
/* socket.c --
 * This file contains the functions that manipulate the sockets
 * that are needed for communication between the server and client
*/
  
#ifndef shift_socket_i
#define shift_socket_i

#include <shift_config.h>
#include "socket.h"
#include "shift_socket_wrappers.h"

#if defined OS_LINUX
#include "/usr/include/asm/ioctls.h"
#include "/usr/include/linux/socket.h"
#include "/usr/include/linux/in.h"
#endif


/*---------------------------------------------------------------
 *  int sockConnect(port, host) 
 *     Create socket and connect to server. Returns socket ID under 
 * normal operation.  If socket cannot connect to server, returns -1.
 * 
 * "port" - portnumber of server
 * "host" - name of server
 */
int
sockConnect (int port, char* host)
{
  struct sockaddr_in sock_in;
  struct hostent *hp;
  int tmpSock = -1;

  /* port += IPPORT_USERRESERVED;  Removing this because the extra
   * 5000 is confusing
   */
  /* printf("sockConnect using port %d.\n", port); */

  /* Create a socket */
  tmpSock = socket (AF_INET, SOCK_STREAM, 0);
  if (tmpSock < 0)
    {
      perror("client: can't open socket");
      exit(1);
    }

  /* Initialize the socket address to the server's address. */
  memset((char *) &sock_in, 0, sizeof(struct sockaddr_in));
  sock_in.sin_family = AF_INET;
  hp = gethostbyname(host);    /* to get host address */
  if (hp == NULL)
    {
      perror(host);
      exit(1);
    }
  /* Old version. Why we need it is a mystery to me.
   * bcopy (hp->h_addr, &(sock_in.sin_addr.s_addr), hp->h_length);
   *
   * Marco Antoniotti 19971027
   */

  /* This is the new version, and it does not compile right now. We
   * are not using this function, so I am commenting this out until we
   * decide to scrap this function or use it.
   * DW 19971028
   *memcpy(&(sock_in.sin_addr.s_addr), hp->h_addr, hp->h_length);
   */

  sock_in.sin_port = htons(port);

  /* Connect to the server. */
  if (connect(tmpSock, (struct sockaddr *) &sock_in, sizeof(struct sockaddr_in)) < 0)
    {
      close(tmpSock);
      return -1;
    }
  return tmpSock;
}


/*---------------------------------------------------------------
 * int initServerSock(portno)
 *      Initialize server socket and listen for connection requests
 *
 * "portno" - portnumber for server
 */
int 
initServerSock(int portno)
{
  /* prepare for connection:
   * create socket and listen
   */
  int err;
  int sock;
  struct sockaddr_in sock_in;

  /*  portno += IPPORT_USERRESERVED; */

  printf("shift: server using port %d\n", portno);

  /* Create a socket in internet domain*/
  sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef WIN32
  if (sock == INVALID_SOCKET)
#else
  if (sock < 0)
#endif
    {
      perror("shift: server error: while opening stream socket");
      exit(1);
    }
  
#if defined OS_SOLARIS || defined OS_SUNOS5
  ioctl(sock, I_SETCLTIME, 1000);
#endif

  /* Create the address we will be connecting to */
  sock_in.sin_family = AF_INET;
  #ifdef WIN32
  sock_in.sin_addr.s_addr = htonl(INADDR_ANY);
  #else
  sock_in.sin_addr.s_addr = INADDR_ANY;
  #endif
  sock_in.sin_port = htons(portno);

  /* Bind an address to this socket */
#ifdef WIN32
  err = bind(sock, (struct sockaddr FAR*)&sock_in, 
	     sizeof(struct sockaddr_in));

#else
  err = bind(sock, (struct sockaddr *)&sock_in, 
	     sizeof(struct sockaddr_in));

#endif
#ifdef WIN32
    if (err < SOCKET_ERROR)
#else
  if (err < 0)
#endif
    {
      close(sock);
      perror("shift: server error: while binding stream socket");
      exit(1);
    }
    
  /* Prepare the socket queue for connection requests */
  fprintf(stderr, "shift: listening for connect requests\n");
  if (listen(sock, 10) < 0)
    {
      close(sock);
      perror("shift: server error: while listening on open and bound socket");
      exit(1);
    }
  return sock;
}

/*---------------------------------------------------------------
 *  int initRun(serverSock)
 *	Initialize socket for byte stream communication
 *
 *  "serverSock" - socket ID for server
 */
int 
initRun(int serverSock)
{
  int fromsock = -1;
  int fromlength = -1;
  struct sockaddr_in fromsin;
  int rc;

  fromlength = sizeof(fromsin); 
  rc  = getsockname(serverSock,(struct sockaddr *)&fromsin,&fromlength);
  if (rc < 0)
    {
      close(serverSock);
      perror("getsockname");
      exit(1);
    }
  fromsock = accept(serverSock, (struct sockaddr *)&fromsin, &fromlength);
  if (fromsock < 0)
    {
      close(fromsock);
      perror("accept");
      exit(1);
    }

  return fromsock;
}



/*---------------------------------------------------------------
 * int transmit(sockID, msg)
 *      Writes n-bytes from the message buffer to the socket. Need
 * to send over size of message and then the message itself.
 *
 * "sockID" - socket ID
 * "msg" - message buffer
 */
int
transmit(int sockID, char* msg, int otherSockID)
{ 
  #ifdef WIN32
  u_long arg = TRUE;
  #else
  int arg = 0;
  #endif
  int cnt, times;
  char message[5];
  int msglen = strlen(msg) + 1;
    
  if (ioctl(sockID, FIONBIO, &arg) == -1)
    {
      perror("shift: server error: polling 'ioctl' call failed.");
    }	
  times = 0;
  cnt = 1;
  do {
    #ifdef WIN32
    cnt = send(sockID, msg, msglen, 0); 
    #else 
    cnt = write(sockID, msg, msglen); 
    #endif
    times = times + 1;
  } while (cnt < 0);

  return 0;
}


int
PC_MesgAvail(int sock) 
{
  int fd;
  int rt;

  #ifdef WIN32
  u_long availCount;
  #else 
  int availCount;
  #endif


  fd = sock;
  rt = ioctl(fd, FIONREAD, &availCount);

#ifdef WIN32
  if (rt < SOCKET_ERROR)
#else
   if (rt < 0)
#endif
      {
	fprintf(stderr,
		"shift: server error: return from ioctl, errno = %d\n",
		errno);
	return 0;
      }
  if (availCount > 0)
    return 1;
  return 0;
}


/*---------------------------------------------------------------
 *  int receive (sockID, msg)
 *      Reads n number of bytes into the a buffer msg from the socket.
 * First reads size of message and then the message itself.      
 *
 * "clientSock" - socket ID
 * "msg" - message buffer
---------------------------------------------------------------*/

int
receive_command(int sockID, char* msg)
{
  int cnt;
  char msglen[4];
  char incomingMessage[5]; 
  int arg0 = 0;
    
  msglen[0] = '\0';

  if (PC_MesgAvail(sockID))
    {
       #ifdef WIN32
      cnt = recv(sockID, (char FAR *) incomingMessage, 4, 0);
      #else
      cnt = read(sockID, incomingMessage, 4);
      #endif

      if (cnt > 0)
	{
	  strcpy(msg, incomingMessage);
	  msg[4] = '\0';
	  return cnt;
	}
    }
  return 0;
}


int
receive_data(int sockID, char* msg, int msg_len)
{
  int cnt;
  int arg0 = 0;
    
  msg[0] = '\0';

#ifdef WIN32
  cnt = recv(sockID, msg, msg_len,0);
#else  /* Assumes Unix flavor of some kind */
  cnt = read(sockID, msg, msg_len);
#endif

  if (cnt > 0)
    {
      msg[msg_len - 1] = '\0';
      /* printf("\nMessage %s.\n", msg); */
      return cnt;
    }
  return 0;
}


#endif /* shift_socket_i */

/* end of file -- socket.c -- */
