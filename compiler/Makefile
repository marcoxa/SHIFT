# -*- Mode: Makefile -*-

# Copyright (c)1996, 1997, 1998 The Regents of the University of
# California (Regents). All Rights Reserved. 
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for educational, research, and not-for-profit purposes,
# without fee and without a signed licensing agreement, is hereby
# granted, provided that the above copyright notice, this paragraph and
# the following two paragraphs appear in all copies, modifications, and
# distributions. 
#
# Contact The Office of Technology Licensing, UC Berkeley, 2150 Shattuck
# Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
# commercial licensing opportunities. 
#
# IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
# SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
# ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
# REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
#
# REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE.  THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
# ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION
# TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
# MODIFICATIONS. 


# -*- Mode: Makefile -*-

# Makefile.in -- Compiler Makefile (in 'configure' format) for SHIC
# project.
#
# Autoconf/Configure tag.
# compiler/Makefile.  Generated from Makefile.in by configure.


#==============================================================================
# Standard (GNU form) variable setup

# General Variables

SHELL  = /bin/sh

CC     = gcc
LEX    = flex
YACC   = yacc

LEXLIB = -lfl

# LIBYACC is needed because of the way LINUX systems run yacc (or bison).
YACCLIB = 


# Installation Variables

prefix      = /opt/local
exec_prefix = /opt/local

bindir      = ${exec_prefix}/bin
includedir  = ${prefix}/include
libdir      = ${exec_prefix}/lib
libexecdir  = ${exec_prefix}/libexec
datarootdir	    = ${prefix}/share

mandir      = ${datarootdir}/man


# Build Variables

top_srcdir  = ..
srcdir      = .

VPATH       = ../include


# Other Variables

# Unused for the time being
GCINCLUDEFLAG = 

# CSRC = main.c \
#        utilities.c \
#        scanaux.c \
#        lisp.c \
#        check.c \
#        type.c \
#        generate.c \
#        gen-iterators.c \
#        ffi-generators.c \
#        gen-special-forms.c \
#        transform.c
#
# HSRC = lisp.h shic.h scanner.h

CSRC = main.c \
       utilities.c \
       scanaux.c \
       check.c \
       type.c \
       generate.c \
       gen-iterators.c \
       ffi-generators.c \
       gen-special-forms.c \
       transform.c

HSRC = shic.h scanner.h

C = $(CSRC) scanner.c parser.c

# O = $(C:.c=.o) symbols.o
O = $(C:.c=.o)

CFLAGS	= -g -I../include -I${prefix}/include -L${exec_prefix}/lib
LDFLAGS	= -lcrscl

SOURCES = README Makefile $(CSRC) $(HSRC) parser.y scanner.l \
	  hasher.c concat.c


# Generic target for preprocessing by 'hasher'.

# .c.o:
# 	$(CC) $(CFLAGS) -E $< | ./hasher SYMBOLS > X.c
# 	$(CC) $(CFLAGS) -c X.c
# 	mv -f X.o $*.o; rm -f X.c SYMBOLS.h

.c.o:
	$(CC) $(CFLAGS) -c $<


# The dependency on parser.h forces (hopefully) making parser.h first.
# Can't think of a better way.

all: parser.h shic shiftprecpp shift_config.h


symbols.o: hasher SYMBOLS
	./hasher SYMBOLS symbols.c
	$(CC) $(CFLAGS) -c symbols.c
	#	$(RM) symbols.c


shiftprecpp: shiftprecpp.c
	$(CC) $(CFLAGS) -o $@ shiftprecpp.c


# hasher: hasher.c lisp.c concat.c lisp.h
# 	$(CC) $(CFLAGS) -o hasher hasher.c lisp.c concat.c
# 	rm -f concat.o

hasher: hasher.c concat.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o hasher hasher.c concat.c
	$(RM) -f concat.o


shic: $(O)
	$(CC) $(CFLAGS) $(LDFLAGS) $(O) -o shic $(YACCLIB) $(LEXLIB)

parser.h: y.tab.h
	cmp -s parser.h y.tab.h || mv y.tab.h parser.h

parser.c y.tab.h: parser.y
	$(YACC) -d parser.y
	cp y.tab.c parser.c
	#	mv y.tab.c parser.c


make-depend: $(C) parser.h shift_config.h
	$(CC) -M $(CFLAGS) $(C) > depend

clean:
	-rm -f $(O) parser.c parser.h y.tab.h scanner.c hasher shic shiftprecpp


distclean: clean
	-rm *~ \#*
	-if test -d obsolete; then rm -fr obsolete; fi


include depend

install: shic shiftprecpp
	cp shic shiftprecpp $(bindir)


uninstall:
	$(RM)$ -f $(bindir)/shic $(bindir)/shiftprecpp

SHIP:
	rcp -pr $(SOURCES) $(D)

X:
	$(C) $(SOURCES)

# end of file -- (produced from) Makefile.in --
