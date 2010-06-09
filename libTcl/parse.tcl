# -*- Mode: Tcl -*-

# parse.tcl --

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
# This procedure gets the names of all the global types in the program
#
#
#############################################################################

##########################################################################
#
#
#
#
##########################################################################

proc ProcessCanvasData {str number} {

    global CanvasXYList

    set CanvasXYList {}
    set ValuList {}
    set counter $number
    
    while {[llength $str] > 1} {
	set ValuList {}
	while {$counter != 0} {
	    #puts $str
	    set item1 [lindex $str 0]
#	    puts "Item 1 is $item1"
	    set value1 [lindex $item1 1]
	    lappend ValuList $value1
	    
	    set str [lrange $str 1 end]
	    incr counter -1
	}
	set IDitem [lindex $str 0]
	set str [lrange $str 1 end]
	lappend ValuList $IDitem
	set counter $number
	lappend CanvasXYList $ValuList
    }
}

##########################################################################
#
#
#
#
##########################################################################

# commented by duke 8/14/97 
# the changes that is made to this function is simple but important. 
# gotinstance and recieved_instance is used by other function to wait 
# for the simulation to send the list of the instances.  

proc GetInstances {type instanceString} {

    global InstanceList InstanceTypeList gotinstance received_instance

#Get the names of the types 

    set line $instanceString
#    puts "Line is: $line"
    set InstanceTypeList($type) {}

    while {[llength $line] > 0} {
	set item [lindex $line 0]
#	puts "Item $item"
	set name [lindex $item 0]
	set index [lindex $item 1]
        lappend InstanceTypeList($type)  [list $name $index]
        lappend InstanceList [list $name $index]
        set line [lrange $line 1 end]
    }

    set gotinstance 1
    set received_instance($type) 1
}

##########################################################################
#
#
#
#
##########################################################################

proc ProcessVariables {number variableString} {

    global yVector GraphedElement TimeClick

    set line $variableString 
    set l1 [string first "\{" $line]
    set l2 [string first ";" $line]
    set info [string range $line [expr $l1 +1] [expr $l2 -1]]
	
    set line2 [string range $line [expr $l2 +1] end]
    set line $line2
    set uno " "

    while {$line != "STUFF"} {
	set is_set 0
	set l1 [string first "\{" $line]

	set l2 [string first "\}" $line]
	
	if {[string index $line [expr $l2 +1]] == "\}"} {
	    set l2 [expr $l2 +1]
	    set is_set 1

	}

	set var_string [string range $line [expr $l1 +1] [expr $l2 - 1]]
	
	set l3 [string first " " $var_string]
	set variable_name [string range $var_string 0 [expr $l3 -1]]

#	set var_valu [string range $var_string [expr $l3 +1] [expr $l3 +4]]

	if {$is_set == 1} {
	    set len [string length $var_string]
	    if {$len > 35} {
		set l3 [string first " " $var_string]
		set variable_name [string range $var_string 0 [expr $l3 -1]]

		set paren1 [string last "\{" $var_string]
		set vars_in_set_str [string range $var_string \
			[expr $paren1 +1]  end]
		set sp1 [string first " " $vars_in_set_str]
		set new_set_str [string range $vars_in_set_str \
			0 $sp1]
		set nu_string [format "(%s (%s ...))" $variable_name \
			$new_set_str]
#		puts "$nu_string"
	    } else {
	    set nu_string $var_string
	    }
	} else {
	    set nu_string [ProcessString $var_string]
	}
	lappend Data [list $nu_string]

	set line2 [string range $line [expr $l2+1] end]
	set line $line2
	    
	set l1 [string first "\}" $line]
	if {$l1 < 5} {
	    set line "STUFF"
	}
    }
return $Data    
}

##########################################################################
#
#
#
#
##########################################################################

proc ProcessGlobalLine {line} {

    set var_list {}
    set l1 [string first "\{" $line]
    set line [string range $line [expr $l1 +1] end]
    
    while {[llength $line] > 1} {
	set item [lindex $line 0]
	set itemName [lindex $item 0]
	set itemValue [lrange $item 1 end]
	set line [lrange $line 1 end]

	lappend var_list [list $itemName $itemValue]
    }
#    puts "Returning $var_list"
    return $var_list
}

#############################################################################
##
##
#############################################################################


proc FillVectorInfo {variableString} {

    global totalGraphs VectorInfoArray
    global GraphedElement TimeClick VectorList

    set counter 0
    set all_vector_data [lindex $variableString 0]
    
    foreach new_vector_info $all_vector_data {
	set vector_data [lindex $VectorList $counter]
	set var [lindex $vector_data 0]
	set vect_number [lindex $vector_data 1]
	set vect_axis [lindex $vector_data 2]
	
	set variable_ID [lindex $new_vector_info 0]
	set variable_name [lindex [lindex $new_vector_info 1] 0]
	set variable_value [lindex [lindex $new_vector_info 1] 1]

	#puts "I have info =>$variable_name $variable_value<= that I\
	#	am trying to match with =>$vector_data"

	## I am not sure why this next line is here, seems like 
	## we are guaranteed that this will be true
	if {$variable_ID == $vect_number} {
	    AssignValu $var $vect_number $vect_axis $variable_value
	} else {
	    ## This means that the value that the back end has sent is
	    ## no longer in the vectors list. This means that the graph
	    ## was closed but the simulation is still sending the data
	    ## until that data is de-queued. 
	}
	incr counter
    }
}

#############################################################################
##
##
#############################################################################


proc ParseTypesData {data} {

    global TypesArray TypeList InstanceTypeList TypeModesArray


    set line $data
    
    while {[llength $line] > 0} {
	set item [lindex $line 0]
	set type [lindex $item 0]
	set variables [lindex $item 1]
	set modes [lindex $item 2]
	set events [lindex $item 3]

#	set l [string first "\{" $line]
#	set line [string range $line [expr $l +1] end]
#	set l [string first "\{" $line]
#	set type [string range $line 0 [expr $l - 1]]
#	set l2 [string first "*" $line]
	lappend TypeList $type
	set InstanceTypeList($type) {}

	ParseVariables $type $variables
	ParseModes $type $modes
	ParseEvents $type $events
	    
	#	set line2 [string range $line [expr $l +1] [expr $l2 -2]]
	#ParseVariables $type $line2
        set line [lrange $line 1 end]
	#string range $line [expr $l2 +1] end]
    }
    set TypeList [lsort $TypeList]
}


##########################################################################
#
#
#
#
##########################################################################


proc ParseVariables {type vars} {

    global TypesArray

    set TypesArray($type) {}
    set line $vars

    while {[llength $line] > 0} {
	set item [lindex $line 0]
	set name [lindex $item 0]
	set var_type [lindex $item 1]
	#[string range $line 0 [expr $l -1]]
	#set l2 [string first ":" $line]
	#	set var_type [string range $line [expr $l +1] [expr $l2 -1]]
	if {$var_type == 0} {
	    set var_type number
	} elseif {$var_type == 1} {
	    set var_type symbol
	} elseif {$var_type == 2} {
	    set var_type logical
	} elseif {$var_type == 3} {
	    set var_type array
	} elseif {$var_type == 4} {
	    set var_type set
	} else  {
	    set var_type $var_type
	}
	set line [lrange $line 1 end]
#	set line [string range $line [expr $l2 +1] end]
	lappend TypesArray($type) [list $name $var_type]
    }
}


## This procedure takes a type and a variable name as arguments.
## It will return 1 of the type has that variable, 0 otherwise.

proc VariableExists {type var} {

    global TypesArray

    set vars $TypesArray($type)

    foreach item $vars {
	set var_name [lindex $item 0]
	if {$var_name == $var} {
	    return 1
	}
    }
return 0
}


#############################3
###########################

proc ParseModes {type modes} {

    global TypeModesArray

    set TypeModesArray($type) {}
    set line $modes

    while {[llength $line] > 0} {
	set name [lindex $line 0]
	set line [lrange $line 1 end]
#	set line [string range $line [expr $l2 +1] end]
	lappend TypeModesArray($type) $name
    }
}

proc ParseEvents {type events} {

    global TypeEventsArray

    set TypeEventsArray($type) {}
    set line $events

    while {[llength $line] > 0} {
	set name [lindex $line 0]
	set line [lrange $line 1 end]
#	set line [string range $line [expr $l2 +1] end]
#	puts "Event $name"
	lappend TypeEventsArray($type) $name
    }
}

############################################################################
#
#
#
############################################################################


proc ProcessString {var} {

    set length [string length $var]
    set str $var
    set result $var
    set l1 [string first " " $var]
    if {$l1 != -1} {

	set str1 [string range $str 0 [expr $l1 -1]]
	set str2 [string range $str [expr $l1 +1] end]
        set space ""
	set length [string length $str1]
	if {$length >= 10} {
	    set tempstr1 [string range $str1 0 9]
	    set str1 $tempstr1
	} else {
	    set spaces [expr 10 - $length]
	    while {$spaces > 0} {
		set uno " "
		set space1 $space$uno
		set space $space1
		incr spaces -1
		
	    }
	    set str1 "$str1$space"
	}
	set length [string length $str2]
	if {$length > 11} {
	    set str2 [string range $str2 0 10]
	}
	set result "$str1   $str2"
	set length [string length $result]
    }
    return $result
    
    
}


proc getVariableFromString {str} {

    if {$str == ""} {
	return Null
    } 
    
    set l [string first "(" $str]
    set variable [string range $str 0 [expr $l -1]]
    return $variable
}

proc getInstanceFromString {str} {
    
    if {$str == ""} {
	return 0
    }
    set l [string first "(" $str]
    set len [string length $str]
    set instance_string [string range $str [expr $l +1] [expr $len -2]]
    set instance [lindex $instance_string 1]
    
    return $instance

}

proc getTypeFromString {str} {

    if {$str == ""} {
	return Null
    }

    set l [string first "(" $str]
    set len [string length $str]
    set instance_string [string range $str [expr $l +1] [expr $len -1]]
    set type [lindex $instance_string 0]
    return $type
}
