/* -*- Mode: C -*- */

/* animation-update.c -- */

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


#ifndef TKSHIFT_ANIMATION_I
#define TKSHIFT_ANIMATION_I

#include <shift_config.h>

#include <tcl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "tkshift-util.h"

int 
C_UpdateItem(ClientData clientData, Tcl_Interp *interp, 
	     int argc, char *argv[])
{
  /* global variable to watch out for */ 

  /* ZoomIndex, DefaultMultiplier, CanvasesArray, CanvasImage, 
     ImageLength, ImageWidth, Colors_Used, AllAnimationData, 
     shift_tk_dir */ 

  char *canvas_number = argv[1]; 
  char *group = argv[2]; 
  char *data = argv[3]; 
  
  int magic_number = 200; 
  int data_size = strlen(data);   
  int tcl_command_size = data_size + magic_number; 
  char *tcl_command = tkshift_safe_malloc(tcl_command_size); 

  char *CanvasXYList; 
  int Counter = 0; 
  
  char * groupTag = tkshift_safe_malloc(10); 
  char * Canvas; 	
  char * Multiplier; 
  char * ZoomIndex;
  char * color; 

  double zoomIndex; 
  double multiplier; 
  int number_of_elements; 

  char * orientationVar; 
  double orientation; 

  /* okay this is for inside of the loop */
  int i = 0; 
  char * XY; 
  int num; 
  char * NEWX; 
  char * NEWY; 
  
  double newX; 
  double newY; 

  double Ypos; 
  double Xpos; 

  char * tag2; 
  char * bit; 
  char * IDlist; 

  double length;
  double width; 

  char * ID0; 
  char * ID; 
  int ID_size; 
  
  char * Tag; 

  /* variable need for the drawing involving orientation */
  double x1, y1, x2, y2, x3, y3, x4, y4; 
  double width_cos_result, width_sin_result, length_cos_result, length_sin_result; 

  char * image_name; 
  int image_size; 
  char * shift_tk_dir;  
  char * image; 

  if (argc != 4) {
    interp->result = "Usage: UpdateItem canvas_number group data "; 
    return TCL_ERROR;
  }							

  sprintf(tcl_command, "lindex {%s} 0", data); 
  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
  CanvasXYList = tkshift_safe_malloc(strlen(interp->result) + 2); 
  sprintf(CanvasXYList, "%s", interp->result); 

  /* get the correct canvas name to do the update */ 
  sprintf(tcl_command, "set CanvasesArray(%s)", canvas_number);
  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 		
  Canvas = tkshift_safe_malloc(strlen(interp->result)); 
  strcpy(Canvas, interp->result); 
  sprintf(groupTag, "Image%s", group); 
  
  /* delete all the items on the canvas with that group */
  sprintf(tcl_command, "catch [%s delete %s]", 
	  Canvas, groupTag);
  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
  
  sprintf(tcl_command, "set DefaultMultiplier(%s,%s)", canvas_number, group);
  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
  Multiplier = tkshift_safe_malloc(strlen(interp->result)); 
  strcpy(Multiplier, interp->result); 

  sprintf(tcl_command, "set ZoomIndex(%s)", canvas_number);
  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
  ZoomIndex = tkshift_safe_malloc(strlen(interp->result)); 
  strcpy(ZoomIndex, interp->result); 
  
  zoomIndex = atof(ZoomIndex); 
  multiplier = atof(Multiplier) * zoomIndex; 

  sprintf(tcl_command, "catch [%s delete %s]", 
	  Canvas, groupTag);
  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
 
  /*
  sprintf(tcl_command, "lindex $Colors_Used %s", group);
  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
  color = tkshift_safe_malloc(strlen(interp->result)); 
  strcpy(color, interp->result); 
  */ 

  sprintf(tcl_command, "llength {%s}", CanvasXYList);
  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
  number_of_elements = atoi(interp->result); 
  
  sprintf(tcl_command, "lindex $AllAnimationData(%s,%s) 3", canvas_number, group); 
  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
  orientationVar = malloc (strlen(interp->result)); 
  strcpy(orientationVar, interp->result); 

  for (i = 0; i < number_of_elements; i++)
    {
      sprintf(tcl_command, "lindex {%s} %d", CanvasXYList, i);
      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
      XY = tkshift_safe_malloc(strlen(interp->result)); 
      strcpy(XY, interp->result); 
      
      num = (number_of_elements - Counter) % 9; 
      sprintf(tcl_command, "lindex $Colors_Used %d", num); 
      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK);
      color = tkshift_safe_malloc(strlen(interp->result)); 
      strcpy(color, interp->result); 
      
      sprintf(tcl_command, "lindex {%s} 0", XY); 
      assert(Tcl_Eval(interp, tcl_command) == TCL_OK);
      NEWX = tkshift_safe_malloc(strlen(interp->result)); 
      strcpy(NEWX, interp->result); 

      sprintf(tcl_command, "lindex {%s} 1", XY); 
      assert(Tcl_Eval(interp, tcl_command) == TCL_OK);
      NEWY = tkshift_safe_malloc(strlen(interp->result)); 
      strcpy(NEWY, interp->result); 

      newX = atof(NEWX); 
      newY = atof(NEWY); 

      Xpos = rint(newX * multiplier); 
      Ypos = rint(newY * multiplier); 

      /*  INT_MAX     2147483647 -> 10 digits */ 
      /* 
      tag2 = tkshift_safe_malloc(20); 
      sprintf(tag2, "Vehicle%d", Counter); 
      */ 

      sprintf(tcl_command, "set CanvasImage(%s,%s)", canvas_number, group); 
      assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
      bit = tkshift_safe_malloc(strlen(interp->result)); 
      strcpy(bit, interp->result); 
      
      if (strcmp(bit, "oval") == 0)
	{
	  if (strcmp(orientationVar, "Nil") != 0)
	    {
	      sprintf(tcl_command, "lindex {%s} 2", XY); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	      orientation = atof(interp->result); 

	      sprintf(tcl_command, "llength {%s}", XY); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	      if (atoi(interp->result) == 5)
		{
		  sprintf(tcl_command, "lindex {%s} 3", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  length = atof(interp->result) * multiplier; 	
		  
		  sprintf(tcl_command, "lindex {%s} 4", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  IDlist = tkshift_safe_malloc(strlen(interp->result)); 
		  strcpy(IDlist, interp->result); 
		}
	      else
		{
		  sprintf(tcl_command, "set ImageLength(%s,%s)", 
			  canvas_number, group); 
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
		  length = atof(interp->result) * zoomIndex; 	

		  sprintf(tcl_command, "lindex {%s} 3", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  IDlist = tkshift_safe_malloc(strlen(interp->result)); 
		  strcpy(IDlist, interp->result); 
		}
	    }
	  else
	    {
	      sprintf(tcl_command, "llength {%s}", XY); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	      if (atoi(interp->result) == 4)
		{
		  sprintf(tcl_command, "lindex {%s} 2", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  length = atof(interp->result) * multiplier; 	
		  
		  sprintf(tcl_command, "lindex {%s} 3", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  IDlist = tkshift_safe_malloc(strlen(interp->result)); 
		  strcpy(IDlist, interp->result); 
		}
	      else
		{
		  sprintf(tcl_command, "set ImageLength(%s,%s)", 	
			  canvas_number, group); 
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
		  length = atof(interp->result) * zoomIndex; 	

		  sprintf(tcl_command, "lindex {%s} 2", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  IDlist = tkshift_safe_malloc(strlen(interp->result)); 
		  strcpy(IDlist, interp->result); 
		}
	    }

	  sprintf(tcl_command, "lindex {%s} 0", IDlist);
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  ID0 = tkshift_safe_malloc(strlen(interp->result)); 
	  strcpy(ID0, interp->result); 
	  ID_size = strlen(ID0) + 1;

	  sprintf(tcl_command, "lindex {%s} 1", IDlist);
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  ID_size += strlen(interp->result); 

	  ID = tkshift_safe_malloc(ID_size); 
	  sprintf(ID, "%s_%s", ID0, interp->result); 
	  
	  Tag = tkshift_safe_malloc(ID_size + 3); 
	  sprintf(Tag, "Tag%s", ID); 
	  
	  /* what would catch give you? */ 
	  sprintf(tcl_command, 
	      "catch {%s create oval %f %f %f %f -tags %s -fill %s -outline black}",
	      Canvas, Xpos - length, Ypos - length, Xpos + length, Ypos + length, 
	      Tag, color); 
	  /* if this gives trouble I might try Tcl_GlobalEval later */ 
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  
	  sprintf(tcl_command, 
		  "catch {%s bind %s <Double-1> \"PrintAnimationInformation %s\"}",
		  Canvas, Tag, Tag); 
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  
	  sprintf(tcl_command, 
		  "catch {%s addtag Image withtag %s}", 
		  Canvas, Tag);
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 

	  sprintf(tcl_command, 
		  "catch {%s addtag %s withtag %s}", 
		  Canvas, groupTag, Tag);
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  tkshift_free(IDlist); 
	  tkshift_free(ID0); 
	  tkshift_free(ID); 
	  tkshift_free(Tag); 
	}
      else if (strcmp(bit, "rectangle") == 0)
	{	
	  if (strcmp(orientationVar, "Nil") != 0)
	    {
	      sprintf(tcl_command, "lindex {%s} 2", XY); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	      orientation = atof(interp->result); 

	      sprintf(tcl_command, "llength {%s}", XY); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	      if (atoi(interp->result) == 6)
		{
		  sprintf(tcl_command, "lindex {%s} 3", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  length = atof(interp->result) * multiplier / 2; 	

		  sprintf(tcl_command, "lindex {%s} 4", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  width = atof(interp->result) * multiplier / 2; 	
		  
		  sprintf(tcl_command, "lindex {%s} 5", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  IDlist = tkshift_safe_malloc(strlen(interp->result)); 
		  strcpy(IDlist, interp->result); 
		}
	      else
		{
		  sprintf(tcl_command, "set ImageLength(%s,%s)", 
			  canvas_number, group); 
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
		  length = atof(interp->result) * zoomIndex; 	

		  sprintf(tcl_command, "set ImageWidth(%s,%s)", 
			  canvas_number, group); 
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
		  width = atof(interp->result) * zoomIndex; 	

		  sprintf(tcl_command, "lindex {%s} 3", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  IDlist = tkshift_safe_malloc(strlen(interp->result)); 
		  strcpy(IDlist, interp->result); 
		}
	    }
	  else
	    {
	      sprintf(tcl_command, "llength {%s}", XY); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	      if (atoi(interp->result) == 5)
		{
		  orientation = 0; 

		  sprintf(tcl_command, "lindex {%s} 2", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  length = atof(interp->result) * multiplier / 2; 	

		  sprintf(tcl_command, "lindex {%s} 3", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  width = atof(interp->result) * multiplier / 2; 	
		  
		  sprintf(tcl_command, "lindex {%s} 4", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  IDlist = tkshift_safe_malloc(strlen(interp->result)); 
		  strcpy(IDlist, interp->result); 
		}
	      else
		{
		  sprintf(tcl_command, "set ImageLength(%s,%s)", 
			  canvas_number, group); 
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
		  length = atof(interp->result) * zoomIndex; 	

		  sprintf(tcl_command, "set ImageWidth(%s,%s)", 
			  canvas_number, group); 
		  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
		  width = atof(interp->result) * zoomIndex; 	

		  sprintf(tcl_command, "lindex {%s} 2", XY); 
		  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
		  IDlist = tkshift_safe_malloc(strlen(interp->result)); 
		  strcpy(IDlist, interp->result); 
		}
	    }

	  sprintf(tcl_command, "lindex {%s} 0", IDlist);
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 

	  ID0 = tkshift_safe_malloc(strlen(interp->result)); 
	  strcpy(ID0, interp->result); 
	  ID_size = strlen(ID0) + 1;

	  sprintf(tcl_command, "lindex {%s} 1", IDlist);
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  ID_size += strlen(interp->result); 

	  ID = tkshift_safe_malloc(ID_size); 
	  sprintf(ID, "%s_%s", ID0, interp->result); 
	  
	  Tag = tkshift_safe_malloc(ID_size + 3); 
	  sprintf(Tag, "Tag%s", ID); 
	  
	  if (strcmp (orientationVar, "Nil") == 0)
	    {
	      x1 = Xpos - (length / 2); 
	      y1 = Ypos + (width / 2); 
	      x2 = Xpos + (length / 2); 
	      y2 = Ypos + (width / 2); 
	      x3 = Xpos + (length / 2); 
	      y3 = Ypos - (width / 2); 	
	      x4 = Xpos - (length /2); 
	      y4 = Ypos - (width / 2);
	    }
	  else
	    {
	      
	      /*orientation = orientation * -1;*/
	      
	      width_cos_result = (width / 2) * cos(orientation); 
	      width_sin_result = (width / 2) * sin(orientation); 
	      length_cos_result = (length / 2) * cos(orientation); 
	      length_sin_result = (length / 2) * sin(orientation); 
	      
	      x1 = Xpos - length_cos_result - width_sin_result; 
	      y1 = Ypos + width_cos_result - length_sin_result; 
	      x2 = Xpos + length_cos_result - width_sin_result; 
	      y2 = Ypos + length_sin_result + width_cos_result; 
	     x3 = Xpos + length_cos_result + width_sin_result; 
	      y3 = Ypos + length_sin_result - width_cos_result; 
	      x4 = Xpos + width_sin_result - length_cos_result; 
	      y4 = Ypos - width_cos_result - length_sin_result; 
	    }

	  sprintf(tcl_command, "winfo exists %s", Canvas); 
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  if (strcmp(interp->result, "1") == 0)
	    {
	      sprintf(tcl_command, 
		      "%s create polygon %f %f %f %f %f %f %f %f -tags %s -fill %s \
                      -outline black", 
		      Canvas, x1, y1, x2, y2, x3, y3, x4, y4, Tag, color); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  
	      sprintf(tcl_command, 
		      "%s bind %s <Double-1> \"PrintAnimationInformation %s\"",
		      Canvas, Tag, Tag); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  
	      sprintf(tcl_command, 
		      "%s addtag Image withtag %s", 
		      Canvas, Tag);
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 

	      sprintf(tcl_command, 
		      "%s addtag %s withtag %s", 
		      Canvas, groupTag, Tag);
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	    }
	  tkshift_free(IDlist); 
	  tkshift_free(ID); 
	  tkshift_free(Tag); 
	}
      else
	{
	  sprintf(tcl_command, "llength {%s}", XY); 
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  if (atoi(interp->result) >= 4)
	    {
	      sprintf(tcl_command, "lindex {%s} 3", XY); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	      IDlist = tkshift_safe_malloc(strlen(interp->result)); 
	      strcpy(IDlist, interp->result); 
	    }
	  else
	    {
	      sprintf(tcl_command, "lindex {%s} 2", XY); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	      IDlist = tkshift_safe_malloc(strlen(interp->result)); 
	      strcpy(IDlist, interp->result); 
	    }		  

	  sprintf(tcl_command, "set ImageLength(%s,%s)", 
		  canvas_number, group); 
	  assert(Tcl_GlobalEval(interp, tcl_command) == TCL_OK); 
	  image_name = tkshift_safe_malloc(strlen(interp->result)); 
	  strcpy(image_name, interp->result); 

	  shift_tk_dir = Tcl_GetVar(interp, "shift_tk_dir", TCL_GLOBAL_ONLY); 
	  
	  image_size = strlen(shift_tk_dir) + strlen(image_name) + 1; 

	  image = tkshift_safe_malloc(image_size); 
	  
	  sprintf(image, "%s/%s", shift_tk_dir, image_name); 

	  tkshift_free(image_name); 
	  
	  sprintf(tcl_command, "lindex {%s} 0", IDlist);
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  ID0 = tkshift_safe_malloc(strlen(interp->result)); 
	  strcpy(ID0, interp->result); 
	  ID_size = strlen(ID0) + 1;

	  sprintf(tcl_command, "lindex {%s} 1", IDlist);
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  ID_size += strlen(interp->result); 

	  ID = tkshift_safe_malloc(ID_size); 
	  sprintf(ID, "%s_%s", ID0, interp->result); 
	  
	  Tag = tkshift_safe_malloc(ID_size + 3); 
	  sprintf(Tag, "Tag%s", ID); 
	  
	  sprintf(tcl_command, "winfo exists %s", Canvas); 
	  assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  if (strcmp(interp->result, "1") == 0)
	    {
	      sprintf(tcl_command, 
		      "%s create bitmap %f %f -tags %s -bitmap @%s", 	
		      Canvas, Xpos, Ypos, Tag, image);
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  
	      sprintf(tcl_command, 
		      "%s bind %s <Double-1> \"PrintAnimationInformation %s\"",
		      Canvas, Tag, Tag); 
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	  
	      sprintf(tcl_command, 
		      "%s addtag Image withtag %s", 
		      Canvas, Tag);
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 

	      sprintf(tcl_command, 
		      "%s addtag %s withtag %s", 
		      Canvas, groupTag, Tag);
	      assert(Tcl_Eval(interp, tcl_command) == TCL_OK); 
	    }
	  tkshift_free(IDlist); 
	  tkshift_free(ID0); 
	  tkshift_free(ID); 
	  tkshift_free(Tag); 
	  tkshift_free(image); 
	}
      Counter++; 
      tkshift_free(color); 
      tkshift_free(NEWX); 
      tkshift_free(NEWY); 
      tkshift_free(bit); 
    }
  
  tkshift_free(tcl_command); 
  tkshift_free(groupTag); 
  tkshift_free(CanvasXYList); 
  tkshift_free(Canvas); 
  tkshift_free(Multiplier); 
  tkshift_free(ZoomIndex); 
  tkshift_free(orientationVar); 
  
  return TCL_OK;
}
	  

#endif /* TKSHIFT_ANIMATION_I */

/* end of file -- animation-update.c -- */
