# -*- Mode: Tcl -*-

# animation-menage.tcl --

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





#############################################################################
##
##
##
#############################################################################

proc DoPreviousAnimation {{animFile 0}} {

    global AnimationRecallFile messageQueue CanvasImage ImageLength
    global ImageWidth DefaultMultiplier CanvasAnimations
    global CanvasTypeArray ExistingCanvases home_directory

    if {$animFile == 0} {
	source "$home_directory/.animationFile"
    } else {
	set f [.sim.middle.file get]
	destroy .sim
	source "$f"
    }
}

proc FindAnimationFile {} {

    global SimulationDirectory

    toplevel .sim 

    set w .sim
    frame $w.top
    frame $w.middle
    frame $w.bottom
    pack $w.top $w.middle $w.bottom -side top -fill both -expand 1
    puts "No argument found"

    fileselectionbox $w.middle.file -directory $SimulationDirectory -height 12c \
		-width 20c
	
    button $w.bottom.ok -text "Run File"  \
	    -command {DoPreviousAnimation 1}

    button $w.bottom.cancel -text "Cancel" -command "destroy .sim"


    pack $w.bottom.ok -fill x -expand 1 -side left
    pack $w.bottom.cancel -side right -fill x -expand 1

    pack $w.middle.file -padx 20 -pady 10 -fill both -expand 1

}

#############################################################################
##
##
##
#############################################################################

proc ExitCanvas {canvas_title} {

    global messageQueue CanvasAnimations CanvasTypeArray ExistingCanvases

    set l [string first "#" $canvas_title]
    set canvas_number [string range $canvas_title [expr $l +1] end]
    set animations {}
#    puts "Got canvas_title: $canvas_title"
    set groups $CanvasAnimations($canvas_number)
    set opt [format "ALL %s" [list $groups]]
    #UpdateCanvasStructures $opt $canvas_number

    foreach i $groups {
	lappend messageQueue 8900
	set msg "$i $canvas_number"
	#puts "Calling the back end with $msg"
	set msg [padMessage $msg]
	lappend messageQueue $msg
#	puts "message sent $msg"
    }
    
    set canvasType $CanvasTypeArray($canvas_number)
    set c_list $ExistingCanvases($canvasType)
    
    set c_list2 {}
    foreach i $c_list {
	if {$i != $canvas_number} {
	    lappend c_list2 $i
	}
    }
    set ExistingCanvases($canvasType) $c_list2
    
    # puts "The new thing is $c_list2"
}


#############################################################################
##
##
##
#############################################################################

proc RemoveAnimationGroup {canvas_title} {
   
    global AnimatedGroups CanvasAnimations

    toplevel .unanimate 
#-bg ivory1
    set w ".unanimate"
    

    frame $w.top
    frame $w.middle -relief raised -bd 3
    frame $w.bottom

    set l [string first "#" $canvas_title]
    set canvas_number [string range $canvas_title [expr $l +1] end]
    label $w.top.lab -text "Remove animations"
    
    pack $w.top $w.middle $w.bottom -fill both -expand 1

    set animations {}
    set groups $CanvasAnimations($canvas_number)
    
    foreach element $groups {
	lappend animations $AnimatedGroups($element)
    }
    set all [format "ALL %s" [list $groups]]
    lappend animations $all
    optionmenu $w.middle.choice \
	    -labeltext "Animations to remove: " \
	    -items $animations
    
    pack $w.middle.choice -fill x -expand 1
    
    button $w.bottom.ok -text OK -command {

	global messageQueue

	set option [.unanimate.middle.choice get]
	set l [string first " " $option]
	set str [string range $option 0 [expr $l -1]]
	if {$str == "ALL"} {

	    UpdateCanvasStructures $option
	    set group_list [lindex $option 1]
	    foreach i $group_list {
		lappend messageQueue 8900
		set msg [padMessage $i]
		lappend messageQueue $msg
#		puts "message sent $msg"
	    }
	    
	} else {
	    UpdateCanvasStructures $option
	    lappend messageQueue 8900
	    set msg [padMessage [lindex $option 5]]

	    lappend messageQueue $msg
#	    puts "message sent $msg"
	}
	destroy .unanimate
    }

    button $w.bottom.cancel -text Cancel -command {destroy .unanimate}

    pack $w.bottom.ok  -fill x -expand 1 -side left
    pack $w.bottom.cancel -fill x -expand 1 -side right
}


#############################################################################
##
##
##
#############################################################################

proc UpdateCanvasStructures {opt canvas_number} {

    global AnimatedGroups CanvasAnimations totalCanvases

    set a [lindex $opt 0]
    set groups {}
    if {$a == "ALL"} {
	set groups [lindex $opt 1]
    } else {
	set groups [lindex $opt 5]
    }
    set g [lindex $groups 0]
#    puts "G is $g"
    set counter 1
    while {$counter <= $totalCanvases} {
	
    set data $CanvasAnimations($canvas_number)
    set ex [lsearch -exact $data $g]
	
    #set CanvasAnimations($canvas_number) {}
		if {$ex >= 0} {
	    if {$a == "ALL"} {

	    } else {
		set new_data {}
		foreach i $data {
		    if {$i != $g} {
			lappend new_data $i
		    }
		    
		}
		set CanvasAnimations($counter) $new_data
		puts "Now has $CanvasAnimations($counter)"
	    }
	}
	incr counter
    }
}



