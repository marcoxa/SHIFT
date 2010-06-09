# -*- Mode: Tcl -*-

# animation-create.tcl --

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
##
##  
##
##
##
#############################################################################

# commented by duke 8/14/97 
# a optional field called caller_function is added to differentiate when 
# this is called by script and non script.  However use of this field 
# can be very general.  What we are trying to do with adding this 
# extra field is not to prompt the use to decide to put multiple animations 
# together on the same canvas.  When running script, it is automatically 
# assumed that all animations run on separate canvase.  This could be 
# improved later 

proc CreateCanvas {CanvasName XVar YVar Instances Type image_Type\
	Multiplier State containerVariable containerType\
	containerInstance Length Width Orientation {frequency -1} \
	{caller_function unknown}} { 
 
    global totalCanvases GroupNumber  choice
    
    set choice -1 
    
    if {$caller_function != "RecordDecide"} {
	##IF there is an existing canvas like this one
	set canvas_type_exists [CanvasRequestExists $CanvasName]
	if {$canvas_type_exists == 1} {
	    SameCanvasPrompt $CanvasName
	    tkwait window .existing
	}
    } else {
	set canvas_type_exists 0 
    }	
    
    ## This first choice means we are going to create an new canvas
    if {$choice == 0 || $canvas_type_exists == 0} {
	##There is one more canvas in the simulation
	incr totalCanvases
	set canvas_number $totalCanvases
	#The group number for this canvas is 1. Any other things
	#animated on this canvas will have a greater group number
	set GroupNumber($canvas_number) 1
	if {1 == 2} {
	    puts "Arguments will be $canvas_number $CanvasName\
		    $Type $XVar $YVar $Instances\
		    $image_Type $Multiplier $State $containerVariable\
		    $containerType $containerInstance $Length $Width\
		    $Orientation $frequency "
	}
	ProcessAnimationCreationRequest $canvas_number $CanvasName\
		$Type $XVar $YVar $Instances $image_Type $Multiplier $State\
		$containerVariable $containerType $containerInstance\
		$Length $Width $Orientation $frequency 

	if {$caller_function == "RecordDecide"} {
	    if {$CanvasName != "SmartAHS"} {
		RequestCanvasCreation $CanvasName $canvas_number
	    } else {
		RequestCanvasCreationWithoutDrawingYet $CanvasName $canvas_number
	    }
	} else {
	    RequestCanvasCreation $CanvasName $canvas_number
	}
    } else {
	# The variable "choice" will hold the number of the canvas we
	# will be adding objects to
	
	set canvas_number $choice
	incr GroupNumber($canvas_number)
	ProcessAnimationCreationRequest $canvas_number $CanvasName\
		$Type $XVar $YVar $Instances $image_Type $Multiplier\
		$State $containerVariable $containerType\
		$containerInstance $Length $Width $Orientation\
		$frequency
    }

    # this function used not to return anything, but not the script function 
    # has to be aware of the windows that it has created.  

    return .canvas${canvas_number}
}

##################################################################
#
#
#
#
##################################################################

proc ProcessAnimationCreationRequest {canvas_number CanvasName Type
XVar YVar Instances image_Type Multiplier State containerVariable containerType
containerInstance Length Width Orientation frequency} {
    
    global messageQueue ImageLength
    global DefaultMultiplier CanvasImage ImageWidth
    global GroupNumber AnimatedGroups
    global CanvasAnimations AnimationRecallFile 
    global RecordingAnimationInfo AllAnimationData
    
    # Get the group number
    set group_number $GroupNumber($canvas_number)
    set CanvasImage($canvas_number,$group_number) $image_Type
    set ImageLength($canvas_number,$group_number) $Length
    set ImageWidth($canvas_number,$group_number) $Width
    set DefaultMultiplier($canvas_number,$group_number) \
	    $Multiplier
    set Number $Instances
    
    set XNumber ALL
    set YNumber ALL
    
    ## Need to insert some typechecking to make sure that the number of
    ## instances is the same in both categories
    
    set desired_state $State
    
    ## Now send the message to the simulation
    lappend messageQueue "8765"
    set message [format "%s %s %s %s %s %s %s %s %s %s" \
	    $Type $canvas_number $XVar $group_number $Number\
	    $containerType $containerVariable $containerInstance\
	    $desired_state $Orientation]
    #puts "Message is: $message"
    set msg [padMessage $message]
    lappend messageQueue $msg
    
    if {$frequency > 0} {
	set Number MORE
    }
    set message2 [format "%s %s %s %s %s %s %s %s %s %s" \
	    $Type $canvas_number $YVar $group_number $Number\
	    $containerType $containerVariable $containerInstance\
	    $desired_state $Orientation]
    
    set AllAnimationData($canvas_number,$group_number) [list $Type\
	    $XVar $YVar $Orientation $containerType $containerVariable\
	    $containerInstance $desired_state $Length $Width $frequency]
    
    #puts "Message2 is: $message"
    set msg2 [padMessage $message2]
    lappend messageQueue $msg2
    #	puts "other message send $msg2"
    
    ######
    if {$frequency > 0} {
	set message3 [format "%s %s %s" $Length $Width $frequency]
	set msg3 [padMessage $message3]
	lappend messageQueue $msg3
	#puts "Message 3: $msg3"
	set animation_type Cont
    } else {
	set animation_type Fixed
    }
    
    set AnimatedGroups($group_number) [list $Type $XVar $YVar \
	    $Number $animation_type $group_number]
    lappend CanvasAnimations($canvas_number) $group_number
}

proc RequestCanvasCreation {CanvasName CanvasNumber} {
    
    global ExistingCanvases CanvasTypeArray RecordingAnimationInfo
    global AnimationRecallFile
    
    if {$CanvasName == "BasketballCourt"} {
	CreateCanvasInstance $CanvasNumber 1 BasketballCourt
	lappend ExistingCanvases(BasketballCourt) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) BasketballCourt
    } elseif {$CanvasName == "StraightRoad"} {
	CreateCanvasInstance $CanvasNumber 1 StraightRoad
	lappend ExistingCanvases(StraightRoad) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) StraightRoad
    } elseif {$CanvasName == "SuperMerge"} {
	CreateCanvasInstance $CanvasNumber 1 SuperMerge
	lappend ExistingCanvases(SuperMerge) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) SuperMerge
    } elseif {$CanvasName == "DemoRoad"} {
	CreateCanvasInstance $CanvasNumber 1 DemoRoad
	lappend ExistingCanvases(DemoRoad) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) DemoRoad
    } elseif {$CanvasName == "DemoOcean"} {
	CreateCanvasInstance $CanvasNumber 1 DemoOcean
	lappend ExistingCanvases(DemoOcean) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) DemoOcean
    } elseif {$CanvasName == "ShopFloor"} {
	CreateCanvasInstance $CanvasNumber 1 ShopFloor
	lappend ExistingCanvases(ShopFloor) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) ShopFloor
    } elseif {$CanvasName == "Ocean"} {
	CreateCanvasInstance $CanvasNumber 1 Ocean
	lappend ExistingCanvases(Ocean) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) Ocean
    } elseif {$CanvasName == "CurvedRoad"} {
	CreateCanvasInstance $CanvasNumber 1 CurvedRoad
	lappend ExistingCanvases(CurvedRoad) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) CurvedRoad
    } elseif {$CanvasName == "BigCanvas"} {
	CreateCanvasInstance $CanvasNumber 1 BigCanvas
	lappend ExistingCanvases(BigCanvas) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) BigCanvas
    } elseif {$CanvasName == "CarmmaTrack"} {
	CreateCanvasInstance $CanvasNumber 1 CarmmaTrack
	lappend ExistingCanvases(CarmmaTrack) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) CarmmaTrack
    } elseif {$CanvasName == "SmartAHS"} {
	CreateCanvasInstance $CanvasNumber 1 SmartAHS
	lappend ExistingCanvases(SmartAHS) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) SmartAHS
    } elseif {$CanvasName == "Generic_500x500"} {
	CreateCanvasInstance $CanvasNumber 1 Generic_500x500
	lappend ExistingCanvases(Generic_500x500) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) Generic_500x500
    }
}



proc RequestCanvasCreationWithoutDrawingYet {CanvasName CanvasNumber} {
    
    global ExistingCanvases CanvasTypeArray RecordingAnimationInfo
    global AnimationRecallFile
    
    if {$CanvasName == "BasketballCourt"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 BasketballCourt
	lappend ExistingCanvases(BasketballCourt) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) BasketballCourt
    } elseif {$CanvasName == "StraightRoad"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 StraightRoad
	lappend ExistingCanvases(StraightRoad) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) StraightRoad
    } elseif {$CanvasName == "SuperMerge"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 SuperMerge
	lappend ExistingCanvases(SuperMerge) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) SuperMerge
    } elseif {$CanvasName == "DemoRoad"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 DemoRoad
	lappend ExistingCanvases(DemoRoad) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) DemoRoad
    } elseif {$CanvasName == "DemoOcean"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 DemoOcean
	lappend ExistingCanvases(DemoOcean) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) DemoOcean
    } elseif {$CanvasName == "ShopFloor"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 ShopFloor
	lappend ExistingCanvases(ShopFloor) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) ShopFloor
    } elseif {$CanvasName == "CurvedRoad"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 CurvedRoad
	lappend ExistingCanvases(CurvedRoad) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) CurvedRoad
    } elseif {$CanvasName == "BigCanvas"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 BigCanvas
	lappend ExistingCanvases(BigCanvas) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) BigCanvas
    } elseif {$CanvasName == "Ocean"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 Ocean
	lappend ExistingCanvases(Ocean) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) Ocean
    } elseif {$CanvasName == "CarmmaTrack"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 CarmmaTrack
	lappend ExistingCanvases(CarmmaTrack) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) CarmmaTrack
    } elseif {$CanvasName == "SmartAHS"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 SmartAHS
	lappend ExistingCanvases(SmartAHS) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) SmartAHS
    } elseif {$CanvasName == "Generic_500x500"} {
	CreateCanvasInstanceHelper $CanvasNumber 1 Generic_500x500
	lappend ExistingCanvases(Generic_500x500) $CanvasNumber
	set CanvasTypeArray($CanvasNumber) Generic_500x500
    }
}




