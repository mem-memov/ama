#ifndef AMA_AST_AST_H
#define AMA_AST_AST_H

#include <stdint.h>
#include "common/diagnostic.h"
#include "common/limits.h"
#include "common/source.h"

#define AST_UNRESOLVED_SYMBOL UINT32_MAX
#define MAX_AST_NESTING 128

typedef struct {
    char text[MAX_IDENTIFIER_LENGTH];
    SourceLocation location;
    uint32_t symbol_id;
} AstName;

typedef enum { AST_NAME_VALUE, AST_LITERAL_VALUE, AST_PATH_VALUE } AstValueKind;
typedef struct {
    AstValueKind kind;
    char text[MAX_TOKEN_TEXT_LENGTH];
    SourceLocation location;
    uint32_t symbol_id;
    int literal_value; /* Filled by semantic validation, not by parsing. */
} AstValue;

typedef enum { AST_PARAMETER_DECLARATION, AST_RESULT_DECLARATION } AstDeclarationKind;
typedef struct AstDeclaration {
    struct AstDeclaration* next;
    AstDeclarationKind kind;
    AstName name;
    AstValue value;
} AstDeclaration;

typedef struct AstArgument {
    struct AstArgument* next;
    bool is_parameter;
    bool is_named;
    AstName label;
    AstValue value;
    SourceLocation location;
    uint32_t callee_symbol_id;
} AstArgument;

typedef struct AstStatement AstStatement;
typedef struct {
    AstStatement* first;
    AstStatement* last;
} AstBlock;

typedef enum {
    AST_ASSIGNMENT, AST_CALL, AST_BRANCH, AST_LOOP, AST_BREAK, AST_RETURN
} AstStatementKind;
struct AstStatement {
    AstStatement* next;
    AstStatementKind kind;
    SourceLocation location;
    union {
        struct { AstName receiver; AstValue source; } assignment;
        struct {
            AstName target;
            AstArgument* arguments;
            AstArgument* last_argument;
            char module_path[MAX_MODULE_PATH_LENGTH]; /* Resolved target. */
        } call;
        struct {
            AstValue condition;
            AstBlock body;
            AstBlock alternative;
            bool has_alternative;
            SourceLocation alternative_location;
            SourceLocation end_location;
        } control;
    } as;
};

typedef struct AstAllocation AstAllocation;
typedef struct {
    AstDeclaration* declarations;
    AstDeclaration* last_declaration;
    AstBlock body;
    AstAllocation* allocations;
    bool validated;
} AstModule;

/* Initialize modules to zero. Nodes and spelling storage belong to the module;
    source paths are borrowed from its source owner. No pointer targets a token.
    Allocation addresses remain stable. Destroy is iterative, including failures.
*/
void* ast_allocate(AstModule* module, size_t size, Error* error);
void ast_destroy(AstModule* module);
void ast_append_statement(AstBlock* block, AstStatement* statement);
void ast_append_declaration(AstModule* module, AstDeclaration* declaration);
bool ast_has_alias(const AstModule* module, const char* name);

#endif
