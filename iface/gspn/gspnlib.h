#ifndef __GSPN_LIB_H__
#define __GSPN_LIB_H__

#include "stdlib.h"
g
/* Header file defining functions for Spot API */

/* -------------- Interface type definitions -------------- */

typedef  unsigned long AtomicProp;
typedef AtomicProp * pAtomicProp;
typedef enum  {STATE_PROP,EVENT_PROP} AtomicPropKind;
typedef AtomicPropKind * pAtomicPropKind ;

typedef  unsigned long State;
typedef State * pState ;

typedef struct EventPropSucc {
  State s;
  AtomicProp p;   
} EventPropSucc ;

#define EVENT_TRUE 0


/* ----------------- Utility Functions --------------------*/

/* Initialize data structures to work with model and properties
   MANDATORY : should be called before any other function in this library 
   Non 0 return correspond to errors in initialization
   Call with arguments similar to WNRG/WNSRG/WNERG
*/
int initialize (int argc, char **argv) ;
/* Close and cleanup after using the library */
int finalize (void);


/* ----------------- Property related Functions ----------------*/

/* Returns the index of the property given as "name" 
   Non zero return = error codes */
int prop_index (const char * name, pAtomicProp  propindex) ;

/* Returns the type of "prop" in "kind" 
   non zero return = error codes 
*/
int prop_kind ( const AtomicProp prop, pAtomicPropKind  kind) ;


/* ------------------ State Space Exploration -----------------*/

/* Returns the identifier of the initial marking state */
int initial_state (pState M0);

/* Given a state "s" and a list of "props_size" property indexes "props" checks the truth value of
   every atomic property in "props" and returns in "truth" the truth value of these properties
   in the same order as the input, ONE CHAR PER TRUTH VALUE (i.e. sizeof(truth[]) = props_size 
   NB : the vector "truth" is allocated in this function 
*/
int satisfy (const State s, const AtomicProp  props [],  unsigned char ** truth, size_t props_size);

/* free the "truth" vector allocated by satisfy 
   !!! NB: Don't forget to call me, or you will get a memory leak !!!
*/
int satisfy_free (unsigned char * truth);


/* Calculates successors of a state "s" that can be reached by verifying at least one 
   event property specified in "enabled_events". In our first implementation enabled
   events is discarded, and ALL successors will be returned. This behavior is also
   obtained by giving "TRUE" in the list of enabled events.
   Each successor is returned in a struct that gives the Event property verified by the transition
   fired to reach this marking; 
   If a marking is reached by firing a transition observed by more than one event property, it will 
   be returned in many copies:
   i.e. E1 and E2 observe different firngs of transition t1 ; M1 is reached from M0 by firing t1 with
   a binding observable by both E1 and E2 :
   succ (M0, [E1,E2] , ...)
   will return {[M1,E1],[M1,E2]}

   NB : "succ" vector is allocated in the function, use succ_free for memory release
*/
int succ (const State s, const AtomicProp  enabled_events [], size_t  enabled_events_size, 
	  EventPropSucc ** succ, size_t * succ_size);

/* free the "succ" vector allocated by succ 
   !!! NB: Don't forget to call me, or you will get a memory leak !!!
*/
int succ_free ( EventPropSucc * succ );




#endif

