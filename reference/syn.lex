%{
/*
 * Copyright (c)1996, 1997, 1998 The Regents of the University of
 * California (Regents). All Rights Reserved. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research, and not-for-profit purposes,
 * without fee and without a signed licensing agreement, is hereby
 * granted, provided that the above copyright notice, this paragraph and
 * the following two paragraphs appear in all copies, modifications, and
 * distributions. 
 *
 * Contact The Office of Technology Licensing, UC Berkeley, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
 * commercial licensing opportunities. 
 *
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
 * ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION
 * TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS. 
 */
    int lineno = 1;
%}
%state S
%%
^---syntax\n		{ printf ("\\begin{center}\n\\begin{tabular}{rl}\n");
			  BEGIN S; lineno++; }
<S>^---xatnys\n		{ printf ("\\end{tabular}\n\\end{center}\n%");
			  BEGIN 0; lineno++; }
<S>" "			{ printf (" "); }
<S>=>			{ printf ("$\\quad\\Rightarrow$ &"); }
<S>"\\{"		{ printf ("\\verb.{."); }
<S>"\\}"		{ printf ("\\verb.}."); }
<S>"["			{ printf ("["); }
<S>"]"			{ printf ("]"); }
<S>"{"			{ printf ("$\\{$"); }
<S>"}+"			{ printf ("$\\}^+$"); }
<S>"}*"			{ printf ("$\\}^*$"); }
<S>\|			{ printf ("$|$"); }
<S>\\[^\]\{\}\\ \n]*	{ printf ("{\\tok %s}", yytext+1); }
<S>\n			{ printf ("\\\\\n"); lineno++; }
<S>[a-z][a-z-]*		{ printf ("{\\nont %s}", yytext); }
<S>^\t\|		{ printf ("\t$|$ &"); }
\n			{ lineno++; printf("\n"); }
<S>.			{ fprintf (stderr, "bad character %c at line %d\n", yytext[0], lineno); 
			  exit(1); }
