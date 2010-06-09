# -*- Mode: Tcl -*-

# widgets.tcl --

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

proc combobox {w {listproc {}} {cmdproc {}} args } {
    global tcl_platform tkPriv
    set tkPriv(relief) raised
    
    if [winfo exists $w] { error "window $w already exists" }
    frame $w

    ## Entry Widget, it gets all the args
    eval entry $w.e $args 
    pack $w.e -side left -fill x 
#-expand 1

    if {[info comm down_bm] == {}} {
	set down_bm {
	    #define dwnarrow.icn_width 15
	    #define dwnarrow.icn_height 15
	    static unsigned char dwnarrow.icn_bits[] = {
		0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07,
		0xe0, 0x07, 0xfc, 0x3f, 0xf8, 0x1f, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03,
		0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	    }
	    image create bitmap down_bm -data [set down_bm]
	    unset down_bm
	}

	## Button.  With the symbol font, it should be a down arrow
	button $w.b -image down_bm -padx 0 -pady 0
	
	bind $w.b <1> "combobox_drop $w [list $listproc];%W config -relief sunken"
	pack $w.b -side right -fill x
	
	## Removable List Box
	toplevel $w.btm -cursor arrow
	wm withdraw $w.btm
	wm overrideredirect $w.btm 1
	wm transient $w.btm [winfo toplevel $w]
	
	listbox $w.btm.lb -bd 2 -relief sunken -height 5 \
		-yscrollcommand "$w.btm.vs set" -selectmode single
	scrollbar $w.btm.vs -orient vertical -command "$w.btm.lb yview"
	pack $w.btm.vs -side right -fill y
	pack $w.btm.lb -side left -fill both -expand 1

#	bind $w.e <Double-1> "combobox_popup $w"
	bind $w.e <KeyPress-Tab> "
	set found 0; set tmp \[%W get]
	foreach item \[$w.btm.lb get 0 end] {
	    if {\[regexp ^\$tmp \$item]} { incr found; set match \$item }
	}
	if {\$found == 1} {
	    $w.e delete 0 end; $w.e insert end \$match
	} elseif {\$found} {bell}
	break
	"

#	bind $w.e <Escape> "combobox_popup $w"
#	bind $w.btm.lb <Escape> "combobox_popup $w"
	

	set tagname [winfo name $w]_cbox
	foreach q "$w.btm $w.btm.lb $w.btm.vs" {
	    bindtags $q [concat ${tagname} [bindtags $q]]
	}

	bind $w.btm.lb <Motion> "
	set tkPriv(y) %y
	tkListboxMotion %W \[%W index @%x,%y]
	combobox_export $w.btm.lb $w.e
	"

	bind $w.btm.lb <Leave> "
	set tkPriv(x) %x
	set tkPriv(y) %y
	combobox_export $w.btm.lb $w.e
	"
	bind $tagname <ButtonPress>  {
	    foreach q {rootx rooty width height} {
		set $q [winfo $q %W]
	    }
	    if {(%X < $rootx) || (%X > ($rootx+$width)) || (%Y < $rooty) || (($rooty+$height) < %Y)} {
		combobox_release %W
	    }
	}
	
	bind $tagname <ButtonRelease> "	$w.b config -relief raised"
	bind $tagname <space> {
	    combobox_release %W
	}
	bind $tagname <Return> {
	    combobox_release %W
	}

	proc combobox_export {l e} {
	    if {[set idx [$l curselection ]] != ""} {
		$e delete 0 end
		$e insert 0 [$l get $idx]
	    }
	}

	proc combobox_release {w} {
	    grab release $w
	    #           place forget $w
	    wm withdraw [winfo toplevel $w]
	}
	
#	proc _$w_ {args} "eval \"combobox_command $w \$args\""

	proc combobox_command {window cmd args} {
	    if {$cmd == "append"} {
		eval "$w.btm.lb insert end $args"
	    } elseif {$cmd == "get"} {
		$w.e get
	    } elseif {$cmd == "ecommand"} {
		bind $w.e <Return> "eval [concat $args];break"
	    } else {
		eval "$w.e $cmd $args"
	    }
	}

	proc combobox_drop {w listproc} {
	    if {[winfo ismapped ${w}.btm]} {
		grab release ${w}.btm
		wm withdraw ${w}.btm
	    } else {
		if {$listproc != ""} {
		    ${w}.btm.lb delete 0 end
		    set the_list [eval $listproc]
		    foreach item $the_list {
			$w.btm.lb insert end $item
		    }
		    #set s [$w.btm.lb size]
		    #puts "It is $s big"
		}
		set x1 [expr [winfo rootx ${w}.b]-[winfo reqwidth ${w}.btm.lb]]
                set y1 [expr [winfo rooty ${w}.b]+[winfo height ${w}.b]]
		#               place ${w}_f -in ${w} -x $x1 -y $y1
                wm geom ${w}.btm +$x1+$y1
                wm deiconify ${w}.btm
                raise ${w}.btm
                focus ${w}.btm.lb
                update
                catch {grab -global ${w}.btm}
		combobox_config $w.btm.lb $w.btm.vs
            }
        }
	
        proc combobox_double {cmdproc w y} {
            grab release $w.btm
            $w.e delete 0 end
            $w.e insert 0 [$w.btm.lb get [$w.btm.lb nearest $y]]
            focus $w.e
            combobox_release $w.btm
            if {"$cmdproc" != {}} {
                eval $cmdproc $w
            }
        }
        bind $w.btm.lb <Double-1> "combobox_double [list $cmdproc] [list $w] %y"
        bind $w.btm.lb <1> "combobox_double [list $cmdproc] [list $w] %y"
        bind $w.btm.lb <ButtonRelease-1> "combobox_double [list $cmdproc] [list $w] %y"

        proc combobox_return {cmdproc window} {
            combobox_export $w.btm.lb $w.e
            focus $w.e
            combobox_release $w.btm
            if {"$cmdproc" != {}} {
                eval $cmdproc $w
            }
        }
        bind $w.btm.lb <Return> "combobox_return [list $cmdproc] [list $w]"

        bind $w.e <Down> "combobox_drop $w [list $listproc]"

        bind $w <Configure> "
	if \{\[winfo ismapped $w.btm\]\} \{
	    combobox_drop $w [list $listproc]
	    \}
	    "

	    if {$listproc != ""} {
		$w.btm.lb delete 0 end
		foreach q [eval $listproc] {
		    $w.btm.lb insert end $q
		}
	   }

	    if {$args != ""} {
		foreach q $args {
		    $w.btm.lb insert end $q
		}
	    }
	    bind $w.btm.lb <Configure> "combobox_config $w.btm.lb $w.btm.vs"
	return $w
}

proc combobox_config {listbox scrollbar} {
    set items [$listbox index end] 
    set size [$listbox cget -height] 
    if {$items <= $size} { 
	if [winfo exists $scrollbar] {
	    pack forget $scrollbar
	}
        $listbox configure -height $items 
    } else {
        pack $scrollbar -side right -fill y 
        $listbox configure -height 5
    }
}


proc selection_box {w labelText args} {

    if [winfo exists $w] { error "window $w already exists" }
    #pack $w.btm.vs -side right -fill y
    frame $w -class list

    ## Entry Widget, it gets all the args
    ## List box

    label $w.lab -text $labelText 
    pack $w.lab -side top -anchor nw
    frame $w.btm
    listbox $w.btm.list -bd 2 -relief sunken -width 5 -height 7 \
	    -yscrollcommand "$w.btm.vs set" -selectmode single\
	    -background gray
    scrollbar $w.btm.vs -orient vertical -command "$w.btm.list yview"
    pack $w.btm -side top -fill both -expand 1
    pack $w.btm.vs -side right -fill y
    pack $w.btm.list -side left -fill both -expand 1
    eval entry $w.e $args 
    pack $w.e -side bottom -fill x -expand 1

    bind $w.btm.list <1> "
    if \[%W size] {
	$w.e delete 0 end
	$w.e insert end \[%W get \[%W nearest %y]]
    }  
    "
    return $w
}


# commented by duke 8/14/97 
# again caller_function is added to use this function more generally.  
# caller_function is passed down to InstanceListWidget and so on to 
# eventually keep track of the windows opened by the script main 
# panel.  We want to get rid of all the window opend by script main 
# panel, when exiting from it. 

proc InstanceListGitter {variable_type {current_list {}} {caller_function other} } {

    global InstanceListGitterVar startup_windows 

    InstanceListWidget $variable_type $current_list $caller_function   
    tkwait variable InstanceListGitterVar
    unset InstanceListGitterVar
    set list [returnInstanceListWidgetEntries .phase]
#    set list [InstanceListWidget $variable_type]
#    tkwait variable list
    destroy .phase
    #tkwait 
    #puts "Have list $list"
    return $list
}

proc InstanceListWidget {variable_type current_list {caller_function other}} {

    global TypeList TypesArray messageQueue InstanceTypeList
    global TempVar1 TempVar2 resultX resultY startup_windows 

    set win .phase
    if {[winfo exists $win] == 1} {
	catch {wm deiconify $win}
	catch {raise $win}	
	return
    } else {
	if {$caller_function == "AddGraph"} {
	    set current_window $win
	    catch {lappend startup_windows $current_window}
	} else {
	    set current_window ""
	}
    }
    
    toplevel .phase -height 500 -width 700 
    wm title .phase "Data Selection"
    #-bg ivory1
    tkwait visibility .phase
    grab .phase
    
    #frame .phase.top
    frame .phase.middle -relief raised -bd 2 -height 350
    frame .phase.set
    frame .phase.axis -relief raised -bd 2
    frame .phase.axis.left
    frame .phase.axis.right
    frame .phase.bottom

    set xType 0
    set yType 0
    set xVar 0
    set yVar 0
    set xInst 0
    set yInst 0
   
    place .phase.middle -in .phase -anchor nw -x 0 -y 0 -y 0\
	    -relwidth 1.0 -height 250
    place .phase.set -y 250 -relwidth 1.0 -height 50
    place .phase.axis -y 300 -relwidth 1.0 -height 150
    place .phase.bottom -y 450 -relwidth 1.0 -height 50

    selection_box .phase.middle.vars "Variables" -background grey
    selection_box .phase.middle.types "Simulation Types" -background grey
    selection_box .phase.middle.instances "Instances" -background grey

    foreach item $TypeList {
	.phase.middle.types.btm.list insert end $item
    }

    set curr_type [.phase.middle.types.btm.list get 0]
    set VarList $TypesArray($curr_type)
    set show_variables {}
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

    listbox .phase.axis.plots -yscrollcommand ".phase.axis.vs set"\
	    -selectmode single\
	    -background gray
    scrollbar .phase.axis.vs -orient vertical\
	    -command ".phase.axis.plots yview"

    if {[llength $current_list] > 0} {
	foreach i $current_list {
	    set new_lab [format "%s(%s %s)" [lindex $i 1]\
		    [lindex $i 0] [lindex $i 2]]
	    .phase.axis.plots insert end $new_lab
	}
    }
    bind .phase.middle.types.btm.list <1> "+SelectComboBoxType $variable_type"

    bind .phase.middle.types.e <Return> "SelectComboBoxType $variable_type"

    place .phase.middle.types -x 1 -y 1 -relwidth .3 -relx 0.025
    place .phase.middle.instances -relwidth .3 -relx 0.35 -x 1 -y 1
    place .phase.middle.vars -relwidth .3 -relx .675
#-relwidth .3 -relx 0.35 -x 1 -y 1

    button .phase.set.setX -text "Add Selection"\
	    -command {
	set type [.phase.middle.types.e get]
	set inst [lindex [.phase.middle.instances.e get] 1]
	set var [lindex [.phase.middle.vars.e get] 0]

	if {[CheckCompleteness $type $var $inst]== 1 } {
	    set new_lab [format "%s(%s %s)"\
		    $var $type $inst ]
	    .phase.axis.plot insert end $new_lab
	} else {
	    tk_dialog .error Error "You must enter a type, a variable, and an instance" error 0 OK
	}
    }
    
    button .phase.set.setY -text "Remove Selection"\
	    -command {
	set item [.phase.axis.plot curselection]
	if {$item == ""} {
	    tk_dialog .error Error "You must select and entry to remove." error 0 OK
	} else {
	.phase.axis.plot delete $item
	}
    }

    place .phase.set.setX  -relheight .7 -relwidth .3 -relx .1 -rely .15
    place .phase.set.setY  -relwidth .3 -relx .6 -relheight .7 -rely .15

    pack .phase.axis.plots -side left -fill both -expand 1
    pack .phase.axis.vs -side right -fill y

    button .phase.bottom.ok -text OK \
	    -command {set InstanceListGitterVar 1}
#stuff [returnInstanceListWidgetEntries .phase]; return $stuff
    
    button .phase.bottom.cancel -text "Cancel" -command "set InstanceListGitterVar 1"

    place .phase.bottom.ok -relheight .7 -relwidth .3 -relx .1 -rely .15
    place .phase.bottom.cancel -relwidth .3 -relx .6 -relheight .7 -rely .15

    wm protocol $win WM_DELETE_WINDOW "RemoveFromStartUpWindows $win; destroy $win"
}

proc returnInstanceListWidgetEntries {w} {

    set number [$w.axis.plots size]
    set itemList {}
    set counter 0
    while {$counter < $number} {
	lappend itemList [list [$w.axis.plots get $counter]]
	incr counter
    }
    grab release .phase
    #puts $itemList
    return $itemList
}


proc SelectComboBoxType {variable_type} {

    global messageQueue gotinstance InstanceTypeList 

    set VarList [ChangeVars2];
    set VarList [lindex $VarList 0];
    .phase.middle.vars.btm.list delete 0 end
    .phase.middle.vars.e delete 0 end
    .phase.middle.instances.e delete 0 end
    set show_variables {}

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
    .phase.middle.instances.btm.list delete 0 end
    set type [.phase.middle.types.e get]
    if {$type != "global"} {

	# commented by duke 8/14/97 
	# after sending request to the simulation server about the list of 
	# instance we need to wait for simulation server to send back the 
	# information.  We used to use function called after which waits 
	# for specific unit of time.  But we will be using vwait. 

	set gotinstance 0
	unset gotinstance 

	lappend messageQueue 5556;
	set msg [padMessage $type];
	lappend messageQueue $msg;

	vwait gotinstance 

	set List $InstanceTypeList([.phase.middle.types.e get]); 
	foreach item $List {
	    .phase.middle.instances.btm.list insert end $item
	}

    } else {
	set List [list [list global 0]]
	foreach item $List {
	    .phase.middle.instances.btm.list insert end $item
	}
    }
}

proc scrolled_canvas {c width height region} {

    frame $c
    canvas $c.canvas -width $width -height $height\
	    -xscrollcommand [list $c.xscroll set]\
	    -yscrollcommand [list $c.yscroll set]\
    	    -scrollregion $region

    scrollbar $c.xscroll -orient horizontal\
	    -command [list $c.canvas xview]
    scrollbar $c.yscroll -orient vertical\
	    -command [list $c.canvas yview]
    
    pack $c.xscroll -side bottom -fill x
    pack $c.yscroll -side right -fill y
    pack $c.canvas -side left -fill both -expand 1
    #update idletasks
    return $c.canvas
}


proc scrolled_listbox {w labelText } {

    frame $w
    label $w.lab -text $labelText 
    pack $w.lab -side top -anchor nw
    frame $w.list
    listbox $w.list.list -bd 2 -relief sunken \
	    -yscrollcommand "$w.list.vs set" -selectmode single\
	    -background gray
    scrollbar $w.list.vs -orient vertical -command "$w.list.list yview"
    pack $w.list -side top -fill both -expand 1
    pack $w.list.vs -side right -fill y
    pack $w.list.list -side left -fill both -expand 1
}









