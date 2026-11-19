/** @file
 * @brief Separate finite equations and variable solutions from immutable static values.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves shared structural terms.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains original binding reasons and owns allocation-free diagnostic work links.
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
    /** A successful binding retains its exact original equation. Diagnostic
     * frames are owned here, never allocated after observing a conflict.
     * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves definitions and equality receipts through normalization.
     */
    const StaticConstraintEquation *reason;
    DiagnosticContext definition_context;
    DiagnosticContext binding_context;
    /** Failure-only work links visit each causal variable once without allocating.
     * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves causal notes even after memory refusal.
     */
    struct StaticConstraintVariable *evidence_next;
    bool evidence_seen;
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
