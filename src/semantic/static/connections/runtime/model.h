/** @file
 * @brief Represent fixed runtime name bindings without requiring concrete specialization Types.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) fixes fields before Type solving.
 * [NAME-06](../../../../../spec/language/declarations.md#name-06) scopes foreach locals lexically.
 */
#ifndef AMA_STATIC_RUNTIME_MODEL_H
#define AMA_STATIC_RUNTIME_MODEL_H
#include "events/model.h"

/** Shared storage and three instance interfaces retain separate original roles.
 * [NAME-04](../../../../../spec/language/declarations.md#name-04) gives them one base-name namespace.
 */
typedef enum { STATIC_FIELD_CONSTANT, STATIC_FIELD_INPUT, STATIC_FIELD_RESULT, STATIC_FIELD_STATE } StaticRuntimeRole;

/** Source-owned field identity. Explicit declarations retain their Type syntax;
 * inferred receivers retain their original access; a new foreach local retains
 * its header/body scope. Type terms are supplied later per actual environment.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) prohibits rebinding during solving.
 */
typedef struct StaticRuntimeField {
    struct StaticRuntimeField *next;
    SourceSpan name;
    SourceSpan site;
    StaticRuntimeRole role;
    const AstNode *statement;
    const AstTypeExpression *type_expression;
    const AstAccess *receiver;
    const AstNode *scope;
    bool reference;
    bool list;
    bool inferred;
} StaticRuntimeField;

/** One actual local/module-variable use, including an index operand. A foreach
 * collection has a source span but no fabricated AstAccess. Constant/module
 * labels and result selector bases never produce a use entry.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) enumerates actual variable occurrences.
 */
typedef struct StaticRuntimeUse {
    struct StaticRuntimeUse *next;
    const AstNode *statement;
    const AstAccess *syntax;
    SourceSpan site;
    const StaticRuntimeField *field;
    const StaticRuntimeField *initializing;
    bool index;
} StaticRuntimeUse;

/** Assignment/output receivers keep the lexical fresh/existing decision even
 * when their eventual source/result Type is still unknown.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) prevents Type-dependent reclassification.
 */
typedef struct StaticRuntimeReceiver {
    struct StaticRuntimeReceiver *next;
    const AstNode *statement;
    const AstAccess *syntax;
    const StaticRuntimeField *field;
    bool introduced;
} StaticRuntimeReceiver;

/** A traversal's source and reused/new scalar reference are fixed at its header.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) forbids shadowing an incompatible visible cell.
 */
typedef struct StaticRuntimeForeach {
    struct StaticRuntimeForeach *next;
    const AstNode *syntax;
    const StaticRuntimeField *collection;
    const StaticRuntimeField *binding;
    bool introduced;
} StaticRuntimeForeach;

/** Original encountered-name evidence excludes local declarations/uses.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) fixes simple receiver introductions.
 */
typedef struct StaticRuntimeEncounter {
    struct StaticRuntimeEncounter *next;
    SourceSpan name;
} StaticRuntimeEncounter;

/** One source module's immutable lexical result, reusable across specialization
 * environments. Partial chains remain arena-owned until successful publication.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) makes name identity independent of Type values.
 */
typedef struct StaticRuntimeModule {
    struct StaticRuntimeModule *next;
    const StaticModule *source;
    StaticRuntimeEvent *first_event;
    StaticRuntimeEvent *last_event;
    size_t event_count;
    StaticRuntimeField *first_field;
    StaticRuntimeField *last_field;
    size_t field_count;
    StaticRuntimeUse *first_use;
    StaticRuntimeUse *last_use;
    size_t use_count;
    StaticRuntimeReceiver *first_receiver;
    StaticRuntimeReceiver *last_receiver;
    size_t receiver_count;
    StaticRuntimeForeach *first_foreach;
    StaticRuntimeForeach *last_foreach;
    size_t foreach_count;
    StaticRuntimeEncounter *encountered;
    size_t encounter_count;
    SourceSpan inferring;
} StaticRuntimeModule;
#endif
