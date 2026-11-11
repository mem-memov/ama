#ifndef AMA_SEMANTIC_PROGRAM_H
#define AMA_SEMANTIC_PROGRAM_H

#include "project/dependencies/project.h"
#include "parser/abi/abi.h"
#include "semantic/relations/graph/graph.h"

typedef enum { SEMANTIC_OK, SEMANTIC_INVALID, SEMANTIC_OUT_OF_MEMORY } SemanticStatus;
typedef enum { SYMBOL_CONSTANT, SYMBOL_PARAMETER, SYMBOL_RESULT, SYMBOL_STATE } SymbolRole;
typedef struct SemanticType SemanticType;
typedef struct SemanticConstantPlan SemanticConstantPlan;
typedef struct SemanticSource SemanticSource;
typedef struct SemanticAccess SemanticAccess;
typedef struct SemanticBinding SemanticBinding;
typedef struct SemanticStatement SemanticStatement;
typedef enum {
    TRANSFER_DEFAULT, TRANSFER_LITERAL, TRANSFER_CLONE, TRANSFER_HANDLE,
    TRANSFER_CONTINUATION, TRANSFER_NOOP
} SemanticTransfer;
typedef struct SemanticInitializer {
    struct SemanticInitializer *next;
    SemanticSource *source;
    SemanticTransfer transfer;
} SemanticInitializer;
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
    SemanticStatement *statements;
    SemanticStatement *last_statement;
};
struct SemanticConstantPlan {
    SemanticConstantPlan *next;
    SemanticConstantPlan *previous;
    SemanticSymbol *symbol;
    SemanticInitializer *initializers;
    size_t ordinal;
};
struct SemanticAccess {
    const AstAccess *syntax;
    SemanticSymbol *symbol;
    SemanticAccess *index;
    bool reference;
    bool list;
};
struct SemanticSource {
    const AstSource *syntax;
    SourceSpan site;
    AstSourceKind kind;
    SemanticAccess *access;
    SemanticSymbol *constant;
    SemanticType *type;
    bool reference;
    bool list;
};
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
typedef struct {
    const char *action;
    const char *index;
    const char *condition;
    const char *outcome;
} SemanticLibraries;
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
    SemanticLibraries libraries;
    SemanticType *library_types[4];
    SemanticConstantPlan *constant_first;
    SemanticConstantPlan *constant_last;
    size_t constant_count;
    bool constants_attempted;
    bool constants_valid;
} SemanticProgram;

/* The loaded valid project outlives this owner. Initialize, run each semantic
 * stage once, inspect its successful output, destroy. Failed partial records
 * are retained only for diagnostics/cleanup, never accepted as validated IR. */
void semantic_program_init(SemanticProgram *program, const Project *project,
    SourceAllocator allocator);
void semantic_program_destroy(SemanticProgram *program);
SemanticStatus semantic_names(SemanticProgram *program);
SemanticStatus semantic_native(SemanticProgram *program);
SemanticStatus semantic_resolve(SemanticProgram *program, SemanticLibraries libraries);
SemanticStatus semantic_relations(SemanticProgram *program);
SemanticStatus semantic_constants(SemanticProgram *program);
SemanticType *semantic_find_type(const SemanticProgram *program, const char *logical_path);
SemanticType *semantic_type(const SemanticProgram *program, SourceSpan path);
SemanticSymbol *semantic_symbol(const SemanticType *type, SourceSpan name);

#endif
