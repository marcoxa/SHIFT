# -*- Mode: Tcl -*-

# startup.tcl --

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

proc startupSaveToFile {} {

    global startupData simulation 

    if {[catch {set startupData(filename) $startupData(filename)}] == 0} {
	if {$startupData(filename) != ""} {
	    set startupData(simulation) $simulation 
	    
	    set stuff [array names startupData]
	    
	    set f [open $startupData(filename) w]
	    foreach item $stuff {
		puts $f [format "set startupData(%s) %s" $item\
			[list $startupData($item)]]
	    }
	    close $f
	    
	    tk_dialog .success Info "The $startupData(file) startup file was
	    saved successfully" info 0 OK
	}
    }
}


proc startupReadFromFile {} {

    global startupData

    .start.main.right.list delete 0 end
    source $startupData(filename)

    set animations $startupData(animations,-1)
    foreach anim $animations {
	set label "Animation $anim"
	.start.main.right.list insert end $label
    }

    set graphs $startupData(graphs,-1)
    foreach gr $graphs {

	# for backward compatability 

	if {[catch {set startupData(graphs,$gr,title) $startupData(graphs,$gr,title)}]} {
	    set startupData(graphs,$gr,title) ""
	}
	if {[catch {set startupData(graphs,$gr,xtext) $startupData(graphs,$gr,xtext)}]} {
	    set startupData(graphs,$gr,xtext) ""
	}
	if {[catch {set startupData(graphs,$gr,ytext) $startupData(graphs,$gr,ytext)}]} {
	    set startupData(graphs,$gr,ytext) ""
	}
	
	set label "Graph $gr"
	.start.main.right.list insert end $label
    }

    set phase $startupData(phase,-1)
    foreach ph $phase {

	# for backward compatability 

	if {[catch {set startupData(phase,$ph,title) $startupData(phase,$ph,title)}]} {
	    set startupData(phase,$ph,title) ""
	}
	if {[catch {set startupData(phase,$ph,xtext) $startupData(phase,$ph,xtext)}]} {
	    set startupData(phase,$ph,xtext) ""
	}
	if {[catch {set startupData(phase,$ph,ytext) $startupData(phase,$ph,ytext)}]} {
	    set startupData(phase,$ph,ytext) ""
	}

	### Backwards compatability for scripts that have single phase graphs (Tolga 102297).
	### New version supports multiple phase graphs per graph item, and saves the data
	### as a list of lists. Old version, used to save one graph data a list.

	if {[llength [lindex $startupData(phase,$ph) 0]] == 1} {
	    ### If this is just a list, then the first element should have a lenght of 1 => old version
	    ### Convert it into list of a list for compatability.
	    set startupData(phase,$ph) [list $startupData(phase,$ph)]
	}
	### Compatability code ends
	

	set label "Phase $ph"
	.start.main.right.list insert end $label
    }

    set breaks $startupData(break,-1)
    foreach br $breaks {
	set label "Break $br"
	.start.main.right.list insert end $label
    }

    set stops $startupData(stop,-1)
    foreach st $stops {
	set label "Stop $st"
	.start.main.right.list insert end $label
    }
    
    set traces $(traces,-1)
    foreach tr $traces {
	
    }

}
