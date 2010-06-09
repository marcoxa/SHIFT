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


set currentSetupObjects(animation) 0
set currentSetupObjects(graph) 0
set currentSetupObjects(break) 0
set currentSetupObjects(phase) 0
set currentSetupObjects(stop) 0
#set currentSetupObjects(animation) 0
set setupFile NoName

#############################################################################
#
#  Explanation of new data structures
# 
#  pending_scritps:  keeps scripts that is not yet executed but will be 
#                    executed in future 
#  
#  script_windows:   this keeps information about the scripts that are 
#                    currently running.  This strucutre holds information 
#                    how to end the script when it expires 
#
#  running_scripts:  this holds information about all the scripts that
#                    have been spawn off from this script main penal. 
#                    this prevents user from running the same script 
#                    twice.  In order for the user to rerun a script
#                    he/she has to exit and reopen the script main 
#                    panel 
#
#  startup_windows:  this is a list of all "non-script" windows that is 
#                    open by script main panel.  By non-script, I mean that 
#                    excluding the windows opened by the scripts.  This is
#                    is used to clean up all the windows when exiting from 
#                    script main panel 
#
#############################################################################


#############################################################################
#
#  Before you do anything lose any of the changes that you might have made 
#  you should call this function.  This funtion will prompt the user of the 
#  potential danger before it actually executes user command 
#
#############################################################################

proc AskToSaveChanges {{next_command null} {lb null}} {    
    
    global script_changed itemselect simulation 
    
    if {$script_changed == "1"} {
	
	bell 
	set selection [tk_dialog .confirm "Confirm" "Changes are not saved! Save changes?" warning 0 "Cancel" "No" "Yes"]
	
	if {$selection == "1"} {
	    set itemselect(current_selection) ""

	    if {[AskToQuitScripts]} {
		$lb delete 0 end 
		set script_changed 0; DestroyStartUpWindows; eval $next_command
	    }
	} elseif {$selection == "2"} {
	    set itemselect(current_selection) ""
	    if {[AskToQuitScripts]} {
		$lb delete 0 end 
		startupSaveToFile; set script_changed 0; DestroyStartUpWindows; eval $next_command
	    } 
	}
    } else {
	set itemselect(current_selection) ""

	if {[AskToQuitScripts]} {
	    $lb delete 0 end 
	    DestroyStartUpWindows
	    eval $next_command 
	}
    }
}

#############################################################################
#
#  AskToQuitScripts:
#  If there are scripts already running, you want to prompt the user if 
#  he/she really wants to quit, destroying the running scripts in the process
#
#############################################################################

proc AskToQuitScripts {} {

    global running_scripts startupData

    set current_file $startupData(file)

    if {[lsearch [array name running_scripts] $current_file] != -1} {
	if {$running_scripts($current_file) != ""} {
	    
	    # something is running 
	    
	    return [tk_dialog .warning Warning "There are one or more scripts already running.  Do you really want to do this?" warning 0 No Yes]
	}
    }
    return 1 
}

#############################################################################
#
#  We keep list of the windows that is open by script, when this function 
#  is called it will get rid of all the windows that created by script
#  do necessary things to clean up after destroying windows 
#
#############################################################################

proc DestroyStartUpWindows {} {
    
    global startup_windows script_windows pending_scripts running_scripts 
    global startupData itemselect 

    if {[catch {set startup_windows $startup_windows}] != 0} {
	foreach w $startup_windows {
	    catch {destroy $w}
	}
	set startup_windows "" 
    }

    if {[array exists startupData]} {

	if {[lsearch [array name running_scripts] $startupData(file)] != -1} {
	    set running_scripts_list $running_scripts($startupData(file))
	    foreach script_name $running_scripts_list {
		set itemselect(current_selection) $script_name
		QuitSelection 
	    }
	    set itemselect(current_selection) "" 
	}
    }

    if {[array exists script_windows]} {
	unset script_windows 
    }
    
    if {[array exists pending_scripts]} {
	unset pending_scripts
    }

    if {[array exists startupData]} {
	unset startupData 
    }

}


#############################################################################
#
#   This is the main procedure for script function 
#
#############################################################################

proc CreateStartFile {} {
    
    global currentSetupObjects startupData Fonts setupFile script_changed 
    global startup_windows itemselect running_scripts startup_listbox 

    if {[winfo exists .start]} {
	catch {wm deiconify .start}
	catch {raise .start}
	return
    } else {
	startupDataStructureInit
    }
    
    #    option add
    set itemselect(current_selection) ""
    set itemselect(current_location) 0
    set startupData(file) ""
    set startupData(filename) "" 
    set script_changed 0 
    
    toplevel .start -height 300 -width 500
    
    set w .start
    frame $w.bar -relief raised -bd 2
    frame $w.main
    frame $w.main.left
    frame $w.main.right
    
    menubutton $w.bar.file -text "Files" -menu $w.bar.file.m
    menubutton $w.bar.add -text "Add " -menu $w.bar.add.m
    
    set m [menu $w.bar.file.m -tearoff 0]
    set m2 [menu $w.bar.add.m -tearoff 0]
    
    set t .start.main.right
    set lb [listbox $t.list \
	    -yscrollcommand [list $t.scroll set]]
    set startup_listbox $lb 
    scrollbar $t.scroll -command [list $lb yview]
    
    bind $lb <1> "start_itemselectClick %W %y; start_typeselectClick"

    $m add command -label "New" \
	    -command "AskToSaveChanges CreateStartupFile $lb"
    $m add command -label "Open" \
	    -command "AskToSaveChanges FindStartupFile $lb"
    $m add command -label "Save" -command "startupSaveToFile; set script_changed 0"
    #$m add command -label "Save As"
    $m add separator
    $m add command -label "Close"  -command "AskToSaveChanges {destroy .start} $lb"
    
    $m2 add command -label "Animation" \
	    -command "CanvasMenu 1 1" -state disabled
    $m2 add command -label "Time Plot"\
	    -command "AddGraph" -state disabled
    $m2 add command -label "Phase Plot"\
	    -command "AddPhaseTrace" -state disabled
    $m2 add command -label "Breakpoint"\
	    -command "TypesBox CreateStartFile" -state disabled
    $m2 add command -label "Stop"\
	    -command "DisplayStop" -state disabled
    $m2 add command -label "Trace"\
	    -command {} -state disabled

    label $w.main.left.file_lab -text "File :"
    label $w.main.left.file -textvariable startupData(file) 

    label $w.main.left.curr_lab -text "Selection:"
    label $w.main.left.curr -textvariable itemselect(current_selection)
    
    button $w.main.left.view_curr -text "Edit Selection" -command\
	    "ViewCurr"
    
    button $w.main.left.remove_curr -text "Remove Selection" -command\
	    "RemoveCurr $lb"

    button $w.main.left.run_curr -text "Run Selection" -command "RunSelection"
    button $w.main.left.run_all -text "Run All" -command "RunAllSelection $lb"
    button $w.main.left.stop_curr -text "Stop Selection" -command "QuitSelection"
	    
    grid rowconfigure $w 0 -weight 0
    grid rowconfigure $w 1 -weight 1
    grid columnconfigure $w 0 -minsize 400 -weight 1
    
    grid $w.bar -row 0 -column 0 -sticky we
    grid $w.main -row 1 -column 0 -sticky news
    grid columnconfigure $w.bar 0 -weight 0
    grid columnconfigure $w.bar 1 -weight 1
    grid columnconfigure $w.main 0 -minsize 200 -weight 1
    grid columnconfigure $w.main 1 -minsize 200 -weight 1

    grid columnconfigure $w.main.left 0 -minsize 50 -weight 0
    grid columnconfigure $w.main.left 1 -weight 1

    grid rowconfigure $w.main.left 0 -minsize 30
    grid rowconfigure $w.main.left 1 -minsize 30
    grid rowconfigure $w.main.left 2 -minsize 30
    grid rowconfigure $w.main.left 3 -minsize 30
    grid rowconfigure $w.main.left 4 -minsize 30

    grid $w.main.left -row 0 -column 0 -sticky news
    grid $w.main.right -row 0 -column 1 -sticky news

    pack $w.main.right.scroll -side right -fill y
    pack $w.main.right.list -side left -fill both

    grid $w.bar.file -row 0 -column 0 -sticky w
    grid $w.bar.add -row 0 -column 1 -sticky w
    
    grid $w.main.left.file_lab -column 0 -row 0 -sticky nw
    grid $w.main.left.file -column 1 -row 0 -sticky w
    grid $w.main.left.curr_lab  -column 0 -row 1 -sticky w
    grid $w.main.left.curr  -column 1 -row 1 -sticky w

    grid $w.main.left.view_curr -columnspan 2 -row 3 -sticky we
    grid $w.main.left.remove_curr  -columnspan 2 -row 4 -sticky we
    grid $w.main.left.run_curr  -columnspan 2 -row 5 -sticky we
    grid $w.main.left.run_all  -columnspan 2 -row 6 -sticky we
    grid $w.main.left.stop_curr -columnspan 2 -row 7 -sticky we

    wm protocol $w WM_DELETE_WINDOW "DestroyStartUpWindows; destroy $w"
    
}

proc RemoveCurr {lb} {

    global itemselect startupData script_changed running_scripts 
    
    set curr $itemselect(current_selection)
    
    if {$curr != ""} {
	if {[lsearch $running_scripts($startupData(file)) $curr] != -1} {
	    bell 
	    # it is currently running 
	    set yes [tk_dialog .caution Caution "The script is currently running, are you sure?" warning 0 No Yes]
	    if {$yes == 0} {
		return 
	    }
	}
	QuitSelection 
	$lb delete $itemselect(current_location)
    } else {
	bell
	tk_dialog .error Error "You must select an item to delete" error 0 OK
	return
    }
    
    set script_changed 1 

    set type [lindex $curr 0]
    set item [lindex $curr 1]
    if {$type == "Animation"} {
	unset startupData(animations,$item)
	unset startupData(animations,$item,start_time)
	unset startupData(animations,$item,end_time)
	set current_list $startupData(animations,-1)
	set index [lsearch $current_list $item]
	set current_list [lreplace $current_list $index $index]
	set startupData(animations,-1) $current_list
    } elseif {$type == "Graph"} {
	unset startupData(graphs,$item)
	unset startupData(graphs,$item,start_time)
	unset startupData(graphs,$item,end_time)
	unset startupData(graphs,$item,title)
	unset startupData(graphs,$item,xtext)
	unset startupData(graphs,$item,ytext)
	set current_list $startupData(graphs,-1)
	set index [lsearch $current_list $item]
	set current_list [lreplace $current_list $index $index]
	set startupData(graphs,-1) $current_list
    } elseif {$type == "Phase"} {
	unset startupData(phase,$item)
	unset startupData(phase,$item,start_time)
	unset startupData(phase,$item,end_time)
	unset startupData(phase,$item,title)
	unset startupData(phase,$item,xtext)
	unset startupData(phase,$item,ytext)
	set current_list $startupData(phase,-1)
	set index [lsearch $current_list $item]
	set current_list [lreplace $current_list $index $index]
	set startupData(phase,-1) $current_list
    } elseif {$type == "Break"} {
	unset startupData(break,$item)
	unset startupData(break,$item,start_time)
	unset startupData(break,$item,end_time)
	set current_list $startupData(break,-1)
	set index [lsearch $current_list $item]
	set current_list [lreplace $current_list $index $index]
	set startupData(break,-1) $current_list
    } elseif {$type == "Stop"} {
	unset startupData(stop,$item)
	unset startupData(stop,$item,start_time)
	unset startupData(stop,$item,end_time)
	set current_list $startupData(stop,-1)
	set index [lsearch $current_list $item]
	set current_list [lreplace $current_list $index $index]
	set startupData(stop,-1) $current_list
    }

    $lb selection clear 0 end
    set itemselect(current_selection) ""
}

proc ViewCurrent {} {

    global itemselect

    set curr $itemselect(current_selection)

    if {$curr == ""} {
	bell
	tk_dialog .error Error "You have not made a selection" error 0 OK
	return
    } elseif {[lindex $curr 0] == "Animation"} {
	DisplayAnim $curr
    } elseif {[lindex $curr 0] == "Graph"} {
	DisplayGraph $curr graphs 
    } elseif {[lindex $curr 0] == "Phase"} {
	DisplayGraph $curr phase 
    } elseif {[lindex $curr 0] == "Break"} {
	DisplayBreak $curr
    } elseif {[lindex $curr 0] == "Stop"} {
	DisplayStop $curr
    } elseif {[lindex $curr 0] == "Trace"} {
	DisplayTrace $curr
    }
}


proc start_itemselectClick { lb y } {
    # Take the item the user clicked on
    global itemselect ListComponents messageQueue

    set itemselect(current_selection) [$lb get [$lb nearest $y]]
    set itemselect(current_location) [$lb nearest $y]
}


proc start_typeselectClick {} {
    
    global itemselect

    #puts "$itemselect(current_selection)"
    set type [lindex $itemselect(current_selection) 0]
    set name [lindex $itemselect(current_selection) 1]


}


#########################################################################
#
#  This function is called by ok button of the animation configuration 
#  window, and it is only called when it is called from startup 
#
#########################################################################


proc RecordDecide {ImageChoice} {

    global CanvasXPlot CanvasYPlot CanvasType ContainerVariable CanvasState
    global ImageToUse MultiplierToUse RadiusToUse HeightToUse
    global WidthToUse BitmapToUse CanvasToUse 

    set CanvasState [.menu.middle.left.state.e get]
    set AnimationType [.menu.middle.left.typechoice.e get]
    set XPlot  [.menu.middle.left.xchoice.e get]
    set YPlot  [.menu.middle.left.ychoice.e get]
    set Instances [.menu.middle.left.instances.e get]
    if {[llength $Instances] > 1} {
	set Instances [lindex $Instances 1]
    }

    set container_string $ContainerVariable
    set containerVariable [getVariableFromString $container_string]
    set containerInstance [getInstanceFromString $container_string]
    set containerType [getTypeFromString $container_string]

    set Orientation  [.menu.middle.left.orientation.e get]
    #puts "Orientation is $Orientation"
    if {$Orientation == "None"} {
	set Orientation Nil
    }
    ## Need to get the contain variables sorted out here

    set valu [VariablesExist $AnimationType $XPlot $YPlot]
    if {$valu == 0} {
	bell
	tk_dialog .error Error "The Type or Variables you requested do not exist" error 0 OK
	return

    }

    if {$ImageChoice  != "fixed"} {
	set RadiusVariable [.menu.lowtop.choices.radius.c.e get]
	set HeightVariable [.menu.lowtop.choices.height.c.e get]
	set WidthVariable [.menu.lowtop.choices.width.c.e get]
	if {$ImageToUse == "oval"} {
	    if {$RadiusVariable == ""} {
		bell
		tk_dialog .error Error "You have not specified the\
			names of the variables defining the variable object size." error 0 OK
		return
	    }
	} elseif {$ImageToUse == "rectangle"} {
	    if {$HeightVariable == "" && $WidthVariable == ""} {
		bell
		tk_dialog .error Error "You have not specified the\
			names of the variables defining the variable object size." error 0 OK
		return
	    }
	}
    }
	

    set N 0	
    if {$ImageChoice  == "variable"} {
	set N 1
    } else {
	set N 2
    }
	
    if {$ImageToUse == "oval"} {
	if {$ImageChoice == "fixed"} {
	    set item [list $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $RadiusToUse\
		    0\
		    $Orientation\
		    -1\
		    RecordDecide]
	    set package [list Animation $item]
	} elseif {$ImageChoice == "variable"} {
	    set RadiusVariable [.menu.lowtop.choices.radius.c.e get]
	    set item [list \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $RadiusVariable\
		    NONE \
		    $Orientation\
		    $N\
		    RecordDecide]
	    set package [list Animation $item]
	} else {
	    set RadiusVariable [.menu.lowtop.choices.radius.c.e get]
	    set item [list \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $RadiusVariable\
		    NONE \
		    $Orientation\
		    $N\
		    RecordDecide]
	    set package [list Animation $item]
	}
	destroy .menu 
    } elseif {$ImageToUse == "rectangle"} {
	if {$ImageChoice == "fixed"} {
	    set item [list \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $HeightToUse\
		    $WidthToUse\
		    $Orientation\
		    -1\
		    RecordDecide]
	    set package [list Animation $item]
	} elseif  {$ImageChoice == "variable"} {
	    set HeightVariable [.menu.lowtop.choices.height.c.e get]
	    set WidthVariable [.menu.lowtop.choices.width.c.e get]
	    set item [list \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $HeightVariable\
		    $WidthVariable\
		    $Orientation\
		    $N\
		    RecordDecide]
	    set package [list Animation $item]
	} else {
	    set HeightVariable [.menu.lowtop.choices.height.c.e get]
	    set WidthVariable [.menu.lowtop.choices.width.c.e get]
	    set item [list \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $HeightVariable\
		    $WidthVariable\
		    $Orientation\
		    $N\
		    RecordDecide]
	    set package [list Animation $item]
	}
	destroy .menu 
    } else {  
	## It's a bitmap
	set item [list\
		$CanvasToUse\
		$XPlot \
		$YPlot \
		$Instances\
		$AnimationType \
		$ImageToUse \
		$MultiplierToUse\
		$CanvasState\
		$containerVariable\
		$containerType\
		$containerInstance\
		$BitmapToUse\
		0\
		$Orientation\
		-1\
		RecordDecide]
	set package [list Animation $item]
	destroy .menu
    }
    addDebugItem $package
}
 

proc addDebugItem {item} {

    global startupData

    set type [lindex $item 0]

    switch $type {
	Animation { 
	    set tot $startupData(animations,total)
	    incr startupData(animations,total)
	    incr tot
	    set label "Animation $tot"
	    set startupData(animations,$tot) [lindex $item 1]
	    lappend startupData(animations,-1) $tot
	    .start.main.right.list insert end $label

	    set startupData(animations,$tot,start_time) 0 
	    set startupData(animations,$tot,end_time) infinity
	}
	Graph {
	    set tot $startupData(graphs,total)
	    incr startupData(graphs,total)
	    incr tot
	    set label "Graph $tot"
	    set startupData(graphs,$tot) [lindex $item 1]
	    lappend startupData(graphs,-1) $tot
	    .start.main.right.list insert end $label

	    set startupData(graphs,$tot,start_time) 0 
	    set startupData(graphs,$tot,end_time) infinity

	    set startupData(graphs,$tot,title) ""
	    set startupData(graphs,$tot,xtext) ""
	    set startupData(graphs,$tot,ytext) ""

	}
	Phase {
	    set tot $startupData(phase,total)
	    incr startupData(phase,total)
	    incr tot
	    set label "Phase $tot"

	    ### Build a list of phase graphs instead of a single phase graph (Tolga 102097)
	    set startupData(phase,$tot) [list [lindex $item 1]]

	    lappend startupData(phase,-1) $tot
	    .start.main.right.list insert end $label

	    set startupData(phase,$tot,start_time) 0 
	    set startupData(phase,$tot,end_time) infinity

	    set startupData(phase,$tot,title) ""
	    set startupData(phase,$tot,xtext) ""
	    set startupData(phase,$tot,ytext) ""

	}
	Break {
	    set tot $startupData(break,total)
	    incr startupData(break,total)
	    incr tot
	    set label "Break $tot"
	    set startupData(break,$tot) [lindex $item 1]
	    lappend startupData(break,-1) $tot
	    .start.main.right.list insert end $label
	    
	    set startupData(break,$tot,start_time) 0 
	    set startupData(break,$tot,end_time) infinity
	}
	Stop {
	    set tot $startupData(stop,total)
	    incr startupData(stop,total)
	    incr tot
	    set label "Stop $tot"
	    set startupData(stop,$tot) [lindex $item 1]
	    lappend startupData(stop,-1) $tot
	    .start.main.right.list insert end $label
	    
	    set startupData(stop,$tot,start_time) 0 
	    set startupData(stop,$tot,end_time) infinity
	}
	Trace {
	    
	    # not yet implemented 

	}
    }
}

proc startupDataStructureInit {} {

    global startupData startup_windows running_scripts 

    set startupData(animations,-1) {}
    set startupData(animations,total) 0
    set startupData(graphs,-1) {}
    set startupData(graphs,total) 0
    set startupData(phase,-1) {}
    set startupData(phase,total) 0
    set startupData(break,-1) {}
    set startupData(break,total) 0
    set startupData(traces,-1) {}
    set startupData(traces,total) 0
    set startupData(stop,-1) {}
    set startupData(stop,total) 0
    set startupData(simulation) "" 

    set startup_windows ""
    set running_scripts(empty) "" 
}

######################################################################
#
#
#
#
#
######################################################################


proc CreateStartupFile {} {
    
    global startupData simulation startup_windows 
    
    set w .new_file

    if {[winfo exists $w]} {
	catch {wm deiconify $w}
	catch {raise $w}
	return
    }

    lappend startup_windows $w 
    toplevel $w -height 150 -width 300 -bg ivory1
    catch {raise $w}
    frame $w.middle -relief raised -bd 2
    frame $w.bottom

    label .new_file.middle.fileName_lab -text "Name: "
    entry .new_file.middle.fileName
    .new_file.middle.fileName insert 0 "startup.suf"
    .start.main.right.list delete 0 end
    button .new_file.bottom.ok -text OK -command {

	global running_scripts startupData 

	set startupFile [.new_file.middle.fileName get]
	if {$startupFile == ""} {
	    bell
	    tk_dialog .error Error "Please enter a file name" "" 0 OK 
	    return
	}
	set startupData(file) $startupFile
	set startupData(filename) $startupFile
	set startupData(simulation) $simulation 
	startupDataStructureInit
	enableAddDebug

	if {[file exists $startupFile]} {
	    bell
	    tk_dialog .error Warning "File \"$startupFile\" already \
	    exists. It will be overwritten next time you save." "" 0 OK 
	}

	set running_scripts($startupData(file)) "" 

	RemoveFromStartUpWindows .new_file  
	destroy .new_file
    }

    button .new_file.bottom.cancel -text "Cancel"\
	    -command {destroy .new_file}
    
    place .new_file.middle -relwidth 1 -height 100
    place .new_file.bottom -relwidth 1 -height 50 -y 100
    
    place .new_file.middle.fileName_lab -relx .05 -rely 0.35
    place .new_file.middle.fileName -relwidth .6 -relx .3 -rely 0.35
    
    place .new_file.bottom.ok -relwidth .3 -relx .1 -relheight .6\
	    -rely .15
    place .new_file.bottom.cancel -relwidth .3 -relx .6 -relheight .6\
	    -rely .15
    
    wm protocol $w WM_DELETE_WINDOW "RemoveFromStartUpWindows $w; destroy $w"
    
    bind $w <Return> {
	
	global running_scripts startupData 
	
	set startupFile [.new_file.middle.fileName get]
	if {$startupFile == ""} {
	    bell
	    tk_dialog .error Error "Please enter a file name" "" 0 OK 
	    return
	}
	set startupData(file) $startupFile
	set startupData(filename) $startupFile
	set startupData(simulation) $simulation 
	startupDataStructureInit
	enableAddDebug

	if {[file exists $startupFile]} {
	    bell
	    tk_dialog .error Warning "File \"$startupFile\" already \
		    exists. It will be overwritten next time you save." "" 0 OK 
	}

	set running_scripts($startupData(file)) "" 
	
	RemoveFromStartUpWindows .new_file  
	destroy .new_file
	
    }
    

}

proc FindStartupFile {} {
    
    global startupData running_scripts simulation
    
    set file [fileselect "Find TkShift Startup File" .suf]
    if {$file == ""} {
	return  
    } else {
	set startupData(filename) $file
	set startupData(file) [file tail $file]
	startupDataStructureInit
	enableAddDebug
	set running_scripts($startupData(file)) "" 
	startupReadFromFile 
	if {$startupData(simulation) != $simulation} {
	    bell 
	    tk_dialog .warning Warning "This simulation file might not correspond \
		    current simulation" warning 0 OK
	}		     
    }    
}

#############################################################################
#
#   RunSelection Now is just a caller function for Run which does most of the 
#   functions that RunSelection do.  This level of indirection is need for
#   prompting users that they are trying to run something that is already 
#   running 
# 
#############################################################################

proc RunSelection {} {
    if {[Run] == 0} {
	bell 
	tk_dialog .error Error "The selection is already running." error 0 OK
    }
}

#############################################################################
#
#   What used to be RunSelection.  This takes it what is selected by the 
#   mouse and run that selection.  If the ending time of the script is already
#   passed it will simply not do anything.  Other wise it will run the selection 
#   if the start time is already passed or put it in a queue called pending 
#   scripts
#
#############################################################################

proc Run {} {
    global itemselect startupData TimeElapsed 
    global script_windows pending_scripts 
    global running_scripts messageQueue
    global Stop gotinstance InstanceTypeList
    global TypesBroken ComponentsBroken CanvasStructure 

    set item $itemselect(current_selection)    
    
    if {$item == ""} {
	bell
	tk_dialog .error Error "You have not made a selection" error 0 OK
	return 1
    }
    
    # see if the time elapsed is set or not. . . if it is not set 
    # simulation is not properly running 

    if {[catch {set TimeElapsed $TimeElapsed}]} {
	bell 
	tk_dialog .error Error "Simulation is not properly running, please restart the simulation" error 0 OK
	return 1
    }
    
    if {[array exists running_scripts]} {
	
	if {[lsearch $running_scripts($startupData(file)) $item] != -1} {
	    
	    # script is already running 
	    return 0 
	    
	}	
    }
    
    set type [lindex $item 0]
    set number [lindex $item 1]
    
    if {$type == "Animation"} {
	
	if {[catch {set end_time $startupData(animations,$number,end_time)}]} {
	    set startupData(animations,$number,end_time) infinity
	    set end_time infinity
	}
	
	if {[catch {set start_time $startupData(animations,$number,start_time)}]} {
	    set startupData(animations,$number,start_time) 0
	    set start_time 0
	}
	    
	if {!($end_time > $TimeElapsed || $end_time == "infinity")} {
	    return 1
	}

	set data $startupData(animations,$number)

	#puts stdout "i am here" 
	#puts stdout $data 
	
	### This next code is to support scripts that have been
	## created before the coming of the orientation option.
	##
	if {[llength $data] == 13} {
	    lappend data "Nil"
	} elseif {[llength $data] == 14} {
	    set item [lindex $data 13]
	    if {$item == "1" || $item == "2"} {
		set data [linsert $data 13 "Nil"]
	    }
	}
	
	if {$startupData(animations,$number,start_time) <= $TimeElapsed} {

	    set animation_type [lindex $data 0]
	    set animation_index $startupData(file),animations,$number 

	    set animation_window [eval CreateCanvas $data]
	    
	    set script_windows($startupData(file),animations,$number) \
		    [list $end_time $animation_window]
	    
	    if {$animation_type == "SmartAHS"} {
		eval DrawSmartAHS $animation_window.animate.canvas
	    }
	    

	} else {
	    set pending_scripts($startupData(file),animations,$number) \
		    [list $start_time $end_time $type $number $data]
	}
	
	lappend running_scripts($startupData(file)) $item 
    
    } elseif {$type == "Graph"} {
	
	if {[catch {set end_time $startupData(graphs,$number,end_time)}]} {
	    set startupData(graphs,$number,end_time) infinity
	    set end_time infinity
	}
	
	if {[catch {set start_time $startupData(graphs,$number,start_time)}]} {
	    set startupData(graphs,$number,start_time) 0
	    set start_time 0
	}

	if {!($end_time > $TimeElapsed || $end_time == "infinity")} {
	    return 1
	}

	set data $startupData(graphs,$number)
	
	if {$startupData(graphs,$number,start_time) <= $TimeElapsed} {
	    set w [CreateNewGraph]
	    set script_windows($startupData(file),graphs,$number) [list $end_time $w]
	    dispatchGraphPlots $w $data 
	    ConfigureGraph $startupData(file),graphs,$number
	} else {
	    set pending_scripts($startupData(file),graphs,$number) \
		    [list $start_time $end_time $type $number $data]
	}

	lappend running_scripts($startupData(file)) $item 
    
    } elseif {$type == "Phase"} {
	
	if {[catch {set end_time $startupData(phase,$number,end_time)}]} {
	    set startupData(phase,$number,end_time) infinity
	    set end_time infinity
	}
	
	if {[catch {set start_time $startupData(phase,$number,start_time)}]} {
	    set startupData(phase,$number,start_time) 0
	    set start_time 0
	}
	
	if {!($end_time > $TimeElapsed || $end_time == "infinity")} {
	    return 1
	}
	
	set data $startupData(phase,$number)
	if {$startupData(phase,$number,start_time) <= $TimeElapsed} {
	    set w [CreateNewGraph]
	    set script_windows($startupData(file),phase,$number) [list $end_time $w]

	    ### data now holds more than one the phase plot 
	    ### that should be drawn into the same graph window (Tolga 102097)
	    foreach pitem $data {
		eval "DispatchPhasePlot $w $pitem"
	    }

	    ConfigureGraph $startupData(file),phase,$number
	} else {
	    set pending_scripts($startupData(file),phase,$number) \
		    [list $start_time $end_time $type $number $data]
	}
	lappend running_scripts($startupData(file)) $item 
    
    } elseif {$type == "Break"} {
	
	if {[catch {set end_time $startupData(break,$number,end_time)}]} {
	    set startupData(break,$number,end_time) infinity
	    set end_time infinity
	}
	
	if {[catch {set start_time $startupData(break,$number,start_time)}]} {
	    set startupData(break,$number,start_time) 0
	    set start_time 0
	}

	if {!($end_time > $TimeElapsed || $end_time == "infinity")} {
	    return 1
	}

	set data $startupData(break,$number)
	set break_type [lindex $data 0]
	set break_inst [lindex $data 1]
	
	if {$startupData(break,$number,start_time) <= $TimeElapsed} {

	    # now this has to be smarter than before in that we have to 
	    # now decide if the instance exists if you are breaking for 
	    # the instance 
	    
	    if {$break_inst != -1} {
		# lets check if the instance exists 
		
		set gotinstance 0
		unset gotinstance 
		
		lappend messageQueue 5556 
		lappend messageQueue [padMessage $break_type]
		
		vwait gotinstance
		set instance [list $break_type $break_inst]
		
		if {[lsearch $InstanceTypeList($break_type) $instance] == -1} {
		    bell 
		    tk_dialog .error Error "failure in running break script: $instance does not exist in simulation at this time" error 0 OK 
		    return 1
		}
	    }
	    
	    set break_data [lindex $data 2]	  
	    set command [lindex $break_data 0]
	    set command_info [lindex $break_data 1]
	    lappend messageQueue $command
	    lappend messageQueue $command_info  	    
	    #	    puts $command 
	    #    puts $command_info 
	    #   puts $messageQueue 
	    
	    if {$break_inst == -1} {
		set script_windows($startupData(file),break,$number) \
			[list $end_time [list 8002 \
			[padMessage $break_type]]]
	    } else {
		set script_windows($startupData(file),break,$number) \
			[list $end_time [list 8004 \
			[padMessage "$break_type $break_inst"]]]
	    }
    
	} else {
	    set pending_scripts($startupData(file),break,$number) \
		    [list $start_time $end_time $type $number $data]
	}

	lappend running_scripts($startupData(file)) "$type $number"
	if {$break_inst == -1} {
	    set TypesBroken($break_type) 1
	    UpdateTypesBox
	} else {
	    set ComponentsBroken($break_type,$break_inst) 1
	    UpdateShowWindow
	}

    } elseif {$type == "Stop"} {
	
	set stop_time $startupData(stop,$number)
	
	if {$stop_time < $TimeElapsed} {
	    return 1
	}

	if {$stop_time == $TimeElapsed} {
	    lappend messageQueue $Stop
	    .middle.main.continue configure -state normal
	    .middle.main.stop configure -state disabled 

	} else {

	    set start_time $stop_time 
	    set end_time infinity

	    set pending_scripts($startupData(file),stop,$number) \
		    [list $start_time $end_time $type $number $stop_time]
	    lappend running_scripts($startupData(file)) $item 
	}

    } elseif {$type == "Trace"} {
	puts "Not implemented yet"
    }
    return 1 
}


#############################################################################
#
#   Iterates over the items listed on the list box to execute them. 
#
#############################################################################

proc RunAllSelection {lb} {
    
    global itemselect TimeElapsed 

    # see if the time elapsed is set or not. . . if it is not set 
    # simulation is not properly running 

    if {[catch {set TimeElapsed $TimeElapsed}]} {
	bell 
	tk_dialog .error Error "Simulation is not properly running, please restart the simulation" error 0 OK
	return 
    }

    set run_list [$lb get 0 end]
    set running_again 0 
    foreach run_element $run_list {
	set itemselect(current_selection) $run_element
	if {[Run] == 0} {
	    set running_again 1
	}
    }
    
    if {$running_again == 1} {
	bell 
	tk_dialog .info Info "Some or all selections are already running." error 0 OK
    }

    set itemselect(current_selection) "" 
    
}

#############################################################################
#
#   This is a general function for saving information for all types, the type 
#   parameter is among these {animations, graphs, break, phase, . . . } 
#   not {Animation, Graph, Break, Phase, . . }  
#
#############################################################################

proc SaveEdit {w id edit_window type} {
    
    global startupData 

    if {$type == "stop"} {
	set end_time infinity
	set start_time 0 
    } else {
	set end_time [$w.end_time get] 
	set start_time [$w.start_time get]
    }

    if {$end_time != "infinity"} {

	# then it has to be an integer 
	if {[catch {set a [expr $end_time + 0]}]} {
	    bell 
	    tk_dialog .error "Error" "only values allowed for end_time is either a number of infinity!" warning 0 "OK" 
	    return
	}
    }

    if {[catch {set a [expr $start_time + 0]}]} {
	bell 
	tk_dialog .error "Error" "only values allowed for start_time is a number!" warning 0 "OK" 
	return
    }


    if {$start_time >= $end_time} {
	bell 
	tk_dialog .error "Error" "end time has to be greater than the start time!" warning 0 "OK" 
    } else {

	if {$type == "graphs" || $type == "phase"} {

	    set startupData($type,$id,title) [$w.title get]
	    set startupData($type,$id,xtext) [$w.xtext get]
	    set startupData($type,$id,ytext) [$w.ytext get]

	}

	set startupData($type,$id,start_time) $start_time 
	set startupData($type,$id,end_time) $end_time 
	RemoveFromStartUpWindows $edit_window 
	destroy $edit_window 
    }
}

#############################################################################
#
#   ConfigureGraph:
#   After starting graph, we want to configure the graph in a way that we 
#   like it, this will configure the title and the x and y axis of the graph
#
#############################################################################

proc ConfigureGraph {index} {

    global script_windows startupData

    # in this function we can assume that the graph is already running, 
    # and we can find the name of the window in script_windows data 
    # structure 

    set next_comma [string first "," $index]
    set file_name [string range $index 0 [expr $next_comma -1]]
    set temp_index [string range $index [expr $next_comma +1] end]
    
    set next_comma [string first "," $temp_index]
    set type [string range $temp_index 0 [expr $next_comma -1]]
    set number [string range $temp_index [expr $next_comma +1] end]

    set w [lindex $script_windows($index) 1]
    set graph_title $startupData($type,$number,title) 
    set graph_xtext $startupData($type,$number,xtext) 
    set graph_ytext $startupData($type,$number,ytext) 

    if {$graph_title != ""} {
	$w.middle.flow configure -title $graph_title
    }
    if {$graph_xtext != ""} {
	$w.middle.flow xaxis configure -title $graph_xtext
    }
    if {$graph_ytext != ""} {
	$w.middle.flow yaxis configure -title $graph_ytext
    }

    ### If this is not a phase plot turn the Legend off (Tolga 102297)
    if {$type != "Phase"} {
	LegendOff $w
    }
}




proc enableAddDebug {} {

    .start.bar.add.m entryconfigure 0 -state normal
    .start.bar.add.m entryconfigure 1 -state normal
    .start.bar.add.m entryconfigure 2 -state normal 
    .start.bar.add.m entryconfigure 3 -state normal 
    .start.bar.add.m entryconfigure 4 -state normal 

}
proc disableAddDebug {} {

    .start.bar.add.m entryconfigure 0 -state disabled
    .start.bar.add.m entryconfigure 1 -state disabled
    .start.bar.add.m entryconfigure 2 -state disabled
    .start.bar.add.m entryconfigure 3 -state disabled
    .start.bar.add.m entryconfigure 3 -state disabled
}

proc AddGraph {} {

    global GraphsInWindow script_changed 

    set items [InstanceListGitter 0 {} AddGraph]

    if {$items != ""} {
	set package [list Graph [list $items]]
	addDebugItem $package
	set script_changed 1
    }
}

#############################################################################
#
#  AddStop will add Stop Instance, this is called by DisplayStop function 
#  DisplayStop fucntion is shared by View funcitonallity and Add command, 
#  When DisplayStop is used as Add command, this function is called to Add
#  this new script 
#
#############################################################################

proc AddStop {w} {
    
    set items [$w.main.stoptime get]
    if {$items != ""} {
	set package [list Stop $items]
	addDebugItem $package
    }
    destroy $w 
}


#############################################################################
#
#   Unlike AddGraph this is called by function outside of startup.  
#   This is how it works.
#   1) "Add Breakpoint" command is selected 
#   2) After parameter is set, the user press Ok
#   3) Then that function calls "AddBreak"
#
#############################################################################

proc AddBreak {items} {
    
    global GraphsInWindow script_changed 
    
    if {$items != ""} {
	set package [list Break $items]
	addDebugItem $package
	set script_changed 1
    }
}

#############################################################################
#
#  
#
#############################################################################

proc DeleteBreak {name} {
    return [DeleteCompBreak $name -1]
}

#############################################################################
#
#   DeleteCompBreak: 
#   What if a user decides to unbreak a type from inspect not from 
#   scripting.  We will have to see if we are already running 
#   this script.  Returns 0 if it is already running, after quiting 
#   this script 
#
#############################################################################

proc DeleteCompBreak {name numb} {
    
    global GraphsInWindow 
    global script_windows running_scripts pending_scripts startupData

    set i [IsBreakRunning $name $numb]
    
    if {$i != -1} {
	
	if {$numb == -1} {
	    tk_dialog .caution CAUTION "script on breaking $name is already running, please restart script if this is a mistake" warning 0 OK 
	} else {
	    tk_dialog .caution CAUTION "script on breaking $name $numb is already running.  please restart script if this is a mistake" warning 0 OK 
	}
	DestroyScript $startupData(file),break,$i
	return 0
    }   
    return 1
}   


#############################################################################
#
#   Unlike AddGraph this is called by function outside of startup.  
#   This is how it works.
#   1) "Add Phase" command is selected 
#   2) After parameter is set, the user press Ok
#   3) Then that function calls "AddPhase", the function below 
#
#############################################################################

proc AddPhase {items first} {

    global GraphsInWindow script_changed startupData

    if {$items != ""} {
	### Add debug item only for the first phase graph in the bundle.
	### Append the rest to startupData.  (Tolga 102097)
	if {$first} {
	    set package [list Phase $items]
	    addDebugItem $package
	} else { 
	    set pno [lindex $startupData(phase,-1) end]
	    lappend startupData(phase,$pno) $items
	}
	
	set script_changed 1
    }

}

#############################################################################
#
#   This function is called by the script to "dispatch" the graph script.
#   When a person creates a graph script, he/she is not creating a graph 
#   on screen, but is saving the information into script data structure. 
#   This function actually dispathes the graph 
#
#############################################################################

proc dispatchGraphPlots {w data} {

    set send_graphs [lindex $data 0]
    #puts $data
    #puts $send_graphs

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


#############################################################################
#
#   This function display information about graph script.   The change from the 
#   previuos version is that now there is extra field called start time and 
#   end time, which you can edit.  Also this function is now shared by 
#   time graph and phase graph. 
#
#############################################################################

proc DisplayGraph {name type} {

    global script_changed startup_windows startupData
    
    set id [lindex $name 1]
    set w .displayg

    if {[winfo exists $w]} {
	catch {wm deiconify $w}
	catch {raise $w}
	DisplayGraphInfo $w $id $type
	return
    } else {
	lappend startup_windows $w
    }

    toplevel $w
    grab $w 

    if {$type == "graphs"} {
	wm title $w "Graph $id Information"
    } else {
	wm title $w "Phase $id Information"
    }

    frame $w.main -relief raised -bd 2
    frame $w.bottom

#    button $w.bottom.close -text "Close" \
#	    -command "destroy $w"

    grid rowconfigure $w 0 -weight 1
    grid columnconfigure $w 0 -weight 1 -minsize 300
    grid rowconfigure $w 1 -minsize 50 -weight 0

    set win $w.main
    frame $win.right
    frame $win.left

    grid columnconfigure $win 0 -minsize 200 -weight 1
    grid columnconfigure $win 1 -minsize 200 -weight 1
    grid rowconfigure $win 0 -minsize 250 -weight 1
    grid $win.left -row 0 -column 0 -sticky news
    grid $win.right -row 0 -column 1 -sticky news
    set lb [listbox $win.right.list -font fixed\
	    -yscrollcommand [list $win.right.scroll set]\
	    -xscrollcommand [list $win.right.xscroll set]]
    scrollbar $win.right.scroll -command [list $lb yview]
    scrollbar $win.right.xscroll -orient horizontal -command [list $lb xview]

    grid columnconfigure $w.main.right 0 -weight 1
    grid columnconfigure $w.main.right 1 -weight 0
    grid rowconfigure $w.main.right 0 -weight 1
    grid $win.right.scroll -row 0 -column 1 -sticky ns
    grid $win.right.list -row 0 -column 0 -sticky enws
    grid $win.right.xscroll -row 1 -column 0 -sticky we
    grid rowconfigure $win.left 0 -weight 0
    grid rowconfigure $win.left 1 -weight 0
    grid rowconfigure $win.left 2 -weight 0
    grid rowconfigure $win.left 3 -weight 0
    grid rowconfigure $win.left 4 -weight 0
    grid rowconfigure $win.left 5 -weight 0
    grid rowconfigure $win.left 6 -weight 0
    grid rowconfigure $win.left 7 -weight 0
    grid rowconfigure $win.left 8 -weight 0

    grid columnconfigure $win.left 0 -weight 0 
    grid columnconfigure $win.left 1 -weight 0

    if {$type == "graphs"} {
	label $win.left.graphname_lab -text "Graph name:"
    } else {
	label $win.left.graphname_lab -text "Phase name:"
    }
    
    label $win.left.title_lab -text "Title: "
    label $win.left.xtext_lab -text "X Text: "
    label $win.left.ytext_lab -text "Y Text: "
    label $win.left.log_lab -text "Log Scale: "

#    label $win.left.type_lab -text "Axis Configuration: "  
    label $win.left.start_time_lab -text "Start Time: "  
    label $win.left.end_time_lab -text "End Time: "  
#Fixed or dynamic
#    label $win.left.xmin_lab -text "X Min: "
#    label $win.left.xmax_lab -text "X Max:"
#    label $win.left.ymin_lab -text "Y Min: "
#    label $win.left.ymax_lab -text "Y Max:"

#    entry $win.left.title -width 15  -state disabled -bg lightgrey
#    entry $win.left.xtext -width 15  -state disabled -bg lightgrey
#    entry $win.left.ytext -width 15  -state disabled -bg lightgrey

    entry $win.left.title -width 15  -bg lightgrey
    entry $win.left.xtext -width 15  -bg lightgrey
    entry $win.left.ytext -width 15  -bg lightgrey

    if {$type == "graphs"} {
	label $win.left.graphname -text "Graph $id"
	$win.left.title insert 0 $startupData(graphs,$id,title)
	$win.left.xtext insert 0 $startupData(graphs,$id,xtext)
	$win.left.ytext insert 0 $startupData(graphs,$id,ytext)
    } else {
	label $win.left.graphname -text "Phase $id"
	$win.left.title insert 0 $startupData(phase,$id,title)
	$win.left.xtext insert 0 $startupData(phase,$id,xtext)
	$win.left.ytext insert 0 $startupData(phase,$id,ytext)
    }


#    radiobutton $win.left.autotype -text "Automatic " -variable\
#	    LogScale -value auto 
#    radiobutton $win.left.conftype -text "Manual     " -variable\
#	    LogScale -value manual -state disabled
    entry $win.left.start_time -width 15  -bg lightgrey
    entry $win.left.end_time -width 15  -bg lightgrey
    
    checkbutton $win.left.xlog -text "X Axis " -width 12 -state disabled
    checkbutton $win.left.ylog -text "Y Axis " -width 12 -state disabled

#n    entry $win.left.xmin
#    entry $win.left.xmax
#    entry $win.left.ymin
#    entry $win.left.ymax
    
    grid $win.left.graphname_lab -row 0 -column 0 -sticky w
    grid $win.left.title_lab -row 1 -column 0 -sticky w
#    grid $win.left.xmin_lab  -row 0 -column 0 -sticky w
#    grid $win.left.xmax_lab  -row 0 -column 0 -sticky w
#    grid $win.left.ymin_lab  -row 0 -column 0 -sticky w
#    grid $win.left.ymax_lab  -row 0 -column 0 -sticky w
    grid $win.left.xtext_lab -row 2 -column 0 -sticky w
    grid $win.left.ytext_lab -row 3 -column 0 -sticky w
    grid $win.left.log_lab  -row 4 -column 0 -sticky w
#    grid $win.left.ylog_lab  -row 5 -column 0 -sticky w
#    grid $win.left.type_lab -row 6 -column 0 -sticky w
    grid $win.left.start_time_lab -row 6 -column 0 -sticky w
    grid $win.left.end_time_lab -row 7 -column 0 -sticky w

    grid $win.left.graphname -row 0 -column 1 -sticky w
    grid $win.left.title -row 1 -column 1 -sticky w
    grid $win.left.xtext -row 2 -column 1 -sticky w
    grid $win.left.ytext -row 3 -column 1 -sticky w

    grid $win.left.xlog  -row 4 -column 1 -sticky w
    grid $win.left.ylog  -row 5 -column 1 -sticky w
    
#    grid $win.left.autotype -row 6 -column 1 -sticky w
#    grid $win.left.conftype -row 7 -column 1 -sticky w
    grid $win.left.start_time -row 6 -column 1 -sticky w
    grid $win.left.end_time -row 7 -column 1 -sticky w

#    grid $win.left.xmin  -row 0 -column 1
#    grid $win.left.xmax  -row 0 -column 1
#    grid $win.left.ymin  -row 0 -column 1
#    grid $win.left.ymax  -row 0 -column 1

    if {$type == "graphs"} {
	button $w.bottom.ok -text "OK" \
		-command "set script_changed 1; SaveEdit $win.left $id $w graphs"
    } elseif {$type == "phase"} {
	button $w.bottom.ok -text "OK" \
		-command "set script_changed 1; SaveEdit $win.left $id $w phase"
    }	

    button $w.bottom.cancel -text "Cancel" \
	    -command "RemoveFromStartUpWindows $w; destroy $w"

    grid $w.main -row 0 -column 0 -sticky news
    grid $w.bottom -row 1 -column 0 -sticky news
    
    grid columnconfigure $w.bottom 0 -weight 0
    grid columnconfigure $w.bottom 1 -weight 0 

    grid $w.bottom.ok -column 0 -row 1 -padx 50  
    grid $w.bottom.cancel -column 1 -row 1 -padx 50 
    
    DisplayGraphInfo $w $id $type 
    
    wm protocol $w WM_DELETE_WINDOW "RemoveFromStartUpWindows $w; destroy $w"
}

#############################################################################
#
#   DisGraphInfo displays information about the graph script.  There is 
#   minor changes from the previous version because now you are able to 
#   enter start and end time, in which the simulation automatically start 
#   and end itself
#
#############################################################################

proc DisplayGraphInfo {w id type} {
    
    global startupData
    
    $w.main.right.list delete 0 end

    if {$type == "graphs"} {
	set items [lindex $startupData(graphs,$id) 0]
	foreach i $items {
	    $w.main.right.list insert end [lindex $i 0]
	}
    } elseif {$type == "phase"} {

	#### Display every phase plot in the bundle (Tolga 102197)
	set data $startupData(phase,$id)
	
	foreach items $data {
	    set num_items [expr [llength $items] / 3]
	    set counter 0 
	    while {$num_items > $counter} {
		set temp_elm "[lindex $items [expr $counter * 3 + 1]][lindex $items [expr $counter * 3 + 2]]\([lindex $items [expr $counter * 3]]\)"
		#### Put "-vs-" after the x axis variable for each pair
		if {$counter == 0} {
		    append temp_elm " -vs- "
		}
		$w.main.right.list insert end $temp_elm 

		incr counter
	    }
	    #### Put a separator after each pair
	    $w.main.right.list insert end ""
	}
	#### Display ends
    }    

    # if there is no variable startupData we will get an error, then we can just 
    # assume that the script starts at 0 and does not end. 

    if {[catch {$w.main.left.start_time insert 0 $startupData($type,$id,start_time)}]} {
	set startupData($type,$id,startup_time) 0 
	$w.main.left.start_time insert 0 0  
    }
    if {[catch {$w.main.left.end_time insert 0 $startupData($type,$id,end_time)}]} {
	set startupData($type,$id,end_time) infinity
	$w.main.left.end_time insert 0 infinity
    }
}

#############################################################################
#
#  As one can tell from the name of this function, this displays Stop 
#  information, as mentioned earlier this functino is shared by 
#  entering stop information and editing stop information. 
#
#############################################################################

proc DisplayStop {{name unknown}} {

    global script_changed startupData startup_windows 
    
    set w .displays

    if {[winfo exists $w]} {
	catch {wm deiconify $w}
	catch {raise $w}
	return
    } else {
	lappend startup_windows $w
    }

    toplevel $w
    grab $w 

    frame $w.main -relief raised -bd 2
    frame $w.bottom 

    if {$name != "unknown"} {
	set id [lindex $name 1]
	wm title $w "Stop $id Information"
	button $w.bottom.ok -text "OK" \
		-command "set script_changed 1; SaveEdit $w.main $id $w stop"
    } else {
	wm title $w "Enter Stop Information"
	button $w.bottom.ok -text "OK" \
		-command "set script_changed 1; AddStop $w"
    }	

    button $w.bottom.cancel -text "Cancel" \
	    -command "RemoveFromStartUpWindows $w; destroy $w"

    grid rowconfigure $w 0 -minsize 50 -weight 1
    grid rowconfigure $w 1 -minsize 50 -weight 0
    grid columnconfigure $w 0 -weight 1 -minsize 300

    grid $w.main -row 0 -column 0 -sticky news
    grid $w.bottom -row 1 -column 0 -sticky news
    
    grid columnconfigure $w.bottom 0 -weight 0
    grid columnconfigure $w.bottom 1 -weight 0 

    grid $w.bottom.ok -column 0 -row 1 -padx 50  
    grid $w.bottom.cancel -column 1 -row 1 -padx 50 

    grid columnconfigure $w.main 0 -weight 0
    grid columnconfigure $w.main 1 -weight 1
    grid rowconfigure $w.main 0 -weight 0
   
    set win $w.main
    label $win.stoptime_lab -text "StopTime: "

    entry $win.stoptime -width 15 -bg lightgrey

    if {$name != "unknown"} {
	set startupData(stop,$id,startup_time) 0 
	set startupData(stop,$id,end_time) infinity
	$win.stoptime insert 0 $startupData(stop,$id)
    } else {
    }

    grid $win.stoptime_lab -row 0 -column 0 -sticky w
    grid $win.stoptime -row 0 -column 1 -sticky w

    wm protocol $w WM_DELETE_WINDOW "RemoveFromStartUpWindows $w; destroy $w"
}

#############################################################################
#
#  As one can tell from the name of this function, this displays break 
#  information 
#
#############################################################################

proc DisplayBreak {name} {

    global script_changed startupData startup_windows 

    set id [lindex $name 1]
    set w .displayb

    if {[winfo exists $w]} {
	catch {wm deiconify $w}
	catch {raise $w}
	return
    } else {
	lappend startup_windows $w
    }

    toplevel $w
    grab $w 

    wm title $w "Break $id Information"

    frame $w.main -relief raised -bd 2
    frame $w.bottom 

    button $w.bottom.ok -text "OK" \
	    -command "set script_changed 1; SaveEdit $w.main $id $w break"

    button $w.bottom.cancel -text "Cancel" \
	    -command "RemoveFromStartUpWindows $w; destroy $w"

    grid rowconfigure $w 0 -weight 1
    grid rowconfigure $w 1 -minsize 50 -weight 0
    grid columnconfigure $w 0 -weight 1 -minsize 300

    grid $w.main -row 0 -column 0 -sticky news
    grid $w.bottom -row 1 -column 0 -sticky news
    
    grid columnconfigure $w.bottom 0 -weight 0
    grid columnconfigure $w.bottom 1 -weight 0 

    grid $w.bottom.ok -column 0 -row 1 -padx 50  
    grid $w.bottom.cancel -column 1 -row 1 -padx 50 

    grid columnconfigure $w.main 0 -weight 0
    grid columnconfigure $w.main 1 -weight 1
    grid rowconfigure $w.main 0 -weight 0
    grid rowconfigure $w.main 1 -weight 0
    grid rowconfigure $w.main 2 -weight 0
    grid rowconfigure $w.main 3 -weight 0
   
    set win $w.main
    label $win.breakname_lab -text "Break Type: "
    label $win.breakinst_lab -text "Break Instance: "
    label $win.start_time_lab -text "Start Time: "
    label $win.end_time_lab -text "End Time: "

    label $win.breakname -text [lindex $startupData(break,$id) 0]

    set inst [lindex $startupData(break,$id) 1]

    if {$inst == -1} {
	label $win.breakinst -text "All" 
    } else {
	label $win.breakinst -text $inst 
    }

    entry $win.start_time -width 15  -bg lightgrey
    entry $win.end_time -width 15  -bg lightgrey

    if {[catch {$win.start_time insert 0 $startupData(break,$id,start_time)}]} {
	set startupData(break,$id,startup_time) 0 
	$win.start_time insert 0 0  
    }
    if {[catch {$win.end_time insert 0 $startupData(break,$id,end_time)}]} {
	set startupData(break,$id,end_time) infinity
	$win.end_time insert 0 infinity
    }

    grid $win.breakname_lab -row 0 -column 0 -sticky w
    grid $win.breakinst_lab -row 1 -column 0 -sticky w
    grid $win.start_time_lab -row 2 -column 0 -sticky w
    grid $win.end_time_lab -row 3 -column 0 -sticky w

    grid $win.breakname -row 0 -column 1 -sticky w
    grid $win.breakinst -row 1 -column 1 -sticky w
    grid $win.start_time -row 2 -column 1 -sticky w
    grid $win.end_time -row 3 -column 1 -sticky w

    wm protocol $w WM_DELETE_WINDOW "RemoveFromStartUpWindows $w; destroy $w"
}


proc DisplayAnim {name} {

    global script_changed startupData startup_windows 

    set id [lindex $name 1]
    set w .displaya

    if {[winfo exists $w]} {
	catch {wm deiconify $w}
	catch {raise $w}
	DisplayAnimInfo $w $id
	return
    } else {
	lappend startup_windows $w
    }

    initializeDisplayvalues
    toplevel $w
    grab $w 

    wm title $w "Animation $id Information"

    frame $w.main -relief raised -bd 2
    frame $w.bottom

    button $w.bottom.ok -text "OK" \
	    -command "set script_changed 1; SaveEdit $w.main $id $w animations"

    button $w.bottom.cancel -text "Cancel" \
	    -command "RemoveFromStartUpWindows $w; destroy $w"

    grid rowconfigure $w 0 -weight 1
    grid columnconfigure $w 0 -weight 1 -minsize 300
    grid rowconfigure $w 1 -minsize 50 -weight 0

    grid $w.main -row 0 -column 0 -sticky news
    grid $w.bottom -row 1 -column 0 -sticky news
    
    grid columnconfigure $w.bottom 0 -weight 0
    grid columnconfigure $w.bottom 1 -weight 0 

    grid $w.bottom.ok -column 0 -row 1 -padx 50  
    grid $w.bottom.cancel -column 1 -row 1 -padx 50 

    grid columnconfigure $w.main 0 -weight 0
    grid columnconfigure $w.main 1 -weight 1
    grid rowconfigure $w.main 0 -weight 0
    grid rowconfigure $w.main 1 -weight 0
    grid rowconfigure $w.main 2 -weight 0
    grid rowconfigure $w.main 3 -weight 0
    grid rowconfigure $w.main 4 -weight 0
    grid rowconfigure $w.main 5 -weight 0
    grid rowconfigure $w.main 6 -weight 0
    grid rowconfigure $w.main 7 -weight 0
    grid rowconfigure $w.main 8 -weight 0
    grid rowconfigure $w.main 9 -weight 0
    grid rowconfigure $w.main 10 -weight 0
    grid rowconfigure $w.main 13 -weight 0
    grid rowconfigure $w.main 14 -weight 0
    
    set win $w.main
    label $win.animname_lab -text "Animation name:"
    label $win.canvas_lab -text "Canvas Used: "
    label $win.multiplier_lab -text "Multiplier: "
    label $win.image_type_lab -text "Image Type: "
    label $win.bitmap_name_lab -text "Bitmap File: "
    label $win.oval_radius_lab -text "Oval Radius: "
    label $win.rect_width_lab -text "Rectangle width: "
    label $win.rect_height_lab -text "Rectangle height:"
    label $win.type_lab -text "Animated type:"
    label $win.xvar_lab -text "X Variable: "
    label $win.yvar_lab -text "Y Variable: "
    label $win.state_lab -text "Discrete state: "
    label $win.container_lab -text "Container: "

#    label $win.start_lab -text "Start: "
#    label $win.stop_lab -text "Stop: "
#    label $win.color_lab -text "Color: "

    label $win.animname -textvariable displayValues(name)
    label $win.canvas -textvariable displayValues(canvas)
    label $win.multiplier  -textvariable displayValues(multiplier)
    label $win.image_type -textvariable displayValues(image)
    label $win.bitmap_name -textvariable displayValues(bitmap)

    label $win.type -textvariable displayValues(type)
    label $win.xvar -textvariable displayValues(x)
    label $win.yvar     -textvariable displayValues(y)
    label $win.state -textvariable displayValues(state)
    label $win.container -textvariable displayValues(container)

    radiobutton $win.image_read_var_auto -text "Auto Set"
    radiobutton $win.image_read_var_once -text "Read Once"
    radiobutton $win.image_read_var_always -text "Read Always"
    label $win.oval_radius -textvariable displayValues(radius)
    label $win.rect_height -textvariable displayValues(height)
    label $win.rect_width -textvariable displayValues(width)
 
    label $win.start_time_lab -text "Start Time: "  
    label $win.end_time_lab -text "End Time: "  
    entry $win.start_time -width 15  -bg lightgrey
    entry $win.end_time -width 15  -bg lightgrey
    grid $win.start_time_lab -row 13 -column 0 -sticky w
    grid $win.end_time_lab -row 14 -column 0 -sticky w
    grid $win.start_time -row 13 -column 1 -sticky w
    grid $win.end_time -row 14 -column 1 -sticky w

    if {[catch {$win.start_time insert 0 $startupData(animations,$id,start_time)}]} {
	set startupData(animations,$id,startup_time) 0 
	$win.start_time insert 0 0  
    }
    if {[catch {$win.end_time insert 0 $startupData(animations,$id,end_time)}]} {
	set startupData(animations,$id,end_time) infinity
	$win.end_time insert 0 infinity
    }

### Place the objects onto the window


    grid $win.animname_lab -row 0 -column 0 -sticky w
    grid $win.canvas_lab -row 1 -column 0 -sticky w
    grid $win.multiplier_lab -row 2 -column 0 -sticky w
    grid $win.image_type_lab -row 3 -column 0 -sticky w
    grid $win.bitmap_name_lab -row 4 -column 0 -sticky w
    grid $win.oval_radius_lab  -row 5 -column 0 -sticky w
    grid $win.rect_width_lab  -row 6 -column 0 -sticky w
    grid $win.rect_height_lab -row 7 -column 0 -sticky w
    grid $win.type_lab  -row 8 -column 0 -sticky w
    grid $win.xvar_lab  -row 9 -column 0 -sticky w
    grid $win.yvar_lab  -row 10 -column 0 -sticky w
    grid $win.state_lab -row 11 -column 0 -sticky w
     grid $win.container_lab  -row 12 -column 0 -sticky w
#     grid $win.color_lab -x 5 -y 630


    grid $win.animname -row 0 -column 1 -sticky w
    grid $win.canvas -row 1 -column 1 -sticky w
    grid $win.multiplier -row 2 -column 1 -sticky w
    grid $win.image_type -row 3 -column 1 -sticky w
    grid $win.bitmap_name -row 4 -column 1 -sticky w
    grid $win.oval_radius  -row 5 -column 1 -sticky w
    grid $win.rect_width  -row 6 -column 1 -sticky w
    grid $win.rect_height -row 7 -column 1 -sticky w
    grid $win.type  -row 8 -column 1 -sticky w
    grid $win.xvar  -row 9 -column 1 -sticky w
    grid $win.yvar  -row 10 -column 1 -sticky w
    grid $win.state -row 11 -column 1 -sticky w
    grid $win.container  -row 12 -column 1 -sticky w

    DisplayAnimInfo $w $id

    wm protocol $w WM_DELETE_WINDOW "RemoveFromStartUpWindows $w; destroy $w"
}



proc DisplayAnimInfo {w id} {
    global startupData displayValues

    set items $startupData(animations,$id)

    set displayValues(name) "Animation $id"
    set displayValues(canvas) [lindex $items 0]
    set displayValues(x) [lindex $items 1]
    set displayValues(y) [lindex $items 2]  
    set displayValues(instances) [lindex $items 3]
    set displayValues(type) [lindex $items 4]
    set displayValues(image) [lindex $items 5]
    set displayValues(multiplier) [lindex $items 6]
    set displayValues(state) [lindex $items 7]
    
    set c [lindex $items 8]
    if {$c == "Null"}  {
	set displayValues(container) NONE
    } else {
	set displayValues(container) [format "%s(%s %s)" [lindex\
	$items 8] [lindex $items 9] [lindex $items 10]]
    }

    if {$displayValues(image) == "rectangle"} {
	set displayValues(width) [lindex $items 11]
	set displayValues(height) [lindex $items 12]
	set displayValues(radius) ""
    } else {
	set displayValues(radius) [lindex $items 11]
	set displayValues(width) ""
	set displayValues(height) ""
    }
    

}

proc initializeDisplayvalues {} {

    global displayValues
    
    set displayValues(name) ""
    set displayValues(canvas) ""
    set displayValues(x)  ""
    set displayValues(y)  ""
    set displayValues(instances) ""
    set displayValues(type)  ""
    set displayValues(image)  ""
    set displayValues(multiplier) ""
    set displayValues(states)  ""
    set displayValues(container) ""
    set displayValues(width)  ""
    set displayValues(height) ""
    set displayValues(radius) ""

}


#############################################################################
#
#   QuitSelection 
# 
#############################################################################
 
proc QuitSelection {} {
    
    global itemselect startupData script_windows pending_scripts 
    global running_scripts messageQueue TypesBroken ComponentsBroken 

    
    set item $itemselect(current_selection)    
    set type [lindex $item 0]
    set number [lindex $item 1]   
    
    if {$item == ""} {
	bell
	tk_dialog .error Error "You have not made a selection" error 0 OK
	return 
    }
    
    if {[array exists running_scripts]} {
	if {[lsearch $running_scripts($startupData(file)) $item] == -1} {
	    # script is not running 
	    tk_dialog .error Error "The selection is not currently running." error 0 OK
	    return 
	}	
    } else {
	# script is not running 
	tk_dialog .error Error "The selection is not currently running." error 0 OK
	return 
    }	
    
    set file_name $startupData(file);
    
    if {$type == "Animation"} {
	if {[array exists script_windows] == 1} {
	    set window_indexes [array names script_windows]
	    set index ${file_name},animations,${number}
	    if {[lsearch $window_indexes $index] != -1} {
		set w [lindex $script_windows(${file_name},animations,${number}) 1]
		DequeueFromOpenWindows $w
		RemoveFromStartUpWindows $w 
		ExitCanvas [wm title $w]
		destroy $w
	    } else {
		RemoveFromPendingScripts ${file_name},animations,${number} 
	    }
	} else {
	    RemoveFromPendingScripts ${file_name},animations,${number} 
	}
    } elseif {$type == "Graph"} {
	if {[array exists script_windows]} {
	    if {[lsearch [array names script_windows] ${file_name},graphs,${number}] != -1} {
		set w [lindex $script_windows(${file_name},graphs,${number}) 1]
		RemoveGraphData $w
		RemoveFromStartUpWindows $w
		DequeueFromOpenWindows $w
		destroy $w
	    } else {
		RemoveFromPendingScripts ${file_name},graphs,${number} 
	    }	
	} else {
	    RemoveFromPendingScripts ${file_name},graphs,${number} 
	}	
    } elseif {$type == "Phase"} {
	if {[array exists script_windows]} {
	    if {[lsearch [array names script_windows] ${file_name},phase,${number}] != -1} {
		set w [lindex $script_windows(${file_name},phase,${number}) 1]
		RemoveGraphData $w
		RemoveFromStartUpWindows $w
		DequeueFromOpenWindows $w
		destroy $w
	    } else {
		RemoveFromPendingScripts ${file_name},phase,${number} 
	    }
	} else {
	    RemoveFromPendingScripts ${file_name},phase,${number} 
	}	    
    } elseif {$type == "Break"} {

	QuitBreak $number 
	
    } elseif {$type == "Stop"} {
	RemoveFromPendingScripts ${file_name},stop,${number} 
    } elseif {$type == "Trace"} {
	puts "Not implemented yet"
    }
}

#############################################################################
#
#    QuitBreak: 
#    This function will quit break script: 
#    Few things to take care of in this function: 
#    1) update TypeBroken or ComponentsBroken 
#    2) call the simluation to quit breaking on this object
#    3) update windows 
#    
#############################################################################

proc QuitBreak {quit_number} {

    global script_windows startupData 

    set file_name $startupData(file)

    if {[array exists script_windows]} {
	
	if {[lsearch [array names script_windows] ${file_name},break,${quit_number}] != -1} {
	    
	    # if by chance the same break type is running currently, we don't want to 
	    # quit break on this item 
	    
	    set break_type [lindex $startupData(break,$quit_number) 0]
	    set break_inst [lindex $startupData(break,$quit_number) 1]
	    
	    RemoveFromRunningScripts $file_name,break,$quit_number
	    
	    set script_num [IsBreakCurrentlyRunning $break_type $break_inst $quit_number]
	    
	    if {$script_num != -1} {
		
		# this means that there are other script with the same
		# break type and instance is running 
		
		UpdateStartBreakInfo $script_num
		
		set w [lindex $script_windows($file_name,break,$quit_number) 1]
		
		set break_cmd [lindex $w 0]
		set break_data [lindex $w 1]
		
		lappend messageQueue $break_cmd
		lappend messageQueue $break_data

	    } else {
		
		set w [lindex $script_windows($file_name,break,$quit_number) 1]
		
		set break_cmd [lindex $w 0]
		set break_data [lindex $w 1]
		
		lappend messageQueue $break_cmd
		lappend messageQueue $break_data
		
	    }
	    
	    unset script_windows($file_name,break,$quit_number)
	    
	    
	} else {
	    RemoveFromPendingScripts $file_name,break,$quit_number 
	}
    } else {
	RemoveFromPendingScripts $file_name,break,$quit_number 
    }
}

#############################################################################
#
#   IsBreakCurrentlyRunning:
#   Returns -1 if break type and inst specified is not currently running, 
#   in other words this will return the index of the break in 
#   script windows structure 
#
#############################################################################

proc IsBreakCurrentlyRunning {name numb quit_number} {

    global running_scripts startupData script_windows 
    
    if {[lsearch [array names startupData] break,-1] != -1} {
	foreach i $startupData(break,-1) {
	    set type [lindex $startupData(break,$i) 0]
	    set number [lindex $startupData(break,$i) 1]
	    
	    if {$name == $type && $numb == $number} {

		set index $startupData(file),break,$i

		if {[lsearch [array name script_windows] $index] != -1} {
		    
		    # the break type is running 
		    
		    if {$quit_number != $numb} {
			return $i
		    }
		    
		}   
	    }   
	}   
    }
    return -1 
}



#############################################################################
#
#  RemoveFromPendingScript: 
#  removes the pending_scripts with the index provided also take the item off
#  the running_scripts list if it exists 
#
#############################################################################

proc RemoveFromPendingScripts {index} {
    
    global pending_scripts running_scripts 
    
    RemoveFromRunningScripts $index 

    unset pending_scripts($index)
    
}

#############################################################################
#
#  RemoveFromRunningScripts: 
#  This finds the scripts from the running_scripts and delete from it. 
#  it also updates typebroken and componenets broken information 
#
#############################################################################

proc RemoveFromRunningScripts {index} {
    
    global running_scripts startupData 
    
    set next_comma [string first "," $index]
    set file_name [string range $index 0 [expr $next_comma -1]]
    set temp_index [string range $index [expr $next_comma +1] end]
    
    set next_comma [string first "," $temp_index]
    set type [string range $temp_index 0 [expr $next_comma -1]]
    set number [string range $temp_index [expr $next_comma +1] end]
    
    if {$type == "animations"} {
	set deleted_item "Animation $number"
    } elseif {$type == "graphs"} {
	set deleted_item "Graph $number"
    } elseif {$type == "phase"} {
	set deleted_item "Phase $number"
    } elseif {$type == "break"} {
	set deleted_item "Break $number"

	UpdateFinishBreakInfo $number 

    } elseif {$type == "trace"} {
	set deleted_item "Trace $number"
    } elseif {$type == "stop"} {
	set deleted_item "Stop $number"
    } else {
	puts "error in remove from running script" 
    }
    
    set deleted_script [lsearch $running_scripts($file_name) $deleted_item]
    
    set running_scripts($file_name) [concat [lrange $running_scripts($file_name) 0 [expr $deleted_script - 1]] [lrange $running_scripts($file_name) [expr $deleted_script + 1] end]]
    
}

#############################################################################
#
#   UpdateFinishBreakInfo: 
#   This is called when the script on break finishes.  
#
#############################################################################

proc UpdateFinishBreakInfo {number} {

    global startupData TypesBroken ComponentsBroken 
    
    set break_type [lindex $startupData(break,$number) 0]
    set break_inst [lindex $startupData(break,$number) 1]
    
    if {$break_inst == -1} {
	# this is breaking type 
	set TypesBroken($break_type) 0
	UpdateTypesBox 
    } else {
	# this is breaking a component
	set ComponentsBroken($break_type,$break_inst) 0
	UpdateShowWindow
    }
}

#############################################################################
#
#   UpdateStartBreakInfo: 
#   This is called when the script on break is called 
#
#############################################################################

proc UpdateStartBreakInfo {number} {

    global startupData TypesBroken ComponentsBroken 

    set break_type [lindex $startupData(break,$number) 0]
    set break_inst [lindex $startupData(break,$number) 1]
    
    if {$break_inst == -1} {
	# this is breaking type 
	set TypesBroken($break_type) 1
	UpdateTypesBox 
    } else {
	# this is breaking a component
	set ComponentsBroken($break_type,$break_inst) 1
	UpdateShowWindow
    }
}

#############################################################################
#
#   UpdateTypesBox: 
#   When script on break starts to run, we want to change the label of the 
#   button, so the user is not confused 
#
#############################################################################

proc UpdateTypesBox {} {
    
    global itemselect startupData TypesBroken 

    if {[winfo exists .typesbox]} {
	
	if {[lsearch [array names itemselect] type_chosen] != -1} {
	    set break_type $itemselect(type_chosen)
	    set break_inst -1
	    
	    set script_index [IsBreakRunning $break_type $break_inst]
	    
	    if {$script_index != -1} {
		
		# the current script is running, and configure the window 
		# correctly 
	    
		if {$TypesBroken($break_type) == 1} {
		    .typesbox.body.buttons.breaktype configure -text "Clear Type"
		} else {
		    .typesbox.body.buttons.breaktype configure -text "Break Type"
		}
	    }
	    
	}
    }
}

#############################################################################
#
#   UpdateShowWindow: 
#   When script on break starts to run, we want to change the label of the 
#   button, so the user is not confused 
#
#############################################################################

proc UpdateShowWindow {} {
    
    global itemselect startupData ComponentsBroken 

    if {[lsearch [array names itemselect] instance_chosen] != -1} {
	set break_type [lindex $itemselect(instance_chosen) 0]
	set break_inst [lindex $itemselect(instance_chosen) 1]
	
	set w .info_$break_type
	
	if {[winfo exists $w]} {
	    
	    set script_index [IsBreakRunning $break_type $break_inst]
	    
	    if {$script_index != -1} {
		
	    # the current script is running, and configure the window 
		# correctly 
		
		if {$ComponentsBroken($break_type,$break_inst) == 1} {
		    $w.body.buttons.breakcomp configure -text "Clear Component"
		} else {
		    $w.body.buttons.breakcomp configure -text "Break Component"
		}
	    }
	    
	}
    }
}

#############################################################################
#
#   IsBreakRunning:
#   Returns -1 if break type and inst specified is not currently running in 
#   script (in other word, it is not in running_script). 
#   Returns the index if the script is in running_script list 
#
#############################################################################

proc IsBreakRunning {name numb} {

    global running_scripts startupData
    
    if {[lsearch [array names startupData] break,-1] != -1} {
	foreach i $startupData(break,-1) {
	    set type [lindex $startupData(break,$i) 0]
	    set number [lindex $startupData(break,$i) 1]
	    
	    if {$name == $type && $numb == $number} {
		
		if {[lsearch $running_scripts($startupData(file)) "Break $i"] != -1} {
		    
		    # the break type is running 
		    
		    return $i
		    
		}   
	    }   
	}   
    }
    return -1 
}




#############################################################################
#
#   This function check to see if the window exists among startup_windows 
#   and get rid of the window from startup_windows 
#
#############################################################################

proc RemoveFromStartUpWindows {{w none}} {
    
    global startup_windows

    if {$w != "none"} {
	set i [lsearch $startup_windows $w]
	set startup_windows [concat [lrange $startup_windows 0 [expr $i - 1] ]  \
		[lrange $startup_windows [expr $i + 1] end] ]
	
	RemoveFromScriptWindows $w
    }
}


#############################################################################
#
#   RemoveFromScriptWindows will remove the entry in script_windows which 
#   this window belongs to 
#
#############################################################################

proc RemoveFromScriptWindows {w} {
    
    global script_windows running_scripts 

    set index_list [array names script_windows]
    foreach index $index_list {
	if {[lindex $script_windows($index) 1] == $w} {

	    RemoveFromRunningScripts $index 
	    catch {unset script_windows($index)}
	}
    }
}

#############################################################################
#
#    This is a function that would sucessfully cleanup a script_window 
#
#############################################################################

proc DestroyScript {index} {
    
    global script_windows messageQueue running_scripts startupData 
    
    set next_comma [string first "," $index]
    set file_name [string range $index 0 [expr $next_comma -1]]
    set temp_index [string range $index [expr $next_comma +1] end]
    
    set next_comma [string first "," $temp_index]
    set type [string range $temp_index 0 [expr $next_comma -1]]
    set number [string range $temp_index [expr $next_comma +1] end]
	
    set w [lindex $script_windows($index) 1]; 
	
    if {$type == "animations"} {
	DequeueFromOpenWindows $w; 
	RemoveFromStartUpWindows $w
	if {[winfo exists $w]}  {
	    ExitCanvas [wm title $w]
	    catch {destroy $w}
	}
    } elseif {$type == "graphs"} {
	RemoveGraphData $w
	DequeueFromOpenWindows $w
	RemoveFromStartUpWindows $w
	catch {destroy $w}
    } elseif {$type == "phase"} {
	RemoveGraphData $w
	DequeueFromOpenWindows $w
	RemoveFromStartUpWindows $w
	catch {destroy $w}
    } elseif {$type == "break"} {
	
	QuitBreak $number 

    } elseif {$type == "stop"} {

	puts "error in DestroyScript: stop is called" 

    } elseif {$type == "trace"} {
	
	catch {destroy $w}

    } else {

	puts "error in DestroyScript: unknown type"

    }
    
    # this function is also called from other functions that 
    # the window might have already been destroyed 
    catch {unset script_windows($index)}
}

#############################################################################
#
#   BreakExists: 
#   This function will return 1, if the break exist in startupData, other 
#   wise 0 
#
#############################################################################

proc BreakExists {name {numb -1}} {

    global running_scripts startupData
    
    if {[lsearch [array names startupData] break,-1] != -1} {
	foreach i $startupData(break,-1) {
	    set type [lindex $startupData(break,$i) 0]
	    set number [lindex $startupData(break,$i) 1]
	    
	    if {$name == $type && $numb == $number} {
		return 1
	    }   
	}   
    }
    return 0
}
