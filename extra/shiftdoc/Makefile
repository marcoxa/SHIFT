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

# Makefile.in -- Compiler Makefile (in 'configure' format) for SHIC
# project.
#
# Autoconf/Configure tag.
# extra/shiftdoc/Makefile.  Generated from Makefile.in by configure.


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

mandir      = ${prefix}/share/man


# Build Variables

top_srcdir  = ../..
srcdir      = .

VPATH       = ../../include


# Other Variables

# Unused for the time being
GCINCLUDEFLAG = 

CSRC = main.c \
       scanaux.c \
       lisp.c \
       dcScanaux.c \
       check.c \
       type.c \
       transform.c

HSRC = lisp.h shic.h scanner.h dcScanaux.h

C = $(CSRC) scanner.c parser.c

O = $(C:.c=.o) symbols.o

CFLAGS = -g -I../../include

SOURCES = README Makefile $(CSRC) $(HSRC) parser.y scanner.l \
	  concat.c


# Generic target for preprocessing by 'hasher'.

.c.o:
	$(CC) $(CFLAGS) -E $< | ./hasher SYMBOLS > X.c
	$(CC) $(CFLAGS) -c X.c
	mv -f X.o $*.o; rm -f X.c SYMBOLS.h


# The dependency on parser.h forces (hopefully) making parser.h first.
# Can't think of a better way.

all: parser.h hasher shiftdoc

symbols.o: hasher SYMBOLS
	./hasher SYMBOLS symbols.c
	$(CC) $(CFLAGS) -c symbols.c
	rm -f symbols.c

hasher: hasher.c lisp.c concat.c lisp.h
	$(CC) $(CFLAGS) -o hasher hasher.c lisp.c concat.c
	rm -f concat.o

shiftdoc: $(O)
	$(CC) $(CFLAGS) $(O) -o shiftdoc $(YACCLIB) $(LEXLIB)

parser.h: y.tab.h
	cmp -s parser.h y.tab.h || mv y.tab.h parser.h

parser.c y.tab.h: parser.y
	$(YACC) -d parser.y
	mv y.tab.c parser.c

make-depend: $(C) parser.h
	$(CC) -M $(CFLAGS) $(C) > depend

clean:
	-rm -f $(O) parser.c parser.h y.tab.h scanner.c hasher shiftdoc


distclean: clean
	-rm *~ \#*
	if test -d obsolete; then rm -fr obsolete; fi


include depend

install: shiftdoc
	cp shiftdoc $(bindir)

uninstall:
	rm -f $(bindir)/shiftdoc

# end of file -- (produced from) Makefile.in --



