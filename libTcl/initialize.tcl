# -*- Mode: Tcl -*-

# initialize.tcl --

# Copyright (c)1996, 1997, 1998 The Regents of the University of
# California (Regents). All Rights Reserved. 
# 
# Permission to use, copy, modify, and distribute this software and
# its documentation for educational, research, and not-for-profit
# purposes, without fee and without a signed licensing agreement, is
# hereby granted, provided that the above copyright notice, this
# paragraph and the following two paragraphs appear in all copies,
# modifications, and distributions.
# 
# Contact The Office of Technology Licensing, UC Berkeley, 2150
# Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510)
# 643-7201, for commercial licensing opportunities. 
# 
# IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT,
# INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
# DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE. 
#   
# REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
# DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
# REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
# ENHANCEMENTS, OR MODIFICATIONS. 

# initialize.tcl -- The initialize.tcl file is sourced by tkAppInit
# on startup.  It is important that it be here.  The init file was
# created with future extentions of functionality in mind.


catch { set shift_tk_dir $env(SHIFT_TKDIR) }
catch { set shift_install_root_dir $env(SHIFT_INSTALLROOT) }
#catch { set shift_itkwidgets_dir $env(SHIFT_ITKWIDGETS_DIR) }


#The Tcl_Files variable should contain all the files that need to be
#sourced

#package require blt
#import add blt*

set Tcl_Files {parameters.tcl graph-window.tcl\
	graph-plots.tcl animation-choose.tcl\
	utility.tcl communication.tcl canvases.tcl\
	front.tcl parse.tcl fileselect.tcl widgets.tcl main.tcl\
	animation-create.tcl animation-menage.tcl break-discrete.tcl\
	canvas-parameters.tcl discrete.tcl\
	type-windows.tcl type-menage.tcl startup-data.tcl ahs-highway.tcl\
	startup.tcl}

foreach tcl_file $Tcl_Files {
    set to_source [format "%s/$tcl_file" $shift_tk_dir]
    #puts "Sourced that one, $to_source"
    source $to_source
}


catch {
    namespace import blt::*
}


set startup_windows ""

###############################################################################
###############################################################################
##
## These are the tcl files used in TkShift:
##
##   parameters.tcl : This file defines and/or initializes (too many)
##   global variables that are used in TkShift. 
##
##   front.tcl : This file contains the code that links the client
##   (TkShift) process with the simulation.
##
##   communication.tcl (and .c): This file contains the functions that
##   menage the communication between the client and the server.
##
##   fileselect.tcl : Brent Welch's fileselect widget.
##
##
##   main.tcl: This file contains the MainPanel function, as well as
##             the other functions that create the various info boxes
##
##   parse.tcl: This file contains the functions that parse the data
##   coming from the simulation and put it into the appropriate places.
##
##   graph-window.tcl: This file includes all the functions needed to operate
##              the graph widget. (the window itself)
##   graph-plots.tcl: This file includes all the functions needed to operate
##              the data that feeds information to the graph widget.
##
##   utility.tcl : This contains the copyright, the credits, some of
##   the error checking, and some of the step/time/zeno functions.
##
##   canvases.tcl: This contains the data that creates and menages the
##   canvases.
##
##   canvas-parameters.tcl : This contains the definitions of the
##   canvases used by TkShift.
##
##   widgets.tcl : This contains just a couple widgets.
##
##   animation-choose.tcl:Creates the animation selection window.
##   animation-create.tcl:Processes the data, sends it to simulation.
##   animation-menage.tcl:Controls the menus and choices one has in
##   the animation
##   animation-update.c: Updates the positions of the items in the canvas.
##   

##    discrete.tcl : menages the messages of the discrete transitions
##    coming to TkShift.
##
##    break-discrete.tcl : Handles the break messages coming from the
##    simulation. 
## 
##    type-windows.tcl : Menages the windows that show component and type
##    data in the simulation
##
##    type-menage.tcl : Functions that handle the data related to the
##    windows in type-windows.tcl
##
##    startup-data.tcl : Functions that control the script module
##    behavior.
##    startup.tcl: Functions that deal and implement the script module.
##
##    ahs-highway.tcl : Functions that draw any AHS highway onto the canvas.
##
##
############################################################################
#################################################################################

# end of file -- initialize.tcl --

