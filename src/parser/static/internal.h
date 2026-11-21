/**
 * @file
 * @brief Declare narrow static-expression construction operations within the AST owner.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01) permits only
 * structural recognition here; unknown names and kinds stay unresolved.
 */
#ifndef AMA_PARSER_STATIC_INTERNAL_H
#define AMA_PARSER_STATIC_INTERNAL_H
#include "../internal.h"

/** Construct one valid head with no child bindings; output changes only on success.
 * Component contract: [TPL-01](../../../spec/language/templates.md#tpl-01)
 * keeps placeholder and module-path spelling distinct before name resolution.
 */
bool static_expression_head(Parser *, SourceSpan, AstTypeExpression **);

/** Recognize a static binding's leading marker class without consuming a line.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * reserves single-dot labels for static inputs/outputs, apart from runtime reads.
 */
bool static_binding_line(const LayoutLine *);

/** Build one current binding head, including a bare input expression or output
 * receiver; nested input children are the iterative driver's responsibility.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * requires nonempty named labels and operands; static skips do not exist.
 */
bool static_binding_head(Parser *, AstStaticBinding **);

/** Consume the current valid bare static-name line as a concrete path's
 * selector, rejecting duplicates. The caller supplies the correct child depth.
 * Child indentation and subsequent phases stay with the expression owner.
 * [TPL-15](../../../spec/language/templates.md#tpl-15) gives a selector no
 * local binding, children or output-action meaning.
 */
bool static_expression_selector(Parser *, AstTypeExpression *);
#endif
