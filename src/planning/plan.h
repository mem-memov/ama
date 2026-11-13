/**
 * @file
 * @brief Own typed logical storage, lifecycle recipes and explicit control graph nodes.
 *
 * [COMP-02](../../spec/implementation/compiler.md#comp-02)
 * Borrow validated semantic identities; logical slots are not guessed C offsets, and emission must not redo resolution.
 */
#ifndef AMA_TYPED_PROJECT_PLAN_H
#define AMA_TYPED_PROJECT_PLAN_H

#include "semantic/semantic.h"
#include "storage/state.h"

/** Typed planning status; invalid/partial arena records remain owned but unusable. */
typedef enum { PLAN_OK, PLAN_INVALID, PLAN_OUT_OF_MEMORY } PlanStatus;
/** Logical ownership/cardinality form, independent of target C representation. */
typedef enum { STORAGE_OWNED, STORAGE_REFERENCE, STORAGE_OWNING_LIST, STORAGE_REFERENCE_LIST } StorageForm;
/**
 * Forward name for one resolved type/storage/control plan borrowing its semantic nominal type.
 */
typedef struct PlannedType PlannedType;
/** Plan-owned ordered nonconstant field with exact semantic declaration/type and logical slot. */
typedef struct StorageField {
    struct StorageField *next;
    struct StorageField *previous;
    SemanticSymbol *symbol;
    PlannedType *type;
    StorageForm form;
    size_t slot;
} StorageField;
/** Immutable shared constant record with startup/reverse links and explicitly absent instance service state. */
typedef struct SharedStorage {
    struct SharedStorage *next;
    struct SharedStorage *previous;
    const SemanticConstantPlan *constant;
    PlannedType *value_type;
    StorageForm form;
    bool immutable;
    unsigned service_storage;
} SharedStorage;
/** Typed lifecycle actions; native hooks and field ownership remain explicit operands. */
typedef enum {
    LIFE_CONSTRUCT_FIELD, LIFE_CONSTRUCT_NATIVE, LIFE_MARK_LIVE,
    LIFE_DESTROY_NATIVE, LIFE_DESTROY_FIELD, LIFE_MARK_FINALIZED,
    LIFE_PREFLIGHT_SOURCE, LIFE_BEGIN_CLONE_MAP, LIFE_CLONE_OWNED,
    LIFE_COPY_REFERENCE, LIFE_CLONE_NATIVE, LIFE_REMAP_REFERENCES,
    LIFE_CLEAR_POSITIONS, LIFE_PRESERVE_POSITIONS, LIFE_RELEASE_CLONE_MAP,
    LIFE_UNWIND_PARTIAL, LIFE_SUCCESS, LIFE_FAILURE
} LifeOperation;
/** Plan-owned lifecycle step with exact source/type/field/hook and explicit failure edge. */
typedef struct LifeNode {
    struct LifeNode *allocated_next;
    struct LifeNode *next;
    struct LifeNode *failure;
    LifeOperation operation;
    PlannedType *type;
    StorageField *field;
    const AbiBinding *binding;
    SourceSpan site;
} LifeNode;
/** Owned lifecycle procedure endpoints and cleanup policy, consumed only through a valid plan. */
typedef struct {
    LifeNode *first;
    LifeNode *last;
    LifeNode *failure;
    bool accumulates_errors;
} LifeProcedure;
/** Resolved control actions with separate selection, mutation, refusal, failure and continuation phases. */
typedef enum {
    CONTROL_COMPLETE, CONTROL_YIELD, CONTROL_CAPTURE_ERROR, CONTROL_CLEANUP_FAILURE,
    CONTROL_FAILURE, CONTROL_CONDITION, CONTROL_SELECT_RECEIVER, CONTROL_SELECT_SOURCE,
    CONTROL_PREFLIGHT_REPLACEMENT, CONTROL_PREPARE_REPLACEMENT,
    CONTROL_COMMIT_REPLACEMENT, CONTROL_RELEASE_VALUES, CONTROL_SAVE_POSITION,
    CONTROL_ACQUIRE_INSTANCE, CONTROL_REFUSAL_SNAPSHOT,
    CONTROL_EVALUATE_INPUT, CONTROL_INSTALL_PARAMETER, CONTROL_INITIALIZE_RESULT,
    CONTROL_SET_RETURN, CONTROL_ENTER_INSTANCE, CONTROL_RESUME_INSTANCE,
    CONTROL_NATIVE_ENTRY,
    CONTROL_TRANSFER_OUTPUT, CONTROL_RELEASE_TARGET
} ControlOperation;
/** Plan-owned uniquely selected control node; allocation links are separate from typed execution/alternative/failure/resume edges. */
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
/** Explicit logical instance-service requirements; shared immutable storage has none of these bits. */
enum {
    STORAGE_GENERATION = 1u, STORAGE_BUSY = 2u, STORAGE_POSITION = 4u,
    STORAGE_RETURN_LINK = 8u, STORAGE_REFUSAL = 16u, STORAGE_ACQUISITION = 32u,
    STORAGE_CONSTRUCTING_IDENTITY = 64u, STORAGE_OWNERSHIP = 128u
};
/** One planned nominal type with logical fields/service requirements, lifecycle procedures and retained body graph. */
struct PlannedType {
    PlannedType *next;
    SemanticType *type;
    StorageField *first;
    StorageField *last;
    size_t field_count;
    size_t layout_ordinal;
    unsigned service_storage;
    bool requires_pool;
    /** General stable-slot representation; no elision is claimed without an exact proof. */
    bool stable_slots;
    StorageFrame *frames;
    StorageFrame *last_frame;
    size_t frame_count;
    ScopedStorageCell *cells;
    ScopedStorageCell *last_cell;
    size_t cell_count;
    ReferenceOriginStorage *origins;
    ReferenceOriginStorage *last_origin;
    size_t origin_count;
    LifeNode *lifecycles;
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
/** Plan arena owner borrowing a complete semantic program and retaining unique selector allocation and valid/failure state. */
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
    ContextStorage context_storage;
    bool storage_ready;
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
/** Require complete valid semantic constant plans and borrow that owner through plan destruction; selectors start at nonzero one.
 */
void program_plan_init(ProgramPlan *plan, const SemanticProgram *semantic, SourceAllocator allocator);
/** Release all storage/lifecycle/control arena records; borrowed semantic/project owners remain untouched.
 */
void program_plan_destroy(ProgramPlan *plan);
/** Build once: all nominal records, storage, per-type procedures/control and forward call links.
 * @return Explicit failure without valid output, or success with one complete typed plan; never parse source or emit C text.
 */
PlanStatus program_plan_build(ProgramPlan *plan);
/** Check completed owner-linked IR without allocation; failure revokes publication and retains an original source site.
 * Ownership inventory chains must be intact; executable edges may contain cycles.
 * Borrowed semantics must remain complete and immutable. Success does not publish an unbuilt plan.
 */
bool program_plan_validate(ProgramPlan *plan);
/** Find the exact semantic type pointer in already created planned types, or return null; no spelling-based type recreation.
 */
PlannedType *program_plan_type(const ProgramPlan *plan, const SemanticType *type);

#endif
