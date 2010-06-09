/* -*- Mode: C -*- */

/* collection.c -- */

/* 
 * Copyright (c)1996, 1997, 1998 The Regents of the University of
 * California (Regents). All Rights Reserved. 
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for educational, research, and not-for-profit
 * purposes, without fee and without a signed licensing agreement, is
 * hereby granted, provided that the above copyright notice, this
 * paragraph and the following two paragraphs appear in all copies,
 * modifications, and distributions. 
 * 
 * Contact The Office of Technology Licensing, UC Berkeley, 2150
 * Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510)
 * 643-7201, for commercial licensing opportunities. 
 * 
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE. 
 *   
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
 * DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
 * REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS. 
 */

/* Collection types (set and array) for SHIFT runtime.
 */

#ifndef COLLECTION_I
#define COLLECTION_I

#include <shift_config.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include "shifttypes.h"
#include "collection.h"


extern void debugger_print_set(GenericSet *s);


#ifndef HAVE_NEW_HASH_P
#define hash(p, s) hash_(((int *)p), (s->et->kind), (*s->lengthp))

#define hash_(p, k, l) (k == NUMBER_T? (p[0] ^ p[1]) % l :		     \
			k == SYMBOL_T? p[0] % l :			     \
			k == CVP_T? ((p[0] ^ p[1]) >> 3) %l :		     \
			(p[0] >> 3) % l)
#endif /* HAVE_NEW_HASH_P */


#ifdef HAVE_NEW_HASH_P

/* New hash definitions.  They are needed because the old ones are
 * 'pointer dependent'.  They may or may not break under the GC, but
 * it is better not to take risks.
 *
 * Notes:
 * - There is one problem with the following scheme:  I.e. how to
 *   'hash' arrays and sets while avoiding simple minded schemes that
 *   would produce collisions (like hashing on the element type.)
 *   I think that a partial solution would be to add an int field to the
 *   set and arrays headers to be added in while computing the hash
 *   code.  This is pretty much equivalent to 'uniquify' the set or array
 *   via its pointer, but it is not as 'apparently' random.
 *   Using this scheme will require a change to 'new_set' and
 *   'new_array' to make them remember the last 'id' used.
 *
 * Marco Antoniotti 19961203
 */

/* Forward declarations */
inline int _hash_1(vid*, TypeKind, int);
inline int instance_hash_code(Component*, int);
inline int array_hash_code(GenericArray*, int);
inline int set_hash_code(GenericSet*, int);

/* Counters for each call to 'new_set' and 'new_array'. */

static int new_set_id = 0;
static int new_array_id = 0;


static inline int
new_set_hash_id()
{
  return new_set_id++ % INT_MAX;
}


static inline int
new_array_hash_id()
{
  return new_array_id++ % INT_MAX;
}


/* hash, _hash_1 --
 * These are the functions that take car of computing the proper hash
 * function.
 * They are somewhat simple minded, but at least are very safe pointer
 * wise (unlike the previous version.
 *
 * Note: '_hash_1' assumes that the 'TypeKind' of the set is
 * compatible with the content of the 'vid*' (cfr. 'collection.h)
 * passed. This is a big assumption and must be taken into account
 * whenever changing any set or hash table related code.
 */
inline int
hash(vid* p, GenericSet* s)
{
  return _hash_1(p, s->et->kind, *(s->lengthp));
}


inline int
_hash_1(vid* p, TypeKind k, int table_length)
{
  switch (k)
    {
    case NUMBER_T:
      return rint(p->d) % table_length;
      break;
      
    case SYMBOL_T:		/* Symbols are really integers. */
    case LOGICAL_T:
      return p->i % table_length;
      break;
      
    case FOREIGN_T:
      /* Select hash bucket for foreign types.
       * Tunc Simsek 16th April, 1998
       */
      return rint(p->v) % table_length;
      break;
      
    case COMPONENT_T:
      return instance_hash_code((Component*)(p->v), table_length);
      break;
      
    case CVP_T:
      return instance_hash_code(p->cvp.c, table_length);
      break;

    case ARRAY_T:
      return array_hash_code((GenericArray *) (p->v), table_length);
      break;

    case SET_T:
      return set_hash_code((GenericSet *) (p->v), table_length);
      break;

    default:
      runtime_error("unknown type kind in hash code computation");
      break;
    }
}


inline int
instance_hash_code(Component* c, int table_length)
{
  /* c->desc->td_cached_hcode must be computed at compile time.
   * Also, this field must be inserted in the
   * 'ComponentTypeDescriptor' declaration in 'shifttypes.h'.
   */
  return (c->name + c->desc->ctd_cached_hcode) % table_length;
}


inline int
set_hash_code(GenericSet* s, int table_length)
{
  int set_id = s->set_hash_id;
  
  if (s->et->kind != UNKNOWN_T)
    {
      return (set_id + ((int) s->et->kind)) % table_length;
    }
  else
    runtime_error("unknown type kind in set hash code computation");
}


inline int
array_hash_code(GenericArray* a, int table_length)
{
  int array_id = a->array_hash_id;
  
  if (a->et->kind != UNKNOWN_T)
    {
      return (array_id + ((int) a->et->kind)) % table_length;
    }
  else
    runtime_error("unknown type kind in set hash code computation");
}

#endif /* HAVE_NEW_HASH_P */


/* Add element U to set S, assume not there yet.
 */
void
blind_add_to_set(vid u, GenericSet *s)
{
    int h;
    SetCell *sc = new(SetCell);
    sc->u = u;
    s->size++;
    h = hash(&u, s);

    /* Insert in link list. */
    sc->next = s->head->next;
    sc->next->previous = sc;
    sc->previous = s->head;
    s->head->next = sc;

    /* Insert in table. */
    sc->link = s->table[h];
    s->table[h] = sc;
}    

#if 1
int HASH_RESIZE_ARRAY[] = {0, 50, 500, 5000, 50000, 500000};
int HASH_LENGTH_ARRAY[] = {0, 100, 1000, 10000, 100000, 1000000};
#else
int HASH_RESIZE_ARRAY[] = {0, 50000 };
int HASH_LENGTH_ARRAY[] = {0, 100000 };
#endif

void clear_set(GenericSet *x);


static void
grow(GenericSet *x)
{
  SetCell **newtable =
    (SetCell **) safe_malloc(sizeof(SetCell *) * *++x->lengthp);
  SetCell *list = x->head->next;
  SetCell *y;

  assert (*x->lengthp != 0);
  assert (newtable != 0);

  if (x->table) free(x->table);
  x->table = newtable;
  x->resize_sizep++;

  /* Detaches list and clears out hashtable: */
  clear_set(x);
  /* (Note that, until this point, "newtable" might have been nonzero. */

  /* Iterate on list and insert into new table */
  for (y = list; y != x->head; y = y->next)
    {
      blind_add_to_set(y->u, x);
    }
}


GenericSet *
new_set(TypeDescriptor *et)
{
  GenericSet *x = new(GenericSet);

  assert (et!=0);
  x->et = et;
  x->head = new(SetCell);
  x->head->previous = x->head;
  x->head->next = x->head;
  x->size = 0;
  x->resize_sizep = HASH_RESIZE_ARRAY;
  x->lengthp = HASH_LENGTH_ARRAY;
  x->table = 0;

#ifdef HAVE_NEW_HASH_P
  x->set_hash_id = new_set_hash_id();
#endif /* HAVE_NEW_HASH_P */

  grow(x);
  return x;
}


/* Test for emptiness */
int
empty_set_p(GenericSet* s)
{
  return (s == (GenericSet*) 0) || (s->size == 0);
}


/* Some accessors.
 * Remember that sets link all the lements in a doubly linked list
 * which always has an empty 'head'.  Hence the dereferinc of the
 * 'next' field.
 */
double
double_first_element(GenericSet* s)
{
  assert(!empty_set_p(s));
  return s->head->next->u.d;
}


int
int_first_element(GenericSet* s)
{
  assert(!empty_set_p(s));
  return s->head->next->u.i;
}


void*
voidstar_first_element(GenericSet* s)
{
  assert(!empty_set_p(s));
  return s->head->next->u.v;
}


cvp
cvp_first_element(GenericSet* s)
{
  assert(!empty_set_p(s));
  return s->head->next->u.cvp;
}


#ifdef HAVE_NEW_HASH_P

/* vid_translator -- A necessary translator for the new hash
 * implementation. Mostly used internally to pass the correct value to
 * the new 'belongs_to'.
 *
 * Marco Antoniotti 19970122
 */
void*
vid_translator(vid u, TypeKind tk)
{
  switch (tk)
    {
    case NUMBER_T:
      return (void*)(&(u.d));

    case LOGICAL_T:
    case SYMBOL_T:
      return (void*)(&(u.i));

    case COMPONENT_T:
    case ARRAY_T:
    case SET_T:
      return u.v;
    case FOREIGN_T:
      return u.v;
    case CVP_T:
      return (void*)(&(u.cvp));
    }
}

#endif /* HAVE_NEW_HASH_P */



/* Clear a set. Do not shrink its hash table.
 */
void
clear_set(GenericSet *x)
{
  x->head->previous = x->head;
  x->head->next = x->head;
  x->size = 0;
  memset(x->table, 0, sizeof(SetCell *) * *(x->lengthp));
}


#define FIND_(forhead, cell, x)						     \
{   assert(s->et!=0);							     \
    switch (s->et->kind) {						     \
    case NUMBER_T:							     \
	forhead {							     \
	    if ((cell)->u.d == x.d) goto done;				     \
	}								     \
	goto done;							     \
    case CVP_T:								     \
	forhead {							     \
	    if ((cell)->u.cvp.c == x.cvp.c &&				     \
		(cell)->u.cvp.p == x.cvp.p) goto done;			     \
	}								     \
	goto done;							     \
    case SYMBOL_T:							     \
    case LOGICAL_T:							     \
	forhead {							     \
	    if ((cell)->u.i == x.i) goto done;				     \
	}								     \
	goto done;							     \
    case FOREIGN_T:                                       \
    default:								     \
	forhead {							     \
	    if ((cell)->u.v == x.v) goto done;				     \
	}								     \
	goto done;							     \
    } }

#define FIND(x) FIND_(for (sc = s->table[h]; sc; sc = sc->link), sc, (x))
#define FINDP(x) \
    FIND_(for (scp = &s->table[h]; *scp; scp = &((*scp)->link)), *scp, (x))


/* add_to_set --
 * Add if not there yet, else do nothing.
 */
void
add_to_set(vid u, GenericSet *s)
{
    SetCell *sc;
    int h = hash(&u, s);

    assert (h >= 0);
    assert (s != 0);

    FIND(u)
done:
    /* On returning from FIND(u) sc points to the
     * member of the set that is equal to 'u'. In
     * that case if it is not nil then we will return
     * hence not adding 'u' to the set.
     */
    if (sc) return;

    if (s->size == *s->resize_sizep) {
	grow(s);
	h = hash(&u, s);
	assert (h >= 0);
    }
    s->size++;
    sc = new(SetCell);
    sc->u = u;
    /* Insert in link list. */
    sc->next = s->head->next;
    sc->next->previous = sc;
    sc->previous = s->head;
    s->head->next = sc;

    /* Insert in table. */
    sc->link = s->table[h];
    s->table[h] = sc;
}


GenericSet*
insert_element(void* el, GenericSet* s)
{
  TypeKind k;
  vid u;

  /* Note: the main assumption behind the next switch is that the
   * pointer 'el' passed as parameter agrees with the type of the
   * elements in the set.
   * This will not be a problem for the code generated by the SHIFT
   * compiler (the type checking will ensure this), but it may be a
   * problem for code written by developers. Be careful.
   *
   * Marco Antoniotti 19961210
   */

  if (s == (GenericSet*) 0)
    runtime_error("insert_element: set not initialized.");

  /* Fill in the structure to be used for actual hashing */
  k = s->et->kind;
  switch (k)
    {
    case NUMBER_T:
      u.d = *((double*) el);
      break;

    case FOREIGN_T:
      u.v = (void *) el;
      break;

    case SYMBOL_T:
    case LOGICAL_T:
      u.i = *((int*) el);
      break;

    case COMPONENT_T:
      u.v = (void*) *(Component**) el;
      break;

    case ARRAY_T:
      u.v = (void*) *(GenericArray**) el;
      break;

    case SET_T:
      u.v = (void*) *(GenericSet**) el;
      break;

    case CVP_T:
      u.cvp.c = ((cvp*) el)->c;
      u.cvp.p = ((cvp*) el)->p;
      break;

    default:
      runtime_error("unknown type kind in hash code computation");
      break;
    }

  add_to_set(u, s);
  return s;
}


/* remove_from_set --
 * Remove C from S, if it's there.  Do nothing otherwise.
 */
void
remove_from_set(vid u, GenericSet *s)
{
  SetCell** scp;
  SetCell*  scp_link;
  int h = hash(&u, s);

  assert (h >= 0);
  FINDP(u);

done:				/* We jump here from the 'FINDP' macro. */

  if (! *scp) return;

  (*scp)->next->previous = (*scp)->previous;
  (*scp)->previous->next = (*scp)->next;

  scp_link = (*scp)->link;
  free(*scp);
  *scp = scp_link;
  s->size--;
}

GenericSet*
remove_element(void* el, GenericSet* s)
{
  TypeKind k;
  vid u;

  /* Note: the main assumption behind the next switch is that the
   * pointer 'p' passed as parameter agrees with the type of the
   * elements in the set.
   * This will not be a problem for the code generated by the SHIFT
   * compiler (the type checking will ensure this), but it may be a
   * problem for code written by developers. Be careful
   *
   * Marco Antoniotti 19961210
   */
  
  if (s == (GenericSet*) 0)
    runtime_warning("remove_element: remiving element from uninitialized set.");

  else
    {
      k = s->et->kind;
      /* Fill in the structure to be used for actual hashing */
      switch (k)
	{
	case NUMBER_T:
	  u.d = *((double*) el);
	  break;

	case SYMBOL_T:
	case LOGICAL_T:
	  u.i = *((int*) el);
	  break;

	case FOREIGN_T:
	  u.v = (void *) el;
	  break;

	case COMPONENT_T:
	  u.v = (void*) *(Component**) el;
	  break;

	case ARRAY_T:
	  u.v = (void*) *(GenericArray**) el;
	  break;

	case SET_T:
	  u.v = (void*) *(GenericSet**) el;
	  break;

	case CVP_T:
	  u.cvp.c = ((cvp*) el)->c;
	  u.cvp.p = ((cvp*) el)->p;
	  break;

	default:
	  runtime_error("unknown type kind in hash code computation");
	  break;
	}
      remove_from_set(u, s);
    }
  return s;
}


/* Set Membership.
 * Let's try to be a little smarter about set membership.
 *
 * Marco Antoniotti 19970612
 */

#ifdef HAVE_NEW_HASH_P
int
set_membership_I(int element, void* a_set)
{
  return belongs_to((void*) &element, a_set);
}

int
set_membership_D(double element, void* a_set)
{
  return belongs_to((void*) &element, a_set);
}

int
set_membership_A_S_C(void* a_s_c_element, void* a_set)
{
  return belongs_to(a_s_c_element, a_set);
}

int
set_membership_CVP(void* a_cvp, void* a_set)
{
  return belongs_to(a_cvp, a_set);
}
#endif /* HAVE_NEW_HASH_P */


int 
in(void *p, void *vs)
{
  return belongs_to(p, vs);
}

#ifndef HAVE_NEW_HASH_P

int
belongs_to(void *p, void *vs)
{
  GenericSet *s = (GenericSet *) vs;
  vid *pu = (vid *) p;	/* unportable */
  int h = hash(p, s);
  SetCell *sc = s->table[h];

  assert (pu != 0);
  assert (h >= 0);
  assert (s != 0);

  FIND(*pu);

done:				/* We jump here from the 'FIND' macro. */
  return sc ? 1 : 0;
}


#else /* HAVE_NEW_HASH_P */


int
belongs_to(void *p, void *vs)
{
  GenericSet *s = (GenericSet *) vs;
  TypeKind k = s->et->kind;
  SetCell* sc;
  vid u;
  int h;

  /* Note: the main assumption behind the next switch is that the
   * pointer 'p' passed as parameter agrees with the type of the
   * elements in the set.
   * This will not be a problem for the code generated by the SHIFT
   * compiler (the type checking will ensure this), but it may be a
   * problem for code written by developers. Be careful
   *
   * Marco Antoniotti 19961210
   */

  /* Fill in the structure to be used for actual hashing */
  switch (k)
    {
    case NUMBER_T:
      u.d = *((double*) p);
      break;

    case SYMBOL_T:
    case LOGICAL_T:
      u.i = *((int*) p);
      break;

    case FOREIGN_T:
      u.v = (void *) p;
      break;

    case COMPONENT_T:
    case ARRAY_T:
    case SET_T:
      u.v = p;
      break;

    case CVP_T:
      u.cvp.c = ((cvp*) p)->c;
      u.cvp.p = ((cvp*) p)->p;
      break;

    default:
      runtime_error("unknown type kind in hash code computation");
      break;
    }

  h = hash(&u, s);		/* This works because the 'vid u' will
				 * be on the stack during until it is
				 * needed.
				 */
  sc = s->table[h];

  /* assert (p != 0); */
  assert (h >= 0);
  assert (s != 0);

  FIND(u);

done:				/* We jump here from the 'FIND' macro. */
  return sc ? 1 : 0;
}

#endif /* HAVE_NEW_HASH_P */


void *
set_union(void *vx, void *vy)
{
  GenericSet *x = (GenericSet *) vx;
  GenericSet *y = (GenericSet *) vy;

  if (empty_set_p(x))
    return y;
  else if (empty_set_p(y))
    return x;
  else
    {
      GenericSet *n = new_set(x->et);
      SetCell *sc;

      for (sc = x->head->next; sc != x->head; sc = sc->next)
	{
	  blind_add_to_set(sc->u, n);
	}
      for (sc = y->head->next; sc != y->head; sc = sc->next)
	{
	  add_to_set(sc->u, n);
	}
      return n;
    }
}


void *
set_difference(void *vx, void *vy)
{
  GenericSet *x = (GenericSet *) vx;
  GenericSet *y = (GenericSet *) vy;

  if (empty_set_p(x) || empty_set_p(y))
    return x;
  else
    {
      GenericSet *n = new_set(x->et);
      SetCell *sc;

      for (sc = x->head->next; sc != x->head; sc = sc->next)
	{
#ifdef HAVE_NEW_HASH_P
	  if (! belongs_to(vid_translator(sc->u, x->et->kind), y))
#else
	  if (! belongs_to(&sc->u, y))
#endif /* HAVE_NEW_HASH_P */
	    {
	      blind_add_to_set(sc->u, n);
	    }
	}
      return n;
    }
}


void *
set_intersection(void *vx, void *vy)
{
  GenericSet *x = (GenericSet *) vx;
  GenericSet *y = (GenericSet *) vy;

  if (empty_set_p(x))
    return x;
  if (empty_set_p(y))
    return y;
  else
    {
      GenericSet *n = new_set(x->et);
      SetCell *sc;

      for (sc = x->head->next; sc != x->head; sc = sc->next)
	{
#ifdef HAVE_NEW_HASH_P
	  if (belongs_to(vid_translator(sc->u, x->et->kind), y))
#else
	  if (belongs_to(&sc->u, y))
#endif /* HAVE_NEW_HASH_P */
	    {
	      blind_add_to_set(sc->u, n);
	    }
	}
      return n;
    }
}

#ifdef HAVE_NEW_HASH_P
int
set_equal(void* vs1, void* vs2)
{
  GenericSet* s1 = (GenericSet*) vs1;
  GenericSet* s2 = (GenericSet*) vs2;
  TypeKind tk = s1->et->kind;

  if (set_size(s1) != set_size(s2))
    return 0;
  else
    {
      FOR_ALL_ELEMENTS(e, s1)
	{
	  if (!belongs_to(vid_translator(e, tk), s2))
	    return 0;
	}
      END_FAE;
      return 1;
    }
}
#else /* ! HAVE_NEW_HASH_P */
int
set_equal(void* vs1, void* vs2)
{
  GenericSet* s1 = (GenericSet*) vs1;
  GenericSet* s2 = (GenericSet*) vs2;

  if (set_size(s1) != set_size(s2))
    return 0;
  else
    {
      FOR_ALL_ELEMENTS(e, s1)
	{
	  if (!belongs_to(&(e), s2))
	    return 0;
	}
      END_FAE;
      return 1;
    }
}
#endif /* HAVE_NEW_HASH_P */


void *
set_cons(TypeDescriptor *et, int n, ...)
{
  va_list ap;
  GenericSet *s = new_set(et);
  vid u;
  int i;

  va_start(ap, n);
  for (i = 0; i < n; i++)
    {
      switch (et->kind)
	{
	case NUMBER_T:
	  u.d = va_arg(ap, double);
	  break;

	case SYMBOL_T:
	case LOGICAL_T:
	  u.i = va_arg(ap, int);
	  break;

	case FOREIGN_T:
	default:
	  u.v = va_arg(ap, void *);
	  break;
	}
      add_to_set(u, s);
    }
  va_end(ap);
  return s;
}


/* Adding support for intirinsically generated sets.
 * i.e. Those that are denoted with the DOT notation.
 *
 * Tunc Simsek 19980528
 */
void *
intrinsic_set_cons(TypeDescriptor *et)
{
  GenericSet *s = new_set(et);
  static int i = 0;
  void intrinsic_set_cons_children(ComponentTypeDescriptor **,
				   GenericSet *);

  i++;
  if (et->kind != COMPONENT_T)
    {
      runtime_error("intrinsic_set_cons: Requesting to generate\n\
an intrinsic set not of type 'component'.");
    }
  else
    {
      ComponentTypeDescriptor *ct = et->ct;
      Component *clist = ct->component_first;

      for ( ; clist; clist = clist->component_next)
	{
	  vid u;

	  u.v = (void *) clist;
	  add_to_set(u, s);
	}

      intrinsic_set_cons_children(ct->children, s);
    }

  return s;
}

void
intrinsic_set_cons_children(ComponentTypeDescriptor **children,
			    GenericSet *s)
{
  for ( ; *children; children++)
    {
      ComponentTypeDescriptor *child = *children;
      Component *clist = child->component_first;


      for ( ; clist; clist = clist->component_next)
	{
	  vid u;
	  
	  u.v = (void *) clist;
	      add_to_set(u, s);
	}      
      intrinsic_set_cons_children(child->children, s);
    }
  return;
}

/* Do a shallow copy of VS and return the new set. 
 */
void *
copy_set(void *vs)
{
  GenericSet *s = (GenericSet *) vs;
  GenericSet *n = new_set(s->et);

  FOR_ALL_ELEMENTS (el, vs)
    {
      add_to_set(el, n);
    }
  END_FAE;
  return (void*) n;
}

/*
void *
copy_set(void *vs)
{
  GenericSet *s = (GenericSet *) vs;
  TypeDescriptor *et = s->et;
  GenericSet *n = new_set(et);
  SetCell *first = s->head;
  SetCell *current = first->previous;

  / * Traverse the list of elements and add each of them to the
     new set N. Since the insertion function inserts at the head, we
     do a backward traversal to have the order among the elements. * /
  while (current != first)	/ * Cells are circularly linked. * /
    {
      switch (et->kind)
	{
	case NUMBER_T:
	  add_to_set(current->u.d, n);
	  break;

	case SYMBOL_T:
	case LOGICAL_T:
	  add_to_set(current->u.i, n);
	  break;

	default:
	  / * It is a shallow copy so do not call recursively
	     copy_set or copy_array here. * /
	  add_to_set(current->u.v, n);
	  break;
	}
      current = current->previous;
    }
  return n;
}
*/

#define set_copier(l, r) l = (GenericSet *) copy_set(r)


void
new_array_low_level_alloc(GenericArray* a, int length, size_t elsize)
{
  a->data = safe_malloc(length * elsize);
  a->l    = length;
}

GenericArray *
new_array(TypeDescriptor *et, int l)
{
  GenericArray *n = new(GenericArray);
  size_t elsize;

#ifdef HAVE_NEW_HASH_P
  n->array_hash_id = new_array_hash_id();
#endif /* HAVE_NEW_HASH_P */

  switch (et->kind)
    {
    case NUMBER_T:
      elsize = sizeof(double);
      break;
      
    case FOREIGN_T:
      elsize = sizeof(void *);
      break;

    case LOGICAL_T:
    case SYMBOL_T:
      elsize = sizeof(int);
      break;

    default:
      elsize = sizeof(void *);
      break;
    }
  
  new_array_low_level_alloc(n, l, elsize);
  n->et = et;

  return n;
}


static void copy_row(GenericArray*, GenericArray*);

void
adjust_array_allocation(GenericArray* a,
			int array_current_size,
			int size,
			size_t elsize)
{
  GenericArray* saved_array_copy;

  void* copy_array(void *);

  assert(a != 0);

  if (array_current_size < size)
    {
      saved_array_copy = (GenericArray*) copy_array((void*) a);
      new_array_low_level_alloc(a, size, elsize);
      copy_row(saved_array_copy, a);
    }
  else if (array_current_size == size)
    {
      ;				/* Do nothing */
    }
  else /*  if (array_current_size > size) */
    runtime_warning("shrinking array: data may be lost");
}


/* ensure_array_allocation -- Ensures that the array has a given
 * allocated size for a given element type.
 *
 * Note:
 * The function has a GenericArray** signature because it might be
 * possible to call it with a null pointer (i.e. a location which
 * could contain an array, but which 
 */
void
ensure_array_allocation(GenericArray** array_loc,
			TypeDescriptor* array_el_type,
			int size
			/*
			int rank,
			int dimensions[]
			*/)
{
  GenericArray* a = *array_loc;
  TypeDescriptor new_array_td;
  TypeKind array_type_kind;
  size_t elsize;
  int array_current_size;
  int current_rank;
  extern char* TypeKindString[]; /* in 'shift.c' */

  if (a != 0)
    {
      array_type_kind = a->et->kind;
      array_current_size = array_size(a);

      if (array_type_kind != array_el_type->kind)
	runtime_error("ensure_array_allocation: array base type mismatch: %s and %s.",
		      TypeKindString[array_type_kind],
		      TypeKindString[array_el_type->kind]);

      switch (array_type_kind)
	{
	case NUMBER_T:
	  elsize = sizeof(double);
	  break;

	case FOREIGN_T:
	  elsize = sizeof(void *);
	  break;

	case LOGICAL_T:
	case SYMBOL_T:
	  elsize = sizeof(int);
	  break;

	default:
	  elsize = sizeof(void *);
	  break;
	}

      if (array_current_size == 0)
	{
	  /* MAYBE WARNING? */
	  new_array_low_level_alloc(a, size, elsize);
	}
      else if (array_current_size != size)
	{
	  adjust_array_allocation(a, array_current_size, size, elsize);
	}
    }
  else				/* the array reference is null. */
    {
      *array_loc = new_array(array_el_type, size);
    }
}


void *
copy_array(void *va)
{
  GenericArray *a = (GenericArray *) va;
  TypeDescriptor *et = a->et;
  GenericArray *n = new_array(et, a->l);

  switch (et->kind)
    {
    case NUMBER_T:
      memcpy(n->data, a->data, a->l * sizeof(double));
      break;

    case FOREIGN_T:
      memcpy(n->data, a->data, a->l*sizeof(void *));
      break;

    case LOGICAL_T:
    case SYMBOL_T:
      memcpy(n->data, a->data, a->l * sizeof(int));
      break;

    case ARRAY_T:
      {
	int i;
	for (i = 0; i < a->l; i++)
	  {
	    ((GenericArray **) n->data)[i] =
	      (GenericArray *) copy_array(((GenericArray **) a->data)[i]);
	  }
	break;
      }

    default:
      memcpy(n->data, a->data, a->l*sizeof(void *));
      break;
    }
  return n;
}


static void
copy_row(GenericArray* from, GenericArray* to)
{
  TypeDescriptor *from_et = from->et;
  TypeDescriptor *to_et = to->et;
  size_t elsize;
  extern char* TypeKindString[]; /*  in shift.c */

  if (from_et->kind != to_et->kind)
    runtime_error("copy_row: array base type mismatch: %s and %s.",
		  TypeKindString[from_et->kind],
		  TypeKindString[to_et->kind]);
  switch (from_et->kind)
    {
    case NUMBER_T:
      elsize = sizeof(double);
      break;
      
    case FOREIGN_T:
      elsize = sizeof(void *);
      break;

    case LOGICAL_T:
    case SYMBOL_T:
      elsize = sizeof(int);
      break;

    case ARRAY_T:
      elsize = sizeof(GenericArray *);
      break;

    default:
      elsize = sizeof(void *);
      break;
    }
  memcpy(to->data, from->data, from->l * elsize);
}


#define simple_copier(l, r) l = r
#define array_copier(l, r) l = (GenericArray *) copy_array(r)


void *
array_cons(TypeDescriptor *et, int n, ...)
{
  va_list ap;
  GenericArray *a = new_array(et, n);
  vid u;
  int i;
  double md;

  va_start(ap, n);

  switch (et->kind)
    {
    case NUMBER_T:
      for (i = 0; i < n; i++)
	{
	  /* ((double *) &a->data)[i] = va_arg(ap, double); */
	  md = va_arg(ap, double);
	  ((double *) a->data)[i] = md;
	}	
      break;
      
    case FOREIGN_T:
      for (i = 0; i < n; i++)
	{
	  ((void * *) a->data)[i] = va_arg(ap, void *);			     
	}	
      break;

    case SYMBOL_T:
    case LOGICAL_T:
      for (i = 0; i < n; i++)
	{
	  ((int *) a->data)[i] = va_arg(ap, int);			     
	}	
      break;

    case ARRAY_T:
      for (i = 0; i < n; i++)
	{
	  ((void * *) a->data)[i] = va_arg(ap, void *);			     
	}	
      break;

    default:
      for (i = 0; i < n; i++)
	{
	  ((void * *) a->data)[i] = va_arg(ap, void *);			     
	}	
      break;
    }
  va_end(ap);
  return a; 
}

/* Replaced with functions.  See below
#define defindex(type)							     \
type *									     \
type ## _index(void *va, double i)					     \
{									     \
    GenericArray *a = (GenericArray *) va;				     \
  if (! a) runtime_error("indexing an uninitialized array");                 \
  if ( ((int) i) >= ( (int) a->l) || ((int) i) < 0)                          \
  runtime_error("indexing beyond the bound of array");                       \
    return &((type *) a->data)[(int) i];				     \
}

defindex(double)
defindex(int)
defindex(voidstar)
defindex(cvp)
*/

void
array_runtime_check(GenericArray* a, int i)
{
  if (! a)
    runtime_error("indexing an uninitialized array");

  if (i >= a->l || i < 0)
    runtime_error("out of bounds array indexing");
}


/* array indices --
 * These are pretty ugly. Probably need to be reworked.  Note that
 * they are corresponding to the possible elements of a set (see
 * 'collection.h'.)
 * Marco Antoniotti 19970107
 */

double *
double_index(void* va, int index)
{
  GenericArray *a = (GenericArray *) va;

  array_runtime_check(a, index);
  return &(((double *) a->data)[index]);
}

int *
int_index(void* va, int index)
{
  GenericArray *a = (GenericArray *) va;

  array_runtime_check(a, index);
  return &(((int *) a->data)[index]);
}

voidstar *
voidstar_index(void* va, int index)
{
  GenericArray *a = (GenericArray *) va;

  array_runtime_check(a, index);
  return &(((voidstar *) a->data)[index]);
}

cvp *
cvp_index(void* va, int index)
{
  GenericArray *a = (GenericArray *) va;

  array_runtime_check(a, index);
  return &(((cvp *) a->data)[index]);
}


/* array_set -- Somehow this was missing. Note that it is pretty gross.
 * In particular it is assumed that the 'value' can be meaningfully
 * converted to the 'right' domain without much work.
 * Marco Antoniotti 19970107
 */
void*
array_set(void* array, int i, void* value)
{
  GenericArray *a = (GenericArray*) array;

  array_runtime_check(a, i);
  
  switch (a->et->kind)
    {
    case NUMBER_T:
      ((double *) (a->data))[i] = *((double*) value);
    break;

    case FOREIGN_T:
      ((void * *) (a->data))[i] = value;
    break;

    case SYMBOL_T:
    case LOGICAL_T:
      ((int *) (a->data))[i] = *((int*) value);
    break;
    
    default:
      ((void * *) (a->data))[i] = value;
    break;
    }
  return value;
}


/* WARNING: Apparently array_size returns correctly its result only if
 * it is an int. So I changed the profile from double to int.
 * Alain Girault. 19970124
 */
int
array_size(GenericArray *a)
{
  if (! a)
    {
      runtime_warning("requesting size of uninitialized array");
      return 0; 
    }
  return a->l;
}

int
array_rank(GenericArray* a)
{
  if (! a)
    {
      runtime_warning("requesting size of uninitialized array");
      return 1;
    }
  
  if (a->et->kind != ARRAY_T)
    return 1;
  else
    {
      /* This recursion should be much more sophisticated.  Given that
       * we do not have any way to ensure that an array has the proper
       * allocation.
       *
       * Marco Antoniotti 19970513
       */
      return 1 + array_rank((GenericArray*) *voidstar_index((void *) a, 0));
    }
}


/* WARNING: Apparently set_size returns correctly its result only if
 * it is an int. So I changed the profile from double to int.
 * Alain Girault. 19970124
 */
int
set_size(GenericSet *s)
{
  if (! s)
    {
      runtime_error("requesting size of uninitialized set");
    }
  return s->size;
}

#endif /* COLLECTION_I */


/* end of file -- collection.c -- */
