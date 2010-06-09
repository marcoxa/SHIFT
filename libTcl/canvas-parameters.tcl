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




########################################
## These are all the canvases supported
########################################
set Canvases {DemoRoad Generic_500x500 ShopFloor StraightRoad Ocean\
	BasketballCourt SuperMerge BigCanvas CarmmaTrack SmartAHS}

###########################################
## These are all the multipliers supported
###########################################


set CanvasMultipliers {"1" "3" "5" "6" "10" "20" "50" "100" "500"}

#########################################
## These are the images supported
#########################################

set CanvasImages {rectangle oval bitmap}


#########################################
## These are the parameters for DemoRoad
#########################################

set XDefault(DemoRoad) "gxp"
set YDefault(DemoRoad) "gyp"
set TypeDefault(DemoRoad) "VREP"
set MultiDefault(DemoRoad) 6
set ImageDefault(DemoRoad) "rectangle"
set CanvasStructure(DemoRoad,height) 500
set CanvasStructure(DemoRoad,width) 1200
set CanvasStructure(DemoRoad,region) {0 0 2500 800}
set CanvasStructure(DemoRoad,procedure) DrawRoad

###########################################################
## These are the parameters for the Generic_500x500 canvas
###########################################################

set XDefault(Generic_500x500) "x"
set YDefault(Generic_500x500) "y"
set TypeDefault(Generic_500x500) "Ball"
set ImageDefault(Generic_500x500) "oval"
set MultiDefault(Generic_500x500) 4
set CanvasStructure(Generic_500x500,height) 550
set CanvasStructure(Generic_500x500,width) 550
set CanvasStructure(Generic_500x500,region) {0 -300 600 600}
set CanvasStructure(Generic_500x500,procedure) DrawGeneric500

###########################################################
## These are the parameters for the ShopFloor
###########################################################

set XDefault(ShopFloor) "x"
set YDefault(ShopFloor) "y"
set TypeDefault(ShopFloor) "Tray"
set ImageDefault(ShopFloor) "rectangle"
set MultiDefault(ShopFloor) 10
set CanvasStructure(ShopFloor,height) 400
set CanvasStructure(ShopFloor,width) 1000
set CanvasStructure(ShopFloor,region) {0 0 1200 500}
set CanvasStructure(ShopFloor,procedure) DrawShop


###########################################################
## These are the parameters for Ocean
###########################################################

set XDefault(Ocean) "yx"
set YDefault(Ocean) "yz"
set TypeDefault(Ocean) "M_Dynamic_Model"
set ImageDefault(Ocean) "rectangle"
set MultiDefault(Ocean) 10
set CanvasStructure(Ocean,height) 500
set CanvasStructure(Ocean,width) 1000
set CanvasStructure(Ocean,region) {-100 -100 1200 500}
set CanvasStructure(Ocean,procedure) DrawOcean


##########################################################
## These are the parameters for StraightRoad
###########################################################

set XDefault(StraightRoad) "x"
set YDefault(StraightRoad) "y"
set TypeDefault(StraightRoad) "Particle"
set ImageDefault(StraightRoad) "rectangle"
set MultiDefault(StraightRoad) 1
set CanvasStructure(StraightRoad,height) 300
set CanvasStructure(StraightRoad,width) 1000
set CanvasStructure(StraightRoad,region) {-100 -100 5100 500}
set CanvasStructure(StraightRoad,procedure) DrawStraightRoad


###########################################################
## These are the parameters for the BasketBall court
###########################################################

set XDefault(BasketballCourt) "gxp"
set YDefault(BasketballCourt) "gyp"
set TypeDefault(BasketballCourt) "BBP"
set ImageDefault(BasketballCourt) "oval"
set MultiDefault(BasketballCourt) 6
set CanvasStructure(BasketballCourt,height) 550
set CanvasStructure(BasketballCourt,width) 550
set CanvasStructure(BasketballCourt,region) {0 0 600 600}
set CanvasStructure(BasketballCourt,procedure) DrawBBC


###########################################################
## These are the parameters for Supermerge
###########################################################

set XDefault(SuperMerge) "gxp"
set YDefault(SuperMerge) "gyp"
set TypeDefault(SuperMerge) "VREP"
set ImageDefault(SuperMerge) "rectangle"
set MultiDefault(SuperMerge) 1
set CanvasStructure(SuperMerge,height) 400
set CanvasStructure(SuperMerge,width) 1000
set CanvasStructure(SuperMerge,region) {0 0 4000 500}
set CanvasStructure(SuperMerge,procedure) DrawHouston


###########################################################
## These are the parameters for the DemoOcean
###########################################################

set XDefault(DemoOcean) "y_xp"
set YDefault(DemoOcean) "y_yp"
set TypeDefault(DemoOcean) "M_Ping_Packet"
set ImageDefault(DemoOcean) "oval"
set MultiDefault(DemoOcean) 1


###########################################################
## These are the parameters for the BigCanvas
###########################################################

set XDefault(BigCanvas) "gxp"
set YDefault(BigCanvas) "gyp"
set TypeDefault(BigCanvas) "VREP"
set MultiDefault(BigCanvas) 1
set ImageDefault(BigCanvas) "rectangle"
set CanvasStructure(BigCanvas,height) 300
set CanvasStructure(BigCanvas,width) 1000
set CanvasStructure(BigCanvas,region) {0 -1000 10000 500}
set CanvasStructure(BigCanvas,procedure) DrawBigCanvas

###########################################################
## These are the parameters for the CarmmaTrack
###########################################################

set XDefault(CarmmaTrack) "gxp"
set YDefault(CarmmaTrack) "gyp"
set TypeDefault(CarmmaTrack) "VREP"
set MultiDefault(CarmmaTrack) 10
set ImageDefault(CarmmaTrack) "rectangle"
set CanvasStructure(CarmmaTrack,height) 200
set CanvasStructure(CarmmaTrack,width) 1000
set CanvasStructure(CarmmaTrack,region) {0 0 130000 200}
set CanvasStructure(CarmmaTrack,procedure) DrawCarmmaTrack


###########################################################
## These are the parameters for the SmartAHS
###########################################################

set XDefault(SmartAHS) "gxp"
set YDefault(SmartAHS) "gyp"
set TypeDefault(SmartAHS) "VREP"
set MultiDefault(SmartAHS) 1
set ImageDefault(SmartAHS) "rectangle"
set CanvasStructure(SmartAHS,height) 200
set CanvasStructure(SmartAHS,width) 1000
#set CanvasStructure(SmartAHS,region) {27950 26950 32000 42000}
set CanvasStructure(SmartAHS,region) {-1000 -1000 1000 1000}
set CanvasStructure(SmartAHS,procedure) DrawSmartAHS







