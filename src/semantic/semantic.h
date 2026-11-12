/**
 * @file
 * @brief Own resolved nominal types, sources, bindings and separate dependency relations.
 *
 * [COMP-02](../../spec/implementation/compiler.md#comp-02)
 * Borrow the loaded project; only successful stage outputs are consumable. Inference/lexical locals remain T030/T031.
 */
#ifndef AMA_SEMANTIC_PROGRAM_H
#define AMA_SEMANTIC_PROGRAM_H

#include "project/dependencies/project.h"
#include "parser/abi/abi.h"
#include "semantic/relations/graph/graph.h"

/** Semantic stage status; successful valid flags gate consumption of owned partial records. */
typedef enum { SEMANTIC_OK, SEMANTIC_INVALID, SEMANTIC_OUT_OF_MEMORY } SemanticStatus;
/** Declared storage roles have independent ordinals and initialization domains. */
typedef enum { SYMBOL_CONSTANT, SYMBOL_PARAMETER, SYMBOL_RESULT, SYMBOL_STATE } SymbolRole;
typedef struct SemanticType SemanticType;
typedef struct SemanticConstantPlan SemanticConstantPlan;
typedef struct SemanticSource SemanticSource;
typedef struct SemanticAccess SemanticAccess;
typedef struct SemanticBinding SemanticBinding;
typedef struct SemanticStatement SemanticStatement;
/** Validated construction/transfer kind; ordinary identity no-op is deliberately not a static enum case. */
typedef enum {
    TRANSFER_DEFAULT, TRANSFER_LITERAL, TRANSFER_CLONE, TRANSFER_HANDLE,
    TRANSFER_CONTINUATION
} SemanticTransfer;
/** Ordered semantic-owned source/transfer metadata borrowing its original syntax and nominal type. */
typedef struct SemanticInitializer {
    struct SemanticInitializer *next;
    SemanticSource *source;
    SemanticTransfer transfer;
} SemanticInitializer;
/** Resolved declaration with exact owner/type/storage/role ordinals and ordered initializer metadata. */
typedef struct SemanticSymbol {
    struct SemanticSymbol *next;
    const AstNode *declaration;
    SemanticType *owner;
    SemanticType *type;
    SymbolRole role;
    size_t ordinal;
    size_t role_ordinal;
    bool reference;
    bool list;
    SemanticInitializer *initializers;
    SemanticInitializer *last_initializer;
    SemanticConstantPlan *shared;
} SemanticSymbol;
/** Nominal project-module identity with declarations, checked ABI and copy-capability cause; no runtime instance is created. */
struct SemanticType {
    SemanticType *next;
    const ProjectModule *module;
    SemanticSymbol *first;
    SemanticSymbol *last;
    size_t symbol_count;
    size_t counts[4];
    AbiTable abi;
    bool executable;
    bool cloneable;
    SemanticType *clone_seed;
    SemanticSymbol *clone_cause;
    SemanticStatement *statements;
    SemanticStatement *last_statement;
};
/** Shared startup/reverse-cleanup order over borrowed resolved constant declarations. */
struct SemanticConstantPlan {
    SemanticConstantPlan *next;
    SemanticConstantPlan *previous;
    SemanticSymbol *symbol;
    SemanticInitializer *initializers;
    size_t ordinal;
};
/** Resolved declaration/index chain retaining exact syntax identity and storage form. */
struct SemanticAccess {
    const AstAccess *syntax;
    SemanticSymbol *symbol;
    SemanticAccess *index;
    bool reference;
    bool list;
};
/** Typed source view preserving literal/local/constant/acquisition/continuation mode and original site. */
struct SemanticSource {
    const AstSource *syntax;
    SourceSpan site;
    AstSourceKind kind;
    SemanticAccess *access;
    SemanticSymbol *constant;
    SemanticSymbol *declaration;
    SemanticType *type;
    bool reference;
    bool list;
};
/** Resolved callee field plus caller-side source/receiver, exact interface ordinal and transfer kind. */
struct SemanticBinding {
    SemanticBinding *next;
    const AstArgument *syntax;
    SemanticSymbol *field;
    SemanticType *type;
    size_t position;
    bool reference;
    bool list;
    SemanticSource *source;
    SemanticAccess *receiver;
    SemanticTransfer transfer;
};
/** Semantic-owned typed statement borrowing one exact AST identity and its ordered bindings. */
struct SemanticStatement {
    SemanticStatement *next;
    const AstNode *syntax;
    SemanticAccess *access;
    SemanticSource *source;
    SemanticType *namespace_type;
    SemanticBinding *bindings;
    SemanticBinding *last_binding;
    SemanticTransfer transfer;
};
/** Explicit native-authoring proposal borrowing its exact project module and proposed ABI source. */
typedef struct {
    const ProjectModule *module;
    const SourceText *abi;
} SemanticNativeAuthoring;
/** Semantic owner of scratch, independent eager/shared graphs, diagnostics and per-stage attempt/valid guards. */
typedef struct {
    SourceArena arena;
    DependencyGraph eager;
    DependencyGraph constants;
    const Project *project;
    SemanticType *first;
    SemanticType *last;
    SemanticStatus status;
    Diagnostic error;
    DiagnosticRelated related[2];
    bool names_attempted;
    bool names_valid;
    bool native_attempted;
    bool native_valid;
    bool resolve_attempted;
    bool resolved;
    bool relations_attempted;
    bool relations_valid;
    SemanticConstantPlan *constant_first;
    SemanticConstantPlan *constant_last;
    size_t constant_count;
    bool constants_attempted;
    bool constants_valid;
} SemanticProgram;

/* The loaded valid project outlives this owner. Initialize, run each semantic
 * stage once, inspect its successful output, destroy. Failed partial records
 * are retained only for diagnostics/cleanup, never accepted as validated IR. */
/** Require a valid borrowed project, initialize semantic ABI/graph/arena owners, and retain explicit allocator authority.
 */
void semantic_program_init(SemanticProgram *program, const Project *project,
    SourceAllocator allocator);
/** Destroy every ABI/graph/arena allocation before its borrowed project disappears; all resolved views expire.
 */
void semantic_program_destroy(SemanticProgram *program);
/** Run ordinary names/native/resolve/relations/constants stages once, returning the first failed status.
 */
SemanticStatus semantic_analyze(SemanticProgram *program);
/** Compose the same stages with an explicitly supplied native ABI authoring proposal, never guessed from missing files. Proposed ABI source storage outlives this semantic owner.
 */
SemanticStatus semantic_analyze_authoring(SemanticProgram *program,
    const SemanticNativeAuthoring *authoring);
/** Build the retained explicit module-wide name inventory once; on failure partial records remain owned and names_valid is false.
 */
SemanticStatus semantic_names(SemanticProgram *program);
/** Check real native companions/protocol tables after names succeed; lexical binding is separate from typed C verification.
 */
SemanticStatus semantic_native(SemanticProgram *program);
/** Apply an explicit proposal only to its exact borrowed module; other required native modules still need real companions.
 */
SemanticStatus semantic_native_authoring(SemanticProgram *program, const SemanticNativeAuthoring *authoring);
/** Link nominal types, derive copy capability, then resolve initializers/statements; publish resolved only after complete success.
 */
SemanticStatus semantic_resolve(SemanticProgram *program);
/** Build/order separate eager-ownership and shared-initialization graphs after resolution; preserve actual cycle sites.
 */
SemanticStatus semantic_relations(SemanticProgram *program);
/** Consume valid ordered shared prerequisites into startup/reverse-cleanup plans; no runtime construction occurs.
 */
SemanticStatus semantic_constants(SemanticProgram *program);
/** Resolve an exact NUL-terminated supported logical identity in the borrowed type inventory or return null.
 */
SemanticType *semantic_find_type(const SemanticProgram *program, const char *logical_path);
/** Resolve exact original path-span bytes without normalization or creating an instance.
 */
SemanticType *semantic_type(const SemanticProgram *program, SourceSpan path);
/** Resolve one exact declared base name in its explicit owner inventory; no spelling-based inference or storage-suffix overloading.
 */
SemanticSymbol *semantic_symbol(const SemanticType *type, SourceSpan name);

#endif
