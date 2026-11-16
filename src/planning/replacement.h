/** @file
 * @brief Describe one typed failure-safe replacement and its source-lifetime handoff.
 * [CALL-08](../../spec/language/calls-and-results.md#call-08) fixes separate
 * input-evaluation, interface-installation and output-transfer orders;
 * [REF-03](../../spec/language/values-and-references.md#ref-03) separates preparation
 * from destructive commit without a whole-call transaction or implicit snapshot.
 */
#ifndef AMA_PLANNED_REPLACEMENT_H
#define AMA_PLANNED_REPLACEMENT_H
#include "storage/state.h"
/** Determine which side owns the replacement receiver, without resolving source syntax again. Component contract: [CALL-08](../../spec/language/calls-and-results.md#call-08), [REF-03](../../spec/language/values-and-references.md#ref-03). */
typedef enum { REPLACE_ASSIGNMENT, REPLACE_PARAMETER, REPLACE_RESULT, REPLACE_OUTPUT, REPLACE_METHOD_PARAMETER } ReplacementContext;
/** Exactly one construction path; a supplied value never constructs then overwrites the default. Component contract: [CALL-08](../../spec/language/calls-and-results.md#call-08), [REF-03](../../spec/language/values-and-references.md#ref-03). */
typedef enum { REPLACEMENT_SUPPLIED_SOURCE, REPLACEMENT_SUPPLIED_LITERAL,
    REPLACEMENT_DECLARED_DEFAULT, REPLACEMENT_RETAINED_RESULT } ReplacementConstruction;
/** Forward tag for endpoints owned by the control instruction inventory. Component contract: [CALL-08](../../spec/language/calls-and-results.md#call-08), [REF-03](../../spec/language/values-and-references.md#ref-03). */
struct ControlNode;
/** One arena-owned recipe shared by all phases of a single replacement.
 * source/receiver are borrowed resolved operands. Parameter/result fields belong
 * to the callee; an output field identifies retained source storage while its
 * receiver is in the initiating caller. Defaults borrow field initializers and
 * run in the callee context. Supplied sources retain caller selection protection
 * until their own release phase, without copying payloads as snapshots.
 * Component contract: [CALL-08](../../spec/language/calls-and-results.md#call-08), [REF-03](../../spec/language/values-and-references.md#ref-03). */
typedef struct ReplacementPlan {
    struct ReplacementPlan *allocated_next;
    const struct ControlNode *first;
    const struct ControlNode *last;
    const struct ControlNode *commit;
    ReplacementContext context;
    ReplacementConstruction construction;
    const SemanticStatement *statement;
    const SemanticBinding *binding;
    const SemanticSymbol *field;
    const SemanticListField *intrinsic;
    size_t position;
    const SemanticSource *source;
    const SemanticAccess *receiver;
    SemanticType *value_type;
    StorageForm form;
    SemanticTransfer transfer;
} ReplacementPlan;
#endif
