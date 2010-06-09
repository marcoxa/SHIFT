# Generated automatically from Makefile.in by configure.
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

# Makefile for SHIFT run time subsystem
#
# Autoconf/Configure tag.
# Generated automatically from Makefile.in by configure.


#==============================================================================
# Standard (GNU form) variable setup

# General Variables

SHELL = /bin/sh

CC    = gcc


# Installation Variables

prefix      = /home/porsche2/varaiya/marcoz
exec_prefix = ${prefix}

bindir      = ${exec_prefix}/bin
includedir  = ${prefix}/include
libdir      = ${exec_prefix}/lib
libexecdir  = ${exec_prefix}/libexec

mandir      = ${prefix}/man


# Build Variables

top_srcdir  = ..
srcdir      = .

VPATH       = .:../include

LIBS        = -lBLT -ltk -ltcl -lX11 -lsocket -lnsl -lm -ldl 


# Other Variables

GCINCLUDEFLAG = 

CSRC =	shift.c \
        integrate.c \
        before_advance.c \
        after_advance.c \
	before_transition.c \
        after_transition.c \
	discrete.c \
	collection.c \
        watch.c \
        built-ins.c \
	information.c \
	user_functions.c \
	ffi-support.c \
	shift_args.c

C       = $(CSRC)
OBJECTS = $(C:.c=.o)

CFLAGS = -g -O -I. -I../include $(GCINCLUDEFLAG)


# Standard targets

all: shift.a


clean:
	rm -f $(OBJECTS) shift.a


distclean: clean
	if test -d obsolete; then rm -fr obsolete; fi
	-rm *~ \#*


install: shift.a
	cp -p shift.a $(libdir)


uninstall:
	rm $(libdir)/shift.a


# Special targets

make-depend: $(C)
	$(CC) $(CFLAGS) -M $(C) > depend


shift.a: $(OBJECTS)
	rm -f shift.a
	$(AR) cr shift.a $(OBJECTS)
	-ranlib shift.a


include depend

# end of file -- Makefile --
