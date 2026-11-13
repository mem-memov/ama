/**
 * @file
 * @brief Declare semantic sibling operations over explicit resolved views and owners.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Keep name/type resolution, compatibility, graph relations and diagnostics distinct from parsing and emission.
 */
#ifndef AMA_SEMANTIC_INTERNAL_H
#define AMA_SEMANTIC_INTERNAL_H

#include "semantic.h"

/** Borrowed semantic AST callback; false stops traversal and its opaque context remains caller-owned. */
typedef bool (*SemanticVisitor)(SemanticProgram *, SemanticType *, const AstNode *, size_t, const AstNode *, void *);
/** Visit retained blocks in lexical order with explicit loop depth; borrow AST/context and own work in the semantic arena.
 */
bool semantic_walk(SemanticProgram *program, SemanticType *type, AstBlock block,
    SemanticVisitor visit, void *context);
/** Compare two valid borrowed spans by exact length/bytes, without case or Unicode normalization.
 */
bool semantic_same_text(SourceSpan left, SourceSpan right);
/** Publish a borrowed primary site/message and optional original related declaration, returning false without allocation.
 */
bool semantic_fail(SemanticProgram *program, SourceSpan span, const char *message,
    const SemanticSymbol *related);
/** Publish out-of-memory status/message without allocating reporting storage.
 */
bool semantic_memory_failure(SemanticProgram *program);
/** Allocate/zero semantic-owned scratch or return null with an allocation-free failure; partial records survive until destroy.
 */
void *semantic_allocate(SemanticProgram *program, size_t size);

/** Bind every explicit field to an executable nominal value type; reject reference constants before value resolution.
 */
bool semantic_link_types(SemanticProgram *program);
/** Compute the greatest fixed point over all declared owned types, keeping a noncopyable native seed/cause for diagnostics.
 */
void semantic_clone_capabilities(SemanticProgram *program);
/** Resolve every explicit initializer in declaration context and record exactly one compatible transfer per list element/value.
 */
bool semantic_initializers(SemanticProgram *program, SemanticType *type);
/** Resolve all retained body statements in lexical order with loop context; partial resolved records remain unusable on failure.
 */
bool semantic_statements(SemanticProgram *program, SemanticType *type);
/** Resolve each declared suffix/index form, writable outer location and read-only index operands.
 * @return Semantic-owned chain or null; no implicit reference dereference or current-name inference.
 */
SemanticAccess *semantic_access(SemanticProgram *program, SemanticType *type,
    const AstAccess *syntax, bool writable, const SemanticSymbol *initializing);
/** Resolve a selected declared result as the leading field, with every index operand in the initiating caller; no clone or implicit dereference occurs. */
SemanticAccess *semantic_selected_access(SemanticProgram *, SemanticType *, const AstAccess *, SemanticSymbol *);
/** Link each checked output action to its exact binding/selected statement after the complete source-ordered statement inventory exists. */
bool semantic_output_actions(SemanticProgram *, SemanticType *);
/** Require an owned scalar with its own condition/index ABI protocol; nullable declaration denotes a real intrinsic value, not a fabricated symbol. */
bool semantic_conversion(SemanticProgram *, SemanticType *, bool, bool, AbiOperation, SourceSpan, const SemanticSymbol *);
/** Find any retained statement by exact syntax identity; null never causes spelling/dynamic-context fallback. */
SemanticStatement *semantic_statement_find(SemanticType *, const AstNode *);
/** Resolve an ordinary or intrinsic result value with all indices in caller context, retaining no runtime storage. */
bool semantic_select_result(SemanticProgram *, SemanticType *, SemanticStatement *, const AstAccess *, SemanticSelection *);
/** Resolve deferred branch/loop conditions and foreach roles after header operations have their complete typed inventories. */
bool semantic_controls(SemanticProgram *, SemanticType *);
/** Retain exact source spans/payload and resolve local/qualified constant modes; return owned metadata or null with context.
 */
SemanticSource *semantic_source(SemanticProgram *program, SemanticType *type,
    const AstSource *syntax, const SemanticSymbol *initializing);
/** Enforce exact nominal/form compatibility and mandatory owned-copy capability before returning transfer metadata. Ordinary identity no-op remains runtime work.
 */
bool semantic_transfer(SemanticProgram *program, SemanticType *type, bool reference,
    bool list, SemanticSource *source, const SemanticAccess *receiver,
    const SemanticSymbol *declaration, SemanticTransfer *transfer);
/** Resolve target namespace and every interface binding, rejecting unsupported whole-list dispatch before it can reach typed IR.
 */
bool semantic_call(SemanticProgram *program, SemanticType *type, SemanticStatement *statement);
/** Require a scalar owned operand with its own explicit condition/index capability; do not inherit capability from a field.
 */
bool semantic_condition(SemanticProgram *program, SemanticAccess *access,
    AbiOperation capability, SourceSpan site);
/** Restrict initializer reads to initialized earlier declarations and legal constant context, independent of module loading cycles.
 */
bool semantic_visible(SemanticProgram *program, const SemanticSymbol *symbol,
    const SemanticSymbol *initializing, SourceSpan site);

/** Build only actual eager owned-construction edges, keeping references/empty recursive lists separate.
 */
bool semantic_eager_graph(SemanticProgram *program);
/** Build shared declaration-order and actual read/construction prerequisites, not generic type or loading edges.
 */
bool semantic_constant_graph(SemanticProgram *program);
/** Reuse/create a graph vertex keyed by exact semantic type identity and its borrowed original module/site.
 */
GraphVertex *semantic_type_vertex(DependencyGraph *graph, SemanticType *type);
/** Reuse/create a graph vertex keyed by exact constant declaration with its original module/role ordinal/site.
 */
GraphVertex *semantic_constant_vertex(DependencyGraph *graph, SemanticSymbol *symbol);

/** Retain both nominal type names and original storage declarations in a borrowed mismatch diagnostic, returning false.
 */
bool semantic_incompatible(SemanticProgram *program, SemanticType *type, bool reference,
    bool list, const SemanticSource *source, const SemanticSymbol *destination);
/** Retain a selected native capability contract, including a transitive noncopyable seed, without allocating a message.
 */
bool semantic_capability_error(SemanticProgram *program, SemanticType *type,
    AbiOperation operation, SourceSpan site, const SemanticSymbol *declaration, const char *message);

#endif
