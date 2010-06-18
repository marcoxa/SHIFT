# -*- Mode: Makefile -*-

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


# -*- Mode: Makefile -*-

# Makefile.in -- Compiler Makefile (in 'configure' format) for SHIC
# project.
#
# Autoconf/Configure tag.
# socket/Makefile.  Generated from Makefile.in by configure.


#==============================================================================
# Standard (GNU form) variable setup

# General Variables

SHELL = /bin/sh

CC    = gcc
CXX   = @CXX@


# Installation Variables

prefix      = /opt/local
exec_prefix = /opt/local

bindir      = ${exec_prefix}/bin
includedir  = ${prefix}/include
libdir      = ${exec_prefix}/lib
libexecdir  = ${exec_prefix}/libexec

mandir      = ${prefix}/share/man


# Build Variables

top_srcdir  = ..
srcdir      = .

VPATH       = ../include


# Clear this for dynamic linkage on sun platform

STATIC      =

# Which libraries to use

LIBS        = 


# collect lots of flags together

GCINCLUDEFLAG = 

CCFLAGS     = $(HDIR) $(O) $(CHECK) $(EXTRA) 
LDFLAGS     = -I -I. -I../include $(GCINCLUDEFLAG) $(STATIC)


# Default rules 
.SUFFIXES: .C ._cxx .c

#   default rule for compiling
.C.o:
	$(CXX) $(CCFLAGS) -c $*.C

#   default rule for interpreting
.C._cxx:
	$(CXX) +Ionly $(CCFLAGS) -c $*.C
	#load $*._cxx

.c.o:
	$(CC) -Dcc -g -c $*.c $(LDFLAGS)


CFILES     = socket.c init.c shift_socket_wrappers.c
OBJECTS    =
APPOBJECTS = socket.o init.o shift_socket_wrappers.o

TARGET_OBJ = libSocket.a


# Standard targets

all: $(TARGET_OBJ)

clean:
	rm -f _* *._* *.o *.sch core *.a

distclean: clean
	if test -d obsolete; then rm -fr obsolete; fi
	-rm -fr *~ \#*


install: $(TARGET_OBJ)
	cp -p $(TARGET_OBJ) $(libdir)


uninstall:
	rm $(libdir)/$(TARGET_OBJ)


# Special targets

make-depend:
	$(CC) -M $(LDFLAGS) $(CFILES) > depend


socket.o : socket.c
init.o : init.c
shift_socket_wrappers.o : shift_socket_wrappers.c

$(TARGET_OBJ): $(APPOBJECTS)
	rm -f $(TARGET_OBJ)
	ar cr $(TARGET_OBJ) $(APPOBJECTS)
	-ranlib $(TARGET_OBJ)


include depend

# end of file -- Makefile
# socket/Makefile.  Generated from Makefile.in by configure.
