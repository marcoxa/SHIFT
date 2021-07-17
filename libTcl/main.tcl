# -*- Mode: Tcl -*-
 
# main.tcl --

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
# This is the new main box, where you call continue and choose types to view.
# (Tolga 102397)
#
#############################################################################

proc MainPanel {} {

    global TimeClick messageQueue  Continue channel1 channel2 Stop Step
    global Start GetShiftTypes GraphedElement SERVER SOCKET1 SOCKET2
    global StepSize Total_Steps env shift_tk_dir
    global ButtonsToDisable AnimationRecallFile 
    global TimeStepSize TimeStep Fonts

    global TkShiftValues simName


##### Put these into appropriate modules (poss. parameters.tcl)
    global Main_Message
    set Main_Message "Welcome to TkShift"

    global dspWidth dspHeight
    set dspWidth 220
    set dspHeight 60

    global TimeElapsed
    set TimeElapsed 0

    set simName ""

    trace variable TimeClick w UpdateDisplay
    trace variable TimeElapsed w UpdateDisplay


#####

    ##################  Images  ##################
    set TRANSPARENT_GIF_COLOR SystemButtonFace
    
    image create photo rew_img -file ${shift_tk_dir}/images/rew.gif
    image create photo stop_img -file ${shift_tk_dir}/images/stop.gif
    image create photo play_img -file ${shift_tk_dir}/images/play.gif
    image create photo rec_img -file ${shift_tk_dir}/images/rec.gif
    image create photo time_img -file ${shift_tk_dir}/images/time.gif
    image create photo step_img -file ${shift_tk_dir}/images/step.gif
    
    ### These are the toolbar buttons
    image create photo run_img -file ${shift_tk_dir}/images/run.gif
    image create photo connect_img -file ${shift_tk_dir}/images/connect.gif
    image create photo clear_img -file ${shift_tk_dir}/images/clear.gif
    image create photo pref_img -file ${shift_tk_dir}/images/pref.gif

    ### These are the debug buttons
    image create photo animate_img -file ${shift_tk_dir}/images/animate.gif
    image create photo graph_img -file ${shift_tk_dir}/images/graph.gif
    image create photo script_img -file ${shift_tk_dir}/images/script.gif
    image create photo inspect_img -file ${shift_tk_dir}/images/inspect.gif

    ### Misc images
#    image create photo path_img -file ${shift_tk_dir}/images/pathlogo_small.gif
    image create bitmap marker_bmp -file ${shift_tk_dir}/images/marker.xbm
    ##################  Menu Layout  ##################
    #
    # This uses Tk8.0 style menus !!
    #
    menu .main_menu -relief raised
    menu .main_menu.project
    menu .main_menu.control
    menu .main_menu.control.options
    menu .main_menu.debug
    menu .main_menu.help
    .main_menu add cascade -label "Project" -menu .main_menu.project
    .main_menu add cascade -label "Control" -menu .main_menu.control
    .main_menu add cascade -label "Debug" -menu .main_menu.debug
    .main_menu add cascade -label "Help" -menu .main_menu.help

    .main_menu.project add command -label "Run" -command {FrontPanel}
    .main_menu.project add command -label "Connect" -command {ConnectSimulation}
    .main_menu.project add command -label "Clear" -command {ClearSimulation} -state disabled
    .main_menu.project add separator
    .main_menu.project add command -label "Preferences" -command {SetEnv}
    .main_menu.project add separator
    .main_menu.project add command -label "Quit" -command {
    	catch {lappend messageQueue "9999"}
	catch {close $channel1}
	catch {close $channel2}
	destroy .
	exit
    }

    .main_menu.control add command -label "Play" -command {.control.play invoke} -state disabled
    .main_menu.control add command -label "Stop" -command {.control.stop invoke} -state disabled
    .main_menu.control add command -label "Step" -command {.control.step invoke} -state disabled
    .main_menu.control add command -label "Time" -command {.control.time invoke} -state disabled
    .main_menu.control add separator
    .main_menu.control add command -label "Reset" -command "" -state disabled
    .main_menu.control add separator
    .main_menu.control add cascade -label "Options" -menu .main_menu.control.options -state disabled

    .main_menu.control.options add command -label "Set Step" -command {StepOptions}
    .main_menu.control.options add command -label "Set Time" -command {TimeOptions}
    .main_menu.control.options add command -label "Set Zeno" -command {ZenoOptions}
    
    .main_menu.debug add command -label "Graph" -command {CreateNewGraph} -state disabled
    .main_menu.debug add command -label "Inspect" -command {TypesBox} -state disabled
    .main_menu.debug add command -label "Animate" -command {CanvasMenu} -state disabled
    .main_menu.debug add separator
    .main_menu.debug add command -label "Script" -command {CreateStartFile} -state disabled
    
    .main_menu.help add command -label "Help" -command {HelpBox}
    .main_menu.help add command -label "Copyright" -command {Copyright}
    .main_menu.help add command -label "Credits" -command {Credit}

    . configure -menu .main_menu
    wm resizable . false false
    
    ### Pop-up menu fails (even with 8.0p1)
    bind . <3> {
	catch {tk_popup .main_menu %X %Y 1}
    }
    
    ##################  Window Layout  ##################
    
    frame .toolbar -relief flat -borderwidth 2 -class ToolBar
    frame .control -relief flat -borderwidth 2
    frame .debug -relief flat -borderwidth 2
    frame .status -relief raised -borderwidth 2 -class StatusBar

    pack .status -side bottom -fill both -expand no
    pack .toolbar -side left -expand no -pady 4 -anchor n
    pack .control -side left -fill both -expand no -pady 2
    pack .debug -side right -expand no -pady 4 -anchor n

    
    ##################  Toolbar Frame  ##################
    set bw 27
    set bh 27
    set bow 1
    set br raised

    button .toolbar.run -image run_img -command {FrontPanel} -text "Run Simulation" \
	    -relief $br -borderwidth $bow -width $bw -height $bh
    button .toolbar.connect -image connect_img -command {ConnectSimulation} -text "Connect to Simulation" \
	    -relief $br -borderwidth $bow -width $bw -height $bh
    button .toolbar.clear -image clear_img -command {ClearSimulation} -state disabled -text "Clear Simulation" \
	    -relief $br -borderwidth $bow -width $bw -height $bh
    button .toolbar.pref -image pref_img -command {SetEnv} -text "Change Preferences"\
	    -relief $br -borderwidth $bow -width $bw -height $bh
    
    grid .toolbar.run -in .toolbar -row 1 -column 1 -padx 1 -pady 1
    grid .toolbar.connect -in .toolbar -row 1 -column 2 -padx 1 -pady 1
    grid .toolbar.pref -in .toolbar -row 2 -column 1 -padx 1 -pady 1
    grid .toolbar.clear -in .toolbar -row 2 -column 2 -padx 1 -pady 1


#    label .toolbar.path -image path_img
#    grid .toolbar.path -in .toolbar -row 3 -column 1 -columnspan 2 \
#	    -sticky sw -padx 1

#    grid rowconfigure .toolbar 3 -minsize 30
 
    
    ##################  Control Frame  ##################
    canvas .control.display -relief sunken -bg black \
	    -width $dspWidth -height $dspHeight -borderwidth 2
    set dispC .control.display
    button .control.rew -image rew_img -state disabled -text "Reset"
    button .control.stop -image stop_img -state disabled\
	    -command "lappend messageQueue $Stop;\
	    .control.play configure -state normal;\
	    .control.stop configure -state disabled;\
	    .control.step configure -state normal;\
	    .control.time configure -state normal;\
	    .main_menu.control entryconfigure 1 -state normal;\
	    .main_menu.control entryconfigure 2 -state disabled;\
	    .main_menu.control entryconfigure 3 -state normal;\
	    .main_menu.control entryconfigure 4 -state normal;\
	    catch {trace vdelete TimeElapsed w CheckTimeStep};\
	    catch {trace vdelete TimeClick w CheckStep};\
	    UpdateDispStatus stop"\
	    -text "Stop" -borderwidth 2
    button .control.play -image play_img -state disabled\
	    -command "lappend messageQueue $Continue;\
	    .control.play configure -state disabled;\
	    .control.stop configure -state normal;\
	    .control.step configure -state disabled;\
	    .control.time configure -state disabled;\
	    .main_menu.control entryconfigure 1 -state disabled;\
	    .main_menu.control entryconfigure 2 -state normal;\
	    .main_menu.control entryconfigure 3 -state disabled;\
	    .main_menu.control entryconfigure 4 -state disabled;\
	    UpdateDispStatus play"\
	    -text "Play" -borderwidth 2
    button .control.step -image step_img -state disabled\
	    -command "SetupStepButton"\
	    -text "Step $StepSize unit(s)" -borderwidth 2
    button .control.time -image time_img -state disabled\
	    -command "SetupTimeStepButton" \
	    -text "Time step $TimeStepSize unit(s)" -borderwidth 2

    bind .control.step <3> { 
	if {[%W cget -state] != "disabled"} {
	    StepOptions
	    break
	}
    }
    bind .control.time <3> { 
	if {[%W cget -state] != "disabled"} {
	    TimeOptions 
	    break
	}
    }    

    grid .control.display -in .control -row 1 -column 1 \
	    -columnspan 7 -rowspan 2 -sticky nesw -pady 2
    grid .control.rew -in .control -row 3 -column 2 
    grid .control.stop -in .control -row 3 -column 3
    grid .control.play -in .control -row 3 -column 4
    grid .control.step -in .control -row 3 -column 5
    grid .control.time -in .control -row 3 -column 6
    
    grid rowconfigure .control 3 -weight 0 -minsize 20

    grid columnconfigure .control {2 4 5 6} -weight 0
    grid columnconfigure .control {1 7} -pad 0 -minsize 5

    
    ################## Debug Frame  ##################
    
    button .debug.graph -image graph_img -command {CreateNewGraph}\
	    -state disabled -text "Graph" -relief $br -borderwidth $bow -width $bw -height $bh
    button .debug.inspect -image inspect_img -command {TypesBox}\
	    -state disabled -text "Inspect" -relief $br -borderwidth $bow -width $bw -height $bh
    button .debug.animate -image animate_img -command {CanvasMenu}\
	    -state disabled -text "Animate" -relief $br -borderwidth $bow -width $bw -height $bh
    button .debug.script -image script_img -command {CreateStartFile}\
	    -state disabled -text "Script" -relief $br -borderwidth $bow -width $bw -height $bh

    grid .debug.graph -in .debug -row 1 -column 1 -padx 1 -pady 1
    grid .debug.inspect -in .debug -row 1 -column 2 -padx 1 -pady 1
    grid .debug.animate -in .debug -row 2 -column 1 -padx 1 -pady 1
    grid .debug.script -in .debug -row 2 -column 2 -padx 1 -pady 1
    
    ################## Status Frame  ##################
    
    label .status.message -textvariable Main_Message -relief sunken -borderwidth 1
    pack .status.message -fill x -expand yes -padx 3 -pady 3
    
    ############ Toplevel Configuration ###############

    wm protocol . WM_DELETE_WINDOW {
	catch {lappend messageQueue "9999"}
	catch {close $channel1}
	catch {close $channel2}
	destroy .
	exit
    }
    . configure -bg LightGrey
    
    #### Create Main Display Message and begin blinking
    .control.display create text [expr $dspWidth/2] [expr $dspHeight/2] -text "NO SIMULATION" \
	    -tags _main_msg -anchor center -fill black
    blink 1

    #### Add SensitiveButton tag to every button in main window
    set frame_list [list .toolbar .control .debug]
    foreach fr $frame_list {
	set child_list [winfo children $fr]
	foreach child $child_list {
	    if {[lsearch -exact Button [winfo class $child]] != -1} {
		bindtags $child [linsert [bindtags $child] 0 SensitiveButton]
	    }
	}   
    }
    
    #### Bind SensitiveButton buttons for help display
    bind SensitiveButton <Enter> {
	if {[%W cget -state] != "disabled"} {
	    switch -exact -- %W {
		.control.step {
		    %W configure -text "Step $TkShiftValues(step) unit(s)"
		}
		.control.time {
		    %W configure -text "Time Step $TkShiftValues(time) unit(s)"
		}
	    }
	    set Main_Message [%W cget -text]
	}
    }
    
    bind SensitiveButton <Leave> {
	set Main_Message ""
    }
}





######################################################################
#
# blink:   Used for the blinking effect in the main display
#
# Input:   mode    (0:text off   1:text on)
#
#
############################# (Tolga 102397) #########################

proc blink {mode} {
    global lastBlink

    switch -exact -- $mode {
	0 {
	    .control.display itemconfigure _main_msg -fill black
	    update
	    set lastBlink [after 700 blink 1]
	    return
	} 
	1 {
	    .control.display itemconfigure _main_msg -fill green
	    update
	    set lastBlink [after 700 blink 0]
	    return
	}
	2 {
	    after idle {after cancel $lastBlink}
	    update idletasks
	    .control.display itemconfigure _main_msg -fill black -text ""
	    return
	}
	3 {
	    after idle {after cancel $lastBlink}
	    update idletasks
	    .control.display itemconfigure _main_msg -fill green
	    return
	}
	default {
	    puts "Invalid Mode for Blink ($mode)"
	}
    }

}



proc ChangeDisplay {} {
    global dspWidth dspHeight
    global TimeClick TimeElapsed simName

    set statHeight [expr $dspHeight*2/7]
    set dataHeight [expr $dspHeight-$statHeight]
    set dataWidth [expr $dspWidth/2]

    .control.display create line 0 $statHeight $dspWidth $statHeight \
	    -fill green4 -tags [list _score _hline]
    .control.display create line $dataWidth $statHeight $dataWidth $dspHeight \
	    -fill green4 -tags [list _score _vline]
    .control.display create text [expr $dspWidth/2] [expr $statHeight/2] -text "$simName ready"\
	    -fill green -tags [list _score _status] -anchor center
    .control.display create text [expr $dataWidth-6] [expr $statHeight+$dataHeight/2]  -text $TimeClick\
	    -fill green -tags [list _score _step] -anchor e -font Font(ScoreBoard)
    .control.display create text [expr $dataWidth/2] [expr $statHeight+$dataHeight-4]  -text STEP\
	    -fill green -tags [list _score _step_text] -anchor center -font Font(ScoreBoardTiny)
    .control.display create text [expr $dataWidth*2-4] [expr $statHeight+$dataHeight/2] \
	    -fill green -tags [list _score _time] -anchor e -font Font(ScoreBoard)
    .control.display create text [expr $dataWidth*3/2] [expr $statHeight+$dataHeight-4]  -text TIME\
	    -fill green -tags [list _score _step_text] -anchor center -font Font(ScoreBoardTiny)

    ### Be careful with this
    set TimeElapsed 0

}


proc UpdateDisplay {type dummy op} {
    upvar #0 $type mytype
    global env TimeClick

    ### Find the length of the decimal part of SHIFT_ISTEP
    set dec [string length [lindex [split $env(SHIFT_ISTEP) .] 1]]

    if { $op == "w" } {
	switch -exact -- $type {
	    TimeClick {
		set msg [format "%s" $mytype] 
		.control.display itemconfigure _step -text $TimeClick
	    }
	    TimeElapsed {
		set msg [format "%.*f" $dec $mytype]
		.control.display itemconfigure _time -text $msg
	    }
	}
    }
}


proc UpdateDispStatus {type} {
    global simName

    switch -exact -- $type {
	play -
	step -
	time {
	    .control.display itemconfigure _status -text "$simName running..."
	}
	stop {
	    .control.display itemconfigure _status -text "$simName stopped"
	}
    }
}

proc SetupStepButton {} {
    global Step TkShiftValues TimeClick messageQueue StepSTOP
    global simName
    
    lappend messageQueue $Step
    .control.play configure -state disabled
    .control.stop configure -state normal
    .control.step configure -state disabled
    .control.time configure -state disabled
    .main_menu.control entryconfigure 1 -state disabled
    .main_menu.control entryconfigure 2 -state normal
    .main_menu.control entryconfigure 3 -state disabled
    .main_menu.control entryconfigure 4 -state disabled
    set StepSTOP [expr $TkShiftValues(step)+$TimeClick]
    catch {trace variable TimeClick w CheckStep}
    UpdateDispStatus step
}


proc SetupTimeStepButton {} {
    global TimeStep TkShiftValues TimeElapsed messageQueue TimeSTOP
    global simName

    lappend messageQueue $TimeStep
    .control.play configure -state disabled
    .control.stop configure -state normal
    .control.step configure -state disabled
    .control.time configure -state disabled
    .main_menu.control entryconfigure 1 -state disabled
    .main_menu.control entryconfigure 2 -state normal
    .main_menu.control entryconfigure 3 -state disabled
    .main_menu.control entryconfigure 4 -state disabled
    set TimeSTOP [expr $TkShiftValues(time)+$TimeElapsed]
    catch {trace variable TimeElapsed w CheckTimeStep}
    UpdateDispStatus time
}

proc CheckStep {type dummy op} {
    upvar #0 $type step
    global StepSTOP
    
    if {$step == $StepSTOP} {
	.control.stop invoke
    }
}

proc CheckTimeStep {type dummy op} {
    upvar #0 $type time
    global TimeSTOP
    
    if {$time == $TimeSTOP} {
	.control.stop invoke
    }
}


proc InitializeSetup {} {

    global Total_Steps env

    set Total_Steps $env(SHIFT_NSTEPS) 
    set Shift_ISTEP $env(SHIFT_ISTEP)

    ### Unbind the display canvas
    bind .control.display <1> {}

    ### Enable Control and Debug buttons
    set button_list [list .control.play .control.step .control.time \
            .debug.graph .debug.animate .debug.inspect .debug.script]

    foreach i $button_list {
	$i configure -state normal
    }

    ### Enable Control and Debug menu items (Note: First item is 1 not 0. 0 is the tear-off)
    .main_menu.control entryconfigure 1 -state normal;   ## Play
    .main_menu.control entryconfigure 3 -state normal;   ## Step
    .main_menu.control entryconfigure 4 -state normal;   ## Time
    .main_menu.control entryconfigure 8 -state normal;   ## Options

    .main_menu.debug entryconfigure 1 -state normal;   ## Graph
    .main_menu.debug entryconfigure 2 -state normal;   ## Inspect
    .main_menu.debug entryconfigure 3 -state normal;   ## Animate
    .main_menu.debug entryconfigure 5 -state normal;   ## Script

    blink 2
    ChangeDisplay
}


#############################################################################
# This is the old main box, where you call continue and choose types to view.
#
#
#############################################################################

proc __MainPanel {} {

    global TimeClick messageQueue  Continue channel1 channel2 Stop Step
    global Start GetShiftTypes GraphedElement SERVER SOCKET1 SOCKET2
    global StepSize Total_Steps env shift_tk_dir
    global ButtonsToDisable AnimationRecallFile 
    global TimeStepSize TimeStep Fonts
    
    frame .bar -relief raised -bd 2 -class menu
    frame .middle -bg ivory1 -relief raised -bd 2 
    frame .middle.main -relief ridge -bd 2 
    . configure -bg ivory1 -width 8c -height 11c

    #Create the menu bar
    menubutton .bar.file -text "Files" -menu .bar.file.m
    menubutton .bar.control -text "Simulation" \
	    -menu .bar.control.menu
    menubutton .bar.help -text "Help" -menu .bar.help.menu

    set m [menu .bar.file.m -tearoff 0]
    set m3 [menu .bar.control.menu -tearoff 0]
    set m4 [menu .bar.help.menu -tearoff 0]

    $m4 add command -label "Help" -command {HelpBox}
    $m4 add command -label "Copyright" -command {Copyright}
    $m4 add command -label "Credits" -command {Credits}
    $m3 add command -label "Configure Step" -command {StepOptions}\
	    -state disabled
    $m3 add command -label "Configure Time" -command {TimeOptions}\
	    -state disabled
    $m3 add command -label "Configure Zeno" -command {ZenoOptions}\
	    -state disabled
# -state disabled
#    $m3 add command -label "Run StartFile" -command \
	    {CreateStartFile} -state disabled

    $m add command -label "Run Simulation" -command {FrontPanel}
    $m add command -label "Connect Simulation" -command {ConnectSimulation}
    $m add command -label "Clear Simulation" -command \
	    "ClearSimulation" -state disabled
    $m add command -label "Set Environment" -command {SetEnv}
    $m add separator
    $m add command -label "Exit TkShift" -command {	
	catch {lappend messageQueue "9999"}
	catch {close $channel1}
	catch {close $channel2}
	destroy .
	exit
    }

    wm protocol . WM_DELETE_WINDOW {
	catch {lappend messageQueue "9999"}
	catch {close $channel1}
	catch {close $channel2}
	destroy .
	exit
    }


    label .middle.main.label -text "Simulation Control" -font $Fonts(OtherFont)
    
    #Create the simulation control buttons
    button .middle.main.continue -text "Start" \
	    -command "lappend messageQueue $Continue;.middle.main.continue\
	    configure -state disabled;.middle.main.stop configure -state\
	    normal " -state disabled
    button .middle.main.stop -text "Stop" \
	    -command "lappend messageQueue $Stop;.middle.main.continue\
	    configure -state normal;.middle.main.stop configure -state\
	    disabled" \
	    -state disabled
    button .middle.main.step -text "Start" \
	    -command "lappend messageQueue $Step" -state disabled
    button .middle.main.timestep -text "Start" \
	    -command "lappend messageQueue $TimeStep"  -state disabled
    #button .middle.main.script -text "Run Script" -command {Script}
    
    #Create the simulation display buttons
    label .middle.main.label2 -text "Display Control" -font $Fonts(OtherFont)
    label .middle.main.label3 -text "Simulation Time" -font $Fonts(OtherFont)
    button .middle.main.graph -text "Graph" -command {CreateNewGraph} \
	    -state disabled
    button .middle.main.animate -text "Animate" \
	    -command {CanvasMenu} -state disabled

    button .middle.main.inspect -text "Inspect" -command {TypesBox} \
	    -state disabled
    button .middle.main.script -text "Script" \
	    -command {CreateStartFile} -state disabled
    
    #set pathImage [image create photo -file "$shift_tk_dir/PathLogo.gif"]

    #label .middle.main.logo -image $pathImage

    label .middle.main.timelab -text "Time Click: "


    label .middle.main.time -text "None" \
	    -relief sunken -bd 2 -anchor w


    label .middle.main.timeellab -text "Time Elapsed: "

    label .middle.main.timeel -text "None"  -relief sunken -bd 2 -anchor w

    ##Place the bar frame
    grid rowconfigure . 0 -weight 0
    grid rowconfigure . 1 -weight 0
    grid columnconfigure . 0 -weight 1 -minsize 250

    grid columnconfigure .bar 0 -weight 0
    grid columnconfigure .bar 1 -weight 1

    grid columnconfigure .middle 0 -minsize 200 -weight 1
    grid rowconfigure .middle 0 -minsize 300 -weight 0

    grid columnconfigure .middle.main 0 -minsize 120 -weight 0
    grid columnconfigure .middle.main 1 -minsize 120 -weight 0

    grid .bar -row 0 -column 0 -sticky we

    ## Place the menubuttons
    grid .bar.file -column 0 -row 0 -sticky w
    grid .bar.control -column 1 -row 0 -sticky w
    grid .bar.help -column 1 -row 0 -sticky e
    #place .bar.control -relwidth 0.35 -relx 0.3
    #place .bar.help -relwidth 0.25 -relx 0.75
    
    #place .middle -relwidth 1.0 -relheight .88 -rely .12
    #place .middle.main -relwidth 1.0 -relheight 1.0 -relx 0 -rely 0.0
    
    grid .middle -row 1 -column 0 -sticky news
    grid .middle.main -row 0 -column 0 -sticky news

    grid rowconfigure .middle.main 0 -weight 0 -minsize 35
    grid rowconfigure .middle.main 1 -weight 0
    grid rowconfigure .middle.main 2 -weight 0
    grid rowconfigure .middle.main 3 -weight 0 -minsize 35
    grid rowconfigure .middle.main 4 -weight 0
    grid rowconfigure .middle.main 5 -weight 0
    grid rowconfigure .middle.main 6 -weight 0
    grid rowconfigure .middle.main 7 -weight 0 -minsize 35
    grid rowconfigure .middle.main 8 -weight 0
    grid rowconfigure .middle.main 9 -weight 0
    grid columnconfigure .middle.main 0 -weight 0
    grid columnconfigure .middle.main 1 -weight 0
    set padNum 10
    #place .middle.main.label -relwidth 1.0 -relheight 0.1 -relx .0 \
	    -rely .0
    #place .middle.main.continue -relwidth 0.4 -relx .05 -rely .1
    grid .middle.main.label -row 0 -columnspan 2 -sticky we -padx $padNum
    grid .middle.main.continue -row 1 -column 0 -sticky we -padx $padNum
    grid .middle.main.stop -row 1 -column 1 -sticky we -padx $padNum

    grid .middle.main.step -row 2 -column 0 -sticky we -padx $padNum
    grid .middle.main.timestep -row 2 -column 1 -sticky we -padx $padNum

    grid .middle.main.label2 -row 3 -columnspan 2 -sticky we -padx $padNum 

    grid .middle.main.graph -row 4 -column 0 -sticky we -padx $padNum
    grid .middle.main.animate -row 4 -column 1 -sticky we -padx $padNum 
    grid .middle.main.inspect -row 5 -column 0 -sticky we -padx $padNum 
    grid .middle.main.script -row 5 -column 1 -sticky we -padx $padNum  

    grid .middle.main.label3 -row 7 -columnspan 2 -sticky we -padx $padNum  
    grid .middle.main.timelab -row 8 -column 0 -sticky e
    grid .middle.main.timeellab -row 9 -column 0 -sticky e
    grid .middle.main.time -row 8 -column 1 -sticky we -padx $padNum
    grid .middle.main.timeel -row 9 -column 1 -sticky we -padx $padNum
    
    #place .middle.main.graph -relwidth 0.4 -relx .05 -rely .5
    #place .middle.main.animate -relwidth 0.4 -relx .55 -rely .5
    #place .middle.main.stop -relwidth 0.4 -relx .55 -rely .1
    #place .middle.main.step -relwidth 0.4 -relx .05 -rely .25
    #place .middle.main.timestep -relwidth 0.4 -relx .55 -rely .25
    #place .middle.main.label2 -relwidth 1.0 -relheight 0.1 -relx .0 \
	    -rely .4
    #place .middle.main.graph -relwidth 0.4 -relx .05 -rely .5
    #place .middle.main.animate -relwidth 0.4 -relx .55 -rely .5

    #place .middle.main.inspect -relwidth 0.4 -relx .05 -rely .5
    #place .middle.main.script -relwidth 0.4 -relx .55 -rely .5

    #place .middle.main.label3 -relwidth 1.0 -relheight 0.1 -relx .0 \
	    -rely .65
    #place .middle.main.timelab -relheight .1 -rely .75 \
	    -x 45
    
    #place .middle.main.timeellab -relheight .1 -rely .85 \
	    -x 20
    #place .middle.main.time -relwidth .4 -relheight .1 -rely .75 \
	    -relx 0.55
    #place .middle.main.timeel -relwidth .4 -relheight .1 -rely .85 \
	    -relx 0.55
}



proc __InitializeSetup {} {

    global Total_Steps env

    set Total_Steps $env(SHIFT_NSTEPS) 
    set Shift_ISTEP $env(SHIFT_ISTEP)
    #initialize_vectors    

    set button_list [list .middle.main.continue .middle.main.stop\
	    .middle.main.step .middle.main.timestep .middle.main.graph\
	    .middle.main.animate .middle.main.inspect .middle.main.script]

    foreach i $button_list {
	$i configure -state normal
    }
}



###########################
# This functions gets a string in this format string1(string2) and
# returns string2
###########################

proc ToPrint {str} {


    #puts $str
    
    set l [string first "(" $str]
    set name [string range $str 0 [expr $l -2]]
    if {$name == "global"} {
	return "global"
    } else {
	set len [string length $str]
	set s [string range $str [expr $l +1 ] [expr $len -2]]
        #putns "String $s"
	return $s
    }
}



################################################
# This is called by the Continue procedure, and it updates the info in the 
#listboxes
#
##############################################


proc UpdateBoxes {} {

    global PackedData

    foreach element $PackedData {
	
	set a [.instances.d.data.w$element.middle.box size]
	.instances.d.data.w$element.middle.box delete 0 [expr $a -1]
	lappend Data2 $element
	RePrint $element
    }
    set PackedData $Data2
}


proc resetWidths {w} {

    set width [winfo width $w]
    set height [winfo height $w]
    set list_w [expr $width -25]
    set list_h [expr $height -40]

    place $w.bar -relwidth 1 -height 40 
    place $w.middle -relwidth 1 -height $list_h -y 40

    if {[winfo exists $w.middle.xscroll] == 1} {

	place $w.middle.list -width $list_w\
		-height [expr $height - 65]
	place $w.middle.scroll -width 25 -height [expr $height - 65]\
		-x $list_w
	place $w.middle.xscroll -width [expr $width - 25] -x 0\
		-height 25 -y [expr $height -65]


    } else {
	place $w.middle.list -width $list_w -relheight 1
	place $w.middle.scroll -width 25 -relheight 1\
	    -x [expr $width - 25]	
    }
}

