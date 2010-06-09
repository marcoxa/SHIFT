/* -*- Mode: C -*- */

/* lisp.h -- */

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

#ifndef SHIFTC_LISP_H
#define SHIFTC_LISP_H

typedef struct lispval
{
  short type;
  char mark;
  char flags;
  struct lispval *link;
  union
  {
    /* symbol */
    struct
    {
      char *vpname; /* print name */
      struct lispval *vshlink; /* hash link */
      struct lispval *vplist; /* property list (an alist) */
      short vsindex; /* used by hasher */
    } s;

    /* cons cell */
    struct
    {
      struct lispval *car, *cdr;
    } c;

    /* string */
    struct
    {
      char *vstr;
    } t;

    /* fixnum */
    struct
    {
      int vnum;
    } f;

    /* IR node */
    struct
    {
      struct lispval *vop; /* tag name (a symbol) */
      struct lispval *vattrs; /* attributes (an alist) */
      struct lispval *vargs; /* arguments */
      int mark;
    } n;

    /* Other */
    struct
    {
      void *x;
    } o;
  } u;
} lv;

/* type */
#define L_FREE		0
#define L_CONS		1
#define L_STRING	2
#define L_SYMBOL	3
#define L_NODE		4
#define L_FIXNUM	5
#define L_OTHER		6

/* flags */
#define L_STATIC	0x0001

/*
 * Encapsulation
 */

#define eq(x, y)	((x) == (y))

#define consp(x)	((x) != 0 && (x)->type == L_CONS)
#define null(x)         ((x) == nil)
#define hd(x)		((x)->u.c.car)
#define tl(x)		((x)->u.c.cdr)

#define stringp(x)	((x) != 0 && (x)->type == L_STRING)
#define str(x)		((x)->u.t.vstr)

#define fixnump(x)	((x) != 0 && (x)->type == L_FIXNUM)
#define num(x)		((x)->u.f.vnum)

#define symbolp(x)	((x) != 0 && (x)->type == L_SYMBOL)
#define pname(x)	((x)->u.s.vpname)
#define plist(x)	((x)->u.s.vplist)
#define sindex(x)	((x)->u.s.vsindex)
#define shlink(x)	((x)->u.s.vshlink)

#define nodep(x)	((x) != 0 && (x)->type == L_NODE)
#define op(x)		((x)->u.n.vop)
#define attrs(x)	((x)->u.n.vattrs)
#define args(x)		((x)->u.n.vargs)
#define arg1(x)		first(args(x))
#define arg2(x)		second(args(x))
#define arg3(x)		third(args(x))
#define mark_attr(x)	((x)->u.n.mark)

#define otherp(x)	((x) != 0 && (x)->type == L_OTHER)
#define oth(y)		((y)->u.o.x)

/*
 * Generally useful stuff
 */

#define first(x)	hd(x)
#define second(x)	hd(tl(x))
#define third(x)	hd(tl(tl(x)))
#define rest(x)		tl(x)

#define list1(x)	cons(x, nil)
#define list2(x, y)	cons(x, list1(y))
#define list3(x, y, z)	cons(x, list2(y, z))
#define list4(x, y, z, w)	cons(x, list3(y, z, w))

#define push(x, l)	((l) = cons(x, l))
#define apush(x, y, l)	push(cons(x, y), l)
#define acons(x, y, l)	cons(cons(x, y), l)
#define reassoc(x, y, l)	reassoc_(x, y, &l)

#define add_attr(attr, node, value) apush(attr, value, attrs(node))

#define alist1(k1, v1)			acons(k1, v1, nil)
#define alist2(k1, v1, k2, v2)		acons(k1, v1, alist1(k2, v2))
#define alist3(k1, v1, k2, v2, k3, v3)	acons(k1, v1, alist2(k2, v2, k3, v3))

#define dolist(x, l)	{ lv *x, *_x; for (_x = (l); _x; _x = tl(_x)) \
			    { (x) = hd(_x); {
									  
#define tsilod }}}

#define strsave(x)	(str(string(x)))
#define strsavel(x, l)	(str(stringl(x, l)))

#define nil 0

/*
 * Symbol hashing
 */

#define L_HBITS		11
#define L_HSHIFT	3
#define L_HSIZE		(1 << L_HBITS)
#define L_HMASK		(L_HSIZE - 1)

#ifndef SHIFTC_LISP_I

/* extern lv *shtab[];
 * This variable needs to be allocated by the programs that want to
 * use the lisp library.
 *
 * A typical declaration would be

     lv* shtab[L_HSIZE];

 * Marco Antoniotti 19971023
 */
extern lv *intern(char *);
extern int shash(char *);

#endif /* SHIFTC_LISP_I */


/*
 * Misc functions
 */

#ifndef SHIFTC_LISP_I

extern lv *read_sexpr(char *);
extern lv *read_sexpr1(FILE *);
extern lv *read_sexpr_list(FILE *);
extern lv *list_to_attrs(lv *);
extern lv *read_sexpr_string(FILE *);
extern lv *read_sexpr_symbol(FILE *);
extern lv *read_c_symbol(FILE *);
extern int write_sexpr(lv *, char *);
extern void print(lv *);
extern void write_sexpr1(lv *, FILE *);
extern lv *cons(lv *, lv *);
extern lv *string(const char *);
extern lv *stringl(char *, int);
extern lv *fixnum(int);
extern lv *symbol(char *);
extern lv *node(lv *, lv *, lv *);
extern lv *attr(lv *, lv *);
#define attr2(a1, a2, n) attr(a1, attr(a2, n))
#define attr3(a1, a2, a3, n) attr2(a1, a2, attr(a3, n))
extern void set_attr(lv *attr, lv *node, lv *value);
extern void push_attr(lv *attr, lv *node, lv *value);
extern lv *other(void *x);
extern lv *assoc(lv *, lv *);
extern int length(lv *);
extern lv *nth(int, lv *);
extern lv *list(lv *, ...);
extern lv *copy_tree(lv *);
extern lv *copy_list(lv *);
extern lv *nreverse(lv *);
extern lv *nreverse2(lv *, lv *);
extern lv *nconc(lv *, lv *);
extern lv *append(lv *, lv *);
extern lv *del_el(lv *, lv *);
extern lv *mapcar(lv *(*)(lv *), lv *);
extern lv *mapcarx(lv *(*)(), lv *, lv *);
extern lv *mapcan(lv *(*)(lv *), lv *);
extern void mapc(void (*)(lv *), lv *);
extern void mapcx(void (*)(lv *, lv*), lv *, lv *);
extern lv *member_op(lv *, lv *);
extern lv *memq(lv *, lv *);
extern lv *lalloc(void);
extern void gc_set_root(lv *);
extern void gc_clear_root(void);
extern void gc();
extern void new_node_marker();
extern void mark_node(lv* node);
extern int node_marked_p(lv* node);

#endif /* SHIFTC_LISP_I */

#endif /* SHIFTC_LISP_H */

/* end of file -- lisp.h -- */
