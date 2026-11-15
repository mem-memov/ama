/** @file
 * @brief Compare source-keyed declaration identities and resolved operand/control graphs after exact name edits.
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) requires graph preservation rather than compilation alone.
 */
#ifndef AMA_EDITOR_RENAME_IDENTITY_H
#define AMA_EDITOR_RENAME_IDENTITY_H
#include "../../internal.h"
/** One declaration correspondence, sorted by the old owner path and original declaration range. */
typedef struct { const SemanticSymbol *original; const SemanticSymbol *candidate; } RenameSymbolPair;
/** Borrow validation-owned correspondence storage while comparing one complete candidate graph. */
typedef struct { EditorRename *plan; RenameSymbolPair *symbols; size_t count; } RenameIdentity;
/** Build a bijection of every persistent/local declaration, checking shape, role, scope and mapped source identity. */
bool rename_identity_symbols(RenameIdentity *);
/** Compare original/candidate occurrence inventories with that exact declaration correspondence. */
bool rename_identity_occurrences(RenameIdentity *);
/** Compare every nominal type's checked statement/binding/control metadata and source structure. */
bool rename_identity_program(RenameIdentity *);
/** Compare optional nominal identities by complete logical path, never type spelling fragments. */
bool rename_same_type(const SemanticType *, const SemanticType *);
/** Compare exact original/candidate source coordinates through the prepared replacement map. */
bool rename_same_span(const RenameIdentity *, SourceSpan, SourceSpan);
/** Match nullable declarations only through the complete source-keyed bijection. */
bool rename_same_symbol(const RenameIdentity *, const SemanticSymbol *, const SemanticSymbol *);
/** Compare complete resolved access chains and their original token ranges without recursion. */
bool rename_same_access(const RenameIdentity *, const SemanticAccess *, const SemanticAccess *);
/** Compare nullable resolved sources, including synthetic output sources that have no AST syntax. */
bool rename_same_source(const RenameIdentity *, const SemanticSource *, const SemanticSource *);
/** Match nullable linked statements through unique AST kind/range identity without traversing control cycles. */
bool rename_same_link(const RenameIdentity *, const SemanticStatement *, const SemanticStatement *);
/** Compare a checked statement and its ordered bindings/actions through exact metadata. */
bool rename_same_statement(const RenameIdentity *, const SemanticStatement *, const SemanticStatement *);
#endif
