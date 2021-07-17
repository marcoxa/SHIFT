# -*- Mode: Tcl -*-

# front.tcl --

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



###################################################################
#
#
#
#
###################################################################

proc FrontPanel {} {

    global SimulationDirectory SmallFont simulation
 
#### Commented out by Tolga (110697)    
#    set simulation [fileselect "Simulation to run" .sim]

    ### Setup new file requestor (Tolga 110697)
    set ftype {
	{{Simulation Files} {.sim}}
	{{All Files} {*}}
    }
    set simulation [tk_getOpenFile -title "Simulation to run" -parent . \
	    -filetypes $ftype]

    puts "Selected $simulation"
    
    if {$simulation == ""} {
	return
    } else {
	### Begin Connection (tolga 110597)
	.control.display itemconfigure _main_msg -text "CONNECTING..."
	blink 3
	update
	global simName
	set simName [file rootname [file tail $simulation]]
	
	RunSim $simulation
    }
    
}


###################################################################
#
#
#
#
###################################################################

proc RunSim {simulation} {
    
    global env sim_pid add_to_socket SimulationDirectory 
    global AnimationRecallFile OpenWindows SocketAddress
    global socket_one socket_two home_directory IntegrationStep

#    set directory_string [.sim.middle.file get]
#    set l0 [string last "/" $directory_string]
#    set SimulationDirectory [string range $directory_string 0 [expr \
	    $l0 -1]]
    

    set machine_name [exec hostname]
    set socket_one [expr $SocketAddress + $add_to_socket]
    set socket_two [expr $socket_one +1]
    set env(SHIFT_GUI_DEBUG) 1
    set env(SHIFT_GUI_SERVER) "$machine_name $socket_one $socket_two"
    puts "We are trying sockets $socket_one and $socket_two"
    #    puts "Got -$simulation -"
    #    destroy .
    
    set ServerName [exec hostname]
    set sim_pid 0
    if {$IntegrationStep == "fixed"} {
	set sim_pid [exec $simulation -v $socket_one -is\
		$env(SHIFT_ISTEP) -ns $env(SHIFT_NSTEPS) &]
    } else {
	set sim_pid [exec $simulation -v $socket_one -vs -is\
		$env(SHIFT_ISTEP) -ns $env(SHIFT_NSTEPS) &]
    }
    set OpenWindows {}

    after 2000 "connect_with_server $sim_pid $simulation"

}


proc test_pid {id} {
    global SimulationSucceeded

    set s [catch {set a [exec ps -p $id ]}]
    set l [string length $s]
    if {$s == 0} {
	set SimulationSucceeded 1
    } else {
	set SimulationSucceeded 0
    }
}

proc ConnectSimulation {}  {

    global env SocketAddress
    global SERVER SOCKET1 SOCKET2 ServerName
    global socket_one socket_two
    
    toplevel .connect -height 250 -width 350
# -bg ivory1
#    frame .connect.top -relief raised -bd 2
    frame .connect.middle -relief ridge -bd 2
    frame .connect.bottom

    label .connect.middle.m_label -text "Machine"
    entry .connect.middle.machine

    label .connect.middle.s1_label -text "Socket 1"
    entry .connect.middle.socket1

    label .connect.middle.s2_label -text "Socket 2"
    entry .connect.middle.socket2

    .connect.middle.machine insert 0 $ServerName
    .connect.middle.socket1 insert 0 $SocketAddress
    .connect.middle.socket2 insert 0 \
	    [expr $SocketAddress + 1]

    button .connect.bottom.ok -text "OK" -command {
	set socket_one [.connect.middle.socket1 get]
	set socket_two [.connect.middle.socket2 get]
	set ServerName [.connect.middle.machine get]
	#InitializeSetup
	connect_with_server -1
	destroy .connect
    }
    
    button .connect.bottom.cancel -text "Cancel" -command {destroy \
	    .connect }
 
#    pack .connect.top -side top -fill both -expand 1
#    pack .connect.middle -side top -fill both -expand 1 -padx 2m -ipadx 1m
#    pack .connect.bottom -side top -fill both -expand 1

    place .connect.middle -relwidth 1.0 -height 200
    place .connect.bottom -relwidth 1.0 -height 50 -y 200

    place .connect.bottom.ok -relwidth .3 -relx .1  -relheight .7\
	    -rely .15
    place .connect.bottom.cancel -relwidth .3 -relx .6  -relheight .7\
	    -rely .15

    place .connect.middle.m_label -relx 0.05 -rely .05
    place .connect.middle.machine -relx 0.3 -rely .05 -relwidth .6

    place .connect.middle.s1_label -relx 0.05 -rely .35
    place .connect.middle.socket1 -relx 0.3 -rely .35 -relwidth .6

    place .connect.middle.s2_label -relx 0.05 -rely .65
    place .connect.middle.socket2 -relx 0.3 -rely .65 -relwidth .6
}


proc OpenRecorder {} {

    global AnimationRecallFile home_directory RecordingAnimationInfo 
    global defFile

    toplevel .recorder -height 6.5c -width 10c
    frame .recorder.top -relief raised -bd 2
    frame .recorder.middle -relief raised -bd 2
    frame .recorder.bottom
    
    
    radiobutton .recorder.top.default -variable defFile -value def\
	    -text "Default file" -command {
	.recorder.middle.fileName configure -state disabled
    }
    radiobutton .recorder.middle.define -variable defFile -value user\
	    -text "Set file" -command {
	.recorder.middle.fileName configure -state normal
    }

    label .recorder.middle.f_label -text "File name:"
    entry .recorder.middle.fileName -state disabled

    button .recorder.bottom.ok -text OK -command {
	global AnimationRecallFile

	set RecordingAnimationInfo 1
	
	if {$defFile == "def"} {
	    set AnimationRecallFile [open\
		    "$home_directory/.animationFile" w]
	} else {
	    set animFile [.recorder.middle.fileName get]
	    set AnimationRecallFile [open\
		    "$home_directory/$animFile" w]
	}

	.bar.control.menu entryconfigure 1 -state disabled
	.bar.control.menu entryconfigure 2 -state normal
	destroy .recorder
    }

    button .recorder.bottom.cancel -text "Cancel" -command\
	    {
	destroy .recorder
    }
    
    place .recorder.top -relwidth 1 -relheight .25 -rely .03
    place .recorder.middle -relwidth 1 -relheight .50 -rely .29
    place .recorder.bottom -relwidth 1 -relheight .15 -rely .80
    
    place .recorder.top.default -relwidth .5 -relx .05 -rely .2

    place .recorder.middle.define -relwidth .5 -relx .05 -rely 0.1

    place .recorder.middle.f_label -relwidth .4 -relx .05 -rely 0.5
    place .recorder.middle.fileName -relwidth .5 -relx .45 -rely 0.5

    place .recorder.bottom.ok -relwidth .3 -relx .1 -relheight 1
    place .recorder.bottom.cancel -relwidth .3 -relx .6 -relheight 1
}

proc CloseRecorder {} {

    global AnimationRecallFile RecordingAnimationInfo home_directory

    close $AnimationRecallFile
    .bar.control.menu entryconfigure 1 -state normal
    .bar.control.menu entryconfigure 2 -state disabled

    set RecordingAnimationInfo 0
}

###################################################################
#
#
#
#
###################################################################

proc connect_with_server {pid {simulation ""}} {

    global SOCKET1 SOCKET2 channel1 channel2 messageQueue SERVER
    global socket_one socket_two ServerName add_to_socket 
    global SimulationSucceeded Continue
    
    set SOCKET1 [format "%s" $socket_one]
    set SOCKET2 [format "%s" $socket_two]

    #    test_pid $pid
    #    vwait SimulationSucceeded

    #    if {$SimulationSucceeded == 1} {
	#	puts "Starting simulation run"
	#	destroy .sim 
	#    } else {
	    #	puts "Trying another set of sockets"
	    #	set add_to_socket [expr $add_to_socket + 2]
	    #	RunSim
	    #	return
	    #    }


    puts "Socket 1:$SOCKET1, socket2:$SOCKET2, and server:$ServerName"
    set a [catch {set channel1 [socket $ServerName $SOCKET1]} errorString]
    set b [catch {set channel2 [socket $ServerName $SOCKET2]} errorString2]

#	    puts "Error code $a for string is: $errorString with socket $SOCKET1"
#	    puts "Error code $b for string2 is: $errorString2 with socket $SOCKET2"


    if {$a != 0 || $b != 0} {
#	ErMsg "The sockets were not able to connect. Please try\
#		again."
#	return
	if {$pid == -1} {
	    puts "These sockets are busy. Try another set."
	    return
	} else {
	    puts "Trying another set of sockets"
	    set add_to_socket [expr $add_to_socket + 2]
	    RunSim  $simulation
	    return 
	}
    }

#### Commented out by Tolga 110597
#    InitializeSetup

    #fconfigure $channel1 -blocking 0
    fconfigure $channel2 -blocking 0
    
    #Execute2 this that those
    #Execute2 this that those
    #fileevent $channel2 writable Execute2
    trace variable messageQueue w Execute2

    fileevent $channel1 readable Execute1

#### Commented out by Tolga 102497
#    .bar.file.m entryconfigure 0 -state disabled
#    #puts "Button Run should be disabled now"
#    .bar.file.m entryconfigure 1 -state disabled
#    .bar.file.m entryconfigure 2 -state normal
#    .bar.file.m entryconfigure 3 -state disabled
#
#    .bar.control.menu entryconfigure 0 -state normal
#    .bar.control.menu entryconfigure 1 -state normal
#    .bar.control.menu entryconfigure 2 -state normal
##    .bar.control.menu entryconfigure 5 -state normal
##    .bar.control.menu entryconfigure 6 -state normal
####

    #puts "Sockets named, and fileevents scheduled"

    ### Newlook menus and corresponding buttons (Tolga 102497)
    .main_menu.project entryconfigure 1 -state disabled;   ## Run
    .main_menu.project entryconfigure 2 -state disabled;   ## Connect
    .main_menu.project entryconfigure 3 -state normal;     ## Clear
    .main_menu.project entryconfigure 5 -state disabled;   ## Preferences

    .control.display itemconfigure _main_msg -text "PRESS HERE TO START"
    blink 0

    ### Enable Debug buttons (Tolga 111097)
    set button_list [list .debug.graph .debug.animate .debug.inspect .debug.script]

    foreach i $button_list {
	$i configure -state normal
    }

    .main_menu.debug entryconfigure 1 -state normal;   ## Graph
    .main_menu.debug entryconfigure 2 -state normal;   ## Inspect
    .main_menu.debug entryconfigure 3 -state normal;   ## Animate
    .main_menu.debug entryconfigure 5 -state normal;   ## Script

    bind .control.display <1> {
	global Continue
	lappend messageQueue $Continue

	InitializeSetup
    }

    .toolbar.run configure -state disabled;      ## Run button
    .toolbar.connect configure -state disabled;  ## Connect button
    .toolbar.clear configure -state normal;      ## Clear button
    .toolbar.pref configure -state disabled;     ## Preferences button

    #### Added by Tolga 102397
    #### Does the effect of Start Button in old main window automatically
#    global Continue
#    lappend messageQueue $Continue

}


###################################################################
#
#
#
#
###################################################################


proc ClearSimulation {} {


    global SOCKET1 SOCKET2 SERVER channel1 channel2 messageQueue
    global TypesArray TypeList InstanceTypeList TimeVectors UnpackedData
    global totalGraphs TotalGraphNumber totalCanvases PackedData 
    global UnpackedData isStartUp TypesBroken ComponentsBroken TkShiftValues
    global CanvasAnimations

    # this is clean up procedure for script 
    DestroyStartUpWindows 

    if {[array exists received_instance]} {
	unset received_instance
    }

    # this is not really general but . . .  
    catch {destroy .start}    

    # this was not being done so, this is a quick fix 
    #.bar.file.m entryconfigure 1 -state normal 


    CloseAllWindows

    catch {lappend messageQueue "9999"}
    catch {close $channel1}
    catch {close $channel2}

    #### Commented out by Tolga 102497
    #    .middle.main.time configure -text "None"
    #    .middle.main.timeel configure -text "None"
    #     
    #    .middle.main.step configure -text "Step 1"
    #    .middle.main.timestep configure -text "Time 1"
    ####

    #    .middle.right.list delete 0 end
    set TimeVectors {}
    set UnpackedData {}
    catch {unset TypesArray}
    foreach i [array names ComponentsBroken] {
	catch {trace vdelete ComponentsBroken($i) w CommunicateBreakComps}
    }
    foreach i [array names TypesBroken] {
	catch {trace vdelete TypesBroken($i) w CommunicateBreakTypes}
    }

    catch {unset CanvasAnimations}
    set TypeList {}
    catch {unset InstanceTypeList}
    set totalGraphs 0
    set totalCanvases 0
    uninitialize_vectors
    set TotalGraphNumber 0
    set PackedData {}
    set UnpackedData {}
    set isStartUp 0
    set TkShiftValues(time) 1
    set TkShiftValues(step) 1
    set TkShiftValues(zeno) 1

#    set CanvasToUse DemoRoad
#    set ImageToUse rectangle

#### Commented out by Tolga 102497
#    .bar.file.m entryconfigure 0 -state normal
#    #puts "Button Run should work again "
#    .bar.file.m entryconfigure 0 -state normal
#    .bar.file.m entryconfigure 1 -state normal
#    .bar.file.m entryconfigure 2 -state disabled
#    .bar.file.m entryconfigure 3 -state normal
#    
#    .bar.control.menu entryconfigure 0 -state disabled
#    .bar.control.menu entryconfigure 1 -state disabled
#    .bar.control.menu entryconfigure 2 -state disabled
##    .bar.control.menu entryconfigure 5 -state disabled
##    .bar.control.menu entryconfigure 6 -state disabled
####

    ### Newlook menus and corresponding buttons (Tolga 102497)
    .main_menu.project entryconfigure 1 -state normal;   ## Run
    .main_menu.project entryconfigure 2 -state normal;   ## Connect
    .main_menu.project entryconfigure 3 -state disabled; ## Clear
    .main_menu.project entryconfigure 5 -state normal;   ## Preferences

    .toolbar.run configure -state normal;      ## Run button
    .toolbar.connect configure -state normal;  ## Connect button
    .toolbar.clear configure -state disabled;  ## Clear button
    .toolbar.pref configure -state normal;     ## Preferences button

#### Commented out by Tolga 102497
#    set button_list [list .middle.main.continue .middle.main.stop\
#	    .middle.main.step .middle.main.timestep .middle.main.graph\
#	    .middle.main.animate .middle.main.inspect .middle.main.script]
#
#    foreach i $button_list {
#	$i configure -state disabled
#    }
####

    ### Enable Control and Debug buttons (Tolga 102497)
    set button_list [list .control.play .control.step .control.time \
            .debug.graph .debug.animate .debug.inspect .debug.script]

    foreach i $button_list {
	$i configure -state disabled
    }

    ### Disable Control and Debug menu items (tolga 102497) 
    ### (Note: First item is 1 not 0. 0 is the tear-off)
    .main_menu.control entryconfigure 1 -state disabled;   ## Play
    .main_menu.control entryconfigure 3 -state disabled;   ## Step
    .main_menu.control entryconfigure 4 -state disabled;   ## Time
    .main_menu.control entryconfigure 8 -state disabled;   ## Options

    .main_menu.debug entryconfigure 1 -state disabled;   ## Graph
    .main_menu.debug entryconfigure 2 -state disabled;   ## Inspect
    .main_menu.debug entryconfigure 3 -state disabled;   ## Animate
    .main_menu.debug entryconfigure 5 -state disabled;   ## Script

    ### Reset main display (tolga 110597)
    .control.display delete _score
    .control.display itemconfigure _main_msg -text "NO SIMULATION"
    blink 1

#### Commented out by Tolga 102497
#    .middle.main.continue configure -text "Start"
#    .middle.main.step configure -text "Start"
#    .middle.main.timestep configure -text "Start"
####

}


###################################################################
#
#
#
#
###################################################################

proc SetEnv {} {

    global env SocketAddress ServerName Shift_ISTEP IntegrationStep

    
    set current_nsteps $env(SHIFT_NSTEPS)
    set current_istep $env(SHIFT_ISTEP)

    toplevel .env -height 8c -width 13c
# -bg ivory1

    frame .env.middle -relief raised -bd 2
    frame .env.bottom

    label .env.middle.n_label -text "   SHIFT_NSTEP"
    entry .env.middle.nstep
    label .env.middle.i_label -text "   SHIFT_ISTEP"
    entry .env.middle.istep 

    label .env.middle.s_label -text "Socket Address:"
    entry .env.middle.socket 
    button .env.middle.socketup -command UpSocketNumber -text "Up"
    button .env.middle.socketdown -command DownSocketNumber -text "^"

    label .env.middle.m_label -text "Remote Machine"
    entry .env.middle.machine 

    .env.middle.nstep insert 0 $current_nsteps
    ##Remove the default 5500 
    .env.middle.istep delete 0 end
    .env.middle.istep insert 0 $current_istep
    .env.middle.socket delete 0 end
    .env.middle.socket insert 0 $SocketAddress

    .env.middle.machine insert 0 [exec hostname]

    button .env.bottom.ok -text "OK" -command {
	set env(SHIFT_NSTEPS) [.env.middle.nstep get]
	set env(SHIFT_ISTEP) [.env.middle.istep get]
	set ServerName [.env.middle.machine get]
	set SocketAddress [.env.middle.socket get]
#	set SocketAddress [.env.middle.socket get]
	destroy .env
    }


        bind .env <Return> {
	set env(SHIFT_NSTEPS) [.env.middle.nstep get]
	set env(SHIFT_ISTEP) [.env.middle.istep get]
	set ServerName [.env.middle.machine get]
	set SocketAddress [.env.middle.socket get]
#	set SocketAddress [.env.middle.socket get]
	destroy .env
	}

    button .env.bottom.cancel -text "Cancel" -command {destroy \
	    .env }
 
    radiobutton .env.middle.varint -variable IntegrationStep \
	    -text "Variable Step" -value variable
    radiobutton .env.middle.fixedint -variable IntegrationStep\
	    -text "Fixed Step" -value fixed

    place .env.middle -relwidth 1 -relheight .8 
    place .env.bottom -relwidth 1 -relheight .15 -rely .85 

#    pack .env.middle.nstep .env.middle.istep \
	    .env.middle.socket .env.middle.machine \
	    -side top -fill x -expand 1 -pady 3m -padx 2m
    place .env.middle.n_label -rely 0 -relwidth .4 -relx .05
    place .env.middle.nstep -rely 0 -relwidth .3 -relx .5

    place .env.middle.i_label -rely 0.2 -relwidth .4 -relx .05
    place .env.middle.istep -rely 0.2 -relwidth .3 -relx .5

    place .env.middle.s_label -rely 0.4 -relwidth .4 -relx .05
    place .env.middle.socket -rely 0.4 -relwidth .3 -relx .5
    place .env.middle.socketup -rely 0.4 -relwidth .1 -relx .8\
	    -height 32

    place .env.middle.m_label -rely 0.6 -relwidth .4 -relx .05
    place .env.middle.machine -rely 0.6 -relwidth .45 -relx .5

    place .env.middle.varint -relwidth .4 -relx .05 -rely .8
    place .env.middle.fixedint -relwidth .4 -relx .55 -rely .8

#    pack .env.bottom.ok .env.bottom.cancel -side left -fill x -expand 1
    place .env.bottom.ok -relwidth .3 -relx .1 -rely .02
    place .env.bottom.cancel -relwidth .3 -relx .6 -rely .02
}

proc UpSocketNumber {} {
    set n [.env.middle.socket get]
    .env.middle.socket delete 0 end
    set new_n [expr $n + 2]
    .env.middle.socket insert 0 $new_n
}

proc DownSocketNumber {} {
    set n [.env.middle.socket get]
    .env.middle.socket delete 0 end
    set new_n [expr $n - 2]
    .env.middle.socket insert 0 $new_n
}


proc HelpBox {} {

	    tk_dialog .info Information "The user manual is online, at\
		    www.path.berkeley.edu/shift/TkShift/TkShift.html\
		    . Consult it if you have a problem. Please email all\
		    problems you have to danielw@path.berkeley.edu" info 0 OK
}


proc CloseAllWindows {} {

    global OpenWindows InstanceTypeList

    foreach i $OpenWindows {
	if {[string range $i 0 4] == ".win_"} { 
	    #it's a component info box 
	    catch {destroy $i}
#	    Dequeue [string range $i 5 end]

	} elseif {[string range $i 0 5] == ".info_"} {
#it's a type box
	    catch {destroy $i}
	} elseif {[string range $i 0 2] == ".w_"} {
#it's a graph
	    #set w [string range $i 1 end]
	    RemoveGraphData $i
	    catch {destroy $i}
	} elseif {[string range $i 0 3] == ".typ"} {
#it's a graph
	    catch {destroy $i}
	} else { 
#it's a canvas
	    ExitCanvas [wm title $i]
	    destroy $i
	}
    }
    if {[winfo exists .break]} {
	destroy .break
    }
    set OpenWindows {}
}

proc DequeueFromOpenWindows {win} {

    global OpenWindows

    set otherList {}
    foreach i $OpenWindows {
	if {$i != $win} {
	    lappend otherList $i
	}   
    }
    set OpenWindows $otherList

}
# end of file -- front.tcl --

