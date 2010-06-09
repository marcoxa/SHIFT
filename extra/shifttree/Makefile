# Generated automatically from Makefile.in by configure.
# -*- Mode: Makefile -*-

# Copyright (c) 1996, 1997 Regents of the University of California.
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

# -*- Mode: Makefile -*-

# Makefile.in -- Main Makefile (in 'configure' format) for SHIC
# project.
#
# Autoconf/Configure tag.
# Generated automatically from Makefile.in by configure.


#==============================================================================
# Standard (GNU form) variable setup

# General Variables

SHELL = /bin/sh


# Installation Variables

prefix      = /users/marcoxa
exec_prefix = ${prefix}

bindir      = ${exec_prefix}/bin
includedir  = ${prefix}/include
libdir      = ${exec_prefix}/lib
libexecdir  = ${exec_prefix}/libexec

mandir      = ${prefix}/man


# Build Variables

top_srcdir  = ../..
srcdir      = .


# Special directives and variables

# .PHONY works for GNU Make, but not for other Make's.  Therefore we
# also use the dummy target FORCE (see "Application's Targets"section)
# to enforce dependencies.

# .PHONY: include compiler run-time debugger graphic.env ttyonly communications
# .PHONY: include.i compiler.i run-time.i debugger.i
# .PHONY: graphic.env.i ttyonly.i communications.i
# .PHONY: FORCE

# Extra Variables

MAT2SCALARDIR = extra/mat2scalar
SHIFTDOCDIR = extra/shiftdoc
SHIFTTREEDIR = extra/shifttree

# Variables for creating the source distribution

SRC_DISTRIBUTION_FILES = Makefile.in \
			 shifttree \
			 shifttree.html \
			 tree.gif


SRC_DISTRIBUTION_DIRS  = 



#==============================================================================
# Standard Targets

all: 


install: shifttree
	chmod a+x shifttree
	cp shifttree $(bindir)


uninstall:
	-rm -f $(bindir)/shifttree


reinstall: clean install


clean:


distclean:


# mostlyclean: # UNUSED


# maintainer-clean: # UNUSED


# TAGS: # UNUSED


# info: # UNUSED


# dvi: # UNUSED



dist:


distprologue:


# check: # UNUSED


# installcheck: # UNUSED


# installdirs: # UNUSED


#==============================================================================
# Special PATH targets
# These are useless to anybody outside California PATH

bindist: 

make-depend:

# end of file -- Makefile --
