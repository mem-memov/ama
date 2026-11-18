/** @file
 * @brief Separate finite equations and variable solutions from immutable static values.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves shared structural terms.
 */
#ifndef AMA_STATIC_CONSTRAINT_MODEL_H
#define AMA_STATIC_CONSTRAINT_MODEL_H
#include "../terms/store.h"
#include "../signature/signature.h"

/** Original/generated pairs retain their source parent even after refinement.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) retains application diagnostics.
 */
typedef struct StaticConstraintEquation {
    struct StaticConstraintEquation *next;
    const StaticTerm *left;
    const StaticTerm *right;
    const struct StaticConstraintEquation *parent;
    SourceSpan site;
} StaticConstraintEquation;

/** Mutable solver-owned links never overwrite immutable term identity.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) refines unknown information.
 */
typedef struct StaticConstraintVariable {
    struct StaticConstraintVariable *next;
    const StaticTerm *variable;
    const StaticTerm *value;
    const StaticTerm *representative;
} StaticConstraintVariable;

/** Caller-selected closure obligations do not make other free variables concrete.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) rejects required underdetermination.
 */
typedef struct StaticConstraintRequirement {
    struct StaticConstraintRequirement *next;
    const StaticTerm *term;
    SourceSpan site;
} StaticConstraintRequirement;
/** Independent fully resolved metadata refines deferred source descriptions;
 * it never guesses an origin or mutates a term/source record.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) refines known evidence.
 */
typedef struct StaticConstraintMetadata {
    struct StaticConstraintMetadata *next;
    const StaticTerm *term;
    StaticCategory category;
    const StaticSignature *signature;
    SourceSpan site;
} StaticConstraintMetadata;
/** Exact immutable signature-pair results remain valid across independent
 * refinements, which introduce a new description pointer rather than mutating
 * source trees. [TPL-10](../../../../spec/language/templates.md#tpl-10).
 */
typedef struct StaticConstraintSignature {
    struct StaticConstraintSignature *next;
    const StaticSignature *left;
    const StaticSignature *right;
    StaticSignatureComparison result;
} StaticConstraintSignature;
#endif
