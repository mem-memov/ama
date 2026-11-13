/**
 * @file
 * @brief Declare project-root companion types and hooks using only qualified author identifiers.
 */
#ifndef AMA_NAMESPACE_PROJECT_FIXTURE_H
#define AMA_NAMESPACE_PROJECT_FIXTURE_H
#include "native/status/library.h"
/** Module-local payload deliberately shares its basename with another companion. */
typedef struct { int value; } AMA_NATIVE(State);
/** Construct one caller-owned payload without allocating. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state);
/** Copy into distinct caller-owned payload without aliasing state. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source);
/** Return an immutable module-specific message and provider-local reason. */
AmaStatus AMA_NATIVE(failure)(void);
/** Evaluate each error constructor argument once, independently of C argument order. */
AmaStatus AMA_NATIVE(evaluate_once)(size_t counts[3]);
#endif
