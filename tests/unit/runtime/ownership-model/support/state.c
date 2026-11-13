/** @file
 * @brief Define the independent two-root model and one running caller with at most one retained read acquisition.
 */
/** Public root identity, stable cell, current owned child identities and expected native data; resume is one opaque available saved marker. */
typedef struct {
    bool live;
    bool resume;
    AmaHandle handle;
    AmaHandle children[2];
    AmaPoolCell *cell;
    size_t numbers[3];
} OwnershipSlot;
/** Model owns no runtime allocation: it predicts two optional scalar trees, one explicit caller and the exact generation history exposed at the API boundary. */
typedef struct {
    OwnershipSlot slots[2];
    AmaHandle caller;
    AmaHandle caller_children[2];
    AmaHandle caller_self;
    AmaHandle caller_selected;
    size_t held;
    bool child;
    bool refusal;
    bool fault_seen;
    AmaHandle retired[128];
    size_t retired_count;
} OwnershipModel;
/** Exact finite scheduler alphabet; ineligible actions are explicit no-ops, not successful runtime operations. */
typedef enum {
    MODEL_CREATE_FIRST, MODEL_CREATE_SECOND,
    MODEL_CLONE_FIRST_TO_SECOND, MODEL_CLONE_SECOND_TO_FIRST,
    MODEL_REPLACE_FIRST_FROM_SECOND, MODEL_REPLACE_SECOND_FROM_FIRST,
    MODEL_SELF_FIRST, MODEL_SELF_SECOND,
    MODEL_ACQUIRE_FIRST, MODEL_ACQUIRE_FIRST_CHILD,
    MODEL_ACQUIRE_SECOND, MODEL_ACQUIRE_SECOND_CHILD,
    MODEL_RELEASE, MODEL_DESTROY_FIRST, MODEL_DESTROY_SECOND,
    MODEL_SEED_FIRST_RESUME, MODEL_SEED_SECOND_RESUME,
    MODEL_PROBE_CALLER_BUSY, MODEL_ACTION_COUNT
} ModelAction;
/** Resolve one currently published identity; this observer may inspect protected state without acting as an Ama read operation. */
static AmaPoolCell *ownership_cell(AmaHandle handle) {
    AmaObjectType *type = ama_world_type(&world, handle.type_id);
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, handle, false, &cell);
    assert(status.code == AMA_OK);
    return cell;
}
/** Initialize one explicit caller and the canonical self-reference construction schema; execute no Ama body. */
static OwnershipModel setup_ownership_model(void) {
    setup_construction_program();
    OwnershipModel model = {.held = 2};
    AmaStatus status = ama_program_start(&program, 2, &model.caller);
    assert(status.code == AMA_OK && constructed == 3);
    assert_constructed_self_tree(model.caller);
    const RootValue *caller_value = object_payload(model.caller);
    model.caller_children[0] = caller_value->left;
    model.caller_children[1] = caller_value->right;
    model.caller_self = model.caller;
    model.caller_selected = ((RootValue *)object_payload(model.caller))->left;
    AmaServiceState *caller = &ownership_cell(model.caller)->metadata.service;
    caller->activity = AMA_INSTANCE_RUNNING;
    caller->return_link = (AmaReturnLink){.attached = true, .host = true};
    program.current = model.caller;
    return model;
}
/** Observe public topology once after successful creation/replacement; expected payload numbers come from fixture semantics or the independent source model. */
static void cache_ownership_slot(OwnershipSlot *slot, AmaHandle handle, const size_t numbers[3]) {
    RootValue *root = object_payload(handle);
    *slot = (OwnershipSlot){.live = true, .handle = handle, .children = {root->left, root->right}, .cell = ownership_cell(handle)};
    memcpy(slot->numbers, numbers, sizeof(slot->numbers));
}
/** Record a formerly public generation; every later model prefix must reject it even if the physical slot is reused. */
static void retire_model_identity(OwnershipModel *model, AmaHandle handle) {
    assert(model->retired_count < 128);
    model->retired[model->retired_count++] = handle;
}
/** Exactly one required allocation may be failed in a trace; its error supersedes later preflight decisions without changing public state. */
static bool model_allocation_failed(OwnershipModel *model, size_t before, AmaStatus status) {
    bool failed = failure_at != 0 && before < failure_at && allocation_count >= failure_at;
    if (failed) { assert(status.code == AMA_OUT_OF_MEMORY); model->fault_seen = true; }
    return failed;
}
