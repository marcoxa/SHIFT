# -*- Mode: Tcl -*-

# canvas-parameters.tcl --

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


### This function receives the AHS data and stores it into two arrays,
## the AHSHighwaySegmentData and the AHSHighwaySectionData arrays


proc StoreAHSData {type information} {


    global SmartAHSData_semaphore
    global AHSHighwaySegmentData AHSHighwaySectionData

    # If the type is Secton, I will store the length and the width in
    # AHSHighwaySection array. I know that the first item is the
    # length, and the second is the width

    if {$type == "Section"} {
	foreach section $information {
	    set section_number [lindex $section 0]
	    set values [lindex $section 1]
	    set length [lindex $values 0]
	    set width [lindex $values 1]
	    set AHSHighwaySectionData($section_number,length) $length
	    set AHSHighwaySectionData($section_number,width) $width
	}
	set info [array names AHSHighwaySectionData]
	#puts "Info stored is $info"
    } elseif {$type == "Segment"} {
	foreach segment $information {
	    
	    set segment_number [lindex $segment 0]
	    set values [lindex $segment 1]
	    set length [lindex $values 0]
	    set gxa [lindex $values 1]
	    set gya [lindex $values 2]
	    set orientation [lindex $values 3]
	    set curvature [lindex $values 4]
	    set section [lindex $values 5]	    
	    set AHSHighwaySegmentData($segment_number,length) $length
	    set AHSHighwaySegmentData($segment_number,gxa) $gxa
	    set AHSHighwaySegmentData($segment_number,gya) $gya
	    set AHSHighwaySegmentData($segment_number,orientation) $orientation
	    set AHSHighwaySegmentData($segment_number,curvature) $curvature
	    set AHSHighwaySegmentData($segment_number,section) $section
	}

	#set info [array names AHSHighwaySegmentData]
	#puts "Info stored is $info"
    set SmartAHSData_semaphore 1
    }
}
    


proc ProcessAHSData {w} {

    global AHSHighwaySegmentData AHSHighwaySectionData



}


proc DrawAHSData {w} {

    global AHSHighwaySegmentData AHSHighwaySectionData MultiDefault
    global Colors_Used

    set elements [expr [array size AHSHighwaySegmentData] / 6]
    #$w configure -background green
    #puts "Found $elements elements"

    
    ### This loop below creates a grid useful when debugging or
    ## visualizing a road. It is disabled now since the counter is set
    ## to start at the end of the loop.
    set other_counter 300 
    while {$other_counter < 300} {
	$w create line [expr $other_counter * 100] 27000 [expr $other_counter *\
		100] 30000
	$w create line 27000 [expr $other_counter * 100] 30000 [expr $other_counter *\
		100] 
	$w create text 27000 [expr $other_counter * 100] -text [expr\
		$other_counter* 100]
	$w create text [expr $other_counter * 100] 27000 -text [expr\
		$other_counter* 100]
	incr other_counter
    }
    set max_gxa -10000
    set max_gya -10000
    set min_gxa 100000
    set min_gya 100000

    set counter 0
    while {$counter < $elements} {
	set length $AHSHighwaySegmentData($counter,length)
	set gxa $AHSHighwaySegmentData($counter,gxa)
	set gya $AHSHighwaySegmentData($counter,gya)
	set orientation $AHSHighwaySegmentData($counter,orientation)
	set curvature $AHSHighwaySegmentData($counter,curvature)
	set section_str $AHSHighwaySegmentData($counter,section)
	set section_number_str [lindex $section_str 1]
	set l [string first ")" $section_number_str]
	set section_number [string range $section_number_str 0 [expr\
		$l -1]]

	set width $AHSHighwaySectionData($section_number,width)
	
	
	if {$gxa > $max_gxa} {
	    set max_gxa $gxa
	}
	if {$gya > $max_gya} {
	    set max_gya $gya
	}
	if {$gxa < $min_gxa} {
	    set min_gxa $gxa
	}
	if {$gya < $min_gya} {
	    set min_gya $gya
	}


	set multiplier $MultiDefault(SmartAHS)

	set start_x [expr $gxa * $multiplier]
	set start_y [expr $gya * $multiplier]
	set total_length [expr $length * $multiplier]
	set total_width [expr $width * $multiplier]

	set num [expr $counter % 9]
	set color [lindex $Colors_Used $num]

	#puts "Current geometric data: $length $gxa $gya $orientation\
	#	$curvature $width with color $color"
	
	if {$curvature == 0} { 
	    set end_x [expr $start_x + $total_length * cos($orientation)]
	    set end_y [expr $start_y + $total_length *\
		    sin($orientation)]
	    set start_x2 [expr $start_x - $total_width *\
		    sin($orientation)]
	    set start_y2 [expr $start_y + $total_width * cos($orientation)]

	    set end_x2 [expr $end_x - $total_width *\
		    sin($orientation)]
	    set end_y2 [expr $end_y + $total_width * cos($orientation)]
 
	    $w create polygon $start_x $start_y $end_x $end_y $end_x2\
		    $end_y2 $start_x2 $start_y2 -fill $color 

	} else { 
	    set data [getCircleData $start_x $start_y $curvature\
		    $orientation $total_width $total_length] 
	    set pt1 [lindex $data 0]
	    set pt2 [lindex $data 1]
	    set extent [lindex $data 2]
	    set start [lindex $data 3]

	    set tag "tag$counter" 
	    #puts "We have the box at $pt1 and $pt2 for segment $counter"
	    eval $w create arc $pt1 $pt2 -start $start -extent $extent\
		    -style arc -outline $color -width $total_width\
		    -tags ArcSection 
	    
	}
	#update  
	#update idletasks 
	incr counter
    } 

    ## Add a little cushioning to the sides of the scrolling 
    set cushioning 300 
    set min_gxa [expr $min_gxa - $cushioning] 
    set min_gya [expr $min_gya - $cushioning] 
    set max_gxa [expr $max_gxa + $cushioning] 
    set max_gya [expr $max_gya + $cushioning]

    set region "\{$min_gxa $min_gya $max_gxa $max_gya\}" 


    eval $w configure -scrollregion $region

}

proc getCircleData {gxp gyp curvature orientation width length} {

### Aleks added nradius since center of circle does not change    

    global MultiDefault

    set data {}
    set multiplier $MultiDefault(SmartAHS)

    set radius [expr 1 / $curvature]
    #set radius [expr $radius * $multiplier]
    if {$radius > 0.0} {	
	set corrected_angle [expr $orientation -(3.14159/2)]
	set nradius [expr $radius * $multiplier - $width / 2.0]
	set rad_x [expr $gxp - $radius * cos($corrected_angle)]
	set rad_y [expr $gyp - $radius * sin($corrected_angle)]
    } else {
	set corrected_angle [expr (3.14159/2) - $orientation]
	set nradius [expr $radius * $multiplier - $width / 2.0]
	set rad_x [expr $gxp - $radius * cos($corrected_angle)]
	set rad_y [expr $gyp + $radius * sin($corrected_angle)]
    }

    #set start_y [expr $gyp]


    set start_x [expr $rad_x - $nradius]
    set start_y [expr $rad_y - $nradius]
    set coords_start [list $start_x $start_y]
    
    set end_x [expr $rad_x + $nradius]
    set end_y [expr $rad_y + $nradius]

    set coords_end [list $end_x $end_y]

    set circumference [expr 2 * $radius * 3.14159]
    
    set this_section [expr $length / $circumference]
    
    set angle [expr $this_section *  -360.0]
    

    set start_angle [expr ($orientation / 6.28318) * 360.0]
    
    if {$radius > 0} {	
	set start_angle [expr 90 - $start_angle]

    } else {
	set start_angle [expr -90 - $start_angle]
    }
    

    set data [list $coords_start $coords_end $angle $start_angle]

    return $data
}


