# -*- Mode: TCL -*-

# canvases.tcl

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



# commented by duke - Fri Aug 22 16:02:00 PDT 1997
# what used to be CreateCanvasInstance became CreateCanvasHelper, this is needed 
# because we had to separate 1) Creating Canvas and 2) Drawing into the canvas for 
# SmartAHS information.  This is because SmartAHS function requires communication.c
# (when CreateCanvasInstance is called from communication.c,  it will create conflict
# in terms of semaphore created by "trace messageQueue" command 

proc CreateCanvasInstance {canvas_number size name} {

    global CanvasStructure 
    
    CreateCanvasInstanceHelper $canvas_number $size $name 
    set draw_procedure $CanvasStructure($name,procedure)
    eval $draw_procedure [format ".canvas%s.animate.canvas" $canvas_number]
    puts "Used $draw_procedure"
}

proc CreateCanvasInstanceHelper {canvas_number size name} {
    global ZoomIndex DefaultMultiplier CanvasesArray
    global OpenWindows OtherFont CanvasStructure
    
    set height $CanvasStructure($name,height) 
    set width $CanvasStructure($name,width) 
    set region $CanvasStructure($name,region) 
    set w [format ".canvas%s" $canvas_number]
    toplevel $w -height $height -width $width
    wm title $w "Canvas#$canvas_number" 
    
    
    wm protocol $w WM_DELETE_WINDOW \
	    "DequeueFromOpenWindows $w; RemoveFromStartUpWindows $w; ExitCanvas [wm title $w];  destroy $w"
    lappend OpenWindows $w
    set ZoomIndex($canvas_number) $size
    frame $w.bar -class menu -relief raised -bd 2
    menubutton $w.bar.control -text "Animation Control" \
	    -menu $w.bar.control.menu 
    menubutton $w.bar.zoom -text "Zoom" -menu $w.bar.zoom.menu
    set m [menu $w.bar.control.menu -tearoff 0]
    set m2 [menu $w.bar.zoom.menu -tearoff 0]
    pack $w.bar -side top -fill x
    pack $w.bar.control $w.bar.zoom -side left
    
    #$m add command -label "Remove Element" -command \
	    "RemoveAnimationGroup [wm title $w]"
    $m add command -label "Close" -command \
	    "DequeueFromOpenWindows $w; RemoveFromStartUpWindows $w; ExitCanvas [wm title $w]; destroy $w"
    $m2 add command -label "Zoom In"  \
	    -command "ZoomIn $w $canvas_number"
    $m2 add command -label "Zoom Out" \
	    -command "ZoomOut $w $canvas_number"

    scrolled_canvas $w.animate $width $height $region
    pack $w.animate -fill both -expand 1

    set CanvasesArray($canvas_number) "$w.animate.canvas"
    #puts "Drew Canvas"
}

#############################################################################
##
##
##
#############################################################################
proc DrawBigCanvas {w} {

}

proc DrawCarmmaTrack {w} {
   
    set counter 0
    set road_length 10000
    set road_width 4


    $w create rectangle 0 0 [expr $road_length * 10]\
         [expr $road_width*10] -fill grey 
    $w create rectangle 0 40 [expr $road_length * 10]\
           [expr 2 *$road_width*10] -fill grey 

    set max [expr $road_length * 10]
    while {$counter < $max} {
	$w create line $counter 20 [expr $counter + 15] 20
	$w create line $counter 60 [expr $counter + 15]\
		60
	set counter [expr $counter + 50]
    }

    set counter 0

    while {$counter < $max} {
	$w create line $counter 100 $counter 110
	$w create text $counter 120 -text [expr $counter/ 10]
	set counter [expr $counter + 250]
    }
}


proc DrawOcean {w} {


    $w configure -background lightblue

}

proc DrawStraightRoad {w} {

    set zoom_index 1

    set length [expr 1000 * 1]
    set width [expr 50]
    
    $w create rectangle 0 -25 $length 25 -fill yellow -width 2

    for {set counter 0} {$counter < 1001} {incr counter 100} {
	set xSpot $counter
	set ySpot 50
	$w create line $xSpot 30 $xSpot 50 -fill black -arrow first
	$w create text $xSpot [expr $ySpot + 10] -text $counter -fill blue
    }
}


#############################################################################
##
##
##
#############################################################################

proc DrawBBC {w} {

    $w create line 0 0 500 0 -tags Line -width 2

    $w create line 0 0 0 500 -tags Line -width 2
    $w create line 0 500 500 500 -tags Line -width 2
    $w create line 500 0 500 500 -tags Line -width 2

    $w create line 180 0 180 210 -tags Line -width 2
    $w create line 320 0 320 210 -tags Line -width 2
    $w create oval 180 140 320 280 -tags Line -width 2
    $w create line 180 210 320 210 -tags Line -width 2

    $w create line 250 0 250 10 -tags Line -width 10
    $w create line 205 10 295 10 -width 3 -tags Hoop
    $w create line 250 10 250 15 -width 5 -tags Hoop
    $w create oval 235 15 265 45 -tags Ring   

}


#############################################################################
##
##
##
#############################################################################

proc DrawGeneric500 {w} {

    set width 500

    $w create line 0 0 0 $width -width 2 -fill black
    $w create line 0 0 $width 0 -width 2 -fill black

    $w create line 0 $width $width $width -width 2 -fill black
    $w create line $width 0 $width $width -width 2 -fill black
}


proc ZoomOut {windowName canvasNo} {

    global ZoomIndex
    

    set ZoomIndex($canvasNo) [expr $ZoomIndex($canvasNo)* 0.666]
    $windowName.animate.canvas scale all 0 0 .666 .666


    set arc_list [$windowName.animate.canvas find withtag ArcSection]
    #puts "List is $arc_list"
    foreach arc $arc_list {
	set width [$windowName.animate.canvas itemcget $arc -width]
	set new_width [expr $width * .666]
	$windowName.animate.canvas itemconfigure $arc -width $new_width
    }
    set x_pos [lindex [$windowName.animate.canvas xview] 0]
    set y_pos [lindex [$windowName.animate.canvas yview] 0]
    set coords [$windowName.animate.canvas cget -scrollregion]
    set x0 [lindex $coords 0]
    set y0 [lindex $coords 1]
    set x1 [lindex $coords 2]
    set y1 [lindex $coords 3]
    set x0 [expr $x0 * .666]
    set y0 [expr $y0 * .666]
    set x1 [expr $x1 * .666]
    set y1 [expr $y1 * .666]
    $windowName.animate.canvas configure -scrollregion [list $x0 $y0\
	    $x1 $y1]
    $windowName.animate.canvas xview moveto $x_pos
    $windowName.animate.canvas xview moveto $y_pos
    
}

proc ZoomIn {windowName canvasNo} {

    global ZoomIndex
    

    set ZoomIndex($canvasNo) [expr $ZoomIndex($canvasNo)* 1.5]
    $windowName.animate.canvas scale all 0 0 1.5 1.5

    set arc_list [$windowName.animate.canvas find withtag ArcSection]
    #puts "List is $arc_list"
    foreach arc $arc_list {
	set width [$windowName.animate.canvas itemcget $arc -width]
	set new_width [expr $width * 1.5]
	$windowName.animate.canvas itemconfigure $arc -width $new_width
    }

    set coords [$windowName.animate.canvas cget -scrollregion]
    set x0 [lindex $coords 0]
    set y0 [lindex $coords 1]
    set x1 [lindex $coords 2]
    set y1 [lindex $coords 3]
    set x0 [expr $x0 * 1.5]
    set y0 [expr $y0 * 1.5]
    set x1 [expr $x1 * 1.5]
    set y1 [expr $y1 * 1.5]
    $windowName.animate.canvas configure -scrollregion [list $x0 $y0\
	    $x1 $y1]
}




proc DrawRoad {w {zoom_index 1}} {

    $w create rectangle 0 0 [expr 300 / $zoom_index] [expr 24 / $zoom_index]; 
    $w create rectangle 0 [expr 24/$zoom_index] \
	    [expr 300/ $zoom_index] [expr 48 / $zoom_index];
    $w create rectangle 0 [expr 48/$zoom_index] \
	    [expr 300/ $zoom_index] [expr 72 / $zoom_index];
    $w create rectangle 0 [expr 72/$zoom_index] \
	    [expr 300/ $zoom_index] [expr 96 / $zoom_index];
    $w create rectangle  [expr 300 / $zoom_index] 0 \
	    [expr 1200/ $zoom_index] [expr 24 / $zoom_index];

#Create Lane 1 : 400 meter radius
    
    set multiplier [expr 6 / $zoom_index]
    set list1 [create_dots [expr 300 / $zoom_index] \
	    [expr 24 /$zoom_index] $multiplier 400 8];
    set coord_line [format "%s %s" [expr 300/$zoom_index]\
	    [expr 24/$zoom_index]]
    foreach i $list1 {
	set coord_line [format "%s %s" $coord_line $i]
    }
    eval $w create line $coord_line -smooth 1 -splinesteps 10 -tags Road

    set list1 {}
    set list1 [create_dots [expr 300 / $zoom_index] \
	    [expr 48/ $zoom_index] $multiplier 396 8]
    set coord_line [format "%s %s" [expr 300/$zoom_index]\
	    [expr 48/$zoom_index]]

    foreach i $list1 {
	set coord_line [format "%s %s" $coord_line $i]
    }
#    puts "Coords: $coord_line"
    eval $w create line $coord_line -smooth 1 -splinesteps 10 -tags Road


#Create Lane 2 : 300 meter radius

    set list1 {}
    set list1 [create_dots [expr 300 / $zoom_index] \
	    [expr 48/ $zoom_index] $multiplier 300 10]
    set coord_line ""
    foreach i $list1 {
	set coord_line [format "%s %s" $coord_line $i]
    }
#    puts "Coords: $coord_line"
    eval $w create line $coord_line -smooth 1 -splinesteps 10 -tags Road

#    $w create line 1200 265 1176 307

    set list1 {}
    set list1 [create_dots [expr 300 / $zoom_index] \
	    [expr 72/ $zoom_index] $multiplier 296 10]
   set coord_line [format "%s %s" [expr 300/$zoom_index]\
	   [expr 72/$zoom_index]] 
    foreach i $list1 {
	set coord_line [format "%s %s" $coord_line $i]
    }
    eval $w create line $coord_line -smooth 1 -splinesteps 10 -tags Road


#Create Lane 3 : 250 meter radius
    set list1 {}
    set list1 [create_dots [expr 300 / $zoom_index] \
	    [expr 72/ $zoom_index] $multiplier 250 12]

   set coord_line [format "%s %s" [expr 300/$zoom_index]\
	   [expr 72/$zoom_index]] 
    foreach i $list1 {
	set coord_line [format "%s %s" $coord_line $i]
    }
    eval $w create line $coord_line -smooth 1 -splinesteps 10 -tags Road

    set list1 {}
    set list1 [create_dots [expr 300 / $zoom_index] \
	    [expr 96/ $zoom_index] $multiplier 246 12]

   set coord_line [format "%s %s" [expr 300/$zoom_index]\
	   [expr 96/$zoom_index]] 
    foreach i $list1 {
	set coord_line [format "%s %s" $coord_line $i]
    }
    eval $w create line $coord_line -smooth 1 -splinesteps 10 -tags Road

#    $w create line 1276 231 1266 253
#    $w create line 1200 289 1188 310
#    $w create line 1182 358 1168 378



}



proc create_dots {x_offset y_offset zoom radius steps} {

    global list300 list296 list292 angles

    set angles { 0.087155743 0.9961947 \
	    0.12186934 0.99254615 0.17364818 0.98480775 0.22495105 0.97437006 \
	    0.25881905 0.96592583  0.30901699 0.95105652 0.35836795 0.93358043 \
	    0.40673664 0.91354546 0.4539905 0.89100652 0.5 0.8660254 \
	    0.54463904 0.83867057 0.58778525 0.80901699}

    set numbers {}

    set total [expr $steps * 2]
    set counter 0
    while {$counter < $total} {
	set i [lindex $angles $counter]
	set r0 [expr $i * $radius]
	lappend numbers $r0
	incr counter
    }


    set result_list {}

    set counter 0
    foreach i $numbers {

	set r1 [expr $i * $zoom]
	set r2 [expr round($r1)]
	
	if {$counter == 1} {
	    set r3 [expr $radius* $zoom - $r2 + $y_offset ]
	    lappend result_list $r3
	    set counter 0
	} else {
	    set r3 [expr $r2 + $x_offset]
	    lappend result_list $r3
	    incr counter
	}

    }
    return $result_list
}
  

# This is is the cos and sin of angles 90-54 down by 3

proc create_opposite_dots {x_offset y_offset zoom radius steps} {

    global list300 list296 list292 angles
   

    #list of angles 54 up by threes

    set angles {0.58778525 0.80901699 0.54463904 0.83867057 0.5 \
	    0.8660254 0.4539905 0.89100652 0.40673664 0.91354546 \
	    0.35836795 0.93358043 0.30901699 0.95105652 0.25881905 \
	    0.96592583 0.22495105 0.97437006 0.17364818 0.98480775 \
	    0.12186934 0.99254615 0.087155743 0.9961947}

    set numbers {}

    set total [expr $steps * 2]
    set counter 0
    while {$counter < $total} {
	set i [lindex $angles $counter]
	set r0 [expr $i * $radius]
	lappend numbers $r0
	incr counter
    }

    set result_list {}

    set counter 0
    foreach i $numbers {

	set r1 [expr $i * $zoom]
	set r2 [expr round($r1)]
	
	if {$counter == 1} {
	    set r3 [expr $radius*6 - $r2 + $y_offset ]
	    lappend result_list $r3
	    set counter 0
	} else {
	    set r3 [expr $r2 + $x_offset]
	    lappend result_list $r3
	    incr counter
	}

    }
    return $result_list
}
 

proc DrawShop {w} {

    #puts "This is here"
    $w create rectangle 0 150 100 250 -fill #92d1c7
    $w create line 50 350 50 260 -arrow last
    $w create text 50 360 -text "Tray Generator"

    $w create rectangle 100 180 360 220 -fill #f5cdb7

    $w create line 230 350 230 240 -arrow last
    $w create text 230 360 -text "Conveyor Belt"

    $w create rectangle 360 150 400 250 -fill #888888

    $w create line 380 350 380 260 -arrow last
    $w create text 380 360 -text "Weighing Station"

    $w create polygon 400 180 600 120 600 220 400 220 -fill #92d1c7

    $w create line 500 350 500 260 -arrow last
    $w create text 500 360 -text "Tray Diverter"

    $w create rectangle 600 120 900 160 -fill  #f5cdb7
    

    $w create line 750 20 750 100 -arrow last
    $w create text 750 10 -text "Conveyor Belt 2: Bad trays"

    $w create rectangle 600 180 900 220 -fill  #f5cdb7

    $w create line 750 350 750 260 -arrow last 

    $w create text 750 360 -text "Conveyor Belt 3: Good trays"

    $w create rectangle 900 120 1000 160 -fill #fd661f -tags Trash
    $w lower Trash

    $w create line 950 20 950 100 -arrow last
    $w create text 950 10 -text "Dump"

    $w create rectangle 900 170 1000 230 -fill #f5430f

    $w create line 950 350 950 260 -arrow last
    $w create text 950 360 -text "Truck"
}   


proc DrawHouston {w} {
    
    global highwayLengths

    getHoustonData
    vwait highwayLengths(l5)

    set l1 $highwayLengths(l1)
    set l2 $highwayLengths(l2)
    set l3 $highwayLengths(l3)
    set l4 $highwayLengths(l4)
    set l5 $highwayLengths(l5)

    $w create rectangle 0 0 $l1 12 -fill grey

    $w create rectangle $l1 0 [expr $l1 + $l2] 12 -fill grey
    $w create rectangle [expr $l1 + $l2] 0 [expr $l1 + $l2 + $l3] 12 -fill grey
    $w create rectangle [expr $l1 + $l2 +$l3] 0 [expr $l1 + $l2 + $l3 +$l4] 12 -fill grey
    $w create rectangle [expr $l1 + $l2 + $l3 + $l4] 0 \
	    [expr $l1 + $l2 + $l3 + $l4 + $l5] 12 -fill grey

    $w create rectangle $l1 12 [expr $l1 + $l2] 24 -fill grey
    $w create rectangle [expr $l1 + $l2] 12 [expr $l1 + $l2 + $l3] 24 -fill grey
    $w create rectangle [expr $l1 + $l2 +$l3] 12 [expr $l1 + $l2\
	    + $l3 +$l4] 24 -fill grey
    $w create oval [expr $l1 + $l2 -3] 22 [expr $l1 + $l2 +3] 26 -fill black
    #This next line is so that it automatically trucates the extra 0es

    set next [expr $l1 + 0]
    $w create line $next 100 $next 50 -arrow last
    set next_N [expr $next / 3]
    $w create text $next 110 -text "x: $next_N"
    set next [expr $l1 + $l2]
    $w create line $next 100 $next 50 -arrow last
    set next_N [expr $next / 3]
    $w create text $next 110 -text "x: $next_N"
    set next [expr $l1 + $l2 + $l3]
    $w create line $next 100 $next 50 -arrow last
    set next_N [expr $next / 3]
    $w create text $next 110 -text "x: $next_N"
    set next [expr $l1 + $l2 + $l3 + $l4]
    $w create line $next 100 $next 50 -arrow last
    set next_N [expr $next / 3]
    $w create text $next 110 -text "x: $next_N"
    set next [expr $l1 + $l2 + $l3 + $l4 +$l5]
    $w create line $next 100 $next 50 -arrow last
    set next_N [expr $next / 3]
    $w create text $next 110 -text "x: $next_N"
}


proc getHoustonData {} {

    global messageQueue highwayLengths PRINT_DEBUG

    set type Section
    set variable length
    set instance1 0
    set instance2 2
    set instance3 3
    set instance4 4
    set instance5 6

    lappend messageQueue 5599
    
    set vars {0 2 3 4 5}
    foreach i $vars {
	set info [format "%s %s %s" $type $i $variable]
	if {$PRINT_DEBUG} {
	    puts "Info is :$info"
	}
	set msg [padMessage $info]
	lappend messageQueue $msg; 
    }
}
    
proc ProcessHoustonInfo {str} {

    global highwayLengths
    
#    puts $str
    set data [lindex [lindex $str 0] 0]
    if {$data == "S1"} {
	tk_dialog .info Information "This canvas was created for the car merge\
		scenario in SmartAHS. Because you are running a\
		different simulation, we will use default values\
		instead" info 0 OK 

    }
    set l1 [lindex [lindex $str 0] 1]
    set l2 [lindex [lindex $str 1] 1]
    set l3 [lindex [lindex $str 2] 1]
    set l4 [lindex [lindex $str 3] 1]
    set l5 [lindex [lindex $str 4] 1]
    
#    puts "$l1 $l2 $l3 $l4 $l5"
    set highwayLengths(l1) [expr $l1 * 3]
    set highwayLengths(l2) [expr $l2 * 3]
    set highwayLengths(l3) [expr $l3 * 3]
    set highwayLengths(l4) [expr $l4 * 3]
    set highwayLengths(l5) [expr $l5 * 3]
}


# This procedure sends information to the simulation requesting
# information about the SmartAHS road structure. This function asks
# for the information about all the sections and then about all of the
# segments in the simulation. This is the information that is then
# going to be used to draw the road on the canvas.


proc DrawSmartAHS {w} {

    global SmartAHSData_semaphore TypesArray AHSHighwaySectionData 
    global AHSHighwaySegmentData
    
    if {[info exists TypesArray(Section)] && [info exists\
	    TypesArray(Segment)]}  {

	catch {unset AHSHighwaySectionData}
	catch {unset AHSHighwaySegmentData}

	#puts "Drawing SmartAHS"
	getSmartAHSData
	#puts "Finished Drawing SmartAHS"
	vwait SmartAHSData_semaphore
	unset SmartAHSData_semaphore
	DrawAHSData $w

    } else {
	tk_dialog .info Information "This canvas was created \
		SmartAHS use. Because you are running a\
		different simulation, we will use a blank canvas instead" info 0 OK 
    }
}

proc getSmartAHSData {} {

    global messageQueue PRINT_DEBUG TypesArray

    ## PART 1 of the information gathering: Get Section DATA

	## Send message that says expect arguments to follow
	lappend messageQueue 5900
	## Need the section length information, and R is the code that
	# will be prepended to the message.
	set info "Section R length width"
	set msg [padMessage $info]
	lappend messageQueue $msg

    ## PART 2 of the information gathering: Get Segment DATA

	lappend messageQueue 5900
	## Need the section width information, and R is the code that
	# will be prepended to the message.
	set info "Segment R length gxa gya orientation curvature section"
	set msg [padMessage $info]
	lappend messageQueue $msg


    ## PART 3 of the information gathering: Get Lane DATA

    #lappend messageQueue 5900
	## Need the section width information, and R is the code that
	# will be prepended to the message.
    #set info "Lane R width"
    #nset msg [padMessage $info]
    #lappend messageQueue $msg


}
    
#############################################################################
#
#   DrawSmartAHSAfterInfo: 
#   This is variation of DrawSmartAHS to be used by communication.c. 
#   communication.c has to make sure that all the information is 
#   is here 
#
#############################################################################

proc DrawSmartAHSAfterInfo {w} {

    global SmartAHSData_semaphore TypesArray AHSHighwaySectionData 
    global AHSHighwaySegmentData
    
    if {[info exists TypesArray(Section)] && [info exists\
	    TypesArray(Segment)]}  {

	DrawAHSData $w

    } else {
	tk_dialog .info Information "This canvas was created \
		SmartAHS use. Because you are running a\
		different simulation, we will use a blank canvas instead" info 0 OK 
    }
}

proc ProcessSmartAHSInfo {str} {

    global highwayLengths
    
#    puts $str
    set data [lindex [lindex $str 0] 0]
    if {$data == "S1"} {
	tk_dialog .info Information "This canvas was created for the car merge\
		scenario in SmartAHS. Because you are running a\
		different simulation, we will use default values\
		instead" info 0 OK 

    }

}


# end of file -- canvases.tcl --

