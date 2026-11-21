/** @file
 * @brief Retain source-bound static expressions and result definitions separately from their values.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates producers and selected results.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves lexical substitution scope.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves guarded use ancestry.
 * [TPL-15](../../../../spec/language/templates.md#tpl-15) keeps declaration
 * scalar selections separate from their supplying producer identities.
 */
#ifndef AMA_STATIC_RESULT_MODEL_H
#define AMA_STATIC_RESULT_MODEL_H
#include "../registry/plan.h"

/** One original expression/environment key. Ordinary expressions have their
 * own value variable; graph producer statements retain no scalar value.
 * A declaration selector retains an independent scalar receiver and an exact
 * foreign result symbol, never an invented producer Type. Selected outputs
 * remain independent derived definition variables in all cases.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates values and producers.
 */
typedef struct StaticResultSource {
    struct StaticResultSource *next;
    const StaticTermEnvironment *environment;
    const AstTypeExpression *expression;
    const StaticOccurrence *occurrence;
    /** Actual static-module origin; its head is null because the producer itself
     * is not a value. Only a declaration selector has a scalar value receiver.
     * [TPL-15](../../../../spec/language/templates.md#tpl-15) separates selected
     * values from their fully determined suppliers.
     */
    const StaticModule *producer;
    const StaticSymbol *selection;
    const StaticTerm *head;
    const StaticTerm *value;
} StaticResultSource;

/** Opaque result-owned preparation cache, never a public application service.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) keeps source metadata
 * separate from the creation/definition checks retained by each actual use.
 */
typedef struct StaticResultOperands StaticResultOperands;
/** Independent whole-graph validation proof, separate from value-read ancestry.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves exact scoped metadata and its validation obligations.
 */
typedef struct StaticResultGraph StaticResultGraph;

/** One named producer request has a complete original-formal input map. Missing
 * written bindings use stable lexical inference variables, not new free inputs.
 * The result arena owns the array/record and borrows source, requests and terms.
 * It is neither a metadata value nor a checked producer environment.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates input closure and residual Templates.
 */
typedef struct StaticResultProducer {
    struct StaticResultProducer *next;
    const StaticResultSource *source;
    const BindingApplication *requests;
    const StaticTermCapture *inputs;
    size_t input_count;
} StaticResultProducer;

/** An explicit result reads its defining expression. An alias reads the exact
 * selected output at its original receiver, not that producer expression's own
 * Type. The original symbol scopes the independent derived value variable.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) defines immutable aliases.
 */
typedef struct StaticResultDefinition {
    struct StaticResultDefinition *next;
    const StaticTermEnvironment *environment;
    const StaticSymbol *symbol;
    const StaticResultSource *source;
    const AstStaticBinding *receiver;
    const StaticTerm *value;
} StaticResultDefinition;

/** A required creation occurrence supplies exact scope provenance; matching
 * source names or modules cannot invent that path.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves incoming paths.
 */
typedef struct StaticResultContext {
    struct StaticResultContext *next;
    const StaticCreationPath *creation;
    StaticRegistryUse *registry_use;
} StaticResultContext;

/** Ready concerns present static structural work, not executable admission or
 * unrelated enclosing input closure. Pending keeps its symbolic terms.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates equations from publication.
 */
typedef enum { STATIC_RESULT_READY, STATIC_RESULT_PENDING } StaticResultOutcome;

/** Forward identity for a persistent source-use evaluation record.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) retains use contexts.
 */
typedef struct StaticResultUse StaticResultUse;

/** Definition occurrences retain checked dependency ancestry, including on a
 * value-cache hit under a new incoming path. checked_rhs borrows an already
 * checked proof: prefer this actual context; a foreign proof requires closed
 * actual ancestor keys. Proof provenance never changes lexical environments.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) validates incoming paths.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves definition dependencies.
 */
typedef struct StaticResultDefinitionUse {
    struct StaticResultDefinitionUse *next;
    const StaticResultDefinition *definition;
    const StaticResultContext *context;
    const StaticDefinitionPath *path;
    StaticResultUse *rhs;
    struct StaticResultDefinitionUse *selected;
    const StaticResultUse *checked_rhs;
    StaticResultOutcome outcome;
    bool active;
    bool cache_hit;
} StaticResultDefinitionUse;

/** Exact checked source dependencies survive value-cache reuse. Receiver
 * equalities remain separate and add no definition edge here.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) retains required producer provenance.
 */
typedef struct StaticResultDependency {
    struct StaticResultDependency *next;
    const StaticResultUse *expression;
    const StaticResultDefinitionUse *definition;
} StaticResultDependency;

/** Persistent identity prevents resubmission from manufacturing fresh creation
 * contexts as apparent progress. Source value identity stays separate from its
 * incoming definition ancestry.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires real refinement.
 */
struct StaticResultUse {
    StaticResultUse *next;
    const StaticResultSource *source;
    const StaticResultContext *context;
    const StaticDefinitionPath *definition_path;
    const BindingApplication *requests;
    const StaticTermApplication *prepared;
    const StaticResultProducer *producer_request;
    StaticResultGraph *required_graph;
    const StaticApplicationRecord *application;
    const StaticResultContext *producer;
    StaticResultDefinitionUse *selection;
    StaticResultDependency *first_dependency;
    StaticResultOutcome outcome;
    bool active;
    bool closed;
};

/** Exactly one root alternative is nonnull. These are validation obligations,
 * not value-definition edges from whichever definition requested the producer.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) checks every public result and statement.
 */
typedef struct StaticResultGraphWork {
    struct StaticResultGraphWork *next;
    StaticResultUse *expression;
    StaticResultDefinitionUse *definition;
} StaticResultGraphWork;

/** One actual incoming context witnesses the canonical graph's local checks.
 * A same-key ancestral recurrence links to an existing ancestor witness instead
 * of expanding again. Independent incoming paths can retain different witnesses.
 * The result arena owns records; contexts/source uses are longer-lived siblings.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves pending ancestry without a first-visit heuristic.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates validation and definition cycles.
 */
struct StaticResultGraph {
    StaticResultGraph *next;
    const StaticResultContext *context;
    StaticResultGraphWork *first;
    StaticResultGraphWork *last;
    size_t work_count;
    bool local_complete;
};

/** Cache independently computed, checked, closed definition values by their
 * complete owner identity and original symbol. The original RHS proof survives
 * so publication can also inspect its independent receiver dependencies.
 * No runtime instance is merged.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves result identity.
 */
typedef struct StaticResultCache {
    struct StaticResultCache *next;
    const StaticTermEnvironment *owner;
    const StaticSymbol *symbol;
    const StaticTerm *value;
    const StaticResultUse *rhs;
} StaticResultCache;
#endif
