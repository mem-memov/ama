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
#include "lexer/tokens/tokens.h"

/** Transient cursor borrowing its AST owner and exclusive section end; first failure prevents usable output. */
typedef struct {
    AstModule *module;
    const LayoutLine *line;
    const LayoutLine *end;
    bool failed;
} Parser;

/** Retained explicit body frame modes; independent of C recursion depth. */
typedef enum { FRAME_ROOT, FRAME_BRANCH, FRAME_LOOP, FRAME_ALTERNATIVE, FRAME_CALL } ParseFrameKind;
/** AST-arena-owned frame with exact indentation. Body frames use block; operation frames use owner and completion state. */
typedef struct ParseFrame {
    struct ParseFrame *parent;
    AstBlock *block;
    AstNode *owner;
    size_t indentation;
    ParseFrameKind kind;
    bool call_arguments_done;
} ParseFrame;

/** Record only the first structural failure, mark the parser failed, and return false. Borrow source/message storage through AST lifetime.
 */
bool parser_fail(Parser *parser, SourceSpan span, const char *message, ParserExpectation expected);
/** Allocate zeroed AST scratch; on failure retain first source site and mark out-of-memory. Caller never frees a partial node independently.
 */
void *parser_allocate(Parser *parser, size_t size, SourceSpan site);
/** Return the current code span or a zero-width section/EOF boundary; all coordinates borrow original source bytes.
 */
SourceSpan parser_site(const Parser *parser);
/** Advance over blank/comment nodes up to an exclusive end; accumulate whether a blank occurred without changing source/layout.
 */
const LayoutLine *parser_significant(const LayoutLine *line, const LayoutLine *end, bool *blank);
/** Skip blank lines and only comments at the specified region depth; leave other-depth comments for the enclosing grammar or an error. */
void parser_skip_region(Parser *, size_t);
/** Move only the parser cursor to its next significant node within the current section.
 */
void parser_skip_layout(Parser *parser);
/** Test a complete Project/Standard root path span without mutation, trimming or registry lookup.
 */
bool parser_path(SourceSpan text);
/** Build an iterative access/index shape; publish output only on full suffix/bracket consumption. Failed partial nodes remain arena-owned.
 */
bool parser_access(Parser *parser, SourceSpan text, AstAccess **output);
/** Test structural declaration eligibility: no indexed access, independent of known bindings.
 */
bool parser_declaration_name(const AstAccess *access);
/** Test scalar-owned spelling only; this does not establish runtime ownership or nominal type.
 */
bool parser_owned_access(const AstAccess *access);
/** Build one structural source with borrowed original payload/span. Literal permission comes from its caller; failed output remains unpublished.
 */
bool parser_source(Parser *parser, const LayoutLine *line, SourceSpan text,
    bool literal_allowed, AstSource **output);
/** Allocate one zeroed source-mapped node with explicit kind; null records an allocation diagnostic.
 */
AstNode *parser_node(Parser *parser, AstNodeKind kind, SourceSpan span);
/** Append an owned node to a block preserving source order; node is not independently freed.
 */
void parser_append(AstBlock *block, AstNode *node);
/** Parse an explicit name/type line and ordered initializer sequence at exact retained indentation. Advance cursor through this declaration only.
 */
bool parser_declaration(Parser *parser, size_t indentation, AstNode *node);
/** Parse a section interval into top-level declaration nodes; source views/partial nodes remain owned by the parent AST on failure.
 */
bool parser_declarations(Parser *parser, ParsedSection section, AstBlock *output);
/** Drive finite explicit frames until the section ends; no recursive C activation represents Ama nesting.
 */
bool parser_body(Parser *parser, ParsedSection section, AstBlock *output);
/** Classify one retained statement from physical adjacency/mode and append its structural node; failure invalidates the parent AST.
 */
bool parser_statement(Parser *parser, ParseFrame **frame);
/** Start one ordinary operation frame with exact target/mode; its iterative driver owns bindings and the optional refusal body.
 */
bool parser_start_call(Parser *parser, ParseFrame **frame, AstNode *node,
    SourceSpan target, AstCallMode mode);
/** Parse and append one ordinary binding at the current checked call-argument line; advance once on success. */
bool parser_call_argument(Parser *, AstNode *);
/** Parse a checked execution/resume suffix or unmarked retained head, preserving exact target bounds. */
bool parser_call_mode(Parser *, SourceSpan, SourceSpan *, AstCallMode *);
/** Resolve only structural target form and mode; no callee lookup or runtime acquisition occurs. */
bool parser_call_target(Parser *, AstNode *, SourceSpan, AstCallMode);
/** Append one arena-owned ordered output action and close its operation's input phase. Exactly one binding/selected operation is supplied. */
bool parser_output_action(Parser *, AstNode *, AstArgument *, AstNode *);
/** Advance an explicit operation frame by one binding, selected child, handler transition or completed frame. */
bool parser_call_step(Parser *, ParseFrame **);
/** Allocate a child frame owning one body view, borrowing its owner/block and parent; cursor updates only after successful allocation.
 */
bool parser_push_frame(Parser *parser, ParseFrame **frame, AstBlock *block,
    AstNode *owner, size_t indentation, ParseFrameKind kind);

#endif
