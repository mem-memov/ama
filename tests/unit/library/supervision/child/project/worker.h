/** @file
 * @brief Keep an address-sensitive child native resource alive across several Ama yields.
 */
#ifndef AMA_NESTED_WORKER_STATE_H
#define AMA_NESTED_WORKER_STATE_H
#include "native/abi/services.h"
/** The pointer is an ordinary C resource, never an Ama reference; cloning is omitted. */
struct AMA_NATIVE(State) { void *resource; const void *address; size_t calls; bool fail_destroy; };
/** Acquire one native allocation at its final stable payload address. */
AmaStatus AMA_NATIVE(initialize)(struct AMA_NATIVE(State) *);
/** Release once even when deliberately reporting a secondary cleanup failure. */
AmaStatus AMA_NATIVE(destroy)(struct AMA_NATIVE(State) *);
#endif
