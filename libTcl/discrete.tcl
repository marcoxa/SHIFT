# -*- Mode: Tcl -*-

# discrete.tcl --

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



proc discrete_break_window {w name data} {

    if {[winfo exists $w]} {
	catch {wm deiconify $w}
	catch {raise $w}
	displayBreakInformation $w.text.text $data
	#$w.text.text insert end $data
	$w.text.text see end
	return
    }
 
    toplevel $w 
#-height 300 -width 300
    wm title $w "Breakpoint information window"

    frame $w.main
	    #-height 250
    frame $w.bottom 
#-height 50
    frame $w.text
    text $w.text.text -bd 2 -relief sunken \
	    -yscrollcommand "$w.text.vs set"\
		    -background gray -height 10 -width 50
    scrollbar $w.text.vs -orient vertical -command "$w.text.text yview"


    pack $w.text -side top -fill both -expand 1
    pack $w.text.vs -side right -fill y	    
    pack $w.text.text -side left -fill both -expand 1
    $w.text.text tag configure CompTag -foreground red -underline 1
    
    $w.text.text tag bind CompTag <Enter> "$w.text.text configure\
	    -cursor hand2"
    $w.text.text tag bind CompTag <Leave> "$w.text.text configure -cursor xterm"

    #$w.text.text insert end $data    
    displayBreakInformation $w.text.text $data

    
    button $w.bottom.close -text Close -command "destroy $w"
    
    pack $w.bottom.close -side top

    pack $w.bottom -side top -fill x
#.break$BreakNumber -title BreakType -text $data
}





proc displayBreakInformation {w data} {


    #global
    #puts "Data is $data"
    set str "-------------------Break Point ------------\n"
    $w insert end $str
    foreach i $data {
	set time_str [lindex $i 0]
	set time [lindex $time_str 2]
	set string "Time before the breakpoint is $time\n"
	$w insert end $string
	set collective_str [lindex $i 1]
	set numb [lindex $collective_str 2]
	set string "Collective Transition ID: $numb\nInstances involved:\n"
	$w insert end $string
	set items [lindex $collective_str 3]
	foreach comp $items {
	    set comp_string "\t$comp\n"
	    set comp_tag [format "Tag%s_%s" [lindex $comp 0] [lindex\
		    $comp 1]]
	    
	    $w tag bind $comp_tag <Double-1> "PrintAnimationInformation\
		    $comp_tag"
	    $w insert end $comp_string [list CompTag $comp_tag]
	}
	set components_str [lindex $i 2]
	PrintDiscreteComponentInfo $w $components_str
    }
    #$data
    $w see end
}


proc PrintDiscreteComponentInfo {w data} {


    foreach item $data {

	set comp [lindex $item 0]
	set time [lindex $item 1]
	set trans [lindex $item 2]
	set trans_from [lindex $trans 1]
	set trans_to [lindex $trans 3]
	
	set local [lindex $item 3]
	set local_events [lindex $local 1]
	set external [lrange [lindex $item 4] 1 end]
	#puts "External $external"
	set external_events ""
	while {[llength $external] > 0} {
	    set this [lindex $external 0]
	    set var [lindex $this 0]
	    set event_name [lindex $this 1]
	    set var_name [lindex $this 2]
	    set var_number [lindex $this 3]
	    set extra [lindex $this 4]
	    set external_event [format "(%s %s):%s:%s$extra" $var_name\
		    $var_number $var $event_name]
	    if {[string length $external_events] > 1} {
		set external_events [format "%s, %s" $external_events\
			$external_event]
	    } else {
		set external_events $external_event
	    }
	    set external [lrange $external 1 end]
	}
	
	set comp_string "Instance:\n\t$comp\n"
	set time_string "Time:\n\t$time\n"
	set trans_string "Transition:\n\t$trans\n"
	set local_string "Local Events participating in transition:\n\t$local\n"
	set external_string "External Events participating in transition:\n\t$external\n"

	set comp_string [format "%s %s -> %s {%s%s}\n" $comp $trans_from\
		$trans_to $local_events $external_events]
	$w insert end $comp_string
    }
}


proc DisplayTraceTypeInformation {data} {


}



proc discrete_trace_window {w name data} {

    DisplayTraceInformation $w.text.text $data    
}

proc DisplayTraceInformation {w data} {

    #global
    #puts "Data is $data"

    set l [llength $data]
    foreach item $data {

	set time [lindex [lindex $item 0] 2]
	set trans_number [lindex [lindex $item 1] 2]
	#puts "We have time $time and transition $trans_number"
	set instances [lindex $item 2]
	foreach instance $instances {
	    set name [lindex [lindex $instance 0] 0]
	    set instance_number [lindex [lindex $instance 0] 1]
	    set type_number "$name$instance_number"
	    set window_name .win_${type_number}.middle.text
	    set transition [lindex $instance 2]
	    set t_from [lindex $transition 1]
	    set t_to [lindex $transition 3]
	    if {[llength [lindex $instance 3]] > 1} {
		set event [lindex [lindex $instance 3] 1]
	    } else {
		set event ""
	    }
	    set external [lrange [lindex $instance 4] 1 end]
	    #puts "External $external"
	    set external_events ""
	    while {[llength $external] > 0} {
		set this [lindex $external 0]
		set var [lindex $this 0]
		set event_name [lindex $this 1]
		set var_name [lindex $this 2]
		set var_number [lindex $this 3]
		set extra [lindex $this 4]
		set external_event [format "(%s %s):%s:%s$extra" $var_name\
			$var_number $var $event_name]
		if {[string length $external_events] > 1} {
		    set external_events [format "%s, %s" $external_events\
			    $external_event]
		} else {
		    set external_events $external_event
		}
		set external [lrange $external 1 end]
	    }
	    set event2 $external_events
	    if {$event2 != ""} {
		if {$event != ""} {
		    set both_events "$event, $event2"
		} else {
		    set both_events "$event2"
		}
	    } else {
		set both_events "$event"
	    }

	    set print_string [format "\[%s %s\]: %s -> %s {%s}\n" $time\
		    $trans_number $t_from $t_to $both_events]
	    #puts "Trying window name $window_name"
	    if {[winfo exists $window_name] == 1} {
		$window_name insert end "$print_string"
		$window_name see end
	    } else {
		#$w insert end "($name $instance_number) $print_string"
		#$w see end
	    }
	}

    }

}
