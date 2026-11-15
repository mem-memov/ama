/** @file
 * @brief Exercise admitted inspection without execution, implicit native reads or history reconstruction.
 * Counted context fixtures retain ownership; each test releases inspection before disposal.
 */
#include "../contexts/common.c"
#include "runtime/debugger/debugger.h"
#include <sys/mman.h>
#include <unistd.h>

static size_t observed, previews;
static AmaDebugSession *active_session;
static unsigned malformed_preview;

/** A real competing thread cannot acquire either inspection or mutable execution authority. */
static int competing_inspector(void *context) {
    uint64_t identity = *(const uint64_t *)context;
    AmaDebugSession session = {0};
    AmaContextLease lease = {0};
    assert(ama_debug_begin(&registry,identity,NULL,NULL,0,&session).code == AMA_BUSY);
    assert(ama_context_enter(&registry,identity,&lease).code == AMA_BUSY);
    assert(session.lease.context == NULL && lease.context == NULL);
    return 0;
}

/** Read only the explicitly authorized fixture native value during its synchronous callback. */
static AmaStatus read_number(const void *payload, AmaDebugNativeVisitor visit, void *context) {
    previews++;
    unsigned char number = (unsigned char)((const NativeValue *)payload)->number;
    AmaDebugNativeValue value = {.kind=AMA_DEBUG_INTEGER,.bytes={&number,1}};
    (void)visit(context,&value);
    return ama_status_success();
}

/** Deliberately break one transport contract at a time without modifying program state. */
static AmaStatus read_malformed(const void *payload, AmaDebugNativeVisitor visit, void *context) {
    (void)payload;
    unsigned char zero[2] = {0,0};
    AmaDebugNativeValue value = {.kind=AMA_DEBUG_INTEGER,.bytes={zero,1}};
    if (malformed_preview == 0) { return ama_status_success(); }
    if (malformed_preview == 1) { value.negative = true; }
    if (malformed_preview == 2) { value.bytes.length = 2; }
    if (malformed_preview == 3) { value.bytes.data = NULL; }
    if (malformed_preview == 4) { value.kind = (AmaDebugValueKind)99; }
    (void)visit(context,&value);
    return ama_status_success();
}

/** Check callback lifetime protection and allow nested read-only context queries. */
static bool observe_instance(void *context, const AmaDebugInstance *value) {
    (void)context;
    assert(value->reference.context != 0 && value->reference.local.generation != 0);
    observed++;
    AmaDebugContext snapshot;
    assert(ama_debug_context(active_session,&snapshot).code == AMA_OK);
    assert(ama_debug_end(active_session).code == AMA_BUSY);
    return true;
}

/** The fixture root has six ordered declared fields; inspecting them executes no native hooks. */
static bool observe_field(void *context, const AmaDebugFieldValue *value) {
    (void)context;
    assert(value->field == observed && value->role == AMA_PROGRAM_STATE);
    observed++;
    return true;
}

/** Copy callback-scoped data while the explicit reader still owns its borrow. */
static bool observe_native(void *context, const AmaDebugNativeValue *value) {
    (void)context;
    assert(value->kind == AMA_DEBUG_INTEGER && value->bytes.length == 1);
    observed++;
    return false;
}

/** Inspection holds context admission, permits busy metadata, and never silently reads native State. */
static void it_inspects_declared_state_under_exclusive_admission(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity,&lease);
    AmaContextReference root = create_root(&lease,view);
    assert(root_value(&lease,root)->native.address != NULL);
    AmaPoolCell *cell = NULL;
    assert(ama_pool_lookup(&view.world->types[1]->pool,root.local,false,&cell).code == AMA_OK);
    cell->metadata.service.current_operation = 123;
    cell->metadata.service.last_refusal = ama_status_make(AMA_BUSY,NULL,0);
    leave_context(&lease);
    AmaDebugSession session = {0};
    active_session = &session;
    assert(ama_debug_begin(&registry,identity,NULL,NULL,0,&session).code == AMA_OK);
    assert(ama_context_enter(&registry,identity,&lease).code == AMA_BUSY);
    assert(ama_context_dispose(&registry,identity).code == AMA_BUSY);
    AmaDebugInstance snapshot;
    assert(ama_debug_instance(&session,root,&snapshot).code == AMA_OK);
    assert(snapshot.recorded_operation == 123 && snapshot.recorded_location == NULL);
    assert(snapshot.type_name == NULL && snapshot.native_opaque && snapshot.last_refusal.code == AMA_BUSY);
    assert(!snapshot.resume.valid);
    observed = 0;
    assert(ama_debug_instances(&session,observe_instance,NULL).code == AMA_OK && observed != 0);
    observed = 0;
    assert(ama_debug_fields(&session,root,observe_field,NULL).code == AMA_OK && observed == 6);
    observed = previews = 0;
    assert(ama_debug_native(&session,root,observe_native,NULL).code == AMA_INVALID_STATE);
    assert(observed == 0 && previews == 0);
    assert(ama_debug_end(&session).code == AMA_OK);
    assert(ama_debug_end(&session).code != AMA_OK);
    assert(ama_debug_instance(&session,root,&snapshot).code != AMA_OK);
    finish_contexts();
}

/** Equal local IDs in foreign contexts never grant authority; explicit readers remain synchronous. */
static void it_checks_origin_and_explicit_preview_authority(void) {
    setup_contexts();
    uint64_t first = create_context(), second = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(first,&lease);
    AmaContextReference root = create_root(&lease,view);
    leave_context(&lease);
    view = enter_context(second,&lease);
    AmaContextReference foreign = create_root(&lease,view);
    leave_context(&lease);
    assert(ama_handle_equal(root.local,foreign.local));
    AmaDebugNativeReader reader = {.type_id=2,.read=read_number};
    AmaDebugSession session = {0};
    assert(ama_debug_begin(&registry,first,NULL,&reader,1,&session).code == AMA_OK);
    AmaDebugInstance snapshot = {.recorded_operation=999};
    assert(ama_debug_instance(&session,foreign,&snapshot).code == AMA_TYPE_MISMATCH);
    assert(snapshot.recorded_operation == 999);
    observed = previews = 0;
    assert(ama_debug_native(&session,root,observe_native,NULL).code == AMA_OK);
    assert(observed == 1 && previews == 1);
    assert(ama_debug_end(&session).code == AMA_OK);
    finish_contexts();
}

/** Reject mismatched metadata and executing/construction windows without leaking the private gate. */
static void it_rejects_invalid_admission_without_publishing_a_session(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaDebugSession session = {0};
    AmaDebugCatalog invalid = {0};
    assert(ama_debug_begin(&registry,identity,&invalid,NULL,0,&session).code == AMA_INVALID_STATE);
    assert(session.lease.context == NULL);
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity,&lease);
    view.world->operation_active = true;
    leave_context(&lease);
    assert(ama_debug_begin(&registry,identity,NULL,NULL,0,&session).code == AMA_BUSY);
    view = enter_context(identity,&lease);
    view.world->operation_active = false;
    view.control->phase = AMA_RUNTIME_EXECUTING;
    leave_context(&lease);
    assert(ama_debug_begin(&registry,identity,NULL,NULL,0,&session).code == AMA_BUSY);
    view = enter_context(identity,&lease);
    view.control->phase = AMA_RUNTIME_IDLE;
    leave_context(&lease);
    assert(ama_debug_begin(&registry,identity,NULL,NULL,0,&session).code == AMA_OK);
    assert(ama_debug_end(&session).code == AMA_OK);
    finish_contexts();
}

/** Busy metadata is observable, but an explicit native reader still cannot inspect an acquired value. */
static void it_separates_busy_metadata_from_native_read_admission(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity,&lease);
    AmaContextReference root = create_root(&lease,view);
    AmaPoolCell *cell = NULL;
    assert(ama_pool_lookup(&view.world->types[1]->pool,root.local,false,&cell).code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    leave_context(&lease);
    AmaDebugNativeReader reader = {.type_id=2,.read=read_number};
    AmaDebugSession session = {0};
    assert(ama_debug_begin(&registry,identity,NULL,&reader,1,&session).code == AMA_OK);
    AmaDebugInstance snapshot;
    assert(ama_debug_instance(&session,root,&snapshot).code == AMA_OK && snapshot.activity == AMA_INSTANCE_ACQUIRED);
    previews = 0;
    assert(ama_debug_native(&session,root,observe_native,NULL).code == AMA_BUSY && previews == 0);
    assert(ama_debug_end(&session).code == AMA_OK);
    view = enter_context(identity,&lease);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    assert(ama_object_destroy(view.world,root.local).code == AMA_OK);
    leave_context(&lease);
    assert(ama_debug_begin(&registry,identity,NULL,NULL,0,&session).code == AMA_OK);
    assert(ama_debug_instance(&session,root,&snapshot).code == AMA_STALE_REFERENCE);
    assert(ama_debug_end(&session).code == AMA_OK);
    finish_contexts();
}

/** Real protected memory detects accidental native-payload reads during generic inspection. */
static void it_never_reads_a_protected_opaque_native_payload(void) {
    setup_contexts();
    long measured = sysconf(_SC_PAGESIZE);
    assert(measured > 0);
    size_t page = (size_t)measured;
    AmaContextType opaque = {.type_id=9,.size=page*2,.alignment=page,.slots_per_block=1,
        .generation_limit=UINT64_MAX,.native_offset=page,.hooks=leaf_type.hooks};
    definition = (AmaContextDefinition){.types=&opaque,.type_count=1};
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity,&lease);
    AmaHandle handle;
    assert(ama_object_create(view.world,9,NULL,&handle).code == AMA_OK);
    AmaPoolCell *cell = NULL;
    assert(ama_pool_lookup(&view.world->types[0]->pool,handle,false,&cell).code == AMA_OK);
    unsigned char *payload = ama_pool_payload(&view.world->types[0]->pool,cell);
    assert(mprotect(payload+page,page,PROT_NONE) == 0);
    leave_context(&lease);
    AmaDebugSession session = {0};
    active_session = &session;
    assert(ama_debug_begin(&registry,identity,NULL,NULL,0,&session).code == AMA_OK);
    AmaDebugInstance snapshot;
    assert(ama_debug_instance(&session,(AmaContextReference){identity,handle},&snapshot).code == AMA_OK);
    observed = 0;
    assert(ama_debug_instances(&session,observe_instance,NULL).code == AMA_OK && observed == 1);
    observed = 0;
    assert(ama_debug_fields(&session,snapshot.reference,observe_field,NULL).code == AMA_OK && observed == 0);
    assert(ama_debug_end(&session).code == AMA_OK);
    assert(mprotect(payload+page,page,PROT_READ|PROT_WRITE) == 0);
    finish_contexts();
}

/** Terminal cleanup leaves diagnostics, not instances or an invented historical caller graph. */
static void it_preserves_terminal_status_without_reconstructing_state(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity,&lease);
    AmaContextReference root = create_root(&lease,view);
    assert(ama_context_fail(&lease,ama_status_make(AMA_NATIVE_FAILURE,"failure",7)).code == AMA_NATIVE_FAILURE);
    leave_context(&lease);
    AmaDebugSession session = {0};
    active_session = &session;
    assert(ama_debug_begin(&registry,identity,NULL,NULL,0,&session).code == AMA_OK);
    AmaDebugContext snapshot;
    assert(ama_debug_context(&session,&snapshot).code == AMA_OK);
    assert(snapshot.failed && snapshot.phase == AMA_RUNTIME_TERMINATED && snapshot.status.code == AMA_NATIVE_FAILURE);
    assert(snapshot.type_count == 0 && snapshot.constant_count == 0 && snapshot.root.context == 0);
    observed = 0;
    assert(ama_debug_instances(&session,observe_instance,NULL).code == AMA_OK && observed == 0);
    AmaDebugInstance instance;
    assert(ama_debug_instance(&session,root,&instance).code == AMA_INVALID_STATE);
    assert(ama_debug_end(&session).code == AMA_OK);
    assert(ama_context_registry_destroy(&registry).code == AMA_NATIVE_FAILURE);
    destroy_objects();
}

/** Explicit metadata is checked before publication; invalid readers cannot change Ama diagnostics. */
static void it_validates_catalogs_and_native_preview_transport(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity,&lease);
    AmaContextReference root = create_root(&lease,view);
    leave_context(&lease);
    AmaDebugField names[6] = {{"left",0},{"right",0},{"branches",0},{"selected",0},{"self",0},{"references",0}};
    AmaDebugType type = {.type_id=2,.name="root",.fields=names,.field_count=6};
    AmaDebugConstant constant = {.owner="root",.name="constant",.operation=17};
    AmaDebugCatalog catalog = {.types=&type,.type_count=1,.constants=&constant,.constant_count=1};
    AmaDebugSession session = {0};
    assert(ama_debug_begin(&registry,identity,&catalog,NULL,0,&session).code == AMA_OK);
    AmaDebugInstance snapshot;
    assert(ama_debug_instance(&session,root,&snapshot).code == AMA_OK && strcmp(snapshot.type_name,"root") == 0);
    assert(ama_debug_end(&session).code == AMA_OK);
    names[0].operation = 999;
    assert(ama_debug_begin(&registry,identity,&catalog,NULL,0,&session).code == AMA_INVALID_STATE);
    names[0].operation = 0;
    type.type_id = 0;
    assert(ama_debug_begin(&registry,identity,&catalog,NULL,0,&session).code == AMA_INVALID_STATE);
    type.type_id = 2;
    AmaDebugNativeReader readers[2] = {{2,read_malformed},{2,read_number}};
    assert(ama_debug_begin(&registry,identity,&catalog,readers,SIZE_MAX,&session).code == AMA_LIMIT_EXCEEDED);
    assert(ama_debug_begin(&registry,identity,&catalog,readers,2,&session).code == AMA_INVALID_STATE);
    assert(ama_debug_begin(&registry,identity,&catalog,readers,1,&session).code == AMA_OK);
    for (malformed_preview = 0; malformed_preview < 5; malformed_preview++) {
        observed = 0;
        assert(ama_debug_native(&session,root,observe_native,NULL).code == AMA_INVALID_STATE);
        assert(observed == 0 && session.visits == 0);
        assert(ama_debug_instance(&session,root,&snapshot).code == AMA_OK && snapshot.last_refusal.code == AMA_OK);
        AmaDebugContext state;
        assert(ama_debug_context(&session,&state).code == AMA_OK && state.status.code == AMA_OK && !state.failed);
    }
    assert(ama_debug_end(&session).code == AMA_OK);
    finish_contexts();
}

/** Exclusive admission rejects a simultaneous observer and executor before either can read mutable owners. */
static void it_rejects_actual_concurrent_inspection_and_execution(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaDebugSession session = {0};
    assert(ama_debug_begin(&registry,identity,NULL,NULL,0,&session).code == AMA_OK);
    thrd_t thread;
    assert(thrd_create(&thread,competing_inspector,&identity) == thrd_success);
    int result = -1;
    assert(thrd_join(thread,&result) == thrd_success && result == 0);
    assert(ama_debug_end(&session).code == AMA_OK);
    finish_contexts();
}

/** Run independent admission, state projection and native authority cases. */
static void run(void) {
    it_inspects_declared_state_under_exclusive_admission();
    it_checks_origin_and_explicit_preview_authority();
    it_rejects_invalid_admission_without_publishing_a_session();
    it_separates_busy_metadata_from_native_read_admission();
    it_never_reads_a_protected_opaque_native_payload();
    it_preserves_terminal_status_without_reconstructing_state();
    it_validates_catalogs_and_native_preview_transport();
    it_rejects_actual_concurrent_inspection_and_execution();
}
/** Return success after all explicit inspection and teardown assertions pass. */
int main(void) { run(); return 0; }
