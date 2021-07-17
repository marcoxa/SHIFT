// -*- Mode: C++ -*-

/* commands.C
// 
// This file contains the functions that are called within the TCL/TK
// environment of our debugger
//
*/

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

#ifndef TKSHIFT_COMMANDS_I
#define TKSHIFT_COMMANDS_I

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/filio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <memory.h>
#include "commands.h"
#include <tcl.h>
#include <tk.h>

extern "C" {
#include <socket.h>
#include "config.h"
#include "init.h"
}

#define MAXLENGTH 2000
int processNumber;
int connect_initialize = 0;
char * INSERT_STATE_TEXT;
char * INSERT_STATIC_TEXT;

int
GetChunkCmd(ClientData clientData,
	    Tcl_Interp *interp,
	    int argc,
	    char **argv);


//---------------------------------------------------------------
//  int ListenCmd(clientData, interp, argc, argv)
//	Listens to the client socket and receives the messages
//  sent to the socket. Returns TCL_OK under normal operation. 
//  Otherwise, returns TCL_RETURN.
//---------------------------------------------------------------

int
ListenCmd(ClientData clientData,Tcl_Interp *interp,int argc,char **argv)
{
  int cnt, error_flag;
  Tcl_Channel channel1, channel2;
  char key[MAXLENGTH];
  char State[MAXLENGTH];
  char windowPathName[MAXLENGTH];
  char buffer[MAXLENGTH];
  static char time[MAXLENGTH];
  static int init_hash = 0;
  int arg1;
  arg1= 1;
  error_flag = InitializeSocket(0);
  

  /* Check for CONFIG file */
  if (error_flag == -1)
    {
      printf("Error in Config file\n");
      Tcl_VarEval(interp, "ErMsg ", "{",
		  "Non-existant CONFIG file! Please set CONFIG file and restart."
		  , "}", (char *) NULL);
      Tcl_VarEval(interp, "set ", "shouldListen ", "0", (char *)NULL); 
      Tcl_Eval(interp, ".frameb.b1 configure -text \"Connect\" -command \"grab release .; Connect_pressed\" ");    	
      Tcl_Eval(interp, "update");
      return TCL_RETURN;
    }

  /* Check for client-server connection */
  if (error_flag == -2) {
    printf("Error 2 in CREATEASdf \n");
    Tcl_VarEval(interp, "ErMsg ", "{",
		"Connect to Server: Connection refused! Please initialize server and try again."
		, "}", (char *) NULL);
    Tcl_VarEval(interp, "set ", "shouldListen ", "0", (char *)NULL); 
    Tcl_Eval(interp, ".frameb.b1 configure -text \"Connect\" -command \"grab release .; Connect_pressed\" ");    	
    Tcl_Eval(interp, "update");
    return TCL_RETURN;
  }

  bzero(key, MAXLENGTH);
  bzero(State, MAXLENGTH);
  bzero(windowPathName, MAXLENGTH);
  bzero(buffer, MAXLENGTH);

 
  connect_initialize = 1;     

  /* Set client socket to non-blocking I/O */

  /*    ioctl(clientSock1, FIONBIO, &arg1);*/

  return TCL_OK;
}

      
      
int GetChunkCmd(ClientData clientData,Tcl_Interp *interp,int argc,char **argv) {    
  int cnt, error_flag;
  char buffer[1024];
  

  bzero(buffer, 1024);
  
  if ((cnt = receive_command(clientSock1, buffer)) > 0) {
    strcpy(interp->result, buffer);

  }
 return TCL_OK;

}

/*---------------------------------------------------------------
//  int SendCmd(clientData, interp, argc, argv)
//	Sends the request to the C/C++ server that specifies which
// object's attributes to retrieve. 
---------------------------------------------------------------*/
int
SendCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
  int cnt;  
  char *command;
  char *buffer1, *buffer2, *buffer3, *buffer4;
    
  char windowname[MAXSIZE];
  int error_flag = InitializeSocket(0);

  /* Check for client-server connection */
  if (error_flag == -2) {
    Tcl_VarEval(interp, "ErMsg ", "{",
		"Connect to Server: Connection refused! Please initialize server and try again."
		, "}", (char *) NULL);
    Tcl_VarEval(interp, "set ", "shouldListen ", "0", (char *)NULL); 
    Tcl_Eval(interp, ".frameb.b1 configure -text \"Connect\" -command \"grab release .; Connect_pressed\" ");    	
    Tcl_Eval(interp, "update");
    return TCL_RETURN;
  }

  /* Need to link with TCL variable "command" */
  command = Tcl_GetVar(interp, "command", TCL_LEAVE_ERR_MSG);
  printf("Command is %s\n", command);


  if (!strcmp(command, "SendRequest"))
    {
      /* Link with TCL variables */
      buffer1 = Tcl_GetVar(interp, "objectname", TCL_LEAVE_ERR_MSG);
      buffer2 = Tcl_GetVar(interp, "request", TCL_LEAVE_ERR_MSG);
    
      /* Send buffers to backend */
      transmit(clientSock2, "NormalRequest");	
      transmit(clientSock2, buffer1);
      transmit(clientSock2, buffer2);
	
    } 
  else if (!strcmp(command, "Cancel"))
    {
      /* Link with TCL variables */
      transmit(clientSock2, "Cancel");
    }
  else if (!strcmp(command, "Stop"))
    {
      transmit(clientSock2, "Stop");
    }    
  else if (!strcmp(command, "Start"))
    {
      transmit(clientSock2, "Start");
    } 
  else if (!strcmp(command, "Cont"))
    {
      transmit(clientSock2, "Cont");
    } 
  else if (!strcmp(command, "GetTypes"))
    {
      transmit(clientSock2, "GetTypes");
    } 
  else
    {
      printf("Sending -%s-\n", command);
      transmit(clientSock2, command);
    }
  return TCL_OK;
}

#endif // TKSHIFT_COMMANDS_I

// end of file -- commands.C --
