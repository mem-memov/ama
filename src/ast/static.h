/**
 * @file
 * @brief Represent source-owned static expressions and named graph bindings.
 *
 * [COMP-02](../../spec/implementation/compiler.md#comp-02) separates immutable
 * syntax from resolved kinds, substitutions and executable layout.
 * [TPL-12](../../spec/language/static-results.md#tpl-12) requires distinct
 * static inputs/results and source order; these records create no runtime slots.
 * [TPL-01](../../spec/language/templates.md#tpl-01) preserves the atomic spelling
 * of placeholder heads. [TPL-07](../../spec/language/templates.md#tpl-07)
 * distinguishes actual constructor metadata, while
 * [TPL-08](../../spec/language/templates.md#tpl-08) requires structural application
 * trees rather than textual substitution; metadata is not syntax.
 * [TPL-15](../../spec/language/templates.md#tpl-15) retains nested declaration
 * input expressions and their selected foreign outputs without runtime values.
 */
#ifndef AMA_AST_STATIC_H
#define AMA_AST_STATIC_H

#include "source/text/source.h"

/** A type head records its spelling class, never its resolved Type/Template kind.
 * Component contract: [TPL-01](../../spec/language/templates.md#tpl-01)
 * distinguishes descriptive placeholder names from filesystem module paths.
 */
typedef enum { AST_TYPE_MODULE_PATH, AST_TYPE_PLACEHOLDER } AstTypeHeadKind;

/** Forward name for one AST-arena-owned static expression.
 * Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02)
 * keeps recursive syntax edges inside the containing AST's lifetime.
 */
typedef struct AstTypeExpression AstTypeExpression;

/** Static directions remain distinct from runtime argument/read operations.
 * Component contract: [TPL-12](../../spec/language/static-results.md#tpl-12)
 * gives static graphs named input/output edges without runtime positions.
 */
typedef enum { AST_STATIC_INPUT, AST_STATIC_OUTPUT } AstStaticBindingKind;

/** An ordered AST-owned binding borrows original label/marker/receiver bytes.
 * Input expressions belong to the same arena; labels and output receivers are
 * nonempty complete names. No ordinal normalization or alias binding occurs.
 * Component contract: [TPL-12](../../spec/language/static-results.md#tpl-12)
 * requires preserving named source edges before semantic validation.
 */
typedef struct AstStaticBinding {
    struct AstStaticBinding *next;
    SourceSpan span;
    SourceSpan label;
    SourceSpan marker;
    SourceSpan receiver;
    AstStaticBindingKind kind;
    AstTypeExpression *input;
} AstStaticBinding;

/** A source head and ordered binding tree with no inferred or substituted type.
 * Component contract: [TPL-08](../../spec/language/templates.md#tpl-08)
 * requires structural applications; later resolution decides their residuals.
 */
struct AstTypeExpression {
    SourceSpan span;
    SourceSpan head;
    AstTypeHeadKind kind;
    AstStaticBinding *first;
    AstStaticBinding *last;
    /** One foreign export selector after this expression's named inputs.
     * [TPL-15](../../spec/language/templates.md#tpl-15) allows nested selected
     * results; semantic admission decides their actual Type/Template category.
     */
    SourceSpan selected_export;
};

#endif
