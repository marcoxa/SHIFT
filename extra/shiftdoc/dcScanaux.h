/* -*- Mode: C -*- */

/* dcScanaux.h -- */

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


/* This is the number of string buffers to which the DocComments are
 * written, when found. We need a buffer per type of clause (input,
 * export and so on).
 * 
 * Marco Zandonadi 19970919
 */
#define NUM_PARSE_BUFFERS 11

int docword(char *s);

void writeHeaderHTML (char * outStr, char * typeID, char * superTypeID);

void writeFooterHTML (char * outStr);

void writeTypeHTML (char * outStr, char * typeID, char * sTypeID);

void writeDocTextHTML (char * outStr, char * stringPar);

void writeClauseHeaderHTML (char * outStr, char * stringPar);

void writeClauseFooterHTML (char * outStr);

void writeVarNameHTML (char * outStr, char * stringPar);

void writeExprHTML (char * outStr, char * stringPar);

void writeVarTypeHTML (char * outStr, char * stringPar);

void writeFlowHeaderHTML (char * outStr, char * stringPar);

void writeFlowFooterHTML (char * outStr);

void writeStateHeaderHTML (char * outStr, char * stringPar);

void writeStateFooterHTML (char * outStr);

void writeExportHeaderHTML (char * outStr, char * stringPar);

void writeExportFooterHTML (char * outStr);

void writeRawHTML (char * outStr, char * stringPar);

/* end of file -- dcScanaux.h -- */
