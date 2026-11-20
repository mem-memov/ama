/** @file
 * @brief Own an immutable editor snapshot backed by the production frontend, lexer and resolved symbols.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) keeps one parser/type system;
 * [NAME-06](../../../spec/language/declarations.md#name-06) supplies exact lexical visibility.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) keeps original static source targets separate from runtime Types.
 */
#ifndef AMA_EDITOR_ANALYSIS_H
#define AMA_EDITOR_ANALYSIS_H
#include "compiler/frontend.h"
#include "lexer/tokens/tokens.h"

/** Distinguish checked executable metadata, intentionally opaque drafts and invalid/unavailable input. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
typedef enum { EDITOR_COMPLETE, EDITOR_DRAFT, EDITOR_INVALID, EDITOR_UNAVAILABLE } EditorState;
/** One exact original spelling linked to its checked declaration or nominal module identity. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
typedef struct EditorOccurrence {
    struct EditorOccurrence *next;
    SourceSpan spelling;
    const SemanticSymbol *symbol;
    const SemanticType *type;
    bool declaration;
    /** Source graph targets have no fabricated runtime Type.
     * [TPL-12](../../../spec/language/static-results.md#tpl-12) keeps written static producers and their exports distinct from returned ordinary Types.
     */
    const StaticSymbol *static_symbol;
    const ProjectModule *module;
    /** Borrowed source identity and remaining actual targets for this spelling.
     * [NAME-06](../../../spec/language/declarations.md#name-06) keeps lexical declarations independent from concrete Type captures.
     */
    const StaticRuntimeField *original;
    const StaticRuntimeModule *runtime;
    struct EditorOccurrence *alternatives;
} EditorOccurrence;
/** Own one immutable project snapshot. All borrowed results expire together at destruction;
 * a changed file requires a fresh owner and never mutates previously returned source bytes.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
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
    /** Whole-workspace analysis owns settings and checks every active catalog module, including unused importers. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
    bool workspace;
    const ModuleRoot *roots;
    size_t root_count;
    const char *entry;
} EditorAnalysis;
/** Lexical highlighting preserves exact complete-name and literal spans; comments are physical-line views. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
typedef struct { SourceSpan spelling; SourceSpan payload; LexKind kind; bool comment; } EditorHighlight;
/** Returning false stops enumeration successfully. The callback record is temporary;
 * copy its spans to retain observations until snapshot destruction.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
typedef bool (*EditorHighlightVisitor)(void *, const EditorHighlight *);
/** Borrow an exact original definition, with runtime or static metadata only in its actual domain; static producers have no invented runtime Type. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
typedef struct {
    const SemanticSymbol *symbol;
    const SemanticType *type;
    SourceSpan definition;
    const StaticSymbol *static_symbol;
    const ProjectModule *module;
    const StaticRuntimeField *original;
    const StaticRuntimeModule *runtime;
    const EditorOccurrence *targets;
} EditorDefinition;
/** Structured diagnostic callback distinguishes nonfatal parser warnings from the first compiler error. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
typedef bool (*EditorDiagnosticVisitor)(void *, const Diagnostic *, bool warning);
/** Synchronous source visitor receives a temporary view; copy it to retain borrowed pointers until snapshot destruction. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
typedef bool (*EditorSymbolVisitor)(void *, const EditorDefinition *);

/** Initialize independent ownership with explicit allocator authority; analyze once. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
void editor_analysis_init(EditorAnalysis *, SourceAllocator);
/** Release index/inspection before frontend source ownership, invalidating every borrowed result. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
void editor_analysis_destroy(EditorAnalysis *);
/** Analyze stable filesystem inputs through the compiler once. Invalid source is a successful
 * analysis result with explicit state/diagnostic; false means allocation or unavailable source.
 * Drafts expose lexical data only; no partial semantic graph is published as valid metadata.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
bool editor_analysis_build(EditorAnalysis *, const ModuleRoot *, size_t, const char *entry);
/** Analyze every configured active module, keeping drafts opaque and owning root settings.
 * False retains a diagnostic and publishes no complete workspace; no source files are changed.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
bool editor_workspace_build(EditorAnalysis *, const ModuleRoot *, size_t, const char *entry);
/** Enumerate lexical tokens/comments using the production layout/lexer. Output may contain a
 * valid prefix before an invalid token; false returns its exact diagnostic. No recovery grammar.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
bool editor_highlights(const EditorAnalysis *, const char *module, EditorHighlightVisitor, void *, Diagnostic *);
/** Enumerate retained compiler errors and valid AST warnings without duplicating selected-source inspection. Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
bool editor_diagnostics(const EditorAnalysis *, EditorDiagnosticVisitor, void *);
/** Borrow all accepted source/concrete targets of one exact spelling. Source
 * anchors have no concrete symbol; alternatives retain each actual context and
 * foreign origin. Ordering gives no preferred target. False preserves output.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) exposes accepted evidence without selecting a first context.
 */
bool editor_binding_targets(const EditorAnalysis *, const char *, size_t, const EditorOccurrence **);
/** Return one original definition only when all accepted targets agree on its
 * source span. Singular runtime metadata is present only when actual values
 * agree; source fields/static symbols remain independently observable. False
 * preserves output for absent or different-origin definitions.
 * [NAME-06](../../../spec/language/declarations.md#name-06) fixes lexical identity independently of Type captures.
 */
bool editor_definition(const EditorAnalysis *, const char *module, size_t offset, EditorDefinition *);
/** Borrow the checked call at its exact written target. Nominal interfaces use namespace_type;
 * intrinsic signatures use list_interface/list_method and semantic_list_position, without a fake module.
 * Zero or multiple concrete contexts, missing and invalid targets preserve output.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
bool editor_operation(const EditorAnalysis *, const char *module, size_t offset, const SemanticStatement **);
/** Borrow the checked field/type/storage at a written binding label or positional marker,
 * including intrinsic fields that intentionally have no source definition. A unique concrete context is required; unavailable queries preserve output.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
bool editor_binding(const EditorAnalysis *, const char *module, size_t offset, const SemanticBinding **);
/** Enumerate source-visible runtime and static names at an exact byte point. Reuse compiler source-scope
 * predicates; inferred fields appear only at/after introduction. No prefix ranking or guessed repair.
 * Construction-time initializers exclude runtime locals. False rejects invalid/unresolved queries.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
bool editor_completions(const EditorAnalysis *, const char *module, size_t offset, EditorSymbolVisitor, void *);
/** Enumerate original runtime input/result/constant fields, or named static graph inputs/results.
 * State/locals are private; static names add no runtime arguments or implicit instances.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [NAME-06](../../../spec/language/declarations.md#name-06). */
bool editor_signature(const EditorAnalysis *, const char *module, SymbolRole, EditorSymbolVisitor, void *);
#endif
