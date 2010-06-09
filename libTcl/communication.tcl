# -*- Mode: Tcl -*-

# communications.tcl --

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



set BytesTransmitted 0
set TenK 0
proc Execute1 {} {

    global channel1 channel2 messageQueue BytesTransmitted TenK 

    if {[info exists channel1] == 0} {
	update idletasks
	catch {trace vdelete channel1 w Execute1}
	return
    }
    if {[fblocked $channel1] == 0} { 
	
	set number [read $channel1 6]
	if {$number == ""} {
	    close $channel1
	    catch {trace vdelete channel1 w Execute1}
	    #catch {unset channel1}
	    tk_dialog .info Info "The client-server connection has\
		    been broken. Exiting." info 0 OK
	    update idletasks
	    return
	}
	#set num [stripSpaces $number]
	#incr BytesTransmitted $num
	#if {$BytesTransmitted >= 10000} {
	    #set BytesTransmitted 0
	    #incr TenK
	#}
	set line [read $channel1 [expr $number +1]]
	#puts "Line is $line"
	ProcessData $line
    }
}

proc Execute2 {name1 name2 op} {

    global channel2 messageQueue Start record 

    # the flow control should be while instead of if since now we are adding
    # sometimes multiple items to messageQueue 

    while {[llength $messageQueue] > 0} {
	
	set message [lindex $messageQueue 0]
	
	if {[llength $messageQueue] > 1} {
	    set messageQueue [lrange $messageQueue 1 end]
	} else {
	    set messageQueue {}
	}
	#puts "Gonna send $message"
	lappend record $message 
	puts -nonewline $channel2 $message
	flush $channel2
	update idletasks
    }
}


proc padMessage {str} {

    set l [string length $str]
    set enlarge [expr 128 - $l]
    set counter 0
    set line $str
    while {$counter < $enlarge} {
	set space "!"
	set line2 $line$space
	set line $line2
	incr counter
    }
    set l [string length $line]
    # puts "The length of the string is now $l"
    # puts "Line $line"

    return $line
}


#############################################################################
#
#    This is helper function for C_ProcessData of communication.c it basically 
#    updates the vector 
#
#############################################################################

proc UpdateTimeVectorsPlease {time_elapsed} {
    global TimeVectors 
    foreach i $TimeVectors {
	set str [format "global $i"]
	eval $str
	set str2 [format "set %s(++end) %s" $i $time_elapsed]
	eval $str2
    }
}

#############################################################################
#
#   This was just a prototype which should be integrated into communcation.c
#   it is not used anymore. . . it used to check the pending_script and 
#   execute it if necessary
#
#############################################################################

proc CheckForPendingScriptPlease {} {

    global TimeElapsed pending_scripts script_windows 
    
    if {[array exists pending_scripts]} {

	foreach script [array name pending_scripts] {

	    set time [lindex $pending_scripts($script) 0]
	    
	    if {$time < $TimeElapsed} {
		
		set end_time [lindex $pending_scripts($script) 1]
		set type [lindex $pending_scripts($script) 2]
		set number [lindex $pending_scripts($script) 3]
		set data [lindex $pending_scripts($script) 4]

		unset pending_scripts($script)
		
		if {$type == "Animation"} {
		    set script_windows(animations,$number) \
			    [list $end_time [eval CreateCanvas $data]]
		} elseif {$type == "Graph"} {
		    set w [CreateNewGraph]
		    set script_windows(graphs,$number) \
			    [list $end_time $w]
		    dispatchGraphPlots $w $data 
		} elseif {$type == "Trace"} {
		    # not implemented yet 
		} elseif {$type == "Break"} {
		    # not implemented yet 
		}
	    }
	}
    }
    
    if {[array exists script_windows]} {
	foreach window [array name script_windows] {

	    set time [lindex $script_windows($window) 0]
	    
	    if {$time < $TimeElapsed} {
		
		set win [lindex $script_windows($window) 1]
		unset script_windows($window)

		destroy $win 
	    }
	}
    }
}

# end of file -- communications.tcl --
