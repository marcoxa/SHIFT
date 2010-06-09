# -*- Mode: Tcl -*-

# animate.tcl --

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
#############################################################################

proc findStates {} {

    global TypeModesArray


    if {[.menu.middle.left.typechoice.e get] == ""} {
	return {} 
    } else {
	set CanvasType [.menu.middle.left.typechoice.e get]
	if {[info exists TypeModesArray($CanvasType)] } {
	    set disc_states $TypeModesArray($CanvasType) 
	}
	set disc_states [linsert $disc_states 0 "All-States"]
	return $disc_states
    }
}

proc findInstances {} {

    global InstanceTypeList messageQueue AllDone
    
    set List {}
    if {[.menu.middle.left.typechoice.e get] == ""} {
	return {}
    } else {
	set type [.menu.middle.left.typechoice.e get]
	if {[info exists InstanceTypeList($type)]} {
	    if {$type != "global"} {
		lappend messageQueue 5556;
		set msg [padMessage $type];
		lappend messageQueue $msg;
		after 500 
		set List $InstanceTypeList($type)
		set instances [linsert $List 0 [list ALL]]
		#puts "List is $instances, list is $List"
		return $instances
		#tkwait variable AllDone
		#unset AllDone
	    } else {
		set List [list [list global 0]]
		return $instances
	    }
	} else {
	    bell 
	    tk_dialog .error Error "The type you have entered does not
	    exist. " error 0 OK
	}
    }
    
#    set instances [linsert $List 0 "ALL"]
#    return $instances
}


proc resetVariablesAndStates {} {

    global TypesArray
    set CanvasType [.menu.middle.left.typechoice.e get]
    if {$CanvasType == ""} {
	return {} 
    } else {
	if {[info exists TypeModesArray($CanvasType)] } {
	    set disc_states $TypeModesArray($CanvasType) 
	}
	set disc_states [linsert $disc_states 0 "All-States"]
	.menu.middle.left.state delete 0 end
	foreach state $disc_states {
	    .menu.middle.left.state insert end $state
	}
        set variables $TypesArray($CanvasType)
	.menu.middle.left.ychoice delete 0 end
	.menu.middle.left.xchoice delete 0 end
	foreach variable $variables {
	    .menu.middle.left.ychoice insert end $variable
	    .menu.middle.left.xchoice insert end $variable
	}
    }

}

proc findTypes {} {

    global TypeList

    return $TypeList

}


proc findVariables {} {

    global TypesArray

    if {[winfo exists .menu.middle.left.typechoice.e]} {
	if {[.menu.middle.left.typechoice.e get] == ""} {
	    return {} 
	} else {
	    set CanvasType [.menu.middle.left.typechoice.e get]
	    if {[info exists TypesArray($CanvasType)] } {
		set vars $TypesArray($CanvasType) 
	    }
	    foreach i $vars {
		if {[lindex $i 1] == "number"} {
		    lappend result [lindex $i 0]
		}
	    }
	    return $result
	}
    } else {
	return {}
    }
}

####################################################################
#
#
#
#
####################################################################

# commented by duke 8/14/97 
# Called_From_Startup is added to differentiate the caller functions. 
# Actually, now I realized that RecordItem should do the same thing. . .
# but it works so. . . . maybe if later when someone is doing 
# restructuring, they can really find out if this extra field is 
# needed or not and get rid of it 

proc CanvasMenu {{RecordItem 0} {Called_From_Startup 0}} {
    
    global  TypeList TypesArray messageQueue InstanceTypeList
    global TempVar1 TempVar2 resultX resultY YDefault XDefault
    global CanvasXPlot CanvasYPlot CanvasXVector 
    global CanvasYVector MultiDefault ImageDefault totalCanvas
    global ImageChoice CanvasType OtherFont CanvasType ContainerVariable
    global TypeModesArray Canvases CanvasMultipliers CanvasImages
    global CanvasToUse ImageToUse MultiplierToUse MediumFont
    global script_changed startup_windows 

    # commented by duke Aug 18 1997
    # if animation canvas is open from main panel we want to start over 
    
    if {[winfo exists .menu]} {
	destroy .menu
    }

    toplevel .menu -height 20c -width 13c 

    set ContainerVariable ""
    #frame .menu.bar -relief raised -bd 2 -class menu
    frame .menu.top -relief raised -bd 3
    frame .menu.lowtop -relief raised -bd 3
    frame .menu.lowtop.top
    frame .menu.lowtop.bottom
    frame .menu.lowtop.radio
    frame .menu.lowtop.choices -relief sunken -bd 3
    frame .menu.middle -relief raised -bd 2
    frame .menu.middle.left -class small
    frame .menu.middle.right -class small
    frame .menu.bottom
    
    ## This is the size it would be with the straight road default.  I
    ## make it the stantard size here.
    #    wm minsize .menu 475 651
    #    place .menu.bar -anchor nw -relwidth 1.0 -relheight 0.06

    place .menu.top -relwidth 1.0 -relheight .2 -rely .00
    place .menu.middle -relwidth 1.0 -relheight .35 -rely .2
    place .menu.lowtop -relwidth 1.0 -relheight .35 -rely .55
    place .menu.bottom -relwidth 1.0 -relheight .10 -rely .9

    place .menu.middle.left -relwidth .75 -relheight 1.0 -relx 0
    place .menu.middle.right -relwidth .25 -relheight 1.0 -relx .75

    #    menubutton .menu.bar.file -text "Files" -menu .menu.bar.file.m 
    #    set m [menu .menu.bar.file.m -tearoff 0]
    #    $m add command -label "Run Animation File" -command {FindAnimationFile}
    #    $m add command -label "Run Default File" \
    #	     -command {destroy .menu;DoPreviousAnimation;}
    #   place .menu.bar.file -relwidth .2 -relx 0 -relheight 1.0 -rely 0

    AnimationCreateCanvasSelectionWindow
    AnimationCreateVariableSelectionWindow
    AnimationCreateImageSelectionWindow

    ##Pack the bottom of the box
    button .menu.bottom.ok -text OK 
    button .menu.bottom.cancel -text "Cancel" -command {destroy .menu}

    if {$Called_From_Startup == "1"} {
	set current_window .menu 
	lappend startup_windows .menu 
	if {$RecordItem == 1} {
	    .menu.bottom.ok configure -command {RecordDecide $ImageChoice; set script_changed 1}
	} else {
	    .menu.bottom.ok configure -command {Decide $ImageChoice; set script_changed 1}
	}
	place .menu.bottom.ok -relwidth .2 -relx .2 -rely .2
	place .menu.bottom.cancel -relwidth .2 -relx .6 -rely .2
    } else {
	set current_window ""
	if {$RecordItem == 1} {
	    .menu.bottom.ok configure -command {RecordDecide $ImageChoice}
	} else {
	    .menu.bottom.ok configure -command {Decide $ImageChoice}
	}
	place .menu.bottom.ok -relwidth .2 -relx .2 -rely .2
	place .menu.bottom.cancel -relwidth .2 -relx .6 -rely .2
    }

    wm protocol .menu WM_DELETE_WINDOW "RemoveFromStartUpWindows $current_window; destroy .menu" 
}


proc AnimationCreateCanvasSelectionWindow {} {

    global CanvasToUse CanvasImages ImageToUse CanvasMultipliers
    global MultiplierToUse Canvases

    label .menu.top.canvaschoice_lab -text "Canvas Type:"
    set MultiplierToUse 1
    eval tk_optionMenu .menu.top.canvaschoice CanvasToUse $Canvases
    eval tk_optionMenu .menu.top.type ImageToUse $CanvasImages
    label .menu.top.type_lab -text "Image Type "
    eval tk_optionMenu .menu.top.multis MultiplierToUse $CanvasMultipliers
    label .menu.top.multis_lab -text "Multiplier "

    trace variable CanvasToUse w MenageAnimationWindowCanvas
    trace variable ImageToUse w CallDeactivateAnimationWindow 

    place .menu.top.canvaschoice_lab -x 20 -rely .05
    place .menu.top.canvaschoice -x 170 -rely .05 -relwidth .4
    place .menu.top.multis  -x 170 -rely .35 -relwidth .4
    place .menu.top.multis_lab -x 20 -rely .35
    place .menu.top.type -x 170 -rely .65 -relwidth .4
    place .menu.top.type_lab -x 20 -rely .65
}


proc AnimationCreateVariableSelectionWindow {} {


    #label .menu.middle.left.typechoice -text "Type :"
    combobox .menu.middle.left.typechoice findTypes
    place .menu.middle.left.typechoice -x 120 -y 30

    label .menu.middle.left.instance_lab -text "Instances :"
    combobox .menu.middle.left.instances findInstances
    .menu.middle.left.instances.e delete 0 end
    .menu.middle.left.instances.e insert 0 "ALL"

    #    label .menu.middle.left.xchoice -text "X :" 
    combobox .menu.middle.left.xchoice findVariables

    #    label .menu.middle.left.ychoice -text "Y :"
    combobox .menu.middle.left.ychoice findVariables
    combobox .menu.middle.left.orientation findVariables
    .menu.middle.left.orientation.e insert 0 "None"


    label .menu.middle.left.state_lab -text "State: "
    combobox .menu.middle.left.state findStates
    .menu.middle.left.state.e delete 0 end
    .menu.middle.left.state.e insert 0 "All-States"


    label .menu.middle.left.typelabel -text "Type:  "
    label .menu.middle.left.xlabel -text "X Axis:  "
    label .menu.middle.left.ylabel -text "Y Axis:  "
    label .menu.middle.left.containlabel  -text "Container: "
    label .menu.middle.left.container -textvariable ContainerVariable
    label .menu.middle.left.orientationlab -text "Orientation: "

    place .menu.middle.left.instance_lab -x 20 -y 60
    place .menu.middle.left.instances -x 120 -y 60
    place .menu.middle.left.xchoice -x 120 -y 90
    place .menu.middle.left.ychoice -x 120 -y 120
    place .menu.middle.left.typelabel -x 20 -y 30
    place .menu.middle.left.xlabel -x 20 -y 90
    place .menu.middle.left.ylabel -x 20 -y 120
    place .menu.middle.left.state_lab -y 180 -x 20 
    place .menu.middle.left.state  -y 180 -x 120
    place .menu.middle.left.orientationlab -y 150 -x 20 
    place .menu.middle.left.orientation  -y 150 -x 120

    place .menu.middle.left.containlabel -x 20 -y 210
    place .menu.middle.left.container -x 120 -y 210
#    place .menu.middle.left.contain -relwidth .65 -relx .35 -rely .62

    button .menu.middle.right.contain -text "Change"\
	    -command {SetContainer}

    place .menu.middle.right.contain -relwidth .6 -relx .2 -y 210
}



proc AnimationCreateImageSelectionWindow {} {

    global ImageChoice MediumFont Fonts
    global RadiusToUse HeightToUse WidthToUse BitmapToUse

    #####################################################
    ###### Here begins the image size section ###########
    #####################################################

    set Lengths {1 2 5 10 15 20 30 50 100 150 200}

    label .menu.lowtop.label -text "Image Options"
    place .menu.lowtop.label -height 35 -relwidth .4 -relx .3\
	    -rely 0
    place .menu.lowtop.radio -relwidth 1 -height 45 -relx 0\
	    -y 35
    place .menu.lowtop.choices -relwidth 1 -height 150 -y 80

    set ImageChoice fixed
    radiobutton .menu.lowtop.radio.fixed \
	    -variable ImageChoice \
	    -value fixed \
	    -text "Set Manually "\
	    -font $Fonts(MediumFont)
    radiobutton .menu.lowtop.radio.variable \
	    -variable ImageChoice \
	    -value variable \
	    -text "From Variable "\
	    -font $Fonts(MediumFont)
    radiobutton .menu.lowtop.radio.cont \
	    -variable ImageChoice \
	    -value contvariable \
	    -text "Always changing"\
	    -font $Fonts(MediumFont)

    frame .menu.lowtop.choices.radius
    frame .menu.lowtop.choices.height
    frame .menu.lowtop.choices.width
    frame .menu.lowtop.choices.bitmap

    place .menu.lowtop.choices.radius -relheight .25 -relwidth 1.0 -rely 0
    place .menu.lowtop.choices.height -relheight .25 -relwidth 1.0 -rely .25
    place .menu.lowtop.choices.width -relheight .25 -relwidth 1.0 -rely .5
    place .menu.lowtop.choices.bitmap -relheight .25 -relwidth 1.0 -rely .75

    set BitMaps {ball.bit car.bit jeep.bit car1.xbm sub.xbm}

    label .menu.lowtop.choices.bitmap.bit_lab -text "Bitmaps: "
    label .menu.lowtop.choices.radius.radius_lab -text "Radius: "
    label .menu.lowtop.choices.height.height_lab -text "Height: "
    label .menu.lowtop.choices.width.width_lab -text "Width: "
    
    place .menu.lowtop.choices.radius.radius_lab -x 10
    place .menu.lowtop.choices.height.height_lab -x 10
    place .menu.lowtop.choices.width.width_lab -x 10
    place .menu.lowtop.choices.bitmap.bit_lab -x 10


    # Do the radius OptionMenu
    eval tk_optionMenu .menu.lowtop.choices.radius.radius RadiusToUse $Lengths
    # Do the height OptionMenu
    eval tk_optionMenu .menu.lowtop.choices.height.height HeightToUse $Lengths
    # Do the width OptionMenu
    eval tk_optionMenu .menu.lowtop.choices.width.width WidthToUse $Lengths
    ## Do the bitmap optionMenu
    eval tk_optionMenu .menu.lowtop.choices.bitmap.bitmap BitmapToUse $BitMaps

    combobox .menu.lowtop.choices.radius.c findVariables
    combobox .menu.lowtop.choices.height.c findVariables
    combobox .menu.lowtop.choices.width.c findVariables

    .menu.lowtop.choices.radius.c.e configure -state disabled
    .menu.lowtop.choices.radius.c.b configure -state disabled
    .menu.lowtop.choices.bitmap.bitmap configure \
	    -state disabled
    .menu.lowtop.choices.radius.radius configure \
	    -state disabled
    place .menu.lowtop.choices.radius.radius -relwidth .15 -relx .2
    place .menu.lowtop.choices.height.height -relwidth .15 -relx .2
    place .menu.lowtop.choices.width.width -relwidth .15 -relx .2
    place .menu.lowtop.choices.bitmap.bitmap -relwidth .35 -relx .3

    place .menu.lowtop.choices.radius.c -relwidth .5 -relx .45
    place .menu.lowtop.choices.height.c -relwidth .5 -relx .45
    place .menu.lowtop.choices.width.c -relwidth .5 -relx .45
#    place .menu.lowtop.choices.bitmap.c -relwidth .5 -relx .45

    
    .menu.lowtop.radio.cont configure \
	    -command {DeactivateAnimationWindow}
    .menu.lowtop.radio.variable configure \
	    -command {DeactivateAnimationWindow}
    .menu.lowtop.radio.fixed configure \
	    -command {DeactivateAnimationWindow}

#    .menu.lowtop.left.fixed invoke

    place .menu.lowtop.radio.fixed -y 10 -relwidth .3 -relx .025
    place .menu.lowtop.radio.variable -y 10 -relwidth .3 -relx .35
    place .menu.lowtop.radio.cont -y 10 -relwidth .3 -relx .675
}


######################################################################
#
#
#
######################################################################

proc CallDeactivateAnimationWindow {this that other} {
    
    #puts "ABadl"
    
    DeactivateAnimationWindow

}


######################################################################
#
#
#
######################################################################


proc DeactivateAnimationWindow {args} {
    
    global ImageToUse ImageChoice

	if {$ImageChoice == "fixed"} {
	    if {$ImageToUse == "oval"} {
		AnimationWindowEnableFixed .menu.lowtop.choices.radius.radius
		EnableAnimationImageChoiceBoxes 
	    } elseif {$ImageToUse == "rectangle"} {
		AnimationWindowEnableFixed .menu.lowtop.choices.height.height
		EnableAnimationImageChoiceBoxes
	    } else {
		AnimationWindowEnableFixed .menu.lowtop.choices.bitmap.bitmap
		EnableAnimationImageChoiceBoxes
	    }
	} else {
	    if {$ImageToUse == "oval"} {
		AnimationWindowEnableFixed
		EnableAnimationImageChoiceBoxes .menu.lowtop.choices.radius.c
	    } elseif {$ImageToUse == "rectangle"} {
		AnimationWindowEnableFixed
		EnableAnimationImageChoiceBoxes .menu.lowtop.choices.height.c
	    } else {
		AnimationWindowEnableFixed
		EnableAnimationImageChoiceBoxes 
	    }
    }
}	


########################################################################
#
#
#
########################################################################


proc AnimationWindowEnableFixed {{toEnable -1}} {
    
    .menu.lowtop.choices.height.height configure -state disabled
    .menu.lowtop.choices.width.width configure -state disabled
    .menu.lowtop.choices.bitmap.bitmap configure -state disabled
    .menu.lowtop.choices.radius.radius configure -state disabled
    
    if {$toEnable == ".menu.lowtop.choices.height.height"} {
	$toEnable configure -state normal
	.menu.lowtop.choices.width.width configure -state normal
    } elseif {$toEnable != -1} {
		$toEnable configure -state normal
    }
}

########################################################################
#
#
#
########################################################################


proc EnableAnimationImageChoiceBoxes {{toEnable -1}} {

    # Height
    .menu.lowtop.choices.height.c.e configure \
	    -state disabled
    .menu.lowtop.choices.height.c.b configure \
	    -state disabled
    #Width
    .menu.lowtop.choices.width.c.e configure\
	    -state disabled
    .menu.lowtop.choices.width.c.b configure\
	    -state disabled

    #Radius
    .menu.lowtop.choices.radius.c.e configure\
	    -state disabled
    .menu.lowtop.choices.radius.c.b configure \
	    -state disabled


    if {$toEnable == ".menu.lowtop.choices.height.c"} {
	$toEnable.e configure -state normal
	$toEnable.b configure -state normal
	.menu.lowtop.choices.width.c.e configure -state normal
	.menu.lowtop.choices.width.c.b configure -state normal
    } elseif {$toEnable != -1} {
		$toEnable.e configure -state normal
		$toEnable.b configure -state normal
    }

}

##
## Call this when there is a change of canvas being used
## Need to put some defaults on the screen
##


proc MenageAnimationWindowCanvas {this that other } {
    
    global CanvasToUse MultiplierToUse ImageToUse InstanceTypeList
    global XDefault YDefault TypeModesArray TypeDefault TypesArray
    global ImageDefault MultiDefault
    
    set canvas $CanvasToUse
    set c_type $TypeDefault($canvas)
    #puts "Here i am with $canvas"
    if {[info exists InstanceTypeList($c_type)]} {
	#Insert the type 
	.menu.middle.left.typechoice.e delete 0 end
	.menu.middle.left.typechoice.e insert 0 $TypeDefault($canvas)

	set toFind [list $XDefault($canvas) number]
	set found [lsearch $TypesArray($c_type) $toFind]
	if {$found > -1} {
	    .menu.middle.left.xchoice.e delete 0 end
	    .menu.middle.left.xchoice.e insert 0 $XDefault($canvas)
	}

	set toFind [list $YDefault($canvas) number]
	set found [lsearch $TypesArray($c_type) $toFind]
	if {$found > -1} {
	    .menu.middle.left.ychoice.e delete 0 end
	    .menu.middle.left.ychoice.e insert 0 $YDefault($canvas)
	}
	#puts "Gonnas set the typesmeifer"
	if {[info exists TypeModesArray($c_type)] } {
	    set disc_states $TypeModesArray($c_type) 
	}
	
	#set disc_states [linsert $disc_states 0 "All-States"]
	#.menu.middle.left.state delete 0 end
	#	foreach state $disc_states {
#	    .menu.middle.left.state insert end $state
#	}

	set ImageToUse $ImageDefault($canvas)
        set MultiplierToUse $MultiDefault($canvas)

    }
}



#############################################################################
##
##
##
#############################################################################


proc Decide {ImageChoice} {

    global CanvasXPlot CanvasYPlot CanvasType ContainerVariable CanvasState
    global ImageToUse MultiplierToUse RadiusToUse HeightToUse
    global WidthToUse BitmapToUse CanvasToUse 


    set CanvasState [.menu.middle.left.state.e get]
    set AnimationType [.menu.middle.left.typechoice.e get]
    set XPlot  [.menu.middle.left.xchoice.e get]
    set YPlot  [.menu.middle.left.ychoice.e get]
    set Instances [.menu.middle.left.instances.e get]
    if {[llength $Instances] > 1} {
	set Instances [lindex $Instances 1]
    }

    set Orientation  [.menu.middle.left.orientation.e get]
    #puts "Orientation is $Orientation"
    if {$Orientation == "None"} {
	set Orientation Nil
    } else {
	if {![VariableExists $AnimationType $Orientation]} {
	    	set Orientation Nil
	}
    }
    
    set container_string $ContainerVariable
    set containerVariable [getVariableFromString $container_string]
    set containerInstance [getInstanceFromString $container_string]
    set containerType [getTypeFromString $container_string]

    ## Need to get the contain variables sorted out here

    set valu [VariablesExist $AnimationType $XPlot $YPlot]
    if {$valu == 0} {
	bell
	tk_dialog .error Error "The Type or Variables you requested do\
		not exist" error 0 OK
	return

    }

    if {$ImageChoice  != "fixed"} {
	set RadiusVariable [.menu.lowtop.choices.radius.c.e get]
	set HeightVariable [.menu.lowtop.choices.height.c.e get]
	set WidthVariable [.menu.lowtop.choices.width.c.e get]
	if {$ImageToUse == "oval"} {
	    if {$RadiusVariable == ""} {
		bell 
		tk_dialog .error Error "You have not specified the\
			names of the variables defining the variable object size." error 0 OK
		return
	    }
	} elseif {$ImageToUse == "rectangle"} {
	    if {$HeightVariable == "" && $WidthVariable == ""} {
		bell 
		tk_dialog .error Error "You have not specified the\
			names of the variables defining the variable object size." error 0 OK
		return
	    }
	}
    }
	

    set N 0	
    if {$ImageChoice  == "variable"} {
	set N 1
    } else {
	set N 2
    }
	
	if {$ImageToUse == "oval"} {
	if {$ImageChoice == "fixed"} {
	    CreateCanvas \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $RadiusToUse\
		    0\
		    $Orientation

	} elseif {$ImageChoice == "variable"} {
	    set RadiusVariable [.menu.lowtop.choices.radius.c.e get]
	    CreateCanvas \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $RadiusVariable\
		    NONE \
		    $Orientation\
		    $N
	} else {
	    set RadiusVariable [.menu.lowtop.choices.radius.c.e get]
	    CreateCanvas \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $RadiusVariable\
		    NONE \
		    $Orientation\
		    $N
	}
	destroy .menu 
    } elseif {$ImageToUse == "rectangle"} {
	if {$ImageChoice == "fixed"} {
	    CreateCanvas \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $HeightToUse\
		    $WidthToUse\
		    $Orientation
	} elseif  {$ImageChoice == "variable"} {
	    set HeightVariable [.menu.lowtop.choices.height.c.e get]
	    set WidthVariable [.menu.lowtop.choices.width.c.e get]
	    CreateCanvas \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $HeightVariable\
		    $WidthVariable\
		    $Orientation\
		    $N
	} else {
	    set HeightVariable [.menu.lowtop.choices.height.c.e get]
	    set WidthVariable [.menu.lowtop.choices.width.c.e get]
	    CreateCanvas \
		    $CanvasToUse\
		    $XPlot \
		    $YPlot \
		    $Instances\
		    $AnimationType \
		    $ImageToUse \
		    $MultiplierToUse\
		    $CanvasState\
		    $containerVariable\
		    $containerType\
		    $containerInstance\
		    $HeightVariable\
		    $WidthVariable\
		    $Orientation\
		    $N
	}
	destroy .menu 
    } else {  
	## It's a bitmap
	CreateCanvas \
		$CanvasToUse\
		$XPlot \
		$YPlot \
		$Instances\
		$AnimationType \
		$ImageToUse \
		$MultiplierToUse\
		$CanvasState\
		$containerVariable\
		$containerType\
		$containerInstance\
		$BitmapToUse\
		0\
		$Orientation
	
	destroy .menu
    }
}
 
#############################################################################
##
##
##
#############################################################################

proc SetContainer {} {

    global ContainerVariable
    
    set container [InstanceListGitter 1]
    
    if {[llength $container] < 1} {
	return
    } elseif {[llength $container] ==1} {
	puts "Cool, man"
	set cont [lindex $container 0]

	set ContainerVariable [lindex $cont 0]
#	set l [string first "(" $cont]
#	set len [string length $cont]
#	set var [string range $cont 0 [expr $l -1]
#	set rest [string range $cont [expr $l+1] [$len -1]]
#	set instance
    } else {
	puts "Sororra"
	
    }
}

####################################################################
#
#
#
#
####################################################################


proc CheckCanvasCompleteness {var} {

    if {$var != ""} {
	return 1
    } else {
	return 0
    }
}
		
#############################################################################
##
##
##
#############################################################################

proc ChangeCanvasVars {} {

    global TypesArray

    set type [.canv_opts.middle.types get]
    set vars [list $TypesArray($type)]

    return $vars
}


#############################################################################
##
##
##
#############################################################################


proc VariablesExist {type XPlot YPlot} {

    global TypeList TypesArray

    #puts "I got $type and $XPlot and $YPlot"
    set name_list {}
    set ex [lsearch -exact $TypeList $type]
#    puts "Ex is now $ex"
    if {$ex < 0 } {
	return 0
    } else {
	set list $TypesArray($type)
	foreach x $list {
	    lappend name_list [lindex $x 0]
	}
	set ex [lsearch $name_list $XPlot]
	#	puts "ex=$ex for var:$XPlot on List: $list"
	if {$ex < 0 } {
	    return 0
	} else {
	    set ex [lsearch -exact $name_list $YPlot]
	    #	    puts "ex=$ex for var:$YPlot on List: $list"
	    if {$ex < 0 } {
		return 0 
	    } else {
		return 1
	    }
	}
    }
}
#############################################################################
##
##
##
#############################################################################


proc CanvasRequestExists {name} {
    global ExistingCanvases

    set exist_list [array names ExistingCanvases]
    if {[lsearch $exist_list $name] > -1} {
	set a_list $ExistingCanvases($name)
	if {[llength $a_list] > 0} {
	    return 1
	} else {
	    return 0
	}
    } else {
	return 0
    }
}


#############################################################################
##
##
##
#############################################################################

proc SameCanvasPrompt {name} {
    global ExistingCanvases WhichCanvas ExistingCanvasMenuValue

    toplevel .existing
# -bg ivory1
    set w ".existing"
    
    frame $w.top
    frame $w.middle
    frame $w.lomiddle -relief raised -bd 3
    frame $w.bottom

    pack $w.top $w.middle $w.lomiddle $w.bottom -side top -fill both -expand 1
    set WhichCanvas New
    set MatchList $ExistingCanvases($name)

    radiobutton $w.middle.new \
	    -text "New Canvas     " \
	    -variable WhichCanvas \
	    -value New \
	    -command "$w.lomiddle.menu configure -state disabled" 

    radiobutton $w.middle.old \
	    -text "Existing Canvas" \
	    -variable WhichCanvas \
	    -value Old \
	    -command "$w.lomiddle.menu configure -state normal" 

    eval tk_optionMenu $w.lomiddle.menu ExistingCanvasMenuValue $MatchList

    pack $w.middle.new $w.middle.old \
	    -side top \
	    -fill x \
	    -expand 1 \
	    -padx 1m \
	    -pady 2m

    pack $w.lomiddle.menu -pady 2m -padx 1m

    button $w.bottom.ok -text "OK" -command {
	global choice ExistingCanvasMenuValue
	if {$WhichCanvas == "New"} {
	    set choice 0
	    destroy .existing
	} else {
	    set value $ExistingCanvasMenuValue
	    set choice $value
	    destroy .existing
	}
    }
    pack $w.bottom.ok -fill x -expand 1
}



# end of file -- animate.tcl --




