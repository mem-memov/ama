/** @file
 * @brief Compose counted source/term/solver/guard fixtures with independently owned key storage.
 */
#ifndef AMA_STATIC_KEY_TEST_COMMON
#define AMA_STATIC_KEY_TEST_COMMON
#include "../../guards/common.c"
#include "semantic/static/registry/identity/internal.h"
#include <string.h>

/** Normalize through the actual solver/guard before using the key codec. */
static inline const StaticTermEnvironment *key_environment(const StaticTermEnvironment *environment) {
    const StaticGuardOwner *owner;
    assert(static_guard_owner(&guards, environment, &owner));
    const StaticTermEnvironment *normalized;
    StaticConstraintOutcome outcome;
    assert(static_guard_owner_key(&guards, owner, &normalized, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE);
    return normalized;
}

/** Construct one complete ordinary owner from exact original formal/value pairs. */
static inline const StaticTermEnvironment *key_ordinary(const StaticModule *module,
    const StaticTermCapture *inputs, size_t count) {
    const StaticTerm *type;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, module, inputs, count, (SourceSpan){0}, &type));
    assert(static_term_environment(&terms, module, type, &environment));
    return key_environment(environment);
}

/** Public descriptor construction preserves source/term owners until teardown. */
static inline StaticKeyEncoding key_encode(SourceArena *arena, const StaticTermEnvironment *environment) {
    StaticKeyEncoding key;
    assert(static_key_encode(&terms, key_environment(environment), arena, &key) == STATIC_KEY_OK);
    assert(strlen(key.descriptor) == key.length);
    return key;
}
#endif
