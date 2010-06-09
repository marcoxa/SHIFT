/* -*- Mode: C -*- */

/* concat.c -- */

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

/*
 * This file is part of the SHIFT compiler.
 *
 * Originally written by Edward Wang (edward@edcom.com)
 * Modified 1995-96 by Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */

/*	@(#)concat.c	1.8 7/23/95	*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>

static char *concat_buf = 0;
static concat_bufsiz = 0;

char *
concat2(s1, s2)
	char *s1, *s2;
{
	int l1 = strlen(s1);
	int l2 = strlen(s2) + 1;
	int l = l1 + l2;
	char *p;

	if (l > concat_bufsiz) {
		if ((concat_bufsiz = l) < 512)
			concat_bufsiz = 512;
		if (concat_buf != 0)
			free(concat_buf);
		concat_buf = malloc(concat_bufsiz);
	}
	memcpy(p = concat_buf, s1, l1);
	memcpy(p += l1, s2, l2);
	return concat_buf;
}

char *
concat3(s1, s2, s3)
	char *s1, *s2, *s3;
{
	int l1 = strlen(s1);
	int l2 = strlen(s2);
	int l3 = strlen(s3) + 1;
	int l = l1 + l2 + l3;
	char *p;

	if (l > concat_bufsiz) {
		if ((concat_bufsiz = l) < 512)
			concat_bufsiz = 512;
		if (concat_buf != 0)
			free(concat_buf);
		concat_buf = malloc(concat_bufsiz);
	}
	memcpy(p = concat_buf, s1, l1);
	memcpy(p += l1, s2, l2);
	memcpy(p += l2, s3, l3);
	return concat_buf;
}

char *
concat4(s1, s2, s3, s4)
	char *s1, *s2, *s3, *s4;
{
	int l1 = strlen(s1);
	int l2 = strlen(s2);
	int l3 = strlen(s3);
	int l4 = strlen(s4) + 1;
	int l = l1 + l2 + l3 + l4;
	char *p;

	if (l > concat_bufsiz) {
		if ((concat_bufsiz = l) < 512)
			concat_bufsiz = 512;
		if (concat_buf != 0)
			free(concat_buf);
		concat_buf = malloc(concat_bufsiz);
	}
	memcpy(p = concat_buf, s1, l1);
	memcpy(p += l1, s2, l2);
	memcpy(p += l2, s3, l3);
	memcpy(p += l3, s4, l4);
	return concat_buf;
}

char *
concat(char *s1, ...)
{
	va_list ap;
	int l1 = strlen(s1);
	int l = l1 + 1;
	char *s;
	char *p;

	va_start(ap, s1);
	while (s = va_arg(ap, char *))
		l += strlen(s);
	va_end(ap);
	if (l > concat_bufsiz) {
		if ((concat_bufsiz = l) < 512)
			concat_bufsiz = 512;
		if (concat_buf != 0)
			free(concat_buf);
		concat_buf = malloc(concat_bufsiz);
	}
	memcpy(p = concat_buf, s1, l1);
	p += l1;
	va_start(ap, s1);
	while (s = va_arg(ap, char *)) {
		int l = strlen(s);
		memcpy(p, s, l);
		p += l;
	}
	*p = 0;
	va_end(ap);
	return concat_buf;
}

char *
concatl(char *s1, int l1, ...)
{
	va_list ap;
	int l = l1 + 1;
	char *s;
	char *p;

	va_start(ap, l1);
	while (s = va_arg(ap, char *))
		l += va_arg(ap, int);
	va_end(ap);
	if (l > concat_bufsiz) {
		if ((concat_bufsiz = l) < 512)
			concat_bufsiz = 512;
		if (concat_buf != 0)
			free(concat_buf);
		concat_buf = malloc(concat_bufsiz);
	}
	memcpy(p = concat_buf, s1, l1);
	p += l1;
	va_start(ap, l1);
	while (s = va_arg(ap, char *)) {
		int l = va_arg(ap, int);
		memcpy(p, s, l);
		p += l;
	}
	*p = 0;
	va_end(ap);
	return concat_buf;
}

/*
 * This one uses a static buffer, so it can be used in conjunction
 * with concat*().
 */
char *
itoa(i)
{
	static char buf[100];
	sprintf(buf, "%d", i);
	return buf;
}

#if 0
/*
 * This one also uses a static buffer, so it can be used in conjunction
 * with concat*().
 */
char *
ttoa(t)
	time_t t;
{
	char *p = ctime(&t);
	struct tm *tm = localtime(&t);
	static char buf[31];
	extern char *tzname[];

	sprintf(buf, "%.24.24s (%3.3s)", p, tzname[tm->tm_isdst]);
	return buf;
}
#endif
