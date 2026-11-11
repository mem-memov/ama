#ifndef AMA_SYNTAX_AST_H
#define AMA_SYNTAX_AST_H

#include "parser/module/sections.h"
#include "parser/status.h"

typedef struct AstAccess {
    SourceSpan span;
    SourceSpan name;
    bool reference;
    bool whole_list;
    struct AstAccess *index;
} AstAccess;

typedef enum {
    AST_VALUE_SOURCE, AST_ACQUIRE_SOURCE, AST_CONSTANT_SOURCE,
    AST_CONTINUATION_SOURCE, AST_LITERAL_SOURCE
} AstSourceKind;
typedef struct AstSource {
    SourceSpan span;
    AstSourceKind kind;
    AstAccess *access;
    SourceSpan module_path;
    SourceSpan member;
    SourceSpan payload;
    bool constant_list;
} AstSource;

typedef struct AstInitializer {
    struct AstInitializer *next;
    AstSource *source;
} AstInitializer;
typedef enum { AST_INPUT, AST_OUTPUT, AST_SAVED_OUTPUT } AstArgumentKind;
typedef struct AstArgument {
    struct AstArgument *next;
    SourceSpan span;
    SourceSpan marker;
    AstArgumentKind kind;
    AstAccess *label;
    AstSource *source;
    AstAccess *receiver;
    size_t position;
    bool omitted;
} AstArgument;

typedef enum { AST_ENTRY_CALL, AST_RESUME_CALL, AST_RETAINED_READ, AST_CONSTANT_READ } AstCallMode;
typedef enum {
    AST_DECLARATION, AST_ASSIGNMENT, AST_CALL, AST_BRANCH,
    AST_LOOP, AST_BREAK, AST_RETURN, AST_SUSPEND
} AstNodeKind;
typedef struct AstNode AstNode;
typedef struct {
    AstNode *first;
    AstNode *last;
} AstBlock;

struct AstNode {
    AstNode *next;
    AstNodeKind kind;
    SourceSpan span;
    union {
        struct {
            AstAccess *name;
            SourceSpan type;
            AstInitializer *first;
            AstInitializer *last;
            size_t initializer_count;
        } declaration;
        struct { AstAccess *receiver; AstSource *source; } assignment;
        struct {
            AstCallMode mode;
            AstAccess *target;
            SourceSpan namespace_path;
            AstArgument *first;
            AstArgument *last;
            size_t input_count;
            size_t output_count;
            bool named;
            AstBlock alternative;
        } call;
        struct { AstAccess *condition; AstBlock body; AstBlock alternative; } control;
    } as;
};

typedef struct {
    SourceLayout layout;
    SourceArena arena;
    ModuleSections sections;
    AstBlock constants;
    AstBlock parameters;
    AstBlock results;
    AstBlock body;
    Diagnostic diagnostic;
    bool valid;
} AstModule;

/* Initialize, parse once, destroy. AST nodes belong to the module; all textual
 * ranges borrow source. No node embeds generated C or resolved symbols/types.
 * Failed partial trees cannot be consumed; diagnostic survives until destroy. */
void ast_module_init(AstModule *module, SourceAllocator allocator);
void ast_module_destroy(AstModule *module);
ParseStatus parser_module_parse(AstModule *module, const SourceText *source);

#endif
