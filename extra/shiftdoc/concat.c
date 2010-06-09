/* -*- Mode: C -*- */

/* concat.c -- */

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
