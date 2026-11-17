/**
 * @file
 * @brief Declare structural parser views, iterative frames and sibling phase contracts.
 *
 * [COMP-01](../../spec/implementation/compiler.md#comp-01)
 * Parsing records syntax only; binding/type/capability decisions belong to later stages.
 */
#ifndef AMA_PARSER_INTERNAL_H
#define AMA_PARSER_INTERNAL_H

#include "ast/ast.h"
#include "ast/static.h"
#include "lexer/tokens/tokens.h"

/** Transient cursor borrowing its AST owner and exclusive section end; first failure prevents usable output. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
typedef struct {
    AstModule *module;
    const LayoutLine *line;
    const LayoutLine *end;
    bool failed;
} Parser;

/** Retained explicit body frame modes; independent of C recursion depth. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
typedef enum { FRAME_ROOT, FRAME_BRANCH, FRAME_LOOP, FRAME_ALTERNATIVE, FRAME_CALL } ParseFrameKind;
/** AST-arena-owned frame with exact indentation. Body frames use block; operation frames use owner and completion state. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
typedef struct ParseFrame {
    struct ParseFrame *parent;
    AstBlock *block;
    AstNode *owner;
    size_t indentation;
    ParseFrameKind kind;
    bool call_arguments_done;
    AstNode *condition_loop;
} ParseFrame;

/** Record only the first structural failure, mark the parser failed, and return false. Borrow source/message storage through AST lifetime.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_fail(Parser *parser, SourceSpan span, const char *message, ParserExpectation expected);
/** Allocate zeroed AST scratch; on failure retain first source site and mark out-of-memory. Caller never frees a partial node independently.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
void *parser_allocate(Parser *parser, size_t size, SourceSpan site);
/** Return the current code span or a zero-width section/EOF boundary; all coordinates borrow original source bytes.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
SourceSpan parser_site(const Parser *parser);
/** Advance over blank/comment nodes up to an exclusive end; accumulate whether a blank occurred without changing source/layout.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
const LayoutLine *parser_significant(const LayoutLine *line, const LayoutLine *end, bool *blank);
/** Skip blank lines and only comments at the specified region depth; leave other-depth comments for the enclosing grammar or an error. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
void parser_skip_region(Parser *, size_t);
/** Move only the parser cursor to its next significant node within the current section.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
void parser_skip_layout(Parser *parser);
/** Test a complete Project/Standard root path span without mutation, trimming or registry lookup.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_path(SourceSpan text);
/** Recognize one complete three-dot-prefixed static name without lookup.
 * Component contract: [TPL-01](../../spec/language/templates.md#tpl-01)
 * treats the whole dotted spelling as an atomic scoped name, not a module path.
 */
bool parser_static_name(SourceSpan);
/** Parse one static expression from the current physical head line and its
 * exact child binding region; publish only a complete AST-owned tree.
 * Component contract: [TPL-12](../../spec/language/static-results.md#tpl-12)
 * fixes child indentation, inputs-before-outputs and explicit skips.
 */
bool parser_type_expression(Parser *, SourceSpan, size_t, AstTypeExpression **);
/** Parse a static formal and its recursively nonempty optional signature.
 * Component contract: [TPL-07](../../spec/language/templates.md#tpl-07)
 * requires explicit named signature structure, independent of actual kinds.
 */
bool parser_static_formal(Parser *, size_t, AstStaticFormal **);
/** Parse a static result name and same-level defining expression.
 * Component contract: [TPL-12](../../spec/language/static-results.md#tpl-12)
 * creates no ordinary result storage or runtime initializer sequence.
 */
bool parser_static_result(Parser *, size_t, AstNode *);
/** Compose a static expression with explicit projection or constant-read actions.
 * Component contract: [TPL-12](../../spec/language/static-results.md#tpl-12)
 * preserves source-only classification before producer resolution.
 */
bool parser_static_statement(Parser *, ParseFrame **, AstNode *);
/** Build an iterative access/index shape; publish output only on full suffix/bracket consumption. Failed partial nodes remain arena-owned.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_access(Parser *parser, SourceSpan text, AstAccess **output);
/** Test structural declaration eligibility: no indexed access, independent of known bindings.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_declaration_name(const AstAccess *access);
/** Test scalar-owned spelling only; this does not establish runtime ownership or nominal type.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_owned_access(const AstAccess *access);
/** Build one structural source with borrowed original payload/span. Literal permission comes from its caller; failed output remains unpublished.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_source(Parser *parser, const LayoutLine *line, SourceSpan text,
    bool literal_allowed, AstSource **output);
/** Allocate one zeroed source-mapped node with explicit kind; null records an allocation diagnostic.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
AstNode *parser_node(Parser *parser, AstNodeKind kind, SourceSpan span);
/** Append an owned node to a block preserving source order; node is not independently freed.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
void parser_append(AstBlock *block, AstNode *node);
/** Parse an explicit name/type line and ordered initializer sequence at exact retained indentation. Advance cursor through this declaration only.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_declaration(Parser *parser, size_t indentation, AstNode *node);
/** Parse a section interval into top-level declaration nodes; source views/partial nodes remain owned by the parent AST on failure.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_declarations(Parser *parser, ParsedSection section, AstBlock *output);
/** Drive finite explicit frames until the section ends; no recursive C activation represents Ama nesting.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_body(Parser *parser, ParsedSection section, AstBlock *output);
/** Classify one retained statement from physical adjacency/mode and append its structural node; failure invalidates the parent AST.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_statement(Parser *parser, ParseFrame **frame);
/** Record exactly one warning for an empty written control body, ordered by marker location. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_empty_warning(Parser *, AstBlock, SourceSpan);
/** Inspect only same-depth comment gaps; blanks and other-depth comments remain observable. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
const LayoutLine *parser_condition_marker(const LayoutLine *, const LayoutLine *, size_t);
/** Recognize exact reference-binding foreach syntax before ordinary access parsing. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_foreach(Parser *, ParseFrame **, AstNode *, const LayoutLine *);
/** Start a named branch/loop at its explicit marker, retaining marker spans. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_control(Parser *, ParseFrame **, AstNode *, AstAccess *, const LayoutLine *);
/** Fold a linear operation stack and its terminal into one loop statement. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_condition_chain(Parser *, ParseFrame **, AstAccess *, const LayoutLine *);
/** Start one ordinary operation frame with exact target/mode; its iterative driver owns bindings and the optional refusal body.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_start_call(Parser *parser, ParseFrame **frame, AstNode *node,
    SourceSpan target, AstCallMode mode);
/** Parse and append one ordinary binding at the current checked call-argument line; advance once on success. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_call_argument(Parser *, AstNode *);
/** Parse a checked execution/resume suffix or unmarked retained head, preserving exact target bounds. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_call_mode(Parser *, SourceSpan, SourceSpan *, AstCallMode *);
/** Resolve only structural target form and mode; no callee lookup or runtime acquisition occurs. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_call_target(Parser *, AstNode *, SourceSpan, AstCallMode);
/** Append one arena-owned ordered output action and close its operation's input phase. Exactly one binding/selected operation is supplied. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_output_action(Parser *, AstNode *, AstArgument *, AstNode *);
/** Advance an explicit operation frame by one binding, selected child, handler transition or completed frame. Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_call_step(Parser *, ParseFrame **);
/** Allocate a child frame owning one body view, borrowing its owner/block and parent; cursor updates only after successful allocation.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
bool parser_push_frame(Parser *parser, ParseFrame **frame, AstBlock *block,
    AstNode *owner, size_t indentation, ParseFrameKind kind);

#endif
