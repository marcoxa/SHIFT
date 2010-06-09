
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

proc PrintInstance {instance} {
    
    global messageQueue PrintInstance PackedData UnpackedData
	
#    puts "Informaiton passes: $instance"
    set type [lindex $instance 0]
    set instance_number [lindex $instance 1]
    set type_number $type$instance_number

    set find [lsearch $UnpackedData $type_number]
    if {$find != -1} {
	return
    } 
    set find [lsearch $PackedData $type_number]
    if {$find != -1} {
	catch {wm deiconify .win_$type_number}
	catch {raise .win_$type_number}	
	return
    } 

    set message "$type $instance_number"
    lappend messageQueue 5505
    set msg [padMessage $message]
    lappend messageQueue $msg
}

##########################################################################
# Print animation type
#
# This function is just like the one above, but for the canvas. These
# two
# functions should be rolled into a single more modular one
#
##########################################################################

proc PrintAnimationInformation {inst_str} {
    
    global messageQueue 
	
    set str [string range $inst_str 3 end]
    set l [string last "_" $str]
    set type [string range $str 0 [expr $l -1]]
    set instance [string range $str [expr $l + 1] end]
    set new_string [list $type $instance]
    PrintInstance $new_string
#    set message "$type $instance"
    #puts "Gonna send ->$message<-"
#    lappend messageQueue 5505
#    set msg [padMessage $message]
#    lappend messageQueue $msg
}



##########################################################################



############################################################################
##
##
############################################################################


proc DestroyInstanceWindow {type_name instance_number} {

    set type_number $type_name$instance_number

    Dequeue $type_name $instance_number
    DequeueFromOpenWindows .win_$type_number
    destroy .win_$type_number
}



proc Dequeue {typeName instance} {
    
    global messageQueue PackedData UnpackedData

    set number "$typeName$instance"
    set otherList {}
    foreach i $PackedData {
	if {$i != $number} {
	    lappend otherList $i
	}   
    }
    set PackedData $otherList
    lappend messageQueue "5777"
    set msg [padMessage "$typeName $instance"]
    lappend messageQueue $msg
    lappend UnpackedData $number
#    puts "Unpacked data is now $UnpackedData"
#    puts "Packed data is now $PackedData"
}

proc DequeueUnpacked {number} {
    
    global UnpackedData


    #puts "Old before message $number is: ==> $UnpackedData"
    set otherList {}
    set data {}
    set data $UnpackedData
    set UnpackedData {}

    foreach i $data {
	if {$i != $number} {
	    lappend otherList $i
	} 
    }

    set UnpackedData $otherList
    #puts "New: $UnpackedData"

}


#############################################################################
# Whenever a timeclick passes, the informaiton in the instaces box must be 
#updated to reflect the current info.  This removes the items in the listbox
#and inserts the new updated ones
#############################################################################


proc RePrint {type_number} {

    global PackedData Time SmallFont PrintInstance messageQueue

	
    lappend messageQueue $PrintInstance
    lappend messageQueue $type_number
	    
	set line [string range $line 1 end]

	set l1 [string first "\{" $line]
	set l2 [string first ";" $line]
	set info [string range $line [expr $l1 +1] [expr $l2 -1]]
	
	set line2 [string range $line [expr $l2 +1] end]
	set line $line2
	set uno " "
#	set astr [list "timeclick $uno$Time"]
#	set bstr [ProcessString $astr]
#	lappend Data $bstr
	while {$line != "STUFF"} {
	    set l1 [string first "\{" $line]
	    set l2 [string first "\}" $line]
	    set var_string [string range $line [expr $l1 +1] [expr $l2 - 1]]
	    set nu_string [ProcessString $var_string]
	    lappend Data [list $nu_string]
	    
	    set line2 [string range $line [expr $l2+1] end]
	    set line $line2
	    
	    set l1 [string first "\}" $line]
	    if {$l1 < 5} {
		set line "STUFF"
	    }
	}
	
	set counter 0
	foreach i $Data {
	    set str [lindex $i 0]
	    set str2 [lindex $i 1]
	    .instances.d.data.w$type_number.middle.box insert end $str
	}
    }


#############################################################################
# Creates the listbox in the main window, and creates the bindings and all
#other data related to this listbox
#
############################################################################# 

proc insert_types_list {} {

    global TypeList TypesBroken TypesTraced



    foreach i $TypeList {
	set TypesBroken($i) 0
	set TypesTraced($i) 0
	#trace variable TypesBroken($i) w CommunicateBreakTypes    
    }   
    ##May have to do a vwait after the lappend here
    ##The other option is to do a trace on the messageQueue array so that we 
    ## process events as soon as they are entered into the message list
    
}


proc CallComponentTrace {name instance} {

    global messageQueue

    
    lappend messageQueue 8005
    set msg "$name $instance"
    set msg [padMessage $msg]
    lappend messageQueue $msg
}

proc CallClearComponentTrace {name instance} {

    global messageQueue

    
    lappend messageQueue 8006
    set msg "$name $instance"
    set msg [padMessage $msg]
    lappend messageQueue $msg
}

