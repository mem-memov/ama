/** @file
 * @brief Retain source-bound static expressions and result definitions separately from their values.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates producers and selected results.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves lexical substitution scope.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves guarded use ancestry.
 */
#ifndef AMA_STATIC_RESULT_MODEL_H
#define AMA_STATIC_RESULT_MODEL_H
#include "../guards/plan.h"

/** One original expression/environment key. The expression result is distinct
 * from every derived output/receiver variable, even when later values are equal.
 * Definition evaluation and provenance uses are separate later operations.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves producer denotation.
 */
typedef struct StaticResultSource {
    struct StaticResultSource *next;
    const StaticTermEnvironment *environment;
    const AstTypeExpression *expression;
    const StaticOccurrence *occurrence;
    const StaticTerm *head;
    const StaticTerm *value;
} StaticResultSource;

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
 * value-cache hit under a new incoming path.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) validates incoming paths.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves definition dependencies.
 */
typedef struct StaticResultDefinitionUse {
    struct StaticResultDefinitionUse *next;
    const StaticResultDefinition *definition;
    const StaticResultContext *context;
    const StaticDefinitionPath *path;
    StaticResultUse *rhs;
    StaticResultOutcome outcome;
    bool active;
    bool cache_hit;
} StaticResultDefinitionUse;

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
    const StaticTermApplication *prepared;
    const StaticApplicationRecord *application;
    const StaticResultContext *producer;
    StaticResultOutcome outcome;
    bool active;
    bool closed;
};

/** Cache independently computed, checked, closed definition values by their
 * complete owner identity and original symbol. No runtime instance is merged.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves result identity.
 */
typedef struct StaticResultCache {
    struct StaticResultCache *next;
    const StaticTerm *owner;
    const StaticSymbol *symbol;
    const StaticTerm *value;
} StaticResultCache;
#endif
