/** @file
 * @brief Separate closed specialization identities from incoming-use checking state.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) shares metadata identities, not instance state.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) retains each incoming path.
 */
#ifndef AMA_STATIC_REGISTRY_MODEL_H
#define AMA_STATIC_REGISTRY_MODEL_H
#include "../guards/plan.h"
#include "identity/key.h"

/** These states concern registry checks only; none admits executable code.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) separates naming from validation.
 */
typedef enum { STATIC_CANDIDATE_NEW, STATIC_CANDIDATE_PENDING,
    STATIC_CANDIDATE_CHECKED, STATIC_CANDIDATE_FAILED } StaticCandidateState;

/** One complete canonical owner map and its collision-checked compiler name.
 * Original source/term metadata is borrowed for the registry lifetime.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) preserves all bindings.
 */
typedef struct StaticRegistryCandidate {
    struct StaticRegistryCandidate *next;
    const StaticTermEnvironment *key;
    const IdentityName *name;
    size_t ordinal;
    StaticCandidateState state;
    bool local_started;
    bool local_complete;
} StaticRegistryCandidate;

/** A required lexical path exists before all its comparison keys necessarily
 * close. Unknown uses retain their exact path without inventing an identity.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves pending provenance.
 */
typedef enum { STATIC_REGISTRY_KEY_PENDING, STATIC_REGISTRY_KEY_CLOSED } StaticRegistryKeyState;

/** Distinct paths retain distinct use handles even when the candidate is shared.
 * Registry checking and whole-program executable admission remain separate.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) prevents cache-path bypass.
 */
typedef struct StaticRegistryUse {
    struct StaticRegistryUse *next;
    const StaticCreationPath *path;
    StaticRegistryCandidate *candidate;
    size_t ordinal;
    StaticRegistryKeyState key_state;
} StaticRegistryUse;
/** Registry checks are separate from known-key closure and executable admission.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) requires every local/dependency check.
 */
typedef enum { STATIC_REGISTRY_CHECKS_PENDING, STATIC_REGISTRY_CHECKS_CLOSED } StaticRegistryCheckState;

/** A validation dependency retains both actual incoming use handles. It is not
 * a constructor-creation edge: callers must separately register/check creation
 * provenance with the guards. Unknown endpoint keys remain obligations.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves every dependency.
 */
typedef struct StaticRegistryDependency {
    struct StaticRegistryDependency *next;
    const StaticRegistryUse *owner;
    const StaticRegistryUse *target;
    SourceSpan site;
} StaticRegistryDependency;
#endif
