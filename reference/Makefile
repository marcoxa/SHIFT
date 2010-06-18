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

# Makefile.in -- Main Makefile (in 'configure' format) for SHIC
# project.
#
# Autoconf/Configure tag.
# reference/Makefile.  Generated from Makefile.in by configure.


#==============================================================================
# Standard (GNU form) variable setup

# General Variables

SHELL	= /bin/sh

CC	= gcc
LEX	= flex

LEXLIB	= -lfl

LATEX	= latex

# Installation Variables

prefix      = /opt/local

mandir      = ${prefix}/share/man


# Build Variables

top_srcdir  = ..
srcdir      = .

DOCDIR = ../doc


.SUFFIXES: .pic .tex .dvi .ps .aux .bbl .fig .syn

SOURCES = Makefile main.tex overview.tex model.tex language.syn \
	  todo.tex examples.tex syn.lex

.fig.ps:
	fig2dev -Lps $< > $*.ps

.syn.tex: syn
	syn < $< > $@

.dvi.ps:
	dvips $*.dvi -o $*.ps

.tex.dvi:
	if test ! -f $*.aux; then touch $*.aux; fi
	mv -f $*.aux $*.auxx; cp $*.auxx $*.aux
	-$(LATEX) $<
	if cmp -s $*.aux $*.auxx; then mv $*.auxx $*.aux; \
	else rm -f $*.auxx; rm -f $*.dvi; $(MAKE) $*.dvi; fi

all: syn main.dvi

install: main.ps
	cp main.ps $(DOCDIR)/shift-reference.ps

uninstall: clean
	-rm $(DOCDIR)/shift-reference.ps

clean:
	-rm *.ps *.dvi *.aux *.log *.bbl *.blg

distclean: clean


main.dvi: main.tex model.tex language.tex todo.tex examples.tex

syn: syn.lex
	$(LEX) syn.lex
	$(CC) lex.yy.c -o syn $(LEXLIB)


# end of file -- Makefile --
# reference/Makefile.  Generated from Makefile.in by configure.
