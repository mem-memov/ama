/**
 * @file
 * @brief Represent scoped static identities separately from runtime semantic fields.
 *
 * [TPL-02](../../../spec/language/templates.md#tpl-02) gives parameters exact
 * module-local identities. [TPL-07](../../../spec/language/templates.md#tpl-07)
 * separates Type from finite named Template signatures.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) separates results
 * and source-visible aliases from free formals and runtime storage.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) keeps bound static
 * metadata distinct from source syntax and executable layout.
 */
#ifndef AMA_SEMANTIC_STATIC_MODEL_H
#define AMA_SEMANTIC_STATIC_MODEL_H

#include "project/dependencies/project.h"

/** Deferred denotes an explicit unresolved obligation, never an inferred Type.
 * Component contract: [TPL-07](../../../spec/language/templates.md#tpl-07)
 * fixes parameter categories independently of which actual is processed first.
 */
typedef enum { STATIC_CATEGORY_DEFERRED, STATIC_CATEGORY_TYPE, STATIC_CATEGORY_TEMPLATE } StaticCategory;

/** Only parameters contribute owner specialization inputs; results and aliases
 * have separate identities and never create runtime fields.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12).
 */
typedef enum { STATIC_PARAMETER, STATIC_RESULT, STATIC_ALIAS } StaticRole;

/** Forward identity for an inventory-owned expected constructor interface.
 * Component contract: [TPL-07](../../../spec/language/templates.md#tpl-07).
 */
typedef struct StaticSignature StaticSignature;

/** An actual named slot owns its category/signature description. A deferred
 * category remains unresolved until its owning symbol supplies metadata.
 * Component contract: [TPL-07](../../../spec/language/templates.md#tpl-07)
 * matches whole foreign names without importing them into caller scope.
 */
typedef struct StaticSignatureEntry {
    struct StaticSignatureEntry *next;
    SourceSpan name;
    StaticCategory category;
    StaticSignature *signature;
} StaticSignatureEntry;

/** Finite inventory-owned signature; source order is retained but is not a
 * positional parameter interface. Equality must match complete name sets.
 * Component contract: [TPL-07](../../../spec/language/templates.md#tpl-07).
 */
struct StaticSignature {
    StaticSignatureEntry *first;
    StaticSignatureEntry *last;
    size_t count;
};

/** Forward scoped definition identity, borrowing exactly one loaded module.
 * Component contract: [TPL-02](../../../spec/language/templates.md#tpl-02).
 */
typedef struct StaticModule StaticModule;

/** One inventory-owned local name with explicit role and original syntax.
 * site is its introduction; aliases become visible at that site's start.
 * Role ordinals are assigned only by completed interface collection. Borrowed
 * syntax/source bytes outlive this record; no value or instance is stored here.
 * Component contract: [TPL-02](../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../spec/language/static-results.md#tpl-12).
 */
typedef struct StaticSymbol {
    struct StaticSymbol *next;
    StaticModule *owner;
    SourceSpan name;
    SourceSpan site;
    StaticRole role;
    StaticCategory category;
    StaticSignature *signature;
    const AstNode *declaration;
    const AstStaticBinding *receiver;
    size_t ordinal;
} StaticSymbol;

/** Source position context, independent of whether any name is already bound.
 * Contract: [TPL-02](../../../spec/language/templates.md#tpl-02) restricts free
 * introductions to interface expressions, apart from constant/body uses.
 */
typedef enum {
    STATIC_CONTEXT_CONSTANT, STATIC_CONTEXT_INPUT, STATIC_CONTEXT_RESULT,
    STATIC_CONTEXT_DEFINITION, STATIC_CONTEXT_BODY, STATIC_CONTEXT_NAMESPACE
} StaticContext;

/** One source head or receiver occurrence; receiver is nonnull only for an
 * output action. Foreign labels have no occurrence in this local namespace.
 * Collection retains context and owning syntax before binding symbol/origin.
 * Contract: [TPL-02](../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../spec/language/static-results.md#tpl-12).
 */
typedef struct StaticOccurrence {
    struct StaticOccurrence *next;
    SourceSpan site;
    const AstTypeExpression *expression;
    const AstStaticBinding *receiver;
    const AstNode *statement;
    StaticContext context;
    bool root;
    const StaticSymbol *symbol;
    const ProjectModule *origin;
} StaticOccurrence;

/** An original qualified constant source has its own AST/dependency identity,
 * separate from atomic static placeholders and fabricated type expressions.
 * [CONST-05](../../../spec/language/declarations.md#const-05) fixes the written namespace.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) retains each request occurrence.
 */
typedef struct StaticConstantOccurrence {
    struct StaticConstantOccurrence *next;
    const AstSource *source;
    const AstNode *statement;
    const ProjectModule *origin;
} StaticConstantOccurrence;

/** One definition's local namespace, with independent parameter/result counts.
 * Component contract: [TPL-02](../../../spec/language/templates.md#tpl-02)
 * makes identical spellings in different definitions independent identities.
 */
struct StaticModule {
    StaticModule *next;
    const ProjectModule *definition;
    StaticSymbol *first;
    StaticSymbol *last;
    size_t parameter_count;
    size_t result_count;
    StaticOccurrence *first_occurrence;
    StaticOccurrence *last_occurrence;
    size_t occurrence_count;
    StaticConstantOccurrence *first_constant_source;
    StaticConstantOccurrence *last_constant_source;
    size_t constant_source_count;
    StaticSignature *constructor_signature;
};

#endif
