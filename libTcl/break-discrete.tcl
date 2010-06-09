# -*- Mode: Tcl -*-

# front.tcl --

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


### This records what the user selects with a double click 

# commented by duke 8/14/97 
# when we are reusing the former inspect function in adding scripts
# we can limit some of the functionality that inspect used to offer
# such as viewing the information about components.  It is just easiear
# if the user didn't do any of that, and they don't need to . . . so 
# I am disabling some of the functionality 

proc itemselectClick {lb y request_type w {caller_function unknown}} {
    # Take the item the user clicked on
    global itemselect ListComponents messageQueue TypesBroken
    global TypesTraced ComponentsTraced ComponentsBroken

    set itemselect($request_type) [$lb get [$lb nearest $y]]
    
    if {$request_type == "type_chosen"} {
	.typesbox.body.buttons.breaktype configure -state normal  
	set type $itemselect($request_type)

	if {$TypesBroken($type) == 1} {
	    if {$caller_function == "CreateStartFile"} {
		.typesbox.body.buttons.breaktype configure -text "Break Type"
	    } else {
		.typesbox.body.buttons.breaktype configure -text "Clear Type"
	    }
	    
	} else {
	    .typesbox.body.buttons.breaktype configure -text "Break Type"
	}
	
	if {$caller_function != "CreateStartFile"} {
	    if {$TypesTraced($type) == 1} {
		.typesbox.body.buttons.tracetype configure -text "Untrace Type"
	    } else {
		.typesbox.body.buttons.tracetype configure -text "Trace Type"
	    }
	}

    } else {
	$w.body.buttons.breakcomp configure -state normal  
	set type $itemselect($request_type)
	set name [lindex $type 0]
	set numb [lindex $type 1]
	#puts "Control demands $name and $numb to be current"

	if {$name != ""} {
	    
	    if {$caller_function == "CreateStartFile"} {
		$w.body.buttons.breakcomp configure -text "Break Component"
	    } else {
		if {$ComponentsBroken($name,$numb) == 1} {
		    $w.body.buttons.breakcomp configure -text "Clear Component"
		    if {$caller_function == "CreateStartFile"} {
			$w.body.buttons.breakcomp configure -state disabled 
		    }
		} else {
		    $w.body.buttons.breakcomp configure -text "Break Component"
		}
	    }
	}
    }
}


proc BreakTypeSet {{caller_function unknown} {caller_window unknown}} {
    
    global itemselect ListComponents messageQueue TypesBroken startupData 

    if {$itemselect(type_chosen) == ""} {
	bell
	tk_dialog .error Error "You must select a type to break" error 0 OK
	return
    }
    if {$itemselect(type_chosen) == "global"} {
	bell
	tk_dialog .error Error "You cannot break global" error 0 OK
	return
    }

    if {$caller_function == "CreateStartFile"} {
	
	# this means that we are not really breaking anything at this time
	# by just clicking but to create a script

	CallBreak $itemselect(type_chosen) $caller_function $caller_window 

    } else {

	if {$TypesBroken($itemselect(type_chosen)) == 0} {
	    CallBreak $itemselect(type_chosen) $caller_function $caller_window 
	    #set TypesBroken($itemselect(type_chosen)) 1
	} elseif {$TypesBroken($itemselect(type_chosen)) == 1} {
	    
	    if {[DeleteBreak $itemselect(type_chosen)]} {
		CallDeleteBreak $itemselect(type_chosen)
	    }
	    set TypesBroken($itemselect(type_chosen)) 0
	    .typesbox.body.buttons.breaktype configure -text "Break Type"
	}
    }
}

proc TraceTypeSet {{caller_function unknown}} {

    global itemselect ListComponents messageQueue TypesTraced

    if {$itemselect(type_chosen) == ""} {
	tk_dialog .error Error "You must select a type to break" error 0 OK
	return
    }

    if {$TypesTraced($itemselect(type_chosen)) == 0} {
	set TypesTraced($itemselect(type_chosen)) 1
	.typesbox.body.buttons.tracetype configure -text "Untrace Type"
    } elseif {$TypesTraced($itemselect(type_chosen)) == 1} {
	set TypesTraced($itemselect(type_chosen)) 0
	.typesbox.body.buttons.tracetype configure -text "Trace Type"
    }
}


proc instanceselectClick {} {
    
    global itemselect 

    if {$itemselect(instance_chosen) == ""} {
	tk_dialog .error Error "You must select an instance to view" error 0 OK
	return
    }
    set type $itemselect(instance_chosen)
    PrintInstance $type
}


proc BreakComponentSet {w {caller_function unknown}} {

    global itemselect ListComponents messageQueue ComponentsBroken

    if {$itemselect(instance_chosen) == ""} {
	tk_dialog .error Error "You must select an instance to break" error 0 OK
	return
    }
    set type $itemselect(instance_chosen)
    set numb [lindex $type 1]
    set name [lindex $type 0]

    if {$caller_function == "CreateStartFile"} {
	CallCompBreak $w $name $numb $caller_function 
    } else {
	
	if {$ComponentsBroken($name,$numb) == 0} {
	    CallCompBreak $w $name $numb $caller_function 
	} elseif {$ComponentsBroken($name,$numb) == 1} {
	    
	    if {[DeleteCompBreak $name $numb]} {
		CallDeleteCompBreak $name $numb 
		set ComponentsBroken($name,$numb) 0
		$w.body.buttons.breakcomp configure -text "Break Component"
	    }
	}
    }
}

####
#### NO LONGER CALLED 

proc CommunicateBreakTypes {name1 name2 op} {

    global TypesBroken

#    puts "I have gotten $name1 and $name2 and $op"

    if {$TypesBroken($name2) == 0} {
#	puts "Came to the 0 stop"
#	set TypesBroken($name2) 1
	CallDeleteBreak $name2
    } elseif {$TypesBroken($name2) == 1} {
#	puts "Came to the 1 stop"
#	set TypesBroken($name2) 0
	CallBreak $name2
    }
#    puts "end of communicate types"
}

proc CommunicateBreakComps {name1 name2 op} {

    global ComponentsBroken

#    puts "I have gotten $name1 and $name2 and $op"
	set l [string first "," $name2]
	set type [string range $name2 0 [expr $l -1]]
	set inst [string range $name2 [expr $l +1] end]

    if {$ComponentsBroken($name2) == 0} {
#	puts "Came to the 0 stop with $name2"
	CallDeleteCompBreak $type $inst
#	set ComponentsBroken($name2) 1
#	CallDeleteBreak $name2
    } elseif {$ComponentsBroken($name2) == 1} {
#	puts "Came to the 1 stop with $name2"
	CallCompBreak $type $inst
#	set ComponenetsBroken($name2) 0
#	CallBreak $name2
    }
#    puts "end of communicate types"
}


proc CallBreak {type {caller_function unknown} {caller_window unknown}} {

    global BreakType messageQueue TypeEventsArray BreakTypeEvents
    global messageQueue TypesBroken startup_windows 

    set events $TypeEventsArray($type)
    if {[llength $events] <= 1} {
	set strings_to_send 0
	set msg "$type $strings_to_send"
	set msg [padMessage $msg]

	if {$caller_function == "CreateStartFile"} {
	    set items "" 
	    lappend items 8000
	    lappend items $msg
	    AddBreak [list $type -1 $items]
	    RemoveFromStartUpWindows $caller_window 
	    destroy $caller_window 
	} else {
	    lappend messageQueue 8000
	    lappend messageQueue $msg

	    #puts "this is messageQueue: $messageQueue" 

	    .typesbox.body.buttons.breaktype configure -text "Clear Type"
	    set TypesBroken($type) 1
	}

	return
    }

    set w .breakwin
    toplevel $w
    frame $w.body -relief raised -bd 2
    frame $w.bottom
    tkwait visibility $w
    grab $w

    canvas $w.body.c -relief raised -bd 2 -bg bisque -yscrollcommand\
	    [list $w.body.yscroll set] -width 230 -height 350\
	    -scrollregion [list 0 0 230 350]
    scrollbar $w.body.yscroll -orient vertical\
	    -command [list $w.body.c yview]
    frame $w.body.c.ev
    frame $w.body.buttons
    button $w.bottom.ok -text "OK" -command "sendBreakMessage $w $type $caller_function $caller_window"
    button $w.bottom.cancel -text "Cancel" -command "destroy $w"
    button $w.body.buttons.all -text "All" -command "selectAllEvents $type"
    button $w.body.buttons.none -text "Clear" -command "selectNoEvents $type"
    set events $TypeEventsArray($type)
    
    
    set counter 1
    foreach event $events {
	set BreakTypeEvents($event) 1
	#checkbutton $w.body.c.ev.$event -variable BreakTypeEvents($event)
	#label $w.body.c.ev.lab_$event -text $event
	#grid $w.body.c.ev.$event -row $counter -column 0
	#grid $w.body.c.ev.lab_$event -row $counter -column 1

	checkbutton $w.body.c.the_$event -variable BreakTypeEvents($event)	
	label $w.body.c.lab_$event -text $event

	$w.body.c create window 15 [expr $counter * 25] -window	$w.body.c.the_$event 
	$w.body.c create window 130 [expr $counter * 25] -window $w.body.c.lab_$event 
	incr counter
    }

    if {$counter > 15} {
	set height [expr $counter * 25 + 10]
	$w.body.c configure -height 400
	$w.body.c configure -scrollregion [list 0 0 0 $height]
	#puts "For a height of $height, I have a counter of $counter"
    }
    
    #grid $w.body.c.ev -row 0 -column 0
    #    grid rowconfigure $w.body.c.ev 0 -weight 1
    #grid columnconfigure $w.body.c.ev 0 -weight 0
    #grid columnconfigure $w.body.c.ev 1 -minsize 70 -weight 1

    grid $w.body.c -row 1 -column 0 -sticky news
    grid $w.body.yscroll -row 1 -column 1 -sticky ns
    grid $w.bottom -row 1 -column 0 -sticky news

    grid $w.body.buttons.all -row 0 -column 0
    grid $w.body.buttons.none -row 0 -column 1
    grid $w.body.buttons -row 0 -column 0 -sticky news

    grid columnconfigure $w 0 -minsize 200 -weight 1
    grid rowconfigure $w 0 -minsize 100 -weight 1
    grid rowconfigure $w 1 -minsize 50

    grid rowconfigure $w.body 0 -minsize 50 -weight 1
    grid rowconfigure $w 1 -minsize 50
    grid $w.body  -row 0 -column 0 -sticky news
    #grid $w.body  -row 0 -column 1 -sticky ns
    grid $w.bottom.ok -column 0 -row 0
    grid $w.bottom.cancel -column 1 -row 0
    
#    lappend messageQueue $BreakType
#    set msg [padMessage $type]
#    lappend messageQueue $msg
}

proc selectAllEvents {type} {

    global BreakTypeEvents TypeEventsArray
    
    foreach i $TypeEventsArray($type) {
	set BreakTypeEvents($i) 1
    }
}

proc selectNoEvents {type} {

    global BreakTypeEvents TypeEventsArray
    
    foreach i $TypeEventsArray($type) {
	set BreakTypeEvents($i) 0
    }
}


proc sendBreakMessage {w type {caller_function unknown} {caller_window unknown}} {

    global BreakTypeEvents TypeEventsArray messageQueue TypesBroken

    set events {}
    foreach i $TypeEventsArray($type) {
	if {$BreakTypeEvents($i) == 1} {
	    lappend events $i
	}   
    }
    if {[llength $events] < 1} {
	    tk_dialog .error Error "You must specify at least one\
		    event in the break type." error 0 OK	    
	return
    }
    
    if {[llength $TypeEventsArray($type)] == [llength $events]} {
	set strings_to_send 0
	set msg "$type $strings_to_send"
	set msg [padMessage $msg]
	
	if {$caller_function == "CreateStartFile"} {
	    set items "" 
	    lappend items 8000
	    lappend items $msg
	    AddBreak [list $type -1 $items]
	} else {
	    lappend messageQueue 8000
	    lappend messageQueue $msg

	    set a ""
	    lappend a 8000
	    lappend a $msg 
	    #puts "this is break command: $a" 
	    set TypesBroken($type) 1
	}

    } else {
	set all_events 0
	set str ""
	set counter 0
	set len [llength $events]
	set strings_to_send 1
	while {$counter < $len} {
	    set ev [lindex $events $counter]
	    set str "$str $ev"
	    incr counter
	}
	set str_len [string length $str]
	set msg "$type $str_len"
	#Send breaktype info, and number of string to expect
	set msg [padMessage $msg]

	if {$caller_function == "CreateStartFile"} {
	    set items ""
	    lappend items 8000
	    lappend items $msg
	    lappend itmes $str 
	    # -1 for instance number 
	    AddBreak [list $type -1 $items]
	} else {
	    lappend messageQueue 8000
	    lappend messageQueue $msg
	    #Now send the strings
	    set msg $str
	    puts "The message is $msg"
	    lappend messageQueue $msg
	    set TypesBroken($type) 1
	}
    }
    
    .typesbox.body.buttons.breaktype configure -text "Clear Type"
    if {$caller_function == "CreateStartFile"} {	
	RemoveFromStartUpWindows $caller_window 
	destroy $caller_window 
    }
    destroy $w
}


proc CallDeleteBreak {type} {

    global BreakType messageQueue TypesBroken

    set TypesBroken($type) 0
    lappend messageQueue 8002
    set msg [padMessage $type]
    lappend messageQueue $msg
#    destroy .info_$type
}

proc CallCompBreak {win type inst {caller_function unknown}} {

    global BreakType messageQueue TypeEventsArray BreakTypeEvents
    global messageQueue ComponentsBroken

    puts "in callcompbreak: caller function is $caller_function" 

    set events $TypeEventsArray($type)
    if {[llength $events] <= 1} {
	set strings_to_send 0
	set msg "$type $inst"
	set msg [padMessage $msg]

	if {$caller_function == "CreateStartFile"} {
	    set items ""
	    lappend items 8003
	    lappend items $msg
	    AddBreak [list $type $inst $items] 
	    RemoveFromStartUpWindows $win
	    #	    RemoveFromStartUpWindows .typesbox
	    #	    catch {destroy .typesbox}
	    destroy $win 
	} else {
	    lappend messageQueue 8003
	    lappend messageQueue $msg
	    $win.body.buttons.breakcomp configure -text "Clear Component"
	    set ComponentsBroken($type,$inst) 1
	}
	
	return
    }

    set w .breakwin
    toplevel $w
    frame $w.body -relief raised -bd 2
    frame $w.bottom
    tkwait visibility $w
    grab $w
    canvas $w.body.c -relief raised -bd 2 -bg bisque -yscrollcommand\
	    [list $w.body.yscroll set] -width 260 -height 350\
	    -scrollregion [list 0 0 230 350]
    scrollbar $w.body.yscroll -orient vertical\
	    -command [list $w.body.c yview]
    #canvas $w.body.c -relief raised -bd 2 -bg gray
    frame $w.body.c.ev
    frame $w.body.buttons
    button $w.bottom.ok -text "OK" -command "sendCompBreakMessage $w $win $type $inst $caller_function"
    button $w.bottom.cancel -text "Cancel" -command "destroy $w"
    button $w.body.buttons.all -text "All" -command "selectAllEvents $type"
    button $w.body.buttons.none -text "Clear" -command "selectNoEvents $type"
    set events $TypeEventsArray($type)
    
    set counter 1
    foreach event $events {
	set BreakTypeEvents($event) 1
	#checkbutton $w.body.c.ev.$event -variable BreakTypeEvents($event)
	#label $w.body.c.ev.lab_$event -text $event
	#grid $w.body.c.ev.$event -row $counter -column 0
	#grid $w.body.c.ev.lab_$event -row $counter -column 1
	#incr counter

	checkbutton $w.body.c.the_$event -variable BreakTypeEvents($event)	
	label $w.body.c.lab_$event -text $event
	$w.body.c create window 15 [expr $counter * 25] -window	$w.body.c.the_$event 
	$w.body.c create window 130 [expr $counter * 25] -window $w.body.c.lab_$event 
	incr counter
    }

    if {$counter > 15} {
	set height [expr $counter * 25 + 10]
	$w.body.c configure -height 400
	$w.body.c configure -scrollregion [list 0 0 0 $height]
	#puts "For a height of $height, I have a counter of $counter"
    }

    
    #grid $w.body.c.ev -row 0 -column 0
#    grid rowconfigure $w.body.c.ev 0 -weight 1
    #grid columnconfigure $w.body.c.ev 0 -weight 0
    #rid columnconfigure $w.body.c.ev 1 -minsize 70 -weight 1

    grid $w.body.c -row 1 -column 0 -sticky news
    grid $w.body.yscroll -row 1 -column 1 -sticky ns
    grid $w.bottom -row 1 -column 0 -sticky ew

    grid $w.body.buttons.all -row 0 -column 0
    grid $w.body.buttons.none -row 0 -column 1
    grid $w.body.buttons -row 0 -column 0 -sticky ew

    grid columnconfigure $w 0 -minsize 200 -weight 1
    grid rowconfigure $w 0 -minsize 100 -weight 1
    grid rowconfigure $w 1 -minsize 50

    grid rowconfigure $w.body 0 -minsize 50 -weight 1
    grid rowconfigure $w 1 -minsize 50
    grid $w.body  -row 0 -column 0 -sticky news
    grid $w.bottom.ok -column 0 -row 0
    grid $w.bottom.cancel -column 1 -row 0

#    global messageQueue

#    lappend messageQueue 8003
#    set str "$name $inst"
#    set msg [padMessage $str]
#    Lappendv messageQueue $msg
}

proc sendCompBreakMessage {w win type inst {caller_function unknown}} {
    
    global BreakTypeEvents TypeEventsArray messageQueue TypesBroken ComponentsBroken


    #    puts "in sendcompbreak: caller function is $caller_function" 

    set events {}
    foreach i $TypeEventsArray($type) {
	if {$BreakTypeEvents($i) == 1} {
	    lappend events $i
	}   
    }
    
    if {[llength $events] < 1} {
	tk_dialog .error Error "You must specify at least one\
		event in the break type." error 0 OK	    
	return
    }
    
    if {[llength $TypeEventsArray($type)] == [llength $events]} {
	set strings_to_send 0
	set msg "$type $inst"
	set msg [padMessage $msg]
	
	if {$caller_function == "CreateStartFile"} {
	    set items 8003
	    lappend items $msg
	    AddBreak [list $type $inst $items]
	} else {
	    lappend messageQueue 8003
	    lappend messageQueue $msg
	    set ComponentsBroken($type,$inst) 1
	}
	
    } else {
	set all_events 0
	set str ""
	set counter 0
	set len [llength $events]
	set strings_to_send 1
	while {$counter < $len} {
	    set ev [lindex $events $counter]
	    set str "$str $ev"
	    incr counter
	}
	set str_len [string length $str]
	set msg "$type $inst $str_len"
	#Send breaktype info, and number of string to expect
	set msg [padMessage $msg]

	if {$caller_function == "CreateStartFile"} {
	    set items 8003
	    lappend items $msg
	    lappend items $str
	    AddBreak [list $type $inst $items]
	} else {
	    lappend messageQueue 8003
	    lappend messageQueue $msg
	    #Now send the strings
	    set msg $str
	    puts "The message is $msg"
	    lappend messageQueue $msg
	    set ComponentsBroken($type,$inst) 1
	}
    }
    $win.body.buttons.breakcomp configure -text "Clear Component"
    if {$caller_function == "CreateStartFile"} {
	RemoveFromStartUpWindows $win
	RemoveFromStartUpWindows .typesbox
	catch {destroy .typesbox}
	destroy $win 
    }
    destroy $w
}


proc CallDeleteCompBreak {name inst} {

    global messageQueue

    lappend messageQueue 8004
    set str "$name $inst"
    set msg [padMessage $str]
    lappend messageQueue $msg
#    destroy .info_$type
}

# end of file -- communications.tcl --


