/** @file
 * @brief Bind compiler-owned foreach continuation mapping to object copy phases.
 * [CONT-04](../../../../../../spec/language/continuations.md#cont-04) copies live
 * boundaries and nonowning bindings through exact membership/reference maps;
 * [COPY-03](../../../../../../spec/language/values-and-references.md#copy-03)
 * leaves ordinary copies' fresh control cells inactive.
 */
#ifndef AMA_FOREACH_COPY_H
#define AMA_FOREACH_COPY_H
#include "runtime/program/lists/traversal/traversal.h"
/** Shared immutable callbacks; each type supplies its own context-local program. */
extern const AmaObjectControlHooks ama_program_foreach_controls;
#endif
