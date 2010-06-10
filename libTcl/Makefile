# -*- Mode: Makefile -*-

# Copyright ©1996, 1997, 1998 The Regents of the University of
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

# -*- Mode: Makefile -*-

# Makefile.in -- Compiler Makefile (in 'configure' format) for SHIC
# project.
#
# Autoconf/Configure tag.
# libTcl/Makefile.  Generated from Makefile.in by configure.


#==============================================================================
# Standard (GNU form) variable setup

# General Variables

SHELL = /bin/sh

CC    = gcc
CXX   = @CXX@

# Installation Variables

prefix      = /usr/local
exec_prefix = ${prefix}

bindir      = ${exec_prefix}/bin
includedir  = ${prefix}/include
libdir      = ${exec_prefix}/lib
libexecdir  = ${exec_prefix}/libexec

mandir      = ${prefix}/share/man


# Build Variables

top_srcdir  = ..
srcdir      = .

VPATH       = ../include

# Which libraries to use

LIBS = -LSocket 


# Change the following if needed

# X Window System Stuff

XWSYSTEMLIBDIR     = /usr/X11/lib
XWSYSTEMINCLUDEDIR = 


# TCL/TK

TCLLIBDIR = 
TKLIBDIR  = 

TCLINCLUDEDIR = 
TKINCLUDEDIR  = 


# BLT

BLTLIBDIR     = 
BLTINCLUDEDIR = 


# Directory locations

INCLUDEDIRS = -I. \
              -I../socket \
              -I$(TCLINCLUDEDIR) \
              -I$(TKINCLUDEDIR) \
              -I$(BLTINCLUDEDIR) \
              -I$(includedir) \
              -I$(XWSYSTEMINCLUDEDIR) \
              -I/usr/sww/share/include


LIBDIRS = -L$(TCLLIBDIR) \
          -L$(TKLIBDIR) \
          -L$(BLTLIBDIR) \
          -L/usr/sww/lib \
          -L/usr/local/lib \
          -L$(XWSYSTEMLIBDIR) \
          -L../socket


# collect lots of flags together

CFLAGS  = -g -O -I. -I../include
LDFLAGS = $(LIBDIRS)


# Default rules 

#.SUFFIXES:
#.SUFFIXES: .c .o
#.SUFFIXES: .C .o $(SUFFIXES)

# Default rules for compiling

.C.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c $*.C

.c.o:
	$(CC) $(CFLAGS) $(INCLUDEDIRS) -c $*.c 

OBJECTS    =
APPOBJECTS = tkshift-util.o tkAppInit.o communication.o animation-update.o

# Standard targets

all: TkShift


clean:
	rm -f TkShift _* *._* *.o *.sch core a.out


distclean: clean
	if test -d obsolete; then rm -fr obsolete; fi
	-rm *~ \#*


# The next target MUST be fixed
# (Ensure presence of TkShift subdirectory and make sure INSTALLxx
# variables are properly defined.)
# Marco Antoniotti 19960909

install: TkShift
	if test ! -d $(libdir)/TkShift; \
	then mkdir $(libdir)/TkShift; \
	fi
	if test ! -d $(libdir)/TkShift/images; \
	then mkdir $(libdir)/TkShift/images; \
	fi
	cp TkShift $(bindir)
	-cp *.tcl $(libdir)/TkShift
	-cp *.bit $(libdir)/TkShift
	-cp *.xbm $(libdir)/TkShift
	-cp images/*.bit $(libdir)/TkShift/images
	-cp images/*.gif $(libdir)/TkShift/images
	-cp images/*.xbm $(libdir)/TkShift/images
	-cp images/*.jpg $(libdir)/TkShift/images


uninstall:
	rm $(bindir)/TkShift
	@echo Uninstalling '.tcl' files from $(libdir)/TkShift
	for tclf in *.tcl; do rm $(libdir)/TkShift/$$tclf; done


# Specialized targets

TkShift: $(APPOBJECTS) 
	$(CC) $(CFLAGS) -o TkShift  $(APPOBJECTS) $(LDFLAGS) $(LIBS)


# Compilation dependecies

tkAppInit.o: tkAppInit.c tkAppInit.h 
communication.o: communication.c tkshift-util.h
animation-update.o: animation-update.c tkshift-util.h
tkshift-util.o: tkshift-util.h

# end of file -- Makefile --
