#ifndef AMA_TYPED_PROJECT_PLAN_H
#define AMA_TYPED_PROJECT_PLAN_H

#include "semantic/semantic.h"

typedef enum { PLAN_OK, PLAN_INVALID, PLAN_OUT_OF_MEMORY } PlanStatus;
typedef enum { STORAGE_OWNED, STORAGE_REFERENCE, STORAGE_OWNING_LIST, STORAGE_REFERENCE_LIST } StorageForm;
typedef struct PlannedType PlannedType;
typedef struct StorageField {
    struct StorageField *next;
    struct StorageField *previous;
    SemanticSymbol *symbol;
    PlannedType *type;
    StorageForm form;
    size_t slot;
} StorageField;
typedef struct SharedStorage {
    struct SharedStorage *next;
    struct SharedStorage *previous;
    const SemanticConstantPlan *constant;
    PlannedType *value_type;
    StorageForm form;
    bool immutable;
    unsigned service_storage;
} SharedStorage;
typedef enum {
    LIFE_CONSTRUCT_FIELD, LIFE_CONSTRUCT_NATIVE, LIFE_MARK_LIVE,
    LIFE_DESTROY_NATIVE, LIFE_DESTROY_FIELD, LIFE_MARK_FINALIZED,
    LIFE_PREFLIGHT_SOURCE, LIFE_BEGIN_CLONE_MAP, LIFE_CLONE_OWNED,
    LIFE_COPY_REFERENCE, LIFE_CLONE_NATIVE, LIFE_REMAP_REFERENCES,
    LIFE_CLEAR_POSITIONS, LIFE_PRESERVE_POSITIONS, LIFE_RELEASE_CLONE_MAP,
    LIFE_UNWIND_PARTIAL, LIFE_SUCCESS, LIFE_FAILURE
} LifeOperation;
typedef struct LifeNode {
    struct LifeNode *next;
    struct LifeNode *failure;
    LifeOperation operation;
    PlannedType *type;
    StorageField *field;
    const AbiBinding *binding;
    SourceSpan site;
} LifeNode;
typedef struct {
    LifeNode *first;
    LifeNode *last;
    LifeNode *failure;
    bool accumulates_errors;
} LifeProcedure;
typedef enum {
    CONTROL_COMPLETE, CONTROL_YIELD, CONTROL_CAPTURE_ERROR, CONTROL_CLEANUP_FAILURE,
    CONTROL_FAILURE, CONTROL_CONDITION, CONTROL_SELECT_RECEIVER, CONTROL_SELECT_SOURCE,
    CONTROL_PREFLIGHT_REPLACEMENT, CONTROL_PREPARE_REPLACEMENT,
    CONTROL_COMMIT_REPLACEMENT, CONTROL_RELEASE_VALUES, CONTROL_SAVE_POSITION,
    CONTROL_ACQUIRE_INSTANCE, CONTROL_ACQUIRE_LIST, CONTROL_REFUSAL_SNAPSHOT,
    CONTROL_EVALUATE_INPUT, CONTROL_INSTALL_PARAMETER, CONTROL_INITIALIZE_RESULT,
    CONTROL_SET_RETURN, CONTROL_ENTER_INSTANCE, CONTROL_RESUME_INSTANCE,
    CONTROL_NATIVE_ENTRY, CONTROL_LIST_INSTALL_INPUT, CONTROL_LIST_ACTION,
    CONTROL_TRANSFER_OUTPUT, CONTROL_RELEASE_TARGET
} ControlOperation;
typedef struct ControlNode {
    struct ControlNode *allocated_next;
    struct ControlNode *next;
    struct ControlNode *alternative;
    struct ControlNode *failure;
    struct ControlNode *resume;
    ControlOperation operation;
    size_t selector;
    size_t position;
    PlannedType *type;
    PlannedType *callee;
    SemanticType *value_type;
    StorageForm value_form;
    const SemanticStatement *statement;
    const SemanticBinding *binding;
    SemanticSymbol *field;
    SourceSpan site;
} ControlNode;
enum {
    STORAGE_GENERATION = 1u, STORAGE_BUSY = 2u, STORAGE_POSITION = 4u,
    STORAGE_RETURN_LINK = 8u, STORAGE_REFUSAL = 16u, STORAGE_ACQUISITION = 32u
};
struct PlannedType {
    PlannedType *next;
    SemanticType *type;
    StorageField *first;
    StorageField *last;
    size_t field_count;
    size_t layout_ordinal;
    unsigned service_storage;
    bool requires_pool;
    LifeProcedure create;
    LifeProcedure literal;
    LifeProcedure destroy;
    LifeProcedure clone;
    LifeProcedure continuation;
    ControlNode *controls;
    ControlNode *entry;
    ControlNode *complete;
    ControlNode *yield;
    ControlNode *failure;
};
typedef struct {
    SourceArena arena;
    const SemanticProgram *semantic;
    PlannedType *first;
    PlannedType *last;
    PlannedType *entry;
    const SemanticConstantPlan *constant_startup;
    const SemanticConstantPlan *constant_cleanup;
    SharedStorage *shared_first;
    SharedStorage *shared_last;
    size_t next_selector;
    PlanStatus status;
    Diagnostic error;
    bool attempted;
    bool valid;
} ProgramPlan;

/* The successfully resolved/ordered/shared-planned semantic owner outlives this
 * plan. Initialize, build once, inspect only success, destroy. Logical slots are
 * not guessed C byte offsets: the native C compiler supplies sizeof/alignment.
 * No AST reparsing, C execution or generated C strings occur during planning. */
void program_plan_init(ProgramPlan *plan, const SemanticProgram *semantic, SourceAllocator allocator);
void program_plan_destroy(ProgramPlan *plan);
PlanStatus program_plan_build(ProgramPlan *plan);
PlannedType *program_plan_type(const ProgramPlan *plan, const SemanticType *type);

#endif
