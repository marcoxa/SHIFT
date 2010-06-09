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

proc TesterProc {} {


    toplevel .sframe


    button .sframe.source -text source -command {
	global env	
	set dir $env(SHIFT_TKDIR)

	set Tcl_Files {main.tcl graph-plots.tcl graph-window.tcl animation-choose.tcl\
	utility.tcl communication.tcl canvases.tcl\
	front.tcl parse.tcl fileselect.tcl startup.tcl \
	tester.tcl animation-create.tcl animation-menage.tcl\
	animation-update.tcl break-discrete.tcl\
	discrete.tcl type-windows.tcl type-menage.tcl tester.tcl}

	foreach tcl_file $Tcl_Files {
	    set to_source [format "%s/$tcl_file" $dir]
	    #    puts "Sourced that one, $to_source"
	    source $to_source
	}
    }
    pack .sframe.source
}

if {![winfo exists .sframe]} {
    TesterProc
}