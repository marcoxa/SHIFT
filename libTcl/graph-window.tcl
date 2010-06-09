# -*- Mode: Tcl -*-

# graph.tcl --

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



##########################################################################3
#  Initialize vectors is called when the debugger starts.  It declares all 
# vectors, makes them global, and initializes them to be empty, so that no
# default values get into the graph.
#
############################################################################

proc initialize_vectors {} {

    global xVector yVector GraphedElement xVector1 yVector1 xVector2 
    global yVector2 xVector3 yVector3 xVector4 yVector4 xVector5 
    global yVector5 xVector6 yVector6 xVector7 yVector7 xVector8 
    global yVector8 xVector9 yVector9 xVector10 yVector10 xVector11 
    global yVector11

    set vectors {1 2 3 4 5 6 7 8 9 10 11}

    vector xVector(1)
    vector yVector(1)
    foreach i $vectors {
	set x_name [format "xVector%s(0)" $i]
	set y_name [format "yVector%s(0)" $i]
	eval vector $x_name
	eval vector $y_name
    }

}

proc uninitialize_vectors {} {
    
    global totalGraphs

    set totalVectors $totalGraphs
    set counter 0
    while {$counter < $totalVectors} {
	set x_name [format "xVector%s" $i]
	set y_name [format "yVector%s" $i]
	unset $x_name
	unset $y_name
	incr counter
    }
}

#############################################################################
#  CreateNewGraph keeps track of the number of graphs open. It calls 
# the function that creates the graph and gives it a unique name.
# To Do: Make sure that the graph name is unique.
#
#############################################################################


proc CreateNewGraph {} {

    global TotalGraphNumber
    
    
    incr TotalGraphNumber
    set w [SetUpGraph [expr $TotalGraphNumber-1]]
    return $w
}

##############################################################################
#  SetUpGraph creates a window with a BLT graph widget in it.  The window name
# is the name of the graph, with w_ as a prefix.  
# ToDo: When exiting the graph, erase the name of the graph from the list of
# existing grapth names.
#
##############################################################################

# commented by duke 8/14/97 
# you will notice that RemoveFromScriptWindows is added to precedures when you 
# exit this function.   It is alright if SetUpGraph is not called by script
# to use RemoveFromScirptWindows since RemoveFromScriptWindow will simply not
# take out a window that doesn't exist in data structure that keep information 
# about the windows opened by scripts 

proc SetUpGraph {graph_id} {

    global GraphedElement totalGraphs GraphsInWindow
    global OpenWindows TotalGraphs GraphNames OtherFont
    
    set name $graph_id
    set w .w_$name

    toplevel $w 
    set GraphsInWindow($w) {}
    wm title $w "Graph#$name"
    lappend OpenWindows $w
    frame $w.bar -relief raised -bd 2 -class menu
    frame $w.middle

    pack $w.bar -side top -fill both
    pack $w.middle -side top -fill both -expand 1
    
    graph $w.middle.flow -title $name

    createGraphBindings $w.middle.flow
    $w.middle.flow crosshairs on
    $w.middle.flow grid  on
    $w.middle.flow legend configure -relief raised \
	    -position left -font fixed -fg blue

    menubutton $w.bar.graph -text "Graph" -menu \
	    $w.bar.graph.menu

    menubutton $w.bar.configure -text "Configure" -menu \
	    $w.bar.configure.menu

    pack $w.bar.graph $w.bar.configure -side left
    set m [menu $w.bar.graph.menu -tearoff 0]
    set m2 [menu $w.bar.configure.menu -tearoff 0]

    wm protocol $w WM_DELETE_WINDOW \
	    "RemoveGraphData $w; RemoveFromStartUpWindows $w; DequeueFromOpenWindows $w; destroy $w"

    $m add command -label "Add Time Plot" -command "AddTimePlot $w"
    $m add command -label "Add Phase Plot" -command "AddPhaseTrace $w"
    $m add separator
    $m add command -label "Print to ps file" -command "PrintGraph $w"
    $m add command -label "Dump to file" -command "DumpGraph $w"
    $m add separator 
    $m add command -label "Close" -command "RemoveGraphData $w;\
	    DequeueFromOpenWindows $w; RemoveFromStartUpWindows $w; destroy $w"

    $m2 add command -label "Change Coordinates"\
	    -command "AxConfig $w.middle.flow"
    $m2 add command -label "Set Title" \
	    -command "SetNewGraphName $w";
    $m2 add command -label "Set Axis Names" \
	    -command "LabelXandYAxis $w";
    $m2 add separator 

    $m2 add command -label "Crosshairs off" -command "CrosshairsOff $w"
    $m2 add command -label "Grid off" -command "GridOff $w"
    $m2 add command -label "Legend off" -command "LegendOff $w"
    $m2 add command -label "Logscale on" -command "LogscaleOn $w"

    pack $w.middle.flow -fill both -expand 1

    return $w
}



proc CrosshairsOff {w} {
    
    global PRINT_DEBUG

    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    if {$PRINT_DEBUG} {
	puts "Graph number: $graph_number"
    }
    $w.middle.flow crosshairs toggle
    $w.bar.configure.menu entryconfigure 4 -label "Crosshairs on"
    $w.bar.configure.menu entryconfigure 4 -command "CrosshairsOn $w"

}

proc CrosshairsOn {w} {
    
    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    puts "Graph number: $graph_number"
    $w.middle.flow crosshairs toggle
    $w.bar.configure.menu entryconfigure 4 -label "Crosshairs off"
    $w.bar.configure.menu entryconfigure 4 \
	    -command "CrosshairsOff $w"
}


proc LogscaleOff {w} {
    
    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    $w.middle.flow yaxis configure -logscale no
    $w.bar.configure.menu entryconfigure 7 -label "Logscale on"
    $w.bar.configure.menu entryconfigure 7 -command "LogscaleOn $w"

}

proc LogscaleOn {w} {
    
    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    puts "Graph number: $graph_number"
    $w.middle.flow yaxis configure -logscale yes
    $w.bar.configure.menu entryconfigure 7 -label "Logscale off"
    $w.bar.configure.menu entryconfigure 7 \
	    -command "LogscaleOff $w"

}


proc GridOff {w} {
    
    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    puts "Graph number: $graph_number"
    $w.middle.flow grid off
    $w.bar.configure.menu entryconfigure 5 -label "Grid on"
    $w.bar.configure.menu entryconfigure 5 -command "GridOn $w"

}
proc GridOn {w} {
    
    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    puts "Graph number: $graph_number"
    $w.middle.flow grid on
    $w.bar.configure.menu entryconfigure 5 -label "Grid off"
    $w.bar.configure.menu entryconfigure 5 \
	    -command "GridOff $w"

}
proc LegendOff {w} {
    
    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    puts "Graph number: $graph_number"
    $w.middle.flow legend configure -mapped no
    $w.bar.configure.menu entryconfigure 6 -label "Legend on"
    $w.bar.configure.menu entryconfigure 6 -command "LegendOn $w"

}

proc LegendOn {w} {
    
    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    puts "Graph number: $graph_number"
    $w.middle.flow legend configure -mapped yes
    $w.bar.configure.menu entryconfigure 6 -label "Legend off"
    $w.bar.configure.menu entryconfigure 6 \
	    -command "LegendOff $w"
 
}

proc SetNewGraphName {graph_win} {

    global GraphNames

    set win .create
    if {[winfo exists $win] == 1} {
	catch {wm deiconify $win}
	catch {raise $win}	
	return
    }
    toplevel .create -height 130 -width 300
    set w .create
    frame $w.main -relief raised -bd 2
    frame $w.bottom
    wm title $w "Name graph"
    #    catch {grab set .create}

    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    #puts "Graph number: $graph_number"

    label $w.main.entry_lab  -text "Graph Name:"
    entry $w.main.entry
    
    place $w.main -height 80 -relwidth 1
    place $w.bottom -height 50 -relwidth 1 -y 80
    
    place $w.main.entry_lab -y 40 -x 10
    place $w.main.entry -y 40 -x 130 -relwidth .45
    
    button $w.bottom.ok -text "  OK " -command  \
	    "AssignName $graph_win"

    bind $w <Return>  "AssignName $graph_win"

    button $w.bottom.cancel -text "Cancel" \
	    -command {destroy .create}

    place $w.bottom.ok -relwidth .3 -relheight .7 -rely .15 -relx .1
    place $w.bottom.cancel -relwidth .3 -relheight .7 -rely .15 -relx .6
    
}


#####################################################################
#
#
#
#
######################################################################


proc LabelXandYAxis {graph_win} {

    global GraphNames

    set win .label
    if {[winfo exists $win] == 1} {
	catch {wm deiconify $win}
	catch {raise $win}	
	return
    }

    toplevel .label -height 250 -width 300
    set w .label 
    frame $w.main -relief raised -bd 2
    frame $w.bottom
    
    wm title $w "Set Graph Axis Names"
    set window_name [wm title $w]
    set delimiter [string first "#" $window_name]
    set graph_number [string range $window_name [expr $delimiter +1] \
	    end]
    
    #    puts "Graph number: $graph_number"

    label $w.main.entry1_lab -text "X Axis Label:"
    entry $w.main.entry1 
    label $w.main.entry2_lab  -text "Y Axis Label:"
    entry $w.main.entry2


    place $w.main -relwidth 1 -height 200
    place $w.bottom -relwidth 1 -height 50 -y 200

    place $w.main.entry1_lab -x 10 -y 40
    place $w.main.entry1 -x 130 -y 40 -relwidth .45
    place $w.main.entry2_lab -x 10 -y 120
    place $w.main.entry2 -x 130 -y 120 -relwidth .45

    button $w.bottom.ok -text "  OK " -command  \
	    "AssignLabels $graph_win"

    bind $w <Return>  "AssignLabels $graph_win"
    button $w.bottom.cancel -text "Cancel" \
	    -command "destroy $w"
    
    place $w.bottom.ok -relwidth .3 -relheight .7 -rely .15 -relx .1
    place $w.bottom.cancel -relwidth .3 -relheight .7 -rely .15 -relx .6

}

#####################################################################
#
#
#
#
######################################################################


proc AssignName {w} {

    $w.middle.flow configure -title [.create.main.entry get]
#    grab release .create
    destroy .create

}


#####################################################################
#
#
#
#
######################################################################


proc AssignLabels {w} {

    $w.middle.flow xaxis configure -title [.label.main.entry1 get]
    $w.middle.flow yaxis configure -title [.label.main.entry2 get]

    destroy .label

}



#############################################################################
## These functions inmplement the zooming in and out of a graph
##
#############################################################################


proc createGraphBindings {w} {

    global graphArray

    bind $w <ButtonPress-1> "CreateHighlightBinding $w %x %y"

    bind $w <B1-Motion> "CreateMotionBinding $w %x %y"

    bind $w <ButtonRelease-1> "CreateEndBinding $w %x %y"

    bind $w <ButtonRelease-3> "GraphZoomOut $w"

}

proc CreateHighlightBinding {w x y} {
    
    global graphArray
    
    set graphArray(x0) [TranslateXCoord $w $x]
    set graphArray(y0) [TranslateYCoord $w $y]
    set x0 $graphArray(x0)
    set y0 $graphArray(y0)
    CreateHighlight $w $x0 $y0
}

proc CreateMotionBinding {w x y} {

    global graphArray

    set graphArray(x1) [TranslateXCoord $w $x]
    set graphArray(y1) [TranslateYCoord $w $y]
    set x0 $graphArray(x0)
    set y0 $graphArray(y0)
    set x1 $graphArray(x1)
    set y1 $graphArray(y1)
    Highlight $w $x0 $y0 $x1 $y1
}

proc CreateEndBinding {w x y} {

    global graphArray

    set graphArray(x1) [TranslateXCoord $w $x]
    set graphArray(y1) [TranslateYCoord $w $y]
    set x0 $graphArray(x0)
    set y0 $graphArray(y0)
    set x1 $graphArray(x1)
    set y1 $graphArray(y1)
    DestroyHighlight $w
    GraphZoomIn $w $x0 $y0 $x1 $y1
}


proc GraphZoomOut {w} {

    $w xaxis configure -min "" -max ""
    $w yaxis configure -min "" -max ""


}


proc GraphZoomIn {w x0 y0 x1 y1} {
    
  
#  puts "Ranges $x0 and $y0 to $x1 $y1"
    if {($x0 == $x1) || ($y0 == $y1)} {
	return
    }

    if {$x0 < $x1} {
	$w xaxis configure -min $x0
	$w xaxis configure -max $x1
    } else {
	$w xaxis configure -max $x0
	$w xaxis configure -min $x1
    }

    if {$y0 < $y1} {
	$w yaxis configure -min $y0
	$w yaxis configure -max $y1
    } else {
	$w yaxis configure -max $y0
	$w yaxis configure -min $y1
    }
}
    
proc TranslateXCoord {w x} {
    
    set Xval [$w xaxis invtransform $x]
    return $Xval
}

proc TranslateYCoord {w y } {
    
    set Yval [$w yaxis invtransform $y]
    return $Yval
}


proc CreateHighlight {w x y} {
    $w marker create line -name "ZoomRegion" -dashes {2 4 2}
}


proc Highlight {w x0 y0 x1 y1} {

    $w marker configure "ZoomRegion" -coords {$x0 $y0 $x1 $y0 $x1 $y1 $x0 $y1 $x0 $y0}
}

proc DestroyHighlight {w} {
    $w marker delete "ZoomRegion"
}

proc PrintGraph {w} {
    
    global Orient Decor defaultGraphName

    set win .print
    if {[winfo exists $win] == 1} {
	catch {wm deiconify $win}
	catch {raise $win}	
	return
    }

    toplevel .print -height 8c -width 11c 
#-bg ivory1
    frame .print.middle -relief raised -bd 2
    frame .print.bottom

    radiobutton .print.middle.decor -variable Decor \
	    -text "Plain" -value 0
    radiobutton .print.middle.nodecor -variable Decor\
	    -text "Decorated" -value 1

    radiobutton .print.middle.port -variable Orient \
	    -text "Portrait" -value 0
    radiobutton .print.middle.land -variable Orient\
	    -text "Landscape" -value 1

    label .print.middle.entry_lab -text "File Name:"
    entry .print.middle.entry 
    .print.middle.entry insert 0 $defaultGraphName

    button .print.bottom.ok -text "OK" -command "PrintG $w"
    button .print.bottom.cancel -text "Cancel" -command "destroy .print"

    place .print.middle -relwidth 1 -relheight .8 
    place .print.bottom -relwidth 1.0 -relheight .18 -rely .82

    place .print.middle.decor -relwidth .4 -relheight .2 -relx 0.05\
	    -rely 0.05
    place .print.middle.nodecor -relwidth .4 -relheight .2 -relx 0.55\
	    -rely 0.05

    place .print.middle.port -relwidth .4 -relheight .2 -relx 0.05\
	    -rely 0.35
    place .print.middle.land -relwidth .4 -relheight .2 -relx 0.55\
	    -rely 0.35

    place .print.middle.entry_lab -rely .7\
	    -x 10
    place .print.middle.entry -relwidth 0.5 -rely .7\
	    -x 130

    place .print.bottom.ok -relwidth .3 -relx .1
    place .print.bottom.cancel -relwidth .3 -relx .6

}

proc DumpGraph {w} {
    
    global Orient Decor defaultDumpName

    set win .dump
    if {[winfo exists $win] == 1} {
	catch {wm deiconify $win}
	catch {raise $win}	
	return
    }

    toplevel .dump -height 8c -width 11c 
#-bg ivory1
    frame .dump.middle -relief raised -bd 2
    frame .dump.bottom

    radiobutton .dump.middle.decor -variable Decor \
	    -text "2 columns" -value 1
    radiobutton .dump.middle.nodecor -variable Decor\
	    -text "x-graph" -value 2

    radiobutton .dump.middle.port -variable Decor \
	    -text "gnuplot" -value 3
    radiobutton .dump.middle.land -variable Decor\
	    -text "Other" -value 4

    label .dump.middle.entry_lab -text "File Name:"
    entry .dump.middle.entry 
    .dump.middle.entry insert 0 $defaultDumpName

    button .dump.bottom.ok -text "OK" -command "DumpG $w $Decor"
    button .dump.bottom.cancel -text "Cancel" -command "destroy .dump"

    place .dump.middle -relwidth 1 -relheight .8 
    place .dump.bottom -relwidth 1.0 -relheight .18 -rely .82

    place .dump.middle.decor -relwidth .4 -relheight .2 -relx 0.05\
	    -rely 0.05
    place .dump.middle.nodecor -relwidth .4 -relheight .2 -relx 0.55\
	    -rely 0.05

    place .dump.middle.port -relwidth .4 -relheight .2 -relx 0.05\
	    -rely 0.35
    place .dump.middle.land -relwidth .4 -relheight .2 -relx 0.55\
	    -rely 0.35

    place .dump.middle.entry_lab -rely .7\
	    -x 10
    place .dump.middle.entry -relwidth 0.5 -rely .7\
	    -x 130

    place .dump.bottom.ok -relwidth .3 -relx .1
    place .dump.bottom.cancel -relwidth .3 -relx .6

}

######################################################################
# This is the dumpgraph fuinction. It takes the name of a window, a
# dumping method as paramters. It opens a file, puts the desired data
# into it from the graph vectors, and returns.
## DW Sep 29, 1997
#################################################################

proc DumpG {w graphtype} {

    global GraphsInWindow 

    
    set GName [.dump.middle.entry get]

    if {$GName == ""} {
	tk_dialog .error Error "Please enter a name for the dump file" error 0 OK
	return
    }
    set f [open $GName w]
    set v_list $GraphsInWindow($w)
    puts $v_list

    foreach item $v_list {
	puts $f "\n"
	set vector_number [lindex $item 3]
	set Xvector_name "xVector$vector_number"
	set Yvector_name  "yVector$vector_number"
	eval global $Xvector_name $Yvector_name

	set len [$Xvector_name length]
	set len2 [$Yvector_name length]
	set counter 1
	while {$counter < $len} {
	    set X_data_name [format "$%s(%s)" $Xvector_name $counter]
	    set Y_data_name [format "$%s(%s)" $Yvector_name $counter]
	    eval puts -nonewline $f "$X_data_name"
	    puts -nonewline $f "\t "
	    eval puts $f $Y_data_name
	    #puts $f "\n"  
	    incr counter 
	}
    }
    close $f
    destroy .dump
}


proc PrintG {w} {

    global Decor Orient

    set GName [.print.middle.entry get]
    
    $w.middle.flow postscript output $GName -decorations $Decor\
	    -maxpect yes -landscape $Orient

    destroy .print
}

#############################################################################
##
##
##
#############################################################################


proc AxConfig {w} {

    global CurrentAxConfigure AutoConfigOn

    set win .axconfig
    if {[winfo exists $win] == 1} {
	catch {wm deiconify $win}
	catch {raise $win}	
	return
    }
    toplevel .axconfig -height 300 -width 350
#-bg ivory1

    frame .axconfig.middle -relief raised -bd 2
    frame .axconfig.bottom
    frame .axconfig.middle.auto -relief ridge -bd 2
    frame .axconfig.middle.manu -relief ridge -bd 2

    
    label .axconfig.middle.manu.xmin_lab -text "Minimum X value"
    label .axconfig.middle.manu.ymin_lab -text "Minimum Y value"
    label .axconfig.middle.manu.xmax_lab -text "Maximum X value"
    label .axconfig.middle.manu.ymax_lab -text "Maximum Y value"

    entry .axconfig.middle.manu.xmin
    entry .axconfig.middle.manu.ymin
    entry .axconfig.middle.manu.xmax
    entry .axconfig.middle.manu.ymax
    
    set xminValu [lindex [$w xaxis configure -min] 4]
    set xmaxValu [lindex [$w xaxis configure -max] 4]
    set yminValu [lindex [$w yaxis configure -min] 4]
    set ymaxValu [lindex [$w yaxis configure -max] 4]
    
    .axconfig.middle.manu.xmin insert 0 $xminValu
    .axconfig.middle.manu.ymin insert 0 $yminValu
    .axconfig.middle.manu.xmax insert 0 $xmaxValu
    .axconfig.middle.manu.ymax insert 0 $ymaxValu

    if {$AutoConfigOn == 1} {
	.axconfig.middle.manu.xmin configure -state disabled
	.axconfig.middle.manu.ymin configure -state disabled
	.axconfig.middle.manu.xmax configure -state disabled
	.axconfig.middle.manu.ymax configure -state disabled
    }

#    puts "Current Xmin value is $xminValu AND OTYHr: $xmaxValu"
    set CurrentAxConfigure $w

    checkbutton .axconfig.middle.auto.auto -text "AutoConfigure Graph" -variable \
	    AutoConfigOn -command {
	global AutoConfigOn
	if {$AutoConfigOn == 1} {
	    .axconfig.middle.manu.xmin configure -state disabled
	    .axconfig.middle.manu.ymin configure -state disabled
	    .axconfig.middle.manu.xmax configure -state disabled
	    .axconfig.middle.manu.ymax configure -state disabled
	} else {
	    .axconfig.middle.manu.xmin configure -state normal
	    .axconfig.middle.manu.ymin configure -state normal
	    .axconfig.middle.manu.xmax configure -state normal
	    .axconfig.middle.manu.ymax configure -state normal
	}   
    }


    
    button .axconfig.bottom.ok -text "OK" -command {
	global CurrentAxConfigure

	if {$AutoConfigOn == 0} { 
	    set w $CurrentAxConfigure
	    $w xaxis configure -min [.axconfig.middle.manu.xmin get]
	    $w xaxis configure -max [.axconfig.middle.manu.xmax get]
	    $w yaxis configure -min [.axconfig.middle.manu.ymin get]
	    $w yaxis configure -max [.axconfig.middle.manu.ymax get]
	} else {
	    set w $CurrentAxConfigure
	    $w xaxis configure -min {}
	    $w xaxis configure -max {}
	    $w yaxis configure -min {}
	    $w yaxis configure -max {}
	}
	destroy .axconfig
    }
    
    
    button .axconfig.bottom.cancel -text "Cancel" -command {destroy .axconfig }


#    labeledwidget::alignLabels .axconfig.middle.xmin .axconfig.middle.ymin .axconfig.middle.xmax .axconfig.middle.ymax


    place .axconfig.middle -relwidth 1 -height 250
    place .axconfig.bottom -relwidth 1 -height 50 -y 250
    place .axconfig.middle.auto -relwidth 1 -height 50 -y 10
    place .axconfig.middle.manu -relwidth 1 -height 170 -y 60
    place .axconfig.middle.auto.auto -y 10 -x 10
    place .axconfig.middle.manu.xmin_lab -x 10 -y 30
    place .axconfig.middle.manu.xmin -x 160 -y 30 -relwidth .35
    place .axconfig.middle.manu.ymin_lab -x 10 -y 60
    place .axconfig.middle.manu.ymin -x 160 -y 60 -relwidth .35
    place .axconfig.middle.manu.xmax_lab -x 10 -y 90
    place .axconfig.middle.manu.xmax -x 160 -y 90 -relwidth .35
    place .axconfig.middle.manu.ymax_lab -x 10 -y 120
    place .axconfig.middle.manu.ymax -x 160 -y 120 -relwidth .35
    
#    pack .axconfig.middle.xmin .axconfig.middle.ymin .axconfig.middle.xmax .axconfig.middle.ymax -side top -fill x -expand 1 -pady 3m -padx 2m

    place .axconfig.bottom.ok -relwidth .3 -relx .1 -relheight .7\
	    -rely .15
    place .axconfig.bottom.cancel -relwidth .3 -relx .6 -relheight .7\
	    -rely .15
}


