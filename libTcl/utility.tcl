# -*- Mode: Tcl -*-

# utilities.tcl --

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
#
#
#
#############################################################################

proc find_socket_numbers {} {

    global SOCKET1 SOCKET2 SERVER dir env

    set str ""
    catch {set str $env(SHIFT_GUI_SERVER)}
    if {$str != ""} {
	set line $str
			    
	set l2 [string first " " $str]
	set SERVER [string range $str 0 [expr $l2 -1]]
	set str2 [string range $str [expr $l2 +1] end]
	set l3 [string first " " $str2]
	set SOCKET1 [string range $str2 0 [expr $l3 -1]]
	set SOCKET2 [string range $str2 [expr $l3 +1] end]

 	puts "Using server:$SERVER with sockets 5$SOCKET1 and 5$SOCKET2"
    } else {
	    tk_dialog .error Error "SHIFT_GUI_SERVER environment\
		    variable is not set. Cannot setup socket\
		    connections. Exiting." error 0 OK
	exit
    }
}

#############################################################################
#
#
#
#############################################################################


proc Credits {} {

    global Fonts

    toplevel .credits
    frame .credits.top 
    frame .credits.middle -relief raised -bd 2
    frame .credits.bottom

    pack .credits.top 
    pack .credits.middle -expand 1 -fill both
    pack .credits.bottom


    label .credits.top.top -text "SHIFT and TkShift are brought to you by:" 

    label .credits.middle.name1 -text "Marco Antoniotti" -font $Fonts(OtherFont) 
    label .credits.middle.name2 -text "Akash Deshpande" -font $Fonts(OtherFont)

    label .credits.middle.name3 -text "Datta Godbole" -font $Fonts(OtherFont)
    label .credits.middle.name4 -text "Tolga G. Goktekin" -font $Fonts(OtherFont)
    label .credits.middle.name5 -text "Aleks Gollu" -font $Fonts(OtherFont)
    label .credits.middle.name6 -text "Michael Kourjanski" -font $Fonts(OtherFont)
    label .credits.middle.name61 -text "Jim Misener" -font $Fonts(OtherFont)
    label .credits.middle.name7 -text "Luigi Semenzato" -font $Fonts(OtherFont)
    label .credits.middle.name8 -text "Raja Sengupta" -font $Fonts(OtherFont)
    label .credits.middle.name81 -text "Tunc Simsek" -font $Fonts(OtherFont)
    label .credits.middle.name9 -text "Joao Sousa" -font $Fonts(OtherFont)
    label .credits.middle.name10 -text "D. Swaroop" -font $Fonts(OtherFont)
    label .credits.middle.name11 -text "Pravin Varaiya" -font $Fonts(OtherFont)
    label .credits.middle.name12 -text "Daniel Wiesmann" -font $Fonts(OtherFont)
    label .credits.middle.name13 -text "Sergio Yovine" -font $Fonts(OtherFont)
    label .credits.middle.name14 -text "Marco Zandonadi" -font $Fonts(OtherFont)

    pack .credits.top.top \
	    .credits.middle.name1 \
	    .credits.middle.name2 \
	    .credits.middle.name3 \
	    .credits.middle.name4 \
	    .credits.middle.name5 \
	    .credits.middle.name6 \
	    .credits.middle.name61 \
	    .credits.middle.name7 \
	    .credits.middle.name8 \
	    .credits.middle.name81 \
	    .credits.middle.name9 \
	    .credits.middle.name10 \
	    .credits.middle.name11 \
	    .credits.middle.name12 \
	    .credits.middle.name13 \
	    .credits.middle.name14 \
	    -side top \
	    -pady 1m \
	    -padx 1m \
	    -fill x

    button .credits.bottom.ok -text "OK" -command "destroy .credits"
    pack .credits.bottom.ok
}


############################################################################
#
#
#
############################################################################

proc Copyright {} {

    toplevel .copy
    wm minsize .copy 400 450 
    frame .copy.t

    set string "# Copyright (c) 1996, 1997 Regents of the University of California.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#      This product includes software developed by the University of
#      California, Berkeley and the California PATH Program.
# 4. Neither the name of the University nor of the California PATH
#    Program may be used to endorse or promote products derived from
#    this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
"

    text .copy.t.text 
    .copy.t.text insert end $string
    button .copy.b -text "OK" -command "destroy .copy"

    pack .copy.t -fill both -expand 1
    pack .copy.t.text -fill both -expand 1
    pack .copy.b
}



######################################################################
## This is the code that implements the Change Step feature
## 
######################################################################



proc StepOptions {} {

    global Continue_Time ChangeTime messageQueue TkShiftValues

    set w .step
    if {[winfo exists $w] == 1} {
	catch {wm deiconify $w}
	catch {raise $w}	
	return
    }
    

    catch "destroy $w"
    toplevel $w
    wm positionfrom $w program
    wm sizefrom $w program
    wm maxsize $w 1024 768
    wm minsize $w 0 0
    wm title $w {Set Step Number}
    # either of these is optional:
    wm transient $w .
    # or
    wm overrideredirect $w

    #  Build the dialog, finally including a frame like this:
    
    # build widget $w.bottom
    frame $w.main -relief raised -borderwidth {2}
    frame $w.bottom -borderwidth {2}

    label $w.main.label -text "Steps:"
    set previous_value $TkShiftValues(step)
    entry $w.main.entry -textvariable TkShiftValues(step)
    trace variable TkShiftValues(step) w forceInt
    set TkShiftValues(step) $TkShiftValues(step)
  # build widget $w.bottom.button20
    button $w.bottom.button20 -padx {9} -pady {3} -text {Ok}\
	    -command "SendStepValue;destroy .step"
    
    bind $w <Return> "SendStepValue;destroy .step" 

    # build widget $w.bottom.button21
    button $w.bottom.button21 -command "set TkShiftValues(step)\
	    $previous_value; destroy $w" -padx {9} \
	    -pady {3} -text {Cancel}

    pack configure $w.bottom.button20 -expand 1 -side left
    pack configure $w.bottom.button21 -expand 1 -side left

    pack configure $w.main.label -side left
    pack configure $w.main.entry -expand 1 -side left

    pack configure $w.main -expand 1 -fill x
    
    pack configure $w.bottom -fill x

    # wait for dialog to become visible
    update idle
    # make it the exclusive I/O thing
    grab $w
    # and wait for it to go away
    tkwait window $w

}

proc SendStepValue {} {
    
    global messageQueue TkShiftValues

    lappend messageQueue 6005
    set StepSize $TkShiftValues(step)

#### Commented out by Tolga 102497
#    .middle.main.step configure -text "Step $StepSize"

    set s [expr $StepSize -1]
    set msg [padMessage $s]
    lappend messageQueue $msg
}

proc SendTimeValue {} {

    global ChangeTime messageQueue TkShiftValues

    lappend messageQueue 6006
    set TimeSize $TkShiftValues(time)

#### Commented out by Tolga 102497
#    .middle.main.timestep configure -text "Time $TimeSize"

    ### New time size button (Tolga 102497)
#    .control.timesize configure -text "$TimeSize"

    set s $TimeSize
    set msg [padMessage $s]
    #puts "sending $msg"
    lappend messageQueue $msg
} 


proc TimeOptions {} {

    global Continue_Time ChangeTime messageQueue TkShiftValues

    set w .step
    if {[winfo exists $w] == 1} {
	catch {wm deiconify $w}
	catch {raise $w}	
	return
    }
    

    catch "destroy $w"
    toplevel $w
    wm positionfrom $w program
    wm sizefrom $w program
    wm maxsize $w 1024 768
    wm minsize $w 0 0
    wm title $w {Set Time}
    # either of these is optional:
    wm transient $w .
    # or
    wm overrideredirect $w

    #  Build the dialog, finally including a frame like this:
    
    # build widget $w.bottom
    frame $w.main -relief raised -borderwidth {2}
    frame $w.bottom -borderwidth {2}
    set previous_value $TkShiftValues(time)
    label $w.main.label -text "Time:"
    entry $w.main.entry -textvariable TkShiftValues(time)
    trace variable TkShiftValues(time) w forceReal
    set TkShiftValues(time) $TkShiftValues(time)
  # build widget $w.bottom.button20
    button $w.bottom.button20 -padx {9} -pady {3} -text {Ok}\
	    -command "SendTimeValue;destroy .step"
    
    bind $w <Return> "SendTimeValue;destroy .step" 

    # build widget $w.bottom.button21
    button $w.bottom.button21 -command "set TkShiftValues(time)\
	    $previous_value;destroy $w" -padx {9} \
	    -pady {3} -text {Cancel}

    pack configure $w.bottom.button20 -expand 1 -side left
    pack configure $w.bottom.button21 -expand 1 -side left

    pack configure $w.main.label -side left
    pack configure $w.main.entry -expand 1 -side left

    pack configure $w.main -expand 1 -fill x
    
    pack configure $w.bottom -fill x

    # wait for dialog to become visible
    update idle
    # make it the exclusive I/O thing
    grab $w
    # and wait for it to go away
    tkwait window $w

}


proc ZenoOptions {} {

    global Continue_Time ChangeTime messageQueue TkShiftValues

    set w .zeno
    if {[winfo exists $w] == 1} {
	catch {wm deiconify $w}
	catch {raise $w}	
	return
    }
    

    catch "destroy $w"
    toplevel $w
    wm positionfrom $w program
    wm sizefrom $w program
    wm maxsize $w 1024 768
    wm minsize $w 0 0
    wm title $w {Set Zeno Level}
    # either of these is optional:
    wm transient $w .
    # or
    wm overrideredirect $w

    #  Build the dialog, finally including a frame like this:
    
    # build widget $w.bottom
    frame $w.main -relief raised -borderwidth {2}
    frame $w.bottom -borderwidth {2}
    set previous_value $TkShiftValues(zeno)
    label $w.main.label -text "Zeno steps:"
    entry $w.main.entry -textvariable TkShiftValues(zeno)
    trace variable TkShiftValues(zeno) w forceInt
    set TkShiftValues(zeno) $TkShiftValues(zeno)
  # build widget $w.bottom.button20
    button $w.bottom.button20 -padx {9} -pady {3} -text {Ok}\
	    -command "SendZenoValue;destroy .zeno"
    
    bind $w <Return> "SendZenoValue;destroy .zeno" 

    # build widget $w.bottom.button21
    button $w.bottom.button21 -command "set TkShiftValues(zeno)\
	    $previous_value;destroy $w" -padx {9} \
	    -pady {3} -text {Cancel}

    pack configure $w.bottom.button20 -expand 1 -side left
    pack configure $w.bottom.button21 -expand 1 -side left

    pack configure $w.main.label -side left
    pack configure $w.main.entry -expand 1 -side left

    pack configure $w.main -expand 1 -fill x
    
    pack configure $w.bottom  -fill x

    # wait for dialog to become visible
    update idle
    # make it the exclusive I/O thing
    grab $w
    # and wait for it to go away
    tkwait window $w

}
proc SendZenoValue {} {

    global TkShiftValues messageQueue

    lappend messageQueue 6003
    set ZenoSize $TkShiftValues(zeno)
    set msg [padMessage $ZenoSize]
    #puts "sending $msg"
    lappend messageQueue $msg

}


proc forceInt {name el op} {
  global $name ${name}_int
  if {$el != ""} {
    set old  "${name}_int\($el)"
    set name "$name\($el)"
  } else { set old "${name}_int" }
  if ![regexp {^[-+]?[0-9]*$} [set $name]] {
    set $name [set $old]
    bell; return
  }
  set $old [set $name]
}


proc forceReal {name el op} {
  global $name ${name}_real
  if {$el != ""} {
    set old  "${name}_real\($el)"
    set name "$name\($el)"
  } else { set old "${name}_real" }
  if ![regexp {^[-+]?[0-9]*\.?[0-9]*([0-9]\.?e[-+]?[0-9]*)?$} [set $name]] {
    set $name [set $old]
    bell; return
  }
  set $old [set $name]
}

proc forceRegexp {regexp name el op} {
  global $name ${name}_regexp
  if {$el != ""} {
    set old  "${name}_regexp\($el)"
    set name "$name\($el)"
  } else { set old "${name}_regexp" }
  if ![regexp "$regexp" [set $name]] {
    set $name [set $old]
    bell; return
  }
  set $old [set $name]
}

proc forceAlpha {name el op} {
  global $name ${name}_alpha
  if {$el != ""} {
    set old  "${name}_alpha\($el)"
    set name "$name\($el)"
  } else { set old "${name}_alpha" }
  if ![regexp {^[a-zA-Z]*$} [set $name]] {
    set $name [set $old]
    bell; return
  }
  set $old [set $name]
}

proc forceLen {len name el op} {
  global $name ${name}_len
  if [string comp $el {}] {
    set old  ${name}_len\($el)
    set name $name\($el)
  } else { set old ${name}_len }
  if {[string length [set $name]] > $len} {
    set $name [set $old]
    bell; return
  }
  set $old [set $name]
}


set _ran [pid]
proc random {range} {
  global _ran
  set _ran [expr ($_ran * 9301 + 49297) % 233280]
  return [expr int($range * ($_ran / double(233280)))]
}


# end of file -- utility.tcl --


