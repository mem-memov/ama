/** @file
 * @brief Represent immutable application snapshots with explicit omission and inference contexts.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) fixes independent expected signatures.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) distinguishes residuals and inferred fields.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) overrides omission for producers.
 */
#ifndef AMA_STATIC_APPLICATION_MODEL_H
#define AMA_STATIC_APPLICATION_MODEL_H
#include "../terms/application.h"

/** Partial expressions retain local residuals; every ordinary declaration uses
 * stable field inference, while constant namespaces must be independently closed.
 * [TPL-04](../../../../spec/language/templates.md#tpl-04) covers interface fields
 * as well as body fields; static producers have a separate graph evaluator.
 */
typedef enum { STATIC_OMIT_PARTIAL, STATIC_OMIT_FIELD, STATIC_OMIT_NAMESPACE } StaticOmissionPolicy;

/** Source-free prepared maps may omit syntax except when generating inference.
 * Source-backed policies must agree with the registered expression's actual role.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves lexical provenance.
 */
typedef struct {
    StaticOmissionPolicy policy;
    const StaticTermEnvironment *environment;
    const AstNode *declaration;
    const AstTypeExpression *expression;
    const StaticTermApplication *source;
    const BindingApplication *requests;
    const StaticTerm *result;
    SourceSpan site;
} StaticApplicationContext;

/** Each named binding owns a nonnull static value; omitted partial formals
 * have no map entry. Generated entries remain separate from written requests.
 * Array inputs ignore next/generated; published links are plan-owned snapshots.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves partial maps.
 */
typedef struct StaticApplicationInput {
    struct StaticApplicationInput *next;
    const StaticSymbol *formal;
    const StaticTerm *value;
    SourceSpan site;
    bool generated;
} StaticApplicationInput;

/** Candidate values are symbolic compiler structure, not executable admission.
 * A null value denotes an unknown actual origin; check() may resolve metadata
 * obligations, while a new structural snapshot is needed after origin discovery.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) retains complete captures.
 */
typedef struct StaticApplicationRecord {
    struct StaticApplicationRecord *next;
    const StaticTerm *head;
    const StaticTerm *constructor;
    const StaticTerm *value;
    StaticApplicationContext context;
    StaticApplicationInput *first;
    StaticApplicationInput *last;
    size_t input_count;
    bool applied;
    bool pending_origin;
} StaticApplicationRecord;
#endif
