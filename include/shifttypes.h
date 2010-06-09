/* -*- Mode: C -*- */

/* shifttypes.h -- */

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

#ifndef SHIFTTYPES_H
#define SHIFTTYPES_H

#include <shift_config.h>
#include <gc_support.h>

#include "run-time.h"
/* #include "collection.h" */
#include "shift_api_types.h"


/* Types for SHIFT run-time support.
 *
 * /--- <.0.>
 *
 * This file contains source code and documentation (in the form of
 * comments) for the interface between the SHIFT compiler and the
 * SHIFT run-time library.
 *
 * The actual code is C, but it is expected to migrate soon to C++.
 *
 * Important sections of the file are marked with section numbers in
 * dotted angle brackets, like <.1.>.  The comments in this file
 * should be read roughly by increasing section number.
 *
 * A smaller version of this file can be produced with the program
 * SHRINK-COMMENTS, which should be included in the same directory as
 * this file.  SHRINK-COMMENTS removes all lines between the line
 * preceding a line containing a slash (/) followed by three dashes
 * (---), and the next line which contains three dashes followed by a
 * slash.
 *
 * The reason for this dual view is that extensive comments are useful
 * for getting acquainted with the code, but they become tedious when
 * someone already familiar with it needs to quickly consult the
 * header file.  ---/
 */

#define FOREIGN_TYPE void
#define new(type) ((type *) safe_malloc(sizeof(type)))
#define new_multiple(type, n) ((type *) safe_malloc(sizeof(type) * (n)))

char *safe_malloc(int);
typedef void * voidstar;
typedef void Array;
typedef void Set;

#define NIL 0

struct Component;	/* get rid of parameter list warnings */

/* Flow functions for differential and algebraic variables.
 *
 * /--- A flow function computes the derivative of a
 * differentially-defined variable, or the value of an
 * algebraically-defined variable.  The first argument is SELF, the
 * second argument is the integration step (currently for RK-4).
 *
 * The flow functions will be replaced, for performance, by `mode
 * functions', which advance the state of all variable in a mode with
 * a single call. ---/
 * 
 * Flow functions now come in many different flavors according to the
 * return value.  The functions 'generate_type' and
 * 'generate_flow_function_type' in file "generate.c" should be kept
 * aligned with this set of declarations.
 */
typedef double (*DoubleFlowFunction)(struct Component *, int, int);

typedef
struct Component* (*ComponentFlowFunction)(struct Component *, int, int);

typedef int (*IntFlowFunction)(struct Component *, int, int);

typedef Set* (*SetFlowFunction)(struct Component *, int, int);

typedef Array* (*ArrayFlowFunction)(struct Component *, int, int);


/* Descriptor for a differentially-defined variable, used by the
 * integrator.
 *
 * /--- STATE_OFFSET is the offset of the state variable in the
 * component's data area.
 *
 * FLOW_FUNCTION is the function that returns the derivative of the
 * variable.  ---/
 */
typedef struct DifferentialVarDescriptor {
  int state_offset;
  DoubleFlowFunction flow_function;
} DifferentialVarDescriptor;


/* Type operators of interest to the run-time system.  The CVP_T is the
 * operator for the component_voidp structure.
 */

/* Adding support for external types.
 *
 * Tunc Simsek 15th April 1998
 */
typedef enum {
  NUMBER_T, SYMBOL_T, LOGICAL_T,
  ARRAY_T, SET_T, COMPONENT_T,
  CVP_T, FOREIGN_T,
  UNKNOWN_T
} TypeKind;


/* Data type descriptor.
 *
 * /--- KIND identifies the type.
 *
 * If KIND is ARRAY_T or SET_T, then S is the element type.
 *
 * If KIND is COMPONENT_T, CT points to a component type descriptor.
 * ---/
 */
typedef struct TypeDescriptor {
  TypeKind kind;			/* Type operator */
  struct TypeDescriptor *s;		/* Subtype */
  struct ComponentTypeDescriptor *ct;	/* Component type */
} TypeDescriptor;


typedef enum {
  GLOBAL_KIND, INPUT_KIND, OUTPUT_KIND, STATE_KIND,
  DIFFERENTIAL_KIND, TEMPORARY_KIND, MODE_KIND,
  CONTEXT_KIND,
  FLOW_FUNCTION_KIND, BP_SET_KIND, SYNC_VAR_KIND,
  LOCAL_KIND, SELF_KIND, TYPE_KIND, NO_KIND
} VarKind;


/* Component variable descriptor.
 *
 * /--- NAME is the user-defined name (or compiler-generated, for
 * auxiliary variables.)
 *
 * KIND gives information on where the variable came from.  Some kinds
 * have obvious meaning (INPUT_KIND, OUTPUT_KIND, etc.), others are
 * implementation specific.
 *
 * OFFSET is a byte offset into the data area for a component
 * instance.  The variable is stored there.
 *
 * TYPE is the variable type.
 *
 * POSSIBLE_MODES defines the possible modes for a variable.  The
 * actual mode depends on the discrete state, and varies at run time.
 * CONSTANT_MODE means the variable is always a constant.
 * ALGEBRAIC_MODE means the variable is either constant or
 * algebraically defined.  DIFFERENTIAL_MODE is like ALGEBRAIC_MODE,
 * but the variable could also be differentially defined.  The macros
 * _VA_ (variable accessor), _AA_ (algebraic accessor) and _DA_
 * (differential accessor) should be used to access the value of the
 * variable, together with the fields MODE_OFFSET, FF_OFFSET,
 * CONTEXT_OFFSET, and STATE_OFFSET.  ---/
 */
typedef struct VariableDescriptor {
  char *name;			/* SHIFT name */
  VarKind kind;			/* Kind of variable */
  int offset;			/* Offset in component data */
  TypeDescriptor *type;		/* Variable type */
  int possible_modes;		/* Possible modes */
  int mode_offset;		/* Offset of variable holding current mode */
  int ff_offset;		/* Offset of variable holding flow function */
  int context_offset;		/* Offset of variable holding context */
  int state_offset;		/* Offset of state vector for integration */
} VariableDescriptor;


/* Descriptor for a mode (discrete state).
 *
 * /--- NAME is the SHIFT name of the state.
 *
 * DIFFERENTIAL_VARIABLES is a pointer to the first element of an
 * array containing the differential variables in this mode.  The last
 * element of the array has an offset of -1 and serves as a
 * terminator.
 *
 * HAS_GUARDS is a logical variable, true when the mode has guards,
 * that is at least one transition exiting the mode is guarded.
 *
 * INVARIANT_FUNCTION is a pointer to a function implementing the
 * invariant for this state, or the null pointer.
 *
 * ENTERING_FUNCTION contains bookkeeping actions that should be
 * performed when entering this mode.
 *
 * OUTGOING points to the first element of a null-terminated array of
 * addresses of transition descriptors, representing all the
 * transitions that leave the mode. ---/
 */
typedef struct ModeDescriptor {
  char *name;
  DifferentialVarDescriptor *differential_variables;
  int has_guards;		/* non-zero if mode has one or more guards */
  int (*invariant_function)(struct Component *);
  void (*entering_function)(struct Component *);
  void (*exiting_function)(struct Component *);
  struct TransitionDescriptor **outgoing;
} ModeDescriptor;


/* Descriptor for an internal (exported) event.
 *
 * /--- NAME is the SHIFT name of the event.
 *
 * TRANSITIONS is a null-terminated descriptor pointer array
 * containing all the transitions labeled by the event.
 *
 * BACKLINK_VAR_OFFSET is the offset of BACKLINK_VAR in the data area
 * of each component instance.  This variable contains the set of
 * components which want to synchronize with this event in this
 * instance. ---/
 */

typedef enum {
  CLOSED_EVENT_T,
  OPEN_EVENT_T
} EventTypeDescriptor;

typedef struct EventDescriptor {
  char *name;			/* SHIFT event name */
  struct TransitionDescriptor **transitions;
  int backlink_var_offset;	/* Synchronizing components */
  int traced;			/* Set to 1 if the event traces */
  int broken;			/* Set to 1 if the event breaks */
  EventTypeDescriptor type;     /* Event Type 
				 * Added by Tunc Simsek 
				 * 16th December, 1997
				 * 22nd December, 1997
				 */
} EventDescriptor;


#define SYNC_ONE 11
#define SYNC_ALL 22
#define SINGLE_CONNECTION 33
#define SET_CONNECTION 44


/* External Event Function Expressions Types
 * (We could - or should - define two types based on the return type:
 * either Component* or GenericSet*.
 */

typedef
void* (*external_event_expr)(struct Component*);


/* Descriptor for an external event.
 *
 * /--- The external event is in the form C : E, optionally followed
 * by (ONE) or (ONE : V) or (V) or (ALL).
 *
 * NAME is the SHIFT name of the external event (after the ':').
 *
 * CONNECTION_TYPE is SINGLE_CONNECTION or SET_CONNECTION.  This
 * determines the type of the variable C at offset LINK_VAR_OFFSET in
 * each component instance.  For a single connection, C is a component
 * pointer. For a set connection, it is a set of components.
 *
 * SYNC_TYPE has a meaning only for set connections.  It specifies
 * whether all components in the set must synchronize (ALL), or just
 * one.
 *
 * If SYNC_TYPE is SYNC_ONE, and SYNC_VAR_OFFSET is not -1, then
 * SYNC_VAR_OFFSET is the offset of V, and the run-time must store in
 * the variable at that offset a pointer to the component that was
 * chosen as a participant in the synchronized transition.
 *

 * EVENT is a pointer to the event descriptor for E. ---/
 */
typedef struct ExternalEventDescriptor {
  char* name;			 /* SHIFT external event name */
  int connection_type;		 /* SINGLE_CONNECTION or SET_CONNECTION */
  int link_var_offset;		 /* Connected component */
  int global;			 /* Is the link variable a global? */
  int sync_type;		 /* SYNC_ONE or SYNC_ALL */
  int sync_var_offset;		 /* Variable for SYNC_ONE, or -1 */
  struct EventDescriptor* event; /* Event reference */
  char* var_name;		 /* Name of the external variable */
  external_event_expr ext_ev_expr; /* Function implementing the
				    * generation of the proper
				    * component (or set) with which to
				    * synchronize.
				    */
} ExternalEventDescriptor;


/* Descriptor for a transition.
 *
 * /--- TO is the destination mode of the transition.  The source is
 * not specified: it can be reconstructed, if needed, from the mode
 * descriptors.
 *
 * EVENTS is a null-terminated array of addresses of all the local
 * (exported) events for the transition.
 *
 * SYNC_LIST is a null-terminated array of addresses of external
 * events which must synchronize with this transition.
 *
 * GUARD is a logical function which when passed a pointer to a
 * component returns the value of the guard for this transition.
 *
 * The Fi functions execute various phases of the actions.  In a
 * synchronized transition involving a set S of components, F1 is
 * called for all elements of S, then F2, etc. ---/
 */
typedef struct TransitionDescriptor {
  ModeDescriptor* to;
  EventDescriptor** events;		/* Local events */
  ExternalEventDescriptor** sync_list;	/* External events */
  int (*guard)(struct Component*);	/* Functions for action phases */
  void (*define_f)(struct Component*);
  void (*f1)(struct Component*);	/* Right hand-side evaluation */
  void (*f2)(struct Component*);	/* Left hand-side evaluation */
  shift_api_transition* api_trans;	/* Corresponding api structure */
} TransitionDescriptor;


typedef struct ComponentEventPairDescriptor {
  int offset;		/* Offset of the variable holding the component */
  EventDescriptor *e;	/* Event */
} ComponentEventPairDescriptor;


/* Descriptor for an external sync from the setup actions.  It is a
 * list of component-event pair descriptors (as in a:b <-> c:d <->
 * e:f).
 */
typedef struct ExternalSyncDescriptor {
  ComponentEventPairDescriptor *ceps;
} ExternalSyncDescriptor;


/* Descriptor for a component type.
 *
 * /--- <.2.>
 *
 * NAME is the SHIFT name of the component.
 *
 * DATA_SIZE is the size (in bytes) of the block of memory that the
 * DATA field in the component points to.
 *
 * EVENTS is a null-terminated array of pointers to exported event
 * descriptors.
 *
 * VARIABLES is a pointer to an array of descriptors for all variables
 * in the component, both user declared and internally generated.
 *
 * MODES is a list of modes (discrete states) of the Type.
 *
 * BREAK_FLAG is the break status of the type.
 *
 * TRACE_FLAG is the trace status of the type.
 *
 * PRINTVAR is a list of attributes printed when debugging/tracing.
 *
 * ALLEVENTSTRACED = 0 means that user specified event name for tracing.
 *
 * ALLEVENTSBROKEN = 0 means that user specified event name for breaking.
 *
 * PARENT is the parent of the current type (if any).
 *
 * CHILDREN is the list of children of the current type (if any).
 *
 * COMPONENT_FIRST is the pointer to the first component of the current
 * type. ---/
 *
 */
typedef struct ComponentTypeDescriptor
{
  /* Structure */
  char* name;			     /* Name of the type */
  int data_size;		     /* Storage requirement, in bytes */
  EventDescriptor** events;	     /* Exported events */
  VariableDescriptor* variables;     /* List of all variables */
  ModeDescriptor** modes;	     /* List of all modes */
  ExternalSyncDescriptor* ext_syncs; /* External syncs from setup */

  /* Inspecting/Debugging*/
  int break_flag;		     /* For debugging: break status */
  int trace_flag;		     /* For debugging: trace status */
  VariableDescriptor** printVars;    /* List of Attributes printed */
  int allEventsTraced;		     /* 1 all; 0 some */
  int allEventsBroken;		     /* 1 all; 0 some */
  struct ComponentTypeDescriptor* parent;
				     /* Parent type (if any) */
  struct ComponentTypeDescriptor** children;
				     /* List of children types (if any) */
  struct Component* component_first; /* First component */

#ifdef HAVE_NEW_HASH_P
  int ctd_cached_hcode;		     /* Cached hash code for this type */
#endif /* HAVE_NEW_HASH_P */

  int synchronizes_externally_p;     /* Does the type have external
				      * synchronizations on the transitions?
				      */
} ComponentTypeDescriptor;


/* The implementation type for SHIFT components.
 *
 * /--- <.1.>
 *
 * DESC points to a descriptor of the component type (see <.2.>).
 *
 * M is the mode descriptor for the current mode of the component,
 * also known as its discrete state.  It is updated by the run-time
 * system during a transition.  Currently, the entering function (see
 * <>) of a mode sets this field.
 *
 * DATA is a pointer to a block of memory where the component's data
 * is stored.  This includes inputs, outputs, states, and several
 * other compiler-generated variables.
 *
 * MARK is used for marking in traversals (not clear if needed yet).
 *
 * LIVE_NEXT links the set of all components that have discrete behavior,
 * that is their current discrete state has outgoing edges.
 *
 * CONTINUOUS_NEXT links the set of all components with continuous
 * behavior in some mode.
 *
 * COMPONENT_NEXT links the set of all components with the same type.
 *
 * BREAK_FLAG is the break status of the component.
 *
 * TRACE_FLAG is the trace status of the component.
 *
 * PRINTVARS is a list of attributes printed when tracing.
 *
 * TRACEDEVENTS is a list of events that cause the component to trace.
 *
 * BROKENEVENTS is a list of events that cause the component to break. ---/
 *
 * Design note.  There has been some discussion as to whether the
 * compiler should instead generate a subclass of COMPONENT for each
 * SHIFT component type, with the component's data, now stored in
 * DATA, mapped instead to attributes of the subclass.  This may make
 * code generation somewhat easier (no hassles with inheritance) but
 * would result in more complex type casting for flow functions and
 * other data-related operations.
 * 
 */
typedef struct Component {
  int name;				/* Unique id within type */
  ComponentTypeDescriptor* desc;	/* Descriptor of component type */
  ModeDescriptor* M;			/* Current mode (discrete state) */
  char* data;				/* Instance variables */
  int mark;				/* For traversals */
  TransitionDescriptor** searching;	/* For synchronization search */
  int break_flag;			/* For debugging: break status */
  int trace_flag;			/* For debugging: trace status */
  struct Component* live_next;		/* List of live components */
  struct Component* continuous_next;	/* List of differentiable components */
  struct Component* component_next;	/* List of components of a same type */
  VariableDescriptor** printVars;	/* List of Attributes printed */
  char** tracedEvents;			/* List of trace events */
  char** brokenEvents;			/* List of break events */
  int allEventsTraced;			/* 1 all; 0 some */
  int allEventsBroken;			/* 1 all; 0 some */
} Component;


/* Component data item accessor.
 */
#define DATA_ADDRESS(type, c, offset) ((type *) ((c)->data + offset))
#define DATA_ITEM(type, c, offset) (*DATA_ADDRESS(type, c, offset))

/* Variable modes.
 */
#define DIFFERENTIAL_MODE	1
#define ALGEBRAIC_MODE		2
#define CONSTANT_MODE		3

#define MODE(c, offset) (*DATA_ADDRESS(int, c, offset))
#define FLOW(c, offset) (*DATA_ADDRESS(DoubleFlowFunction, c, offset))
#define TFLOW(c, offset, fftype) \
            (*DATA_ADDRESS(fftype, c, offset))
#define CONTEXT(c, offset) (*DATA_ADDRESS(Component *, c, offset))

/* Iterate V through all indices of differentiable variables of C.
 */
#define FOR_ALL_DIFFERENTIAL_VARIABLES(v, c)				     \
{									     \
    Component *_c = c;							     \
    ComponentTypeDescriptor *_t = _c->desc;				     \
    ModeDescriptor *_m = _c->M;						     \
    DifferentialVarDescriptor *v = _m->differential_variables;		     \
    for (; v->state_offset != -1; v++) {

#define END_FADV }}

/* Macros to access derivative function and integration vectors.
 */
#define F(c, v, step) ((v->flow_function)(c, step, _recur_depth))
#define X(c, v, step) (DATA_ADDRESS(double, c, v->state_offset)[step])
#define F0(c, v) X(c, v, 4)
#define F1(c, v) X(c, v, 5)
#define F2(c, v) X(c, v, 6)
#define DYM(c, v) X(c, v, 7)
#define XNEW(c, v) X(c, v, 8)



/* Macros to access component variables. */
/* Luigi, Marco, Michael 19960613
 * Changed the accessors to take the 'context' parameters too.
 */

#define _VA_(c, type, offset) DATA_ITEM(type, c, offset)

#define _AA_(c, mode, v, ff, context, step) \
	(DATA_ITEM(int, c, mode) == CONSTANT_MODE? \
	 DATA_ITEM(double, c, v) : \
 	 ((DATA_ITEM(DoubleFlowFunction, c, ff))\
	  (DATA_ITEM(Component *, c, context), step, _recur_depth)))


/* _AAT_ -- Algebraic Accessor with type parameter for flow function.
 * See also the LHS accessor '_AATL_'.
 * Marco Antoniotti, Aleks Gollu, Mikhail Kourjanski 19961107
 */

#define _AAT_(c, mode, v, vtype, ff, fftype, context, step) \
	(DATA_ITEM(int, c, mode) == CONSTANT_MODE? \
	 DATA_ITEM(vtype, c, v) : \
 	 ((DATA_ITEM(fftype, c, ff))\
	  (DATA_ITEM(Component *, c, context), step, _recur_depth)))

#define _DA_(c, mode, state, ff, context, step) \
	(DATA_ITEM(int, c, mode) == DIFFERENTIAL_MODE? \
	 (DATA_ADDRESS(double, c, state))[step] : \
	 _AA_(c, mode, state, ff, context, step))

#define _AAL_(c, mode, v, ff, context) DATA_ITEM(double, c, v)

/* Maybe the AATL accessor needn't be like this.
 * Let's see what happens if we maintain a simple setup.
 * Marco Antoniotti 19961212
 */
/* #define _AATL_(c, mode, v, ff, fftype, context) DATA_ITEM(fftype, c, v) */
#define _AATL_(c, mode, v, vtype, ff, fftype, context) \
     DATA_ITEM(vtype, c, v)

#define _DAL_(c, mode, state, ff, context, step) \
	_AAL_(c, mode, state, ff, context)
#define _VAL_(c, type, offset) _VA_(c, type, offset)

#define FOR_ALL_COMPONENTS(x, set)					     \
     FOR_ALL_ELEMENTS(_d, set) {					     \
	 Component *x = (Component *) _d.v;

#define END_FAC } END_FAE

/* Functions and global variables. */

/* List of all component type descriptors.
 */
extern ComponentTypeDescriptor *component_type_list[];

/* List of components which are capable of actions (that is, they have
 * transitions going out of their current discrete state).
 */
extern Component *live_components;	

/* List of components which are either live (see above) or have
 * differential behavior in the current state.
 */
extern Component *continuous_components;	

extern struct GenericSet *potential_transitions;
extern struct GenericSet *differential_components;

extern TypeDescriptor generic_component_type;
extern TypeDescriptor cvp_type;

Component *_global;

Component *createMaster();
void continuous_step();
void discrete_step();


/* Debugging stuff.
 */
extern char *dump_continuous_state;
extern int shift_debug;
extern int shift_gui_debug;
extern long tclick;

#if 0
/* Special assignments.
 *
 * /--- The compiler could generate type-specific code, but for the
 * moment we interpret these assignments.  They need to be handled
 * specially because they alter the synchronization structure of
 * groups of components (the backpointer sets must be updated). ---/
 */
#define link_assign(l, r, s, c) do_link_assign(&l, r, s, c)
#define linkset_assign(l, r, s, c) do_linkset_assign(&l, r, s, c)
#else
/* Backpointers are too much trouble to maintain in the presence of
 * global variables.  For now we recompute them after every
 * transition.
 */
#define link_assign(l, r, s, c) l = r
#define linkset_assign(l, r, s, c) l = r
#endif


void do_link_assign(Component **lp, Component *r, Component *self,
		    ExternalEventDescriptor **ex);

void do_linkset_assign(void **lp, void *r, Component *self,
		       ExternalEventDescriptor **ex);

#endif /* SHIFTTYPES_H */
