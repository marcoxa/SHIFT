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


# commented by duke 8/14/97 

# caller_function is added to distinguish the cases when it is called by 
# script function or other functions to reuse in script function 

# when we are reusing the former inspect function in adding scripts
# we can limit some of the functionality that inspect used to offer
# such as viewing the information about components.  It is just easiear
# if the user didn't do any of that, and they don't need to . . . so 
# I am disabling some of the functionality 

proc TypesBox {{caller_function unknown}} {
    
    global TypeList OtherFont OpenWindows itemselect shift_tk_dir startup_windows 
    
    set w .typesbox
    if {[winfo exists $w]} {
	#	catch {wm deiconify $w}
	#	catch {raise $w}
	#	return
	destroy $w 
    } else {
	lappend startup_windows $w
    }
    
    toplevel .typesbox  
    #-width 450 -height 350
    
    set itemselect(type_chosen) ""
    set w .typesbox
    wm title .typesbox "Simulation types"
    lappend OpenWindows ".typesbox"
    
    #frame .typesbox.bar -relief raised -bd 2 -class menu
    frame .typesbox.body -relief raised -bd 2
    frame $w.bottom
    frame $w.body.list
    frame $w.body.buttons
    
    #bind $w <Configure> "ConfigureTypesBoxWindow .typesbox"
    
    #menubutton .typesbox.bar.control -text "Types" -menu .typesbox.bar.control.menu1
    #set menu1 [menu .typesbox.bar.control.menu1 -tearoff 0]
    #$menu1 add command -label "Close"  -command \
	    "DequeueFromOpenWindows .typesbox;destroy .typesbox"
    
    if {$caller_function == "CreateStartFile"} {
	grab $w 
	wm protocol .typesbox WM_DELETE_WINDOW "DequeueFromOpenWindows $w; RemoveFromStartUpWindows $w; destroy $w"
    } else {
	wm protocol .typesbox WM_DELETE_WINDOW "DequeueFromOpenWindows .typesbox;destroy .typesbox"
    }
    
    set t .typesbox.body.list
    set lb [listbox $t.list  -setgrid 1\
	    -yscrollcommand [list $t.scroll set]]
    scrollbar $t.scroll -command [list $lb yview]
    
    foreach i $TypeList {
	$lb insert end $i
    }

    if {$caller_function == "CreateStartFile"} {
	button $w.body.buttons.view -text "View Instances" \
		-command "typeselectClick $caller_function; RemoveFromStartUpWindows $w; destroy $w"
    } else {
	button $w.body.buttons.view -text "View Instances" \
		-command "typeselectClick $caller_function"
	button $w.body.buttons.tracetype -text "Trace Type" \
		-command "TraceTypeSet $caller_function" -state disabled
    }
    
    button $w.body.buttons.breaktype -text "Break Type" \
	    -command "BreakTypeSet $caller_function $w"
    
    if {$caller_function == "CreateStartFile"} {
	button $w.bottom.close -text "Close" \
		-command "DequeueFromOpenWindows $w; RemoveFromStartUpWindows $w; destroy .typesbox"
    } else {
	button $w.body.buttons.viewinfo -text "View Information" -command "ViewTypeInfo"
	button $w.bottom.close -text "Close" \
		-command "DequeueFromOpenWindows .typesbox;destroy .typesbox"
	bind $lb <Double-Button-1> "itemselectClick %W %y type_chosen $w $caller_function;\
		typeselectClick $caller_function"
    
	
    }

    bind $lb <1> "itemselectClick %W %y type_chosen $w $caller_function"
    
    set width [winfo width $w]
    set height [winfo height $w]
    
    #   place $w.bar -height 40 -relwidth 1 -y 0 -relwidth 1
    #pack $w.body.buttons -fill both
    
    grid $w.body -row 0 -column 0 -sticky nsew
    grid $w.bottom -row 1 -column 0 -sticky sewn
    grid rowconfigure $w 1 -minsize 50 -weight 0
    grid rowconfigure $w 0 -minsize 300 -weight 1
    grid columnconfigure $w 0 -minsize 450 -weight 1
    # -sticky s
    
    grid $w.body.buttons -column 0 -row 0 -sticky n -pady 8
    grid $w.body.list -column 1 -row 0 -sticky nsew

    grid rowconfigure $w.body 0 -weight 1
    grid columnconfigure $w.body 0 -minsize 225 -weight 1
    grid columnconfigure $w.body 1 -minsize 225  -weight 1
    # -relheight 1 -relwidth .5 -relx .5
    
    grid columnconfigure $w.body.buttons 0 -weight 1
    grid rowconfigure $w.body.list 0 -weight 1
    #    grid rowconfigure $w.body.buttons 0 -weight 5
    
    grid $w.body.buttons.view -sticky new -row 0 -column 0

    if {$caller_function != "CreateStartFile"} {
	grid $w.body.buttons.viewinfo -sticky new -row 1 -column 0
	grid $w.body.buttons.tracetype -sticky new -row 2 -column 0
    }

    grid $w.body.buttons.breaktype -sticky new -row 3 -column 0
    
    grid $w.bottom.close 
    pack $w.body.list.scroll -side right -fill y
    pack $w.body.list.list -side left -fill both -expand 1
}


proc ViewTypeInfo {} {

    global itemselect TypeEventsArray TypeModesArray TypesArray

    if {$itemselect(type_chosen) == ""} {
	tk_dialog .error Error "You must select a type to view" error 0 OK
	return
    }
    set type $itemselect(type_chosen)

    set w .type_info$type
    if {[winfo exists $w]} {
	
	return
    }

    toplevel $w -height 350 -width 550
    wm title $w "$type Information"
    frame $w.main -height 300 -relief raised -bd 2
    frame $w.bottom -height 50 
    place $w.main -relwidth 1 
    place $w.bottom -relwidth 1 -y 300
    scrolled_listbox $w.vars "Variables"
    scrolled_listbox $w.modes "Discrete Modes"
    scrolled_listbox $w.events "Exported Events"

    place $w.vars -relwidth 0.3 -relx 0.025
    place $w.modes -relwidth 0.3 -relx 0.35 
    place $w.events -relwidth 0.3 -relx .675

    foreach i $TypeEventsArray($type) {
	$w.events.list.list insert end $i
    }
    foreach i $TypeModesArray($type) {
	$w.modes.list.list insert end $i
    }
    foreach i $TypesArray($type) {
	$w.vars.list.list insert end $i
    }

    button $w.bottom.close -text "Close" -command "destroy $w"
    place $w.bottom.close -relwidth .3 -relx .35 -relheight .7 -rely .15
}


proc typeselectClick {{caller_function unknown}} {
    
    global itemselect ListComponents messageQueue startup_windows 
    
    if {$itemselect(type_chosen) == ""} {
	tk_dialog .error Error "You must select a type to view" error 0 OK
	return
    }
    
    lappend messageQueue $ListComponents
    set msg [padMessage $itemselect(type_chosen)]
    lappend messageQueue $msg

    # commented by duke 8/14/97 
    # This request the simulation for information about the instances. 
    # Then communication.c calls Show function to create box to put 
    # instance in the window.  Inside of the Show function we can see 
    # that the name of the window created is .info_$type 

    # it would be more logical to think that we should append the 
    # window when the .info$type windows are created. . . but 
    # there is no other way for .info$type window to know what it is 
    # called by script 
    
    if {$caller_function == "CreateStartFile"} {
	set w .info_$itemselect(type_chosen)
	if {[winfo exists $w] == 0} {
	    lappend startup_windows $w
	}
    }
}


proc ConfigureTypesBoxWindow {w} {

    set width [winfo width $w]
    set height [winfo height $w]

    #    place $w.bar -height 40 -relwidth 1 -y 0 -relwidth 1
    place $w.body -relheight 1 -relwidth 1

        place $w.body.list -relheight 1 -relwidth .5 -relx .5
    place $w.body.buttons -relheight 1 -relwidth .5 


    pack $w.body.list.scroll -side right  -fill y
    pack $w.body.list.list -side left -fill both -expand 1
}

######################################################################
## 
## 
##
######################################################################



############################################################################
#  This procedure is called when one clicks on one of the types in the typesbox
# menu.  This brings up a list of all instances of a particular type. 
# 
#
############################################################################

# commented by duke 8/14/97 
# in order to know what it is in fact called by script functionality we have
# to look into startup_windows which is a data structure that keeps all the 
# windows non-script windows that is opened by script main panel.  Before
# we ask the simluation to send us information about the instances, we 
# append the following window name into startup_windows.  Yes, this is kind of 
# heck and when a person want to change the name of this window you also 
# have to change the window name in function typebox and typeselectclick  

proc Show {type} {

    global InstanceList InstanceTypeList TypeR SmallFont
    global messageQueue specialData BreakType BreakComp GlobalBreakType
    global TypesBroken OpenWindows OtherFont itemselect ComponentsBroken

    global startup_windows 

    if {[winfo exists .info_$type]} {
	#	catch {wm deiconify .info_$type}
	#	catch {raise .info_$type}
	#	return
	destroy .info_$type 
    }

    set itemselect(instance_chosen) ""
    toplevel .info_$type -height 300 -width 450
    set w .info_$type

    wm title .info_$type "Type $type"
    lappend OpenWindows ".info_$type"
    frame .info_$type.body -relief raised -bd 2
    frame $w.bottom
    frame $w.body.buttons
    frame $w.body.list

    
    ######################This is the code that created the menubar

    #menubutton .info_$type.bar.control -text "Control" -menu \
	    .info_$type.bar.control.menu1
    #menubutton .info_$type.bar.info -text "Information" -menu \
	    .info_$type.bar.info.menu2

    #menubutton .info_$type.bar.help -text "Help" -menu \
	    .info_$type.bar.help.menu3

    #set menu1 [menu .info_$type.bar.control.menu1 -tearoff 0]
    #set menu2 [menu .info_$type.bar.info.menu2 -tearoff 0]
    #set menu3 [menu .info_$type.bar.help.menu3 -tearoff 0]

    #$menu1 add checkbutton -label "Break Type"  \
	    -variable TypesBroken($type)
    #$menu1 add separator
    #$menu1 add command -label "Close"\
	   -command "DequeueFromOpenWindows .info_$type;\
	   set InstanceTypeList($type) {}; destroy .info_$type"
    
    if {[lsearch $startup_windows $w] != -1} {
	#this means that this function is called from CreateStartFile 
	wm protocol $w WM_DELETE_WINDOW "DequeueFromOpenWindows .info_$type; RemoveFromStartUpWindows info_$type; set InstanceTypeList($type) {};destroy .info_$type"
	grab $w 
    } else {
	wm protocol $w WM_DELETE_WINDOW "DequeueFromOpenWindows .info_$type; set InstanceTypeList($type) {};destroy .info_$type"
    }
    
    set t .info_$type.body.list
    set lb [listbox $t.list \
		-yscrollcommand [list $t.scroll set]]
    scrollbar $t.scroll -command [list $lb yview]

    
    button $w.body.buttons.tracecomp -text "Trace Component" -state disabled

    if {[lsearch $startup_windows $w] != -1} {
	#this means that this function is called from CreateStartFile 
	button $w.body.buttons.breakcomp -text "Break Component" \
		-command "BreakComponentSet $w CreateStartFile"
	button $w.bottom.close -text "Close" \
		-command "DequeueFromOpenWindows .info_$type;\
		RemoveFromStartUpWindows .info_$type; \
		set InstanceTypeList($type) {};destroy .info_$type"
	bind $lb <1> "itemselectClick %W %y instance_chosen $w CreateStartFile"
    } else {
	button $w.body.buttons.breakcomp -text "Break Component" \
		-command "BreakComponentSet $w"
	button $w.bottom.close -text "Close" \
		-command "DequeueFromOpenWindows .info_$type;\
		set InstanceTypeList($type) {};destroy .info_$type"
	bind $lb <1> "itemselectClick %W %y instance_chosen $w"
	bind $lb <Double-Button-1> "itemselectClick %W %y instance_chosen $w; instanceselectClick"
	button $w.body.buttons.view -text "View Component" \
		-command instanceselectClick
	grid $w.body.buttons.view -sticky new -row 0 -column 0
    }
    

    foreach i $InstanceTypeList($type) {
	    set numb [lindex $i 1]
	    if {[info exists ComponentsBroken($type,$numb)]} {
		# Do nothing
	    } else {
		set ComponentsBroken($type,$numb) 0
		#trace variable ComponentsBroken($type,$numb) w CommunicateBreakComps
	    }
	    $lb insert end $i
    }


    grid $w.body -row 0 -column 0 -sticky nsew
    grid $w.bottom -row 1 -column 0 -sticky snew
    grid rowconfigure $w 1 -minsize 50 -weight 0
    grid rowconfigure $w 0 -minsize 300 -weight 1
    grid columnconfigure $w 0 -minsize 450 -weight 1
# -sticky s

    grid $w.body.buttons -column 0 -row 0 -sticky n -pady 8
    grid $w.body.list -column 1 -row 0 -sticky nsew

    grid rowconfigure $w.body 0 -weight 1
    grid columnconfigure $w.body 0 -minsize 225 -weight 1
    grid columnconfigure $w.body 1 -minsize 225  -weight 1
# -relheight 1 -relwidth .5 -relx .5

    grid columnconfigure $w.body.buttons 0 -weight 1
    grid rowconfigure $w.body.list 0 -weight 1
#    grid rowconfigure $w.body.buttons 0 -weight 5

#    grid $w.body.buttons.tracecomp -sticky new -row 1 -column 0
    if {$type != "global"} {
	grid $w.body.buttons.breakcomp -sticky new -row 1 -column 0
    }
    grid $w.bottom.close
    pack $w.body.list.scroll -side right -fill y
    pack $w.body.list.list -side left -fill both -expand 1
}



############################################################################
# This function creates a Instance box and fills it with its buttons, listbox,
# and information.  It adds the number to a the PackedData list.
#
#
############################################################################

proc Print {type_name instance_number data_list {last_msg N}} {

    global PackedData Time Fonts messageQueue BreakComp TempMessageNumber
    global UnpackedData OpenWindows OtherFont ComponentView

    set type_number "$type_name$instance_number"

#    puts "Printing $data_list"
    if {$last_msg == "L"} {
	#puts "Do not want this $type_name $instance_number anymore"
	DequeueUnpacked $type_number
	return
    }

    set find [lsearch $UnpackedData $type_number]
    if {$find != -1} {

	#puts "This $type_name $instance_number component exists already"
	return
    } 

   
   set find [lsearch $PackedData $type_number]

#   puts "Find: $find"

    if {$find == -1} {
	set w .win_$type_number
	if {[winfo exists $w] == 0} {

	    set ComponentView($type_name,$instance_number,cont) 1
	    set ComponentView($type_name,$instance_number,disc) 0
	    lappend PackedData $type_number  

	    toplevel .win_$type_number -bg ivory1 -height 300 -width 250

	    wm title .win_$type_number "$type_name $instance_number variables"
	    lappend OpenWindows ".win_$type_number"

	    frame $w.bar -relief raised -bd 2 -class menu
	    frame $w.middle 

	    #	pack .win_$type_number.bar.control \
		    -side left
	    
	    #place .win_$type_number.bar -relwidth 1 -height 40
	    #place .win_$type_number.middle -relwidth 1 -relheight 1

	    ######### This is menubar code

	    menubutton .win_$type_number.bar.control -text "Control" -menu\
		    .win_$type_number.bar.control.menu1
	    #menubutton .win_$type_number.bar.info -text "Information" -menu\
		    .win_$type_number.bar.info.menu2
	    #menubutton .win_$type_number.bar.help -text "Help" -menu \
		    .win_$type_number.bar.help.menu3
	    wm protocol .win_$type_number WM_DELETE_WINDOW "DestroyInstanceWindow $type_name $instance_number"

	    set menu1 [menu .win_$type_number.bar.control.menu1 -tearoff 0]
	    #set menu2 [menu .win_$type_number.bar.info.menu2 -tearoff 0]
	    #set menu3 [menu .win_$type_number.bar.help.menu3 -tearoff 0]

	    $menu1 add checkbutton -label "Show Continuous"  \
		    -variable ComponentView($type_name,$instance_number,cont)\
		    -command "ChangeContWindow $w $type_name $instance_number"
	    $menu1 add checkbutton -label "Show Discrete"  \
		    -variable ComponentView($type_name,$instance_number,disc)\
		    -command "ChangeDiscWindow $w $type_name $instance_number"
	    #$menu1 add separator
	    #$menu1 add command -label "Close"  \
		    -command "DestroyInstanceWindow $type_name $instance_number"
	    
	    #    $menu1 add command -label "Configure"  \
		    -command {puts "Not implemented"} -state disabled
	    #    $menu2 add command -label "Show Type Information" \
		    #	    -command {puts "Not implemented"} -state disabled
	    #    $menu2 add command -label "Show Parent Information" \
		    #	    -command {puts "Not implemented"} -state disabled
	    #    $menu2 add command -label "Show Children Information" \
		    #	    -command {puts "Not implemented"} -state disabled
	    #    $menu3 add command -label "Help"  \
		    #	    -command {puts "Not implemented"} -state disabled
	    ## end of menubar code

	    #pack .win_$type_number.bar.help -side right

	    set t .win_$type_number.middle
	    set lb [listbox $t.list \
		    -yscrollcommand "$t.scroll set"\
		    -xscrollcommand "$t.xscroll set" -font $Fonts(SmallFont)]
	    scrollbar $t.scroll -command [list $lb yview]
	    scrollbar $t.xscroll -orient horizontal -command "$lb xview"
	    

	    set text [text $t.text \
		    -yscrollcommand "$t.tscroll set"\
		    -font $Fonts(SmallFont) -width 50 -height 10]
	    scrollbar $t.tscroll -command [list $text yview]


	    foreach i $data_list {
		$lb insert end $i
	    }

	    grid $w.bar -sticky ew
	    grid $w.middle -sticky snew
	    
	    grid rowconfigure $w 0 -weight 0
	    grid rowconfigure $w 1 -weight 1 -minsize 250
	    grid columnconfigure $w 0 -minsize 300 -weight 1
	    grid $w.bar.control -sticky w
	    grid columnconfigure $w.bar 0 -weight 1

	    #pack .win_$type_number.middle.xscroll -side bottom -fill x
	    #pack .win_$type_number.middle.scroll -side right -fill y
	    #pack .win_$type_number.middle.list -side left -fill both\
		    -expand true

	    grid $w.middle.xscroll -row 1 -column 0 -sticky ews
	    grid $w.middle.scroll -row 0 -column 0 -sticky nse
	    grid $w.middle.list -row 0 -column 0 -sticky news
	    
	    grid rowconfigure $w.middle 0 -weight 1
	    grid rowconfigure $w.middle 1 -weight 0
	    grid rowconfigure $w.middle 2 -weight 0
	    #	grid rowconfigure $w.middle 2 

	    grid columnconfigure $w.middle 0 -weight 1 -minsize 300
	} else {
	    .win_$type_number.middle.list delete 0 end
	    foreach i $data_list {
		.win_$type_number.middle.list insert end $i
	    }
	    update idletasks
	}
	
    } else {
	set spot [lindex [.win_$type_number.middle.list yview] 0]
	.win_$type_number.middle.list delete 0 end
	foreach i $data_list {
	    .win_$type_number.middle.list insert end $i
	}
	.win_$type_number.middle.list yview moveto $spot
	update idletasks
    }
}



proc ChangeContWindow {w name instance} {

    global ComponentView


    if {$ComponentView($name,$instance,cont) == 1} {
	
	PrintInstance [list $name $instance]
	grid rowconfigure $w.middle 0 -weight 1 -minsize 50 
	grid columnconfigure $w.middle 0 -weight 1 -minsize 300
	grid $w.middle.list -row 0 -column 0 -sticky news
	grid $w.middle.scroll -row 0 -column 0 -sticky nes
	grid $w.middle.xscroll -row 1 -column 0 -sticky news
	
    } else {
	if {$ComponentView($name,$instance,disc) == 0} {
	    DestroyInstanceWindow $name $instance
	    return
	}
	Dequeue $name $instance
	grid rowconfigure $w.middle 0 -weight 0 -minsize 0
	grid rowconfigure $w.middle 1 -weight 0 -minsize 0
	grid forget $w.middle.list
	grid forget $w.middle.scroll
	grid forget $w.middle.xscroll
    }
}


proc ChangeDiscWindow {w name instance} {
    global ComponentView
    
    if {$ComponentView($name,$instance,disc) == 1} {

	CallComponentTrace $name $instance

	grid rowconfigure $w.middle 2 -weight 1 -minsize 50 
	grid columnconfigure $w.middle 0 -weight 1 -minsize 300
	grid $w.middle.tscroll -row 2 -column 0 -sticky nse
	grid $w.middle.text -row 2 -column 0 -sticky news

	
    } else {
	if {$ComponentView($name,$instance,cont) == 0} {
	    DestroyInstanceWindow $name $instance
	    return
	}
	
	CallClearComponentTrace $name $instance
	grid forget $w.middle.text
	grid forget $w.middle.tscroll
	grid rowconfigure $w.middle 2 -weight 0 -minsize 0
    }
}

