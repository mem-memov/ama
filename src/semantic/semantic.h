/**
 * @file
 * @brief Own resolved nominal types, sources, bindings and separate dependency relations.
 *
 * [FLOW-04](../../spec/language/control-flow.md#flow-04) retains exact header
 * restart roots and incoming loop identities independently from nested calls.
 * [COMP-02](../../spec/implementation/compiler.md#comp-02)
 * [LIST-04](../../spec/language/lists.md#list-04) represents intrinsic interfaces
 * without fabricated module types, declarations or method instances.
 * [CALL-09](../../spec/language/calls-and-results.md#call-09) retains checked
 * parent-result identities and ordered output actions without copying targets.
 * [NAME-04](../../spec/language/declarations.md#name-04) shares one exact base
 * namespace; [CALL-08](../../spec/language/calls-and-results.md#call-08) shares
 * declared interface role/ordinal metadata between consumers.
 * [TYPE-06](../../spec/language/declarations.md#type-06) keeps inferred field
 * metadata independent from explicit declaration syntax;
 * [NAME-06](../../spec/language/declarations.md#name-06) retains lexical local cells separately.
 * Borrow the loaded project; only successful stage outputs are consumable. Only successful explicit/inferred stages publish usable metadata.
 */
#ifndef AMA_SEMANTIC_PROGRAM_H
#define AMA_SEMANTIC_PROGRAM_H

#include "project/dependencies/project.h"
#include "parser/abi/abi.h"
#include "semantic/relations/graph/graph.h"
#include "semantic/lists/interface.h"

/** Semantic stage status; successful valid flags gate consumption of owned partial records. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef enum { SEMANTIC_OK, SEMANTIC_INVALID, SEMANTIC_OUT_OF_MEMORY } SemanticStatus;
/** Declared storage roles have independent ordinals and initialization domains. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef enum { SYMBOL_CONSTANT, SYMBOL_PARAMETER, SYMBOL_RESULT, SYMBOL_STATE } SymbolRole;
/**
 * Forward name for one nominal module type owned by the semantic program.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticType SemanticType;
/**
 * Forward name for an immutable-value construction plan whose storage belongs to semantic
 * analysis.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticConstantPlan SemanticConstantPlan;
/**
 * Forward name for an exactly resolved source descriptor borrowing original syntax and semantic
 * symbols.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticSource SemanticSource;
/**
 * Forward name for a resolved finite field/index access graph, owned by the semantic program.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticAccess SemanticAccess;
/**
 * Forward name for one ordered resolved invocation binding borrowing its syntax source.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticBinding SemanticBinding;
/**
 * Forward name for a typed statement and its explicit nested blocks, owned by semantic analysis.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticStatement SemanticStatement;
/** Validated construction/transfer kind; ordinary identity no-op is deliberately not a static enum case. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef enum {
    TRANSFER_DEFAULT, TRANSFER_LITERAL, TRANSFER_CLONE, TRANSFER_HANDLE,
    TRANSFER_CONTINUATION
} SemanticTransfer;
/** Ordered semantic-owned source/transfer metadata borrowing its original syntax and nominal type. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticInitializer {
    struct SemanticInitializer *next;
    SemanticSource *source;
    SemanticTransfer transfer;
} SemanticInitializer;
/** Typed persistent field or lexical reference cell with original source metadata.
 * Only explicit fields retain declaration syntax; inferred fields have no creation-time
 * initializer source. Module/role ordinals apply to persistent fields; locals retain
 * their foreach scope and are excluded from the normal field/ownership inventory.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticSymbol {
    struct SemanticSymbol *next;
    const AstNode *declaration;
    SourceSpan name;
    SourceSpan spelling;
    SourceSpan site;
    SourceSpan type_site;
    const AstInitializer *initializer_syntax;
    size_t initializer_count;
    bool inferred;
    bool local;
    const AstNode *scope;
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
/** One lexical foreach role, borrowing its syntax and retaining the chosen mutable scalar-reference cell. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticForeachBinding {
    struct SemanticForeachBinding *next;
    const AstNode *syntax;
    SemanticSymbol *collection;
    SemanticSymbol *binding;
    bool introduced;
} SemanticForeachBinding;
/** Nominal project-module identity with declarations, checked ABI and copy-capability cause; no runtime instance is created. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
struct SemanticType {
    SemanticType *next;
    const ProjectModule *module;
    SemanticSymbol *first;
    SemanticSymbol *last;
    size_t symbol_count;
    size_t counts[4];
    SemanticSymbol *locals;
    SemanticForeachBinding *foreach_bindings;
    SemanticForeachBinding *last_foreach_binding;
    AbiTable abi;
    bool executable;
    bool cloneable;
    /** Native emission must check the fixed qualified from_size declaration against the actual payload type. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
    bool requires_size_bridge;
    SemanticType *clone_seed;
    SemanticSymbol *clone_cause;
    SemanticStatement *statements;
    SemanticStatement *last_statement;
};
/** Shared startup/reverse-cleanup order over borrowed resolved constant declarations. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
struct SemanticConstantPlan {
    SemanticConstantPlan *next;
    SemanticConstantPlan *previous;
    SemanticSymbol *symbol;
    SemanticInitializer *initializers;
    size_t ordinal;
};
/** Resolved declaration/index chain retaining exact syntax identity and storage form. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
struct SemanticAccess {
    const AstAccess *syntax;
    SemanticSymbol *symbol;
    SemanticAccess *index;
    bool reference;
    bool list;
};
/** Typed source view preserving literal/local/constant/acquisition/continuation mode and original site. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
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
/** Resolved source-declared or intrinsic field plus caller-side transfer metadata.
 * Exactly one field/intrinsic descriptor is present. Intrinsics have no invented
 * source declaration; diagnostics use real operands and nominal type contracts.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
struct SemanticBinding {
    SemanticBinding *next;
    const AstArgument *syntax;
    SemanticSymbol *field;
    const SemanticListField *intrinsic;
    SemanticType *type;
    size_t position;
    bool reference;
    bool list;
    SemanticSource *source;
    SemanticAccess *receiver;
    SemanticTransfer transfer;
};
/** Ordered checked output action; exactly one binding or selected operation is present.
 * Selection retains the existing result identity and never implies an owned copy.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct SemanticOutputAction {
    struct SemanticOutputAction *next;
    const AstOutputAction *syntax;
    SemanticBinding *binding;
    SemanticStatement *operation;
} SemanticOutputAction;
/** An intrinsic service/method has no executable module identity of its own. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef enum { SEMANTIC_INSTANCE_OPERATION, SEMANTIC_CONSTANT_OPERATION,
    SEMANTIC_LIST_SERVICE_OPERATION, SEMANTIC_LIST_METHOD_OPERATION } SemanticOperationKind;
/** Exact specialization of one list interface, independent of current contents or reference liveness. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct { SemanticType *element; SemanticType *integer; bool reference; } SemanticListInterface;
/** Exact selected scalar/list shape, with an original field/access or explicit intrinsic result ordinal. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct {
    SemanticType *type;
    SemanticSymbol *field;
    SemanticAccess *access;
    bool reference;
    bool list;
    bool intrinsic;
    size_t position;
} SemanticSelection;
/** Semantic-owned statement borrowing exact syntax, parent identity and ordered actions.
 * Call kind distinguishes module instances/namespaces from intrinsic services.
 * Ordinary module calls retain namespace_type; services/methods instead retain
 * list_interface/list_service and have no synthetic namespace or method access.
 * An instance selected from an intrinsic output has intrinsic_target/result and
 * target_reference, with no fake field access. Its type is namespace_type.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
struct SemanticStatement {
    SemanticStatement *next;
    const AstNode *syntax;
    SemanticStatement *enclosing_operation;
    SemanticStatement *enclosing_loop;
    SemanticStatement *loop_root;
    SemanticStatement *condition_owner;
    SemanticSelection condition;
    const SemanticForeachBinding *traversal;
    bool empty_body;
    bool negative_branch;
    SemanticOperationKind operation_kind;
    SemanticListMethod list_method;
    SemanticListInterface list_interface;
    SemanticStatement *list_service;
    bool intrinsic_target;
    bool target_reference;
    size_t intrinsic_result;
    SemanticOutputAction *actions;
    SemanticOutputAction *last_action;
    SemanticAccess *access;
    SemanticSource *source;
    SemanticType *namespace_type;
    SemanticBinding *bindings;
    SemanticBinding *last_binding;
    SemanticTransfer transfer;
};
/** Explicit native-authoring proposal borrowing its exact project module and proposed ABI source. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct {
    const ProjectModule *module;
    const SourceText *abi;
} SemanticNativeAuthoring;
/** Semantic owner of scratch, independent eager/shared graphs, diagnostics and per-stage attempt/valid guards. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef struct {
    SourceArena arena;
    DependencyGraph eager;
    DependencyGraph constants;
    const Project *project;
    SemanticType *first;
    SemanticType *last;
    SemanticStatus status;
    Diagnostic error;
    /** Bounded diagnostic storage: destination, first owned copy witness, native contract. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
    DiagnosticRelated related[3];
    bool names_attempted;
    bool names_valid;
    bool native_attempted;
    bool native_valid;
    SemanticType *list_integer;
    bool inference_attempted;
    bool inference_valid;
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
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
void semantic_program_init(SemanticProgram *program, const Project *project,
    SourceAllocator allocator);
/** Destroy every ABI/graph/arena allocation before its borrowed project disappears; all resolved views expire.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
void semantic_program_destroy(SemanticProgram *program);
/** Run ordinary names/native/inference/resolve/relations/constants stages once, returning the first failed status.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticStatus semantic_analyze(SemanticProgram *program);
/** Compose the same stages with an explicitly supplied native ABI authoring proposal, never guessed from missing files. Proposed ABI source storage outlives this semantic owner.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticStatus semantic_analyze_authoring(SemanticProgram *program,
    const SemanticNativeAuthoring *authoring);
/** Build the retained explicit module-wide name inventory once; on failure partial records remain owned and names_valid is false.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticStatus semantic_names(SemanticProgram *program);
/** Check real native companions/protocol tables after names succeed; lexical binding is separate from typed C verification.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticStatus semantic_native(SemanticProgram *program);
/** Apply an explicit proposal only to its exact borrowed module; other required native modules still need real companions.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticStatus semantic_native_authoring(SemanticProgram *program, const SemanticNativeAuthoring *authoring);
/** Link nominal types, derive copy capability, then resolve initializers/statements; publish resolved only after complete success.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticStatus semantic_resolve(SemanticProgram *program);
/** Build/order separate eager-ownership and shared-initialization graphs after resolution; preserve actual cycle sites.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticStatus semantic_relations(SemanticProgram *program);
/** Consume valid ordered shared prerequisites into startup/reverse-cleanup plans; no runtime construction occurs.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticStatus semantic_constants(SemanticProgram *program);
/** Resolve an exact NUL-terminated supported logical identity in the borrowed type inventory or return null.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticType *semantic_find_type(const SemanticProgram *program, const char *logical_path);
/** Resolve exact original path-span bytes without normalization or creating an instance.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticType *semantic_type(const SemanticProgram *program, SourceSpan path);
/** Resolve one exact declared base name in its explicit owner inventory; no spelling-based inference or storage-suffix overloading.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticSymbol *semantic_symbol(const SemanticType *type, SourceSpan name);

/** Find one exact declared interface base name in its requested constant/input/result role.
 * State fields and absent/wrong-role names return null. Storage suffix and capability checks
 * remain the consumer's responsibility; lookup allocates nothing and never changes the inventory.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticSymbol *semantic_interface_name(const SemanticType *, SymbolRole, SourceSpan);
/** Find one direction-specific interface ordinal, including fields omitted at a call site.
 * State roles and absent positions return null; after nominal linking the result
 * metadata carries its exact type/storage.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticSymbol *semantic_interface_position(const SemanticType *, SymbolRole, size_t);
/** Classify source-ordered receivers and foreach bindings after explicit/native inventories.
 * Success publishes typed inferred fields and lexical binding metadata; full transfer,
 * capability and execution validation remains in the subsequent semantic stages.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticStatus semantic_infer(SemanticProgram *);
/** Exact pure constructor availability categories shared by diagnostics and editor consumers. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
typedef enum { SEMANTIC_VISIBLE, SEMANTIC_CONSTANT_ONLY, SEMANTIC_NOT_INITIALIZED } SemanticVisibility;
/** Inspect constructor ordering/constant-only rules without changing diagnostics or ownership. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticVisibility semantic_initializer_visibility(const SemanticSymbol *, const SemanticSymbol *initializing);
/** Inspect a completed inventory at an original source point, excluding out-of-scope locals and future inferred names. Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
bool semantic_symbol_available(const SemanticType *, const SemanticSymbol *, SourceSpan, const SemanticSymbol *initializing);
/** Find the innermost introduced foreach-local binding containing this executable source site.
 * Module fields are excluded; no runtime contents or declaration-name guesses participate.
 * Component contract: [FLOW-04](../../spec/language/control-flow.md#flow-04), [COMP-02](../../spec/implementation/compiler.md#comp-02), [LIST-04](../../spec/language/lists.md#list-04), [CALL-09](../../spec/language/calls-and-results.md#call-09), [NAME-04](../../spec/language/declarations.md#name-04), [CALL-08](../../spec/language/calls-and-results.md#call-08), [TYPE-06](../../spec/language/declarations.md#type-06), [NAME-06](../../spec/language/declarations.md#name-06). */
SemanticSymbol *semantic_local_at(const SemanticType *, SourceSpan, SourceSpan);

#endif
