# -*- Mode: Tcl -*-

# parameters.tcl --

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

##The initialization data that follows is used for the animation package

set Colors_Used {Yellow Blue Red Green Gray Orange Black White Brown}
set Graph_Colors_Used {Yellow Blue Red LightGrey Green Orange Black Purple Brown}

set ZoomIndex(0) 1
set ZoomIndex(1) 1


#### Other Configure Parameters

##Initialize the message queue to be empty
set messageQueue {}
## Initialize the list of open windows to be empty
set PackedData {}
## Initialize the list 
set GraphedElement {}
set TimeVectors {}
set UnpackedData {}

set CanvasToUse DemoRoad
set ImageToUse rectangle


set IntegrationStep fixed
set StepSize 1
set CanvasType "VREP"
set CanvasXPlot "gxp:ALL"
set CanvasYPlot "gyp:ALL"
set AutoConfigOn 1
set InstanceCloseupsBox 0
set FirstUnit 0
set totalCanvases 0
set totalGraphs 0
set Time 0
set TimeClick 0
set SimulationDirectory .
set TEMPLATE template
set GRAPHFORMAT GraphConfigFile
set init 0
set add_to_socket 0
set BreakNumber 0
set RecordingAnimationInfo 0
set SocketAddress 5550
set ServerName [exec hostname]
set OpenWindows {}
set TotalGraphNumber 0
set GraphNames(0) "empty"

set defFile def
set TimeStepSize 1
set PRINT_DEBUG 0
set home_directory [pwd]
set isStartUp 0
set Orient 0
set Decor 1
set defaultGraphName graph.ps
set defaultDumpName dumpFile
set TkShiftValues(zeno) 100
set TkShiftValues(step) 1
set TkShiftValues(time) 1
######  Message codes ######
set Continue "1111"
set Stop "2222"
set GetShiftTypes "3333"
set ListComponents "4444"
set PrintInstance "5555"
set ChangeStep "6005"
set ChangeTime "6006"
set Step "6001"
set TimeStep "6002"
set BreakType "8000"
set BreakComp "8003"

#############################################################################
#
# These are summary of the codes that I found from Tk_server.c 
# 
# 9999 Quit
# 1111 Cont
# 2222 Stop
# 5559 PrintValue 
# 3333 PrintTypes
# 4444 Listcomponents
# 5505 PrintInstance
# 5900 ValuesOfAll
# 5777 RemoveInstance
# 5778 RemoveGType
# 5556 ListInstances 
# 6001 Time
# 6003 ChangeZeno
# 6005 ChangeStep
# 6006 ChangeTime
# 8000 BreakType
# 8002 UnbreakType
# 8003 BreakComp
# 8004 ClearComp
# 8005 TraceComp
# 8006 ClearCompTrace
# 8765 CanvasOn
#
#############################################################################



#####Font and color options #######


#### Commented out by Tolga 111897
#### Old Font format
# set Fonts(SmallFont) -misc-fixed-medium-r-normal--*-100-*-100-*-*-*-*
# set Fonts(OtherFont) -adobe-times-bold-i-*-*-*-180-*-*-*-*-*-*
# set Fonts(MediumFont) -adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*
# set Fonts(MainAdobe) -adobe-helvetica-bold-r-*-*-16-*-*-*-*-*-*-*
# set Fonts(TextAdobe) -adobe-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*
# set Fonts(Dummy) -adobe-times-medium-r-normal--18-180-75-75-p-94-iso8859-1
# 
# set font_list [list SmallFont OtherFont MediumFont MainAdobe TextAdobe Dummy]
# foreach font $font_list {
#     set err [catch {eval label .a$font -text "Test" -font $Fonts($font)}]
#     if {$err == 1} {
# 	set Fonts($font) fixed
#     }
#     #puts "The $font is $Fonts($font)"
# }


##################  Options Added by Tolga (111897) ##################
font create Font(SmallFont) -family fixed -size 10
font create Font(OtherFont) -family times -slant italic -weight bold -size 12
font create Font(MediumFont) -family helvetica -weight bold -size 12
font create Font(MainAdobe) -family helvetica -weight bold -size 16
font create Font(TextFont) -family helvetica -weight bold -size 14
font create Font(system_bold) -family system -weight bold -size 10
font create Font(system) -family system
font create Font(ScoreBoard) -family terminal -size 14
font create Font(ScoreBoardTiny) -family system -size 6 

set Fonts(MediumFont) Font(MediumFont)
set Fonts(OtherFont) Font(OtherFont)
set Fonts(SmallFont) Font(SmallFont)
set Fonts(TextFont) Font(TextFont)
set Fonts(MainAdobe) Font(MainAdobe)


option add *main_menu*Font Font(system_bold)
option add *status*Font Font(system)
option add TkShift*list*font Font(MediumFont)
option add TkShift*text*font Font(TextAdobe)
option add TkShift*bar*font Font(system_bold)
option add TkShift*small*font Font(SmallFont)

option add TkShift*background LightGrey
option add *main_menu*Background #afafaf
option add *main_menu*activeForeground black
option add *main_menu*activeBackground #cccccc
option add *status*Background #0cbac0
option add *Button*highlightThickness 0
option add TkShift*list*background LightGrey
option add TkShift*text*background LightGrey

option add TkShift*bar*background #afafaf
option add TkShift*bottom*background LightGrey

#### Commented out by Tolga 102397
# option add TkShift*background bisque
# option add TkShift*bar*background LightCoral
# option add TkShift*bottom*background LightCoral


if {[info exists env(SHIFT_ISTEP)]} {
    set Shift_ISTEP $env(SHIFT_ISTEP)
} else {
    set Shift_ISTEP 0.05
    set env(SHIFT_ISTEP) 0.05
}

if {[info exists env(SHIFT_NSTEPS)]} {
    set Shift_NSTEPS $env(SHIFT_NSTEPS)
} else {
    set Shift_NSTEPS 1000000
    set env(SHIFT_NSTEPS) 1000000
}


# end of file -- parameters.tcl --


