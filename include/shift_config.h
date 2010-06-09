/* include/shift_config.h.  Generated automatically by configure.  */
/* -*- Mode: C -*- */

/* shift_config.h.in -- */

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

/* shift_config.h.in.  Generated automatically from configure.in by
 * autoheader and later manually modified.
 *
 * NOTE: This file must always be included BEFORE any other SHIFT
 * include file.
 */

#ifndef shift_config_h
#define shift_config_h

/* Discriminating the platform and the operating system */

/* Various Sun */

#define OS_SOLARIS 1
#define OS_SUNOS5 1
/* #undef OS_SUNOS4 */


/* Various SGI */

/* #undef OS_IRIX */


/* Various IBM */

/* #undef OS_AIX */


/* Various SCO */

/* #undef OS_SCOUNIX */


/* Various FreeBSD */

/* #undef OS_FREEBSD */


/* Various Linux */

/* #undef OS_LINUX */


/* MS Windows 95/NT */

/* #undef OS_MSWINDOWS */
/* #undef WIN32 */


/* Other OS's and platforms */

/* #undef OS_OTHER */


/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the drand48 function.  */
#define HAVE_DRAND48 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the dl library (-ldl).  */
#define HAVE_LIBDL 1

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Define if you have the nsl library (-lnsl).  */
#define HAVE_LIBNSL 1

/* Define if you have the socket library (-lsocket).  */
#define HAVE_LIBSOCKET 1

/* Define if you have the tcl library (-ltcl).  */
#define HAVE_LIBTCL 1

/* Define if you have the tk library (-ltk).  */
#define HAVE_LIBTK 1

/* Define if you have the BLT library (-lBLT).  */
#define HAVE_LIBBLT 1

/* Define if Graphic Environment was built */
#define HAVE_SHIFT_GUI_P 1

/* Define if support for the Boehm C Garbage Collector is provided/ */
/* #undef USE_GC_SUPPORT_P */ 

/* Define if new hash table code is desired. (It should be a good idea
 * to use this with the new GC).
 */
#define HAVE_NEW_HASH_P 1

#endif  /* shift_config_h */

/* end of file -- shift_config.h */
