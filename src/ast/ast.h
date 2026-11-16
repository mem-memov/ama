/**
 * @file
 * @brief Own source-mapped structural syntax without resolved symbols or generated C.
 *
 * [COMP-02](../../spec/implementation/compiler.md#comp-02)
 * Separate parser shape from typed IR; source tags carry no resolved symbols.
 * [CALL-08](../../spec/language/calls-and-results.md#call-08) records output
 * phase independently from ordinary result ordinals.
 * [CALL-09](../../spec/language/calls-and-results.md#call-09) retains ordered
 * result-selection actions and each refusal body's owning operation. Control headers retain one lexical repeat root under
 * [FLOW-04](../../spec/language/control-flow.md#flow-04);
 * [SYN-01](../../spec/language/source-and-names.md#syn-01) retains warnings at written empty markers.
 * [FLOW-05](../../spec/language/lists.md#flow-05) has distinct reference binders.
 */
#ifndef AMA_SYNTAX_AST_H
#define AMA_SYNTAX_AST_H

#include "parser/module/sections.h"
#include "parser/status.h"
#include "parser/error/error.h"

/** Structural base/suffix/index view; every text span borrows source, every linked node belongs to the AST arena. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef struct AstAccess {
    SourceSpan span;
    SourceSpan name;
    bool reference;
    bool whole_list;
    struct AstAccess *index;
} AstAccess;

/** Syntactic source modes; native constructor/type selection occurs after parsing. Standalone self carries no named access. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef enum {
    AST_VALUE_SOURCE, AST_ACQUIRE_SOURCE, AST_CONSTANT_SOURCE,
    AST_CONTINUATION_SOURCE, AST_LITERAL_SOURCE, AST_SELF_REFERENCE_SOURCE
} AstSourceKind;
/** Tagged borrowed source syntax: access, constant path/member or untouched literal payload. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef struct AstSource {
    SourceSpan span;
    AstSourceKind kind;
    AstAccess *access;
    SourceSpan module_path;
    SourceSpan member;
    SourceSpan payload;
    bool constant_list;
} AstSource;

/** AST-owned ordered declaration-initializer link; no runtime value is constructed here. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef struct AstInitializer {
    struct AstInitializer *next;
    AstSource *source;
} AstInitializer;
/** Direction markers distinguish entry inputs, active outputs and retained reads. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef enum { AST_INPUT, AST_OUTPUT, AST_SAVED_OUTPUT } AstArgumentKind;
/** Ordered binding/omission view; direction position and label/source/receiver remain structural, not typed. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
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

/** Explicit call syntax modes, without target acquisition or body execution. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef enum { AST_ENTRY_CALL, AST_RESUME_CALL, AST_RETAINED_READ, AST_CONSTANT_READ } AstCallMode;
/** Structural statement kinds; execution support is tracked separately by feature tasks. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef enum {
    AST_DECLARATION, AST_ASSIGNMENT, AST_CALL, AST_BRANCH,
    AST_LOOP, AST_BREAK, AST_RETURN, AST_SUSPEND, AST_FOREACH
} AstNodeKind;
/**
 * Forward declaration of an arena-owned structural syntax node; its complete tagged body follows
 * and references borrow the module lifetime.
 * Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef struct AstNode AstNode;
/** AST-owned source-order list with explicit append tail; empty bodies have null endpoints. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef struct {
    AstNode *first;
    AstNode *last;
} AstBlock;

/** Ordered output actions distinguish ordinary transfers from selected-result operations. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef enum { AST_BINDING_ACTION, AST_SELECTED_ACTION } AstOutputActionKind;
/** AST-owned action link; binding/operation views alias nodes owned by the same module arena. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef struct AstOutputAction {
    struct AstOutputAction *next;
    AstOutputActionKind kind;
    AstArgument *binding;
    AstNode *operation;
} AstOutputAction;

/** Tagged AST-only declaration/assignment/call/control payload with original source span. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
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
            AstOutputAction *first_output;
            AstOutputAction *last_output;
            AstNode *enclosing_operation;
            SourceSpan refusal_marker;
            size_t input_count;
            size_t output_count;
            bool named;
            /** [CALL-08](../../spec/language/calls-and-results.md#call-08) keeps
             * output phase independent of ordinary result-position counts. */
            bool outputs_started;
            bool has_selected;
            bool has_alternative;
            AstBlock alternative;
        } call;
        struct {
            AstAccess *condition;
            AstNode *command;
            AstNode *condition_owner;
            SourceSpan body_marker;
            SourceSpan alternative_marker;
            SourceSpan closing_marker;
            AstBlock body;
            AstBlock alternative;
        } control;
        struct {
            SourceSpan collection;
            SourceSpan binder;
            bool reference_elements;
            SourceSpan body_marker;
            SourceSpan closing_marker;
            AstBlock body;
        } traversal;
    } as;
};

/** Arena-owned nonfatal diagnostic in deterministic physical-marker order. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef struct AstWarning {
    struct AstWarning *next;
    Diagnostic diagnostic;
} AstWarning;

/** Module owns layout and AST arenas; textual spans borrow an external source manager, and invalid partial trees are not consumable. Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
typedef struct {
    SourceLayout layout;
    SourceArena arena;
    ModuleSections sections;
    AstBlock constants;
    AstBlock parameters;
    AstBlock results;
    AstBlock body;
    ParseError error;
    AstWarning *warnings;
    bool valid;
} AstModule;

/* Initialize, parse once, destroy. AST nodes belong to the module; all textual
 * ranges borrow source. No node embeds generated C or resolved symbols/types.
 * Failed partial trees cannot be consumed; diagnostic survives until destroy. */
/** Initialize layout/node owners with explicit allocator authority; all source spans remain borrowed.
 * Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
void ast_module_init(AstModule *module, SourceAllocator allocator);
/** Release all successful/partial syntax storage and invalidate the module. Text source ownership remains with its manager.
 * Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
void ast_module_destroy(AstModule *module);
/** Parse once after initialization. Publish valid only after complete required structural stages; failed partial nodes remain owned until destroy.
 * @return Explicit invalid/allocation/success status and source-mapped error; never resolve names or infer types.
 * Component contract: [COMP-02](../../spec/implementation/compiler.md#comp-02), [CALL-08](../../spec/language/calls-and-results.md#call-08), [CALL-09](../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../spec/language/control-flow.md#flow-04), [SYN-01](../../spec/language/source-and-names.md#syn-01), [FLOW-05](../../spec/language/lists.md#flow-05). */
ParseStatus parser_module_parse(AstModule *module, const SourceText *source);

#endif
