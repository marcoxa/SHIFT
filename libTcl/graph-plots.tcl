# -*- Mode: Tcl -*-

# graph-plots.tcl --

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
## This is a complex function. Need to re-write it
##
##
#############################################################################


proc RemoveGraphData {window_name} {

    global VectorList messageQueue TimeVectors PRINT_DEBUG
    global GraphsInWindow

    set graph_name [format "%s.middle.flow" $window_name]
    set remove_list {}  
    set graphsInWindow $GraphsInWindow($window_name)
    #puts $graphsInWindow
	
    
    foreach to_remove $graphsInWindow {
	set list2 {}
	set type [lindex $to_remove 0]
	set variable [lindex $to_remove 1]
	set instance [lindex $to_remove 2]
	set vector_id [lindex $to_remove 3]
	foreach i $VectorList {
	    set vector_type [lindex $i 4]
	    set vector_instance [lindex $i 5]
	    set vector_number [lindex $i 1]
	    if {$vector_id == $vector_number} {
		#$type == $vector_type && $instance == $vector_instanc\e
		#puts "gonna dequeue $vector_id with $type, $instance"
	    } else {
		lappend list2 $i
	    }
	}
	set VectorList $list2
    }


    foreach to_remove $graphsInWindow {
	
	set type [lindex $to_remove 0]
	set variable [lindex $to_remove 1]
	set instance [lindex $to_remove 2]
	set vector_num [lindex $to_remove 3]
	lappend messageQueue 5778
	
	if {$PRINT_DEBUG} {
	    puts "Type:$type, instance:$instance"
	}
	set info [format "%s:%s:%s" $type $instance $vector_num]
	if {$PRINT_DEBUG} {
	    puts "message:$info"
	}
	set msg [padMessage $info]
	lappend messageQueue $msg
    }
}


########################################################################
#
#
#
#
########################################################################


proc RemoveSingleGraph {window_name vector_type vrctor_variable vector_instance vector_number} {

    global VectorList messageQueue TimeVectors PRINT_DEBUG
    global GraphsInWindow

    set graph_name [format "%s.middle.flow" $window_name]
    set remove_list {}  
    set graphsInWindow $GraphsInWindow($window_name)
    #puts $graphsInWindow
	

    set counter 0
	set list2 {}
    foreach graph $graphsInWindow {
	set type [lindex $graph 0]
	set variable [lindex $graph 1]
	set instance [lindex $graph 2]
	set vector_id [lindex $graph 3]
	if {$vector_id == $vector_number} {
	    set index $counter
	    #$type == $vector_type && $instance == $vector_instanc\e
	    #puts "gonna dequeue $vector_id with $type, $instance"
	} else {
	    lappend list2 $graph
	}
	incr counter
    }
#    set graphsInWindow
    set GraphsInWindow($window_name) $list2

    set list2 {}
    foreach i $VectorList {
	set vector_type [lindex $i 4]
	set vector_instance [lindex $i 5]
	set vector_index [lindex $i 1]
	if {$vector_index == $vector_number} {
	    #$type == $vector_type && $instance == $vector_instanc\e
	    #puts "gonna dequeue $vector_id with $type, $instance"
	} else {
	    lappend list2 $i
	}
    }
    set VectorList $list2 


    set line [format "line%s" $vector_number]
    eval ${window_name}.middle.flow element delete $line


    lappend messageQueue 5778
    set info [format "%s:%s:%s" $vector_type $vector_instance $vector_number]
    if {$PRINT_DEBUG} {
	puts "message:$info"
    }
    set msg [padMessage $info]
    lappend messageQueue $msg

}


##############################################################################
# ConfirmSetUp takes a window name as an argument, and then instantiates a 
#trace on the graph with that window name.  It tells the graph what vectors
# will hold the values it has to graph.
#
##############################################################################

proc AssignDataToVectors {X Y vector w} {

    global TimeVectors VectorArray currentVector Graph_Colors_Used
    global shift_tk_dir

    set Xvector_name [format "xVector%s" $vector]
    set Yvector_name [format "yVector%s" $vector]
    set Xvector_name_init [format "xVector%s(0)" $vector]
    set Yvector_name_init [format "yVector%s(0)" $vector]
    set line [format "line%s" $vector]

    eval global $Xvector_name $Yvector_name
    eval "catch {unset $Xvector_name}"
    eval "catch {unset $Yvector_name}"
    eval vector $Xvector_name_init 
    eval vector $Yvector_name_init 
    set color [lindex $Graph_Colors_Used [expr $vector % 9]]
    $w.middle.flow element create $line \
	    -xdata $Xvector_name -ydata $Yvector_name -color $color



    if {$X == "Time"} {
	lappend TimeVectors $Xvector_name
	set lab [format "%s-vs-Time" $Y $X]
	$w.middle.flow element configure $line -label "$lab"
    } else {
	set lab [format "%s-vs-%s" $Y $X]
	$w.middle.flow element configure $line -label "$lab"

	### Create the new marker at (0,0) and make it unvisible (white). (Tolga 102197)
	$w.middle.flow marker create bitmap -bitmap @${shift_tk_dir}/images/marker.xbm -name pmarker_$vector -anchor center \
		-background "" -foreground white -coords {0 0}
    }
    set VectorArray($vector) $w
    set currentVector $vector
}


########################################################################
#
#
#
#
########################################################################


proc ConfirmSetUp {w} {

    global totalGraphs
    global resultX resultY

    set currentVector -1

    AssignDataToVectors $resultX $resultY [expr $totalGraphs +1] $w

    incr totalGraphs
}


##############################################################################
# This displays all the types, their varaibles, and the instances of the types
#that currently exist in the simulation.  The uses must select a type, a varia
#and an instance to graph.  If one of the choices is missing, a error message
#appears.
# Every time a new type is selected, the client communicates to the server to
#get the number and name of all instances of that type.
#Clicking on OK calls the SetVector procedure.
##############################################################################

proc AddTimePlot {w} {
    
    global GraphsInWindow

    set curr $GraphsInWindow($w)
    set items [InstanceListGitter 0 $curr]

    set send_graphs {}
    set to_remove {}

    foreach j $items {
	set found 0
	foreach i $curr {
	    set new_lab [list [format "%s(%s %s)" [lindex $i 1]\
		    [lindex $i 0] [lindex $i 2]]]
	    if {$new_lab == $j} {
		set found 1
	    } else {
		#set found 0
	    }
	}
	if {$found == 1} {
	    #Do nothing
	} else {
	    lappend send_graphs $j
	}
    }

    #puts "To send $send_graphs"

    foreach j $curr {
	set new_lab [list [format "%s(%s %s)" [lindex $j 1]\
		[lindex $j 0] [lindex $j 2]]]
	set found 0
	foreach i $items {
	    if {$i == $new_lab} {
		set found 1
		# Do nothing, this on is in both lists
	    } else {
		#set found 0
	    }
	}
	if {$found == 0} {
	    # Remove, this one in old and not in new
		lappend to_remove $j
	} else {
	    #Do nothing
	}
    }

    #puts "To remove $to_remove"

    if {[llength $to_remove] < 1} {
	#return
    } else {
	foreach item $to_remove {
	    RemoveSingleGraph $w [lindex $item 0] [lindex $item 1]\
		    [lindex $item 2] [lindex $item 3]
	}
    }
    
    if {[llength $send_graphs] < 1} {
	return
    } else {
	foreach item $send_graphs {
	    set myItem [lindex $item 0]
	    #puts "This item is $myItem"
	    set plotVariable [getVariableFromString $myItem]
	    set plotInstance [getInstanceFromString $myItem]
	    set plotType [getTypeFromString $myItem]
	    SendTimePlotRequest $w $plotType $plotVariable $plotInstance
	}
    }
}

##########################################################################
#
#
#
#
##########################################################################

# commented by duke 8/14/97 
# here the window field is made optional because this functin is now
# used by script function.  When script is running this procedure, of
# course, we are not going to have the window ready.  For script 
# case, AddPhase function in startup.tcl will eventually be called to 
# add this information into script data structure 

proc AddPhaseTrace {{w none}} {

    global TypeList TypesArray messageQueue InstanceTypeList
    global TempVar1 TempVar2 resultX resultY startup_windows 

    set win .phase
    if {[winfo exists $win] == 1} {
	catch {wm deiconify $win}
	catch {raise $win}	
	return
    } else {
	if {$w == "none"} {
	    
	    # commented by duke 8/14/97 
	    # we want to make this window disappear when we destroy
	    # script main panel. so we will keep track of the 
	    # window opened by script main panel. 

	    lappend startup_windows $win 
	}
    }

    toplevel .phase -height 400 -width 700 
    grab $win
#-bg ivory1
    #frame .phase.top
    frame .phase.middle -relief raised -bd 2 -height 350
    frame .phase.set
    frame .phase.axis -relief raised -bd 2
    frame .phase.bottom

    set xType 0

    set yType 0
    set xVar 0
    set yVar 0
    set xInst 0
    set yInst 0
   

    set VarList $TypesArray(global)

    set show_variables {}
    foreach variable $VarList {
	if {[lindex $variable 1] == "number"} {
	    lappend show_variables $variable
	}
    }
######

#### Commented out by Tolga (101697)
#    place .phase.middle -in .phase -anchor nw -x 0 -y 0 -y 0\
#	    -relwidth 1.0 -height 250
#    place .phase.set -y 250 -relwidth 1.0 -height 50
#    place .phase.axis -y 300 -relwidth 1.0 -height 50
#    place .phase.bottom -y 350 -relwidth 1.0 -height 50
#
#    place .phase.axis.left -relwidth .3 -relheight 1.0 -relx .0 -rely 0.0
#    place .phase.axis.right -relwidth .7 -relheight 1.0 \
#	    -relx .35 -rely 0.0
####

    #### Additional widgets to support creating multiple phase plots (Tolga 101697)
    ####
    pack .phase.middle .phase.set .phase.axis .phase.bottom \
	    -side top -fill x -padx 4 -pady 4

    pack [frame .phase.axis.up -relief groove -borderwidth 2] -fill x -padx 2 -pady 4
    pack [frame .phase.axis.mid] -fill x -pady 4
    pack [frame .phase.axis.down] -side bottom -fill x

    frame .phase.axis.up.left 
    frame .phase.axis.up.right

    pack .phase.axis.up.left -side left -fill both -expand yes -padx 10
    pack .phase.axis.up.right -side left -fill both -expand yes -padx 10

    button .phase.axis.mid.add -text "Add Selection" -width 16\
	    -command {
	set x_lab [.phase.axis.up.right.xValu cget -text]
	set y_lab [.phase.axis.up.right.yValu cget -text]
	
	if {($x_lab != "UNDEFINED") && ($y_lab != "UNDEFINED")} {
	    set new_lab "$x_lab vs $y_lab"
	    .phase.axis.plots insert end $new_lab
	} else {
	    tk_dialog .error Error "Undefined Axis!!" error 0 OK
	} 
    }
    pack .phase.axis.mid.add -side left -expand yes

    button .phase.axis.mid.del -text "Remove Selection" -width 16\
	    -command {
	set item [.phase.axis.plots curselection]
	if {$item == ""} {
	    tk_dialog .error Error "You must select and entry to remove." error 0 OK
	} else {
	    .phase.axis.plots delete $item
	}
    }
    pack .phase.axis.mid.del -side right -expand yes    

    listbox .phase.axis.plots -yscrollcommand ".phase.axis.vs set"\
	    -selectmode single\
	    -background gray -height 5
    scrollbar .phase.axis.vs -orient vertical\
	    -command ".phase.axis.plots yview"

    pack .phase.axis.plots -in .phase.axis.down \
	    -side left -fill both -expand 1
    pack .phase.axis.vs -in .phase.axis.down -side right -fill y
    set plb .phase.axis.plots
    ####
    #### Additional widget code ends


    selection_box .phase.middle.vars "Variables" -background grey
    selection_box .phase.middle.types "Simulation Types" -background grey
    selection_box .phase.middle.instances "Instances" -background grey

    foreach item $TypeList {
	.phase.middle.types.btm.list insert end $item
    }

    set curr_type [.phase.middle.types.btm.list get 0]
    set VarList $TypesArray($curr_type)
    set show_variables {}
    set variable_type 0
    foreach variable $VarList {
	if {$variable_type == 0} {
	    if {[lindex $variable 1] == "number"} {
		lappend show_variables $variable
	    }
	} elseif {$variable_type == 1} {
	    if { [lindex $variable 1] == "set" ||\
		    [lindex $variable 1] == "array"} {
		lappend show_variables $variable
	    }
	}
    }

    foreach item $show_variables {
	.phase.middle.vars.btm.list insert end $item
    }

    bind .phase.middle.types.btm.list <1> "+SelectComboBoxType $variable_type"

    bind .phase.middle.types.e <Return> "SelectComboBoxType $variable_type"

#### Commented out by Tolga (101697)
#    place .phase.middle.types -x 1 -y 1 -relwidth .3 -relx 0.025
#    place .phase.middle.vars -relwidth .3 -relx 0.35 -x 1 -y 1
#    place .phase.middle.instances -relwidth .3 -relx .675
####

    #### This replaces the place lines above (Tolga 101697)
    pack .phase.middle.types .phase.middle.vars .phase.middle.instances \
	    -side left -fill both

    button .phase.set.setX -text "Assign to X axis" -width 16\
	    -command {
	set type [.phase.middle.types.e get]
	set inst [lindex [.phase.middle.instances.e get] 1]
	set var [lindex [.phase.middle.vars.e get] 0]

	if {[CheckCompleteness $type $var $inst] == 1 } {
	    set new_lab [format "%s(%s %s)" $var $type $inst]
	    .phase.axis.up.right.xValu configure -text $new_lab
	} else {
	    tk_dialog .error Error "You must enter a type, a variable, and an instance" error 0 OK
	}

    }

    button .phase.set.setY -text "Assign to Y axis" -width 16\
	    -command {
	set type [.phase.middle.types.e get]
	set inst [lindex [.phase.middle.instances.e get] 1]
	set var [lindex [.phase.middle.vars.e get] 0]
	if {[CheckCompleteness $type $var $inst] == 1 } {
	    set new_lab [format "%s(%s %s)" $var $type $inst]
	    .phase.axis.up.right.yValu configure -text $new_lab
	} else {
	    tk_dialog .error Error "You must enter a type, a variable, and an instance" error 0 OK
	}
    }

#### Commented out by Tolga (101697)
#    place .phase.set.setX  -relwidth .3 -relx .1
#    place .phase.set.setY  -relwidth .3 -relx .6
####

    #### This replaces the place lines above (Tolga 101697)
    pack .phase.set.setX .phase.set.setY -side left -expand yes

    label .phase.axis.up.left.xLabel -text "X Axis:" -justify right
    label .phase.axis.up.left.yLabel -text "Y Axis:" -justify right

    label .phase.axis.up.right.xValu -text "UNDEFINED" -justify left
    label .phase.axis.up.right.yValu -text "UNDEFINED" -justify left

#### Commented out by Tolga (101697)
#    place .phase.axis.left.xLabel -rely .0 -relx .7
#    place .phase.axis.left.yLabel -rely .5 -relx .7
#
#    place .phase.axis.right.xValu -rely .0 -relx .0 
#    place .phase.axis.right.yValu -rely .5 -relx .0 
####

    #### These replace the place lines above (Tolga 101697)
    pack .phase.axis.up.left.xLabel -anchor ne
    pack .phase.axis.up.left.yLabel -anchor se
    pack .phase.axis.up.right.xValu -anchor nw
    pack .phase.axis.up.right.yValu -anchor sw


#    set TempVar1 $currentVector
#    set TempVar2 $axis
    
    if {$w == "none"} {
	button .phase.bottom.ok -text OK -command "SendPlotRequest"
    } else {
	button .phase.bottom.ok -text OK -command "SendPlotRequest $w"
    }

    button .phase.bottom.cancel -text "Cancel" -command "RemoveFromStartUpWindows $win; destroy .phase"

#### Commented out by Tolga (101697)
#    place .phase.bottom.ok -relwidth .3 -relx .1 -relheight .7 -rely .15
#    place .phase.bottom.cancel -relwidth .3 -relx .6 -relheight .7 -rely .15
#### 

    #### These replace the place lines above (Tolga 101697)
    pack .phase.bottom.ok -side left -expand yes
    pack .phase.bottom.cancel -side right -expand yes

    wm protocol $win WM_DELETE_WINDOW \
	    "RemoveFromStartUpWindows $win; destroy $win"
}

##########################################################################
#
#
#
#
##########################################################################

# commented by duke 8/14/97 
# please look at the comment above "proc AddPhaseTrace"

proc SendPlotRequest {{w none}} {
    
    global PRINT_DEBUG script_changed 

    
    ### first indicates whether this is the first phase plot in the bundle (Tolga 102097)
    set first 1

    ### Process every phase plot request contained in the plots listbox
    foreach pitem [.phase.axis.plots get 0 end] {
    
	#### Commented out by Tolga (101697)   
	#    set xPlot [.phase.axis.up.right.xValu cget -text]

	#### Every line in the plots listbox is in the form "$xValu vs $yValu".
	#### Thus extracting xValu involves searching for " vs " and extracting
	#### the string before it (Tolga 101697).
	set xPlot [string range $pitem 0 [expr [string first " vs " $pitem]-1]]

	set xVar [getVariableFromString $xPlot]
	set xInst [getInstanceFromString $xPlot]
	set xType [getTypeFromString $xPlot]

	#### Commented out by Tolga (101697)
	#    set yPlot [.phase.axis.up.right.yValu cget -text]

	#### Every line in the plots listbox is in the form "$xValu vs $yValu".
	#### Thus extracting yValu involves searching for " vs " and extracting
	#### the string after it (Tolga 101697).
	set yPlot [string range $pitem [expr [string last " vs " $pitem]+4] end]

	set yVar [getVariableFromString $yPlot]
	set yInst [getInstanceFromString $yPlot]
	set yType [getTypeFromString $yPlot]

	if {$PRINT_DEBUG} {
	    puts "have $xType $xVar $xInst $yType $yVar $yInst"
	}
	if {[CheckCompleteness $xType $xVar $xInst]== 1 \
		&& [CheckCompleteness $yType $yVar $yInst]== 1}  {
	
	    if {$w == "none"} {
		#### New Version of Script (Tolga 102197)
		#### Adds only one script item per bundle
		AddPhase [list $xVar $xType $xInst $yVar $yType $yInst] $first
		#### Old Version of Script (Tolga 102197)
		#	    AddPhase [list $xVar $xType $xInst $yVar $yType $yInst] 
	    } else {
		DispatchPhasePlot $w $xVar $xType $xInst $yVar $yType $yInst 
	    }
	
	    if {$w == "none"} {
		# commented by duke 8/14/97 
		# since .phase is added in the caller_function of this function 
		# which is AddPhaseTrace we need to take this out of the 
		# starup_windows that keeps track of all the windows that is
		# opened by the script main panel 
		RemoveFromStartUpWindows .phase 
	    }
	    destroy .phase;
	    
	} else {
	    tk_dialog .error Error "Please complete all information before clicking OK" error 0 OK
	}
	set first 0
    }
}

#############################################################################
#
#   This is just a helper function to dispatch the plot,  this was taken out 
#   of above function.  since it was going to be messy if it was there, after
#   adding script funcitonality.  Beside I think this segment can be 
#   a function by itself.  so why not? 
#
#############################################################################


proc DispatchPhasePlot {w xVar xType xInst yVar yType yInst} {
    global totalGraphs resultX resultY
    set currVector [expr $totalGraphs+1]
    
    set xresult [format "%s(%s %s)\n" $xVar $xType $xInst]
    set yresult [format "%s(%s %s)\n" $yVar $yType $yInst]
    
    set resultX $xresult
    set resultY $yresult
    SetVector $xType $xVar $xInst $currVector X $w;
    SetVector $yType $yVar $yInst $currVector Y $w;
	ConfirmSetUp $w
    # Make sure they don't do a time plot
    $w.bar.graph.menu entryconfigure 0 -state disabled
}


########################################################################
#
#
#
#
########################################################################


proc SendTimePlotRequest {w type variable instance} {

    global PRINT_DEBUG

    set yType $type
    set yVar $variable
    set yInst $instance
   
    if {$PRINT_DEBUG} {
	puts "have $yType $yVar $yInst"
    }
    if {[CheckCompleteness $yType $yVar $yInst]== 1}  {

	global totalGraphs resultX resultY
	set currVector [expr $totalGraphs+1]
	
	set xresult "Time"
	set yresult [format "%s(%s %s)\n" $yVar $yType $yInst]

	set resultX $xresult
	set resultY $yresult
	SetVector $yType $yVar $yInst $currVector Y $w;
	ConfirmSetUp $w
	#Make sure they don't do a phase plot afterwards
	$w.bar.graph.menu entryconfigure 1 -state disabled
    } else {
	tk_dialog .error Error "Please complete all information before clicking OK" error 0 OK

    }
    
}


##############################################################################
# This procedure is called by GetOpts when the user clicks on OK.  Here we make
#sure that the user has inserted all the needed information, or we return 0.
#
##############################################################################

proc CheckCompleteness {type var instance} {

    if {$type != ""} {
	if {$var != ""} {
	    if {$instance != ""} {
		return 1
	    } else {
		return 0
	    }
	} else {
	    return 0
	}
    }  else {
	return 0
    }
}
		

###############################################################################
# SetVector takes a type, a variable, an instance, a vector number, and an 
# axis.  It then updates the global variables VectorList and VectorInfoArray
#It then sends a message to the server telling it what information it will need
#to receive at every timeclick in the future.
#
###############################################################################


proc UpdateVectorArrays {type variable instance currentVector axis} {
    
    global VectorList VectorInfoArray

    lappend VectorList [list $variable $currentVector $axis $instance $type $instance]
    lappend VectorInfoArray($instance) [list $variable $currentVector $axis]
    
}


########################################################################
# SetVector sends the information we want to receive to the back end.
#
#
#
########################################################################


proc SetVector {type variable instance currentVector axis w} {

    global messageQueue PRINT_DEBUG GraphsInWindow

    ##Set the data structures
    lappend GraphsInWindow($w) [list $type $variable $instance\
	    $currentVector $axis]

    UpdateVectorArrays $type $variable $instance $currentVector $axis

    ## Send out the info to the simulation
    set instanceNo $instance
    lappend messageQueue 5550
    set info [format "%s:%s:%s:%s" $type $instanceNo $variable $currentVector]
    if {$PRINT_DEBUG} {
	puts "Info is :$info"
    }
    set msg [padMessage $info]
    lappend messageQueue $msg;

}

###############################################################################
# AssignValu takes a vector information and a value, and assigns the valu to
#the correct vector.
# Could do all this in a few lines with an eval
#
###############################################################################

proc AssignValu {variable currentVector axis valu {Marker NO} } {

    global VectorInfoArray TimeClick TimeVectors VectorArray 

    if {$axis == "X"} {
		
	set vector_g [format "xVector%s" $currentVector]
	set vector_name [format "xVector%s(++end)" $currentVector]
    } else {
	set vector_g [format "yVector%s" $currentVector]
	set vector_name [format "yVector%s(++end)" $currentVector]
    }

    #set line [format "line%s" $vector]

    eval global $vector_g
    eval set $vector_name $valu

    #### Code for the marker (Tolga 102197)
    #### Find out graph info:
    ### 1. What is the window name?
    set w $VectorArray($currentVector)
    #### 2. Is there a marker created for this plot?
    if {[$w.middle.flow marker exists pmarker_$currentVector]} {

	### 3. What are the old coordinates of the marker?
	set xy [$w.middle.flow marker cget pmarker_$currentVector -coords]
	set x [lindex $xy 0]
	set y [lindex $xy 1]

	### 4. What axis are we updating now?
	if {$axis == "X"} {
	    $w.middle.flow marker configure pmarker_$currentVector -coords [list $valu $y] -fg black
	} else {
	    $w.middle.flow marker configure pmarker_$currentVector -coords [list $x $valu]
	}
    }
    #### Marker code ends

}

##############################################################################
#
#
#
#
##############################################################################


proc ChangeVars {} {

    global TypesArray

    set type [.choices.middle.types get]
    set vars [list $TypesArray($type)]

    return $vars
    
}

proc ChangeVars2 {} {

    global TypesArray

    set type [.phase.middle.types.e get]
    set vars [list $TypesArray($type)]

    return $vars
}


