#ifndef AMA_SEMANTIC_PROGRAM_H
#define AMA_SEMANTIC_PROGRAM_H

#include "project/dependencies/project.h"
#include "parser/abi/abi.h"

typedef enum { SEMANTIC_OK, SEMANTIC_INVALID, SEMANTIC_OUT_OF_MEMORY } SemanticStatus;
typedef enum { SYMBOL_CONSTANT, SYMBOL_PARAMETER, SYMBOL_RESULT, SYMBOL_STATE } SymbolRole;
typedef struct SemanticType SemanticType;
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
};
typedef struct {
    SourceArena arena;
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
} SemanticProgram;

/* The loaded valid project outlives this owner. Initialize, run each semantic
 * stage once, inspect its successful output, destroy. Failed partial records
 * are retained only for diagnostics/cleanup, never accepted as validated IR. */
void semantic_program_init(SemanticProgram *program, const Project *project,
    SourceAllocator allocator);
void semantic_program_destroy(SemanticProgram *program);
SemanticStatus semantic_names(SemanticProgram *program);
SemanticStatus semantic_native(SemanticProgram *program);
SemanticType *semantic_type(const SemanticProgram *program, SourceSpan path);
SemanticSymbol *semantic_symbol(const SemanticType *type, SourceSpan name);

#endif
