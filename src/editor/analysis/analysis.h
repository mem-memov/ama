/** @file
 * @brief Own an immutable editor snapshot backed by the production frontend, lexer and resolved symbols.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) keeps one parser/type system;
 * [NAME-06](../../../spec/language/declarations.md#name-06) supplies exact lexical visibility.
 */
#ifndef AMA_EDITOR_ANALYSIS_H
#define AMA_EDITOR_ANALYSIS_H
#include "compiler/frontend.h"
#include "lexer/tokens/tokens.h"

/** Distinguish checked executable metadata, intentionally opaque drafts and invalid/unavailable input. */
typedef enum { EDITOR_COMPLETE, EDITOR_DRAFT, EDITOR_INVALID, EDITOR_UNAVAILABLE } EditorState;
/** One exact original spelling linked to its checked declaration or nominal module identity. */
typedef struct EditorOccurrence {
    struct EditorOccurrence *next;
    SourceSpan spelling;
    const SemanticSymbol *symbol;
    const SemanticType *type;
    bool declaration;
} EditorOccurrence;
/** Own one immutable project snapshot. All borrowed results expire together at destruction;
 * a changed file requires a fresh owner and never mutates previously returned source bytes.
 */
typedef struct {
    CompilerFrontend frontend;
    SourceArena arena;
    AstModule inspection;
    const ModuleSource *selected;
    EditorOccurrence *first;
    EditorOccurrence *last;
    Diagnostic diagnostic;
    EditorState state;
    bool attempted;
    bool indexed;
    /** Whole-workspace analysis owns settings and checks every active catalog module, including unused importers. */
    bool workspace;
    const ModuleRoot *roots;
    size_t root_count;
    const char *entry;
} EditorAnalysis;
/** Lexical highlighting preserves exact complete-name and literal spans; comments are physical-line views. */
typedef struct { SourceSpan spelling; SourceSpan payload; LexKind kind; bool comment; } EditorHighlight;
/** Returning false stops enumeration successfully. The callback record is temporary;
 * copy its spans to retain observations until snapshot destruction.
 */
typedef bool (*EditorHighlightVisitor)(void *, const EditorHighlight *);
/** Borrow a checked symbol/type signature and its exact declaration; no rendered C names are involved. */
typedef struct { const SemanticSymbol *symbol; const SemanticType *type; SourceSpan definition; } EditorDefinition;
/** Structured diagnostic callback distinguishes nonfatal parser warnings from the first compiler error. */
typedef bool (*EditorDiagnosticVisitor)(void *, const Diagnostic *, bool warning);
/** Synchronous completion visitor borrows a checked declaration until snapshot destruction. */
typedef bool (*EditorSymbolVisitor)(void *, const SemanticSymbol *);

/** Initialize independent ownership with explicit allocator authority; analyze once. */
void editor_analysis_init(EditorAnalysis *, SourceAllocator);
/** Release index/inspection before frontend source ownership, invalidating every borrowed result. */
void editor_analysis_destroy(EditorAnalysis *);
/** Analyze stable filesystem inputs through the compiler once. Invalid source is a successful
 * analysis result with explicit state/diagnostic; false means allocation or unavailable source.
 * Drafts expose lexical data only; no partial semantic graph is published as valid metadata.
 */
bool editor_analysis_build(EditorAnalysis *, const ModuleRoot *, size_t, const char *entry);
/** Analyze every configured active module, keeping drafts opaque and owning root settings.
 * False retains a diagnostic and publishes no complete workspace; no source files are changed.
 */
bool editor_workspace_build(EditorAnalysis *, const ModuleRoot *, size_t, const char *entry);
/** Enumerate lexical tokens/comments using the production layout/lexer. Output may contain a
 * valid prefix before an invalid token; false returns its exact diagnostic. No recovery grammar.
 */
bool editor_highlights(const EditorAnalysis *, const char *module, EditorHighlightVisitor, void *, Diagnostic *);
/** Enumerate retained compiler errors and valid AST warnings without duplicating selected-source inspection. */
bool editor_diagnostics(const EditorAnalysis *, EditorDiagnosticVisitor, void *);
/** Resolve a byte within one indexed spelling to the exact checked declaration/signature.
 * Missing/invalid/unindexed input preserves output; half-open spans exclude their trailing edge.
 */
bool editor_definition(const EditorAnalysis *, const char *module, size_t offset, EditorDefinition *);
/** Borrow the checked call at its exact written target. Nominal interfaces use namespace_type;
 * intrinsic signatures use list_interface/list_method and semantic_list_position, without a fake module.
 * Missing or invalid targets preserve output.
 */
bool editor_operation(const EditorAnalysis *, const char *module, size_t offset, const SemanticStatement **);
/** Borrow the checked field/type/storage at a written binding label or positional marker,
 * including intrinsic fields that intentionally have no source definition. Missing queries preserve output.
 */
bool editor_binding(const EditorAnalysis *, const char *module, size_t offset, const SemanticBinding **);
/** Enumerate module-visible names at an exact executable byte point. Reuse semantic local
 * lookup; inferred fields appear only at/after introduction. No prefix ranking or guessed repair.
 * Construction-time initializers exclude runtime locals. False rejects invalid/unresolved queries.
 */
bool editor_completions(const EditorAnalysis *, const char *module, size_t offset, EditorSymbolVisitor, void *);
/** Enumerate exact declared input/result/constant interface fields of a checked module type.
 * State/local storage is not an external interface; caller chooses a declared role explicitly.
 */
bool editor_signature(const EditorAnalysis *, const char *module, SymbolRole, EditorSymbolVisitor, void *);
#endif
