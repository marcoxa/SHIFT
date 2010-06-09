/* -*- Mode: C -*- */

/* dcScanaux.c -- */

/*
 * Copyright (c) 1996, 1997 Regents of the University of California.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and the California PATH Program.
 * 4. Neither the name of the University nor of the California PATH
 *    Program may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * This file is part of the Shiftdoc distribution.
 *
 * 19970922 Marco Zandonadi
 */
#include <stdio.h>
#include "lisp.h"
#include "scanner.h"
#include "parser.h"
#include "dcScanaux.h"

/* docliteral -- This funcion is used by the scanner to return DocComment
 * tokens to the parser.
 *
 * Marco Zandonadi 19970904
 */
int
docword(char *s)
{
  yylval = string(s);

  return DOCWORD;
}

/* This function writes the header of an HTML file associated to a type.
 *
 * outStr: output HTML string.
 * typeID: type name identifier.
 * superTypeID: optional supertype name identifier.
 *
 * Marco Zandonadi 19970905
 */
void
writeHeaderHTML (char * outStr, char * typeID, char * superTypeID)
{
	strcat (outStr, "<HTML>\n<HEAD>\n<TITLE> ");
	strcat (outStr, typeID);

	if (superTypeID) {
		strcat (outStr, " : ");
		strcat (outStr, superTypeID);
	}

	strcat(outStr,"</TITLE>\n</HEAD>\n\n<BODY BGCOLOR=\"#FFFFFF\" ");
	strcat(outStr,"TEXT=\"#00000\" LINK=\"#0000A0\" VLINK=\"#C00000\" ");
	strcat(outStr,"ALINK=\"#FF8000\">\n");
}

void
writeFooterHTML (char * outStr)
{
	strcat(outStr, "</BODY>\n</HTML>\n");
}


/* This function writes the type-related informations to the HTML body.
 *
 * outStr: output HTML string.
 * typeID: type name identifier.
 * superTypeID: optional supertype name identifier.
 *
 * Marco Zandonadi 19970905
 */
void
writeTypeHTML (char * outStr, char * typeID, char * sTypeID)
{
	strcat (outStr, "<CENTER> <H1> Type ");
	strcat (outStr, typeID);

	if (sTypeID) {
		strcat (outStr, " : <A HREF = \"");
		strcat (outStr, sTypeID);
		strcat (outStr, ".html\"> ");
		strcat (outStr, sTypeID);
		strcat (outStr, " </A>");	
}

	strcat (outStr, " </H1> </CENTER> \n");
}


/* This function writes a DocComment text to the HTML body.
 *
 * outStr: output HTML string.
 * stringPar: processed contents of the DocComment.
 *
 * Marco Zandonadi 19970905
 */
void
writeDocTextHTML (char * outStr, char * stringPar)
{
	strcat (outStr, stringPar);
	strcat (outStr, " <P>\n");
}


/* This function writes the header of a clause (e.g. input, export, etc.).
 *
 * outStr: output HTML string.
 * stringPar: clause name
 *
 * Marco Zandonadi 19970905
 */
void
writeClauseHeaderHTML (char * outStr, char * stringPar)
{
	strcat (outStr, "<HR ALIGN=\"center\" SIZE=4 WIDTH=80%>\n"); 
	
	strcat (outStr, "<H2> ");
	strcat (outStr, stringPar);
	strcat (outStr, " </H2>\n<UL>\n");
}


void
writeClauseFooterHTML (char * outStr)
{
	strcat (outStr, "\n</UL>\n<P>\n");
}


void
writeVarNameHTML (char * outStr, char * stringPar)
{
	strcat (outStr, " ");
	strcat (outStr, stringPar);
	strcat (outStr, " ");
}

void
writeExprHTML (char * outStr, char * stringPar)
{
	strcat (outStr, " ");
	strcat (outStr, stringPar);
	strcat (outStr, " ");
}


/* This function writes the last part of the HTML file.
 *
 * outStr: output HTML file.
 *
 * Marco Zandonadi 19970905
 */
void
writeVarTypeHTML (char * outStr, char * stringPar)
{
	strcat (outStr, " <I> ");
	strcat (outStr, stringPar);
	strcat (outStr, " </I> ");
}


void
writeFlowHeaderHTML (char * outStr, char * stringPar)
{
	strcat (outStr, "<LI>\n");
	strcat (outStr, "<H3> ");
	strcat (outStr, stringPar);
	strcat (outStr, " </H3>\n");
	strcat (outStr, "<UL>\n");
}


void
writeFlowFooterHTML (char * outStr)
{
	strcat (outStr, "</UL>\n");
	strcat (outStr, "</LI>\n");
	strcat (outStr, "<P>\n");
}

void
writeStateHeaderHTML (char * outStr, char * stringPar)
{
	strcat (outStr, "<LI>\n");
	strcat (outStr, "<H3> ");
	strcat (outStr, stringPar);
	strcat (outStr, " </H3>\n");
}


void
writeStateFooterHTML (char * outStr)
{
	strcat (outStr, "</LI>\n");
	strcat (outStr, "<P>\n");
}

void
writeExportHeaderHTML (char * outStr, char * stringPar)
{
	strcat (outStr, "<LI>\n");
	strcat (outStr, "<H3> ");
	strcat (outStr, stringPar);
	strcat (outStr, " </H3>\n");
}


void
writeExportFooterHTML (char * outStr)
{
	strcat (outStr, "</LI>\n");
	strcat (outStr, "<P>\n");
}

void
writeRawHTML (char * outStr, char * stringPar)
{
	strcat (outStr, stringPar);
}


/* end of file -- dcScanaux.h -- */

