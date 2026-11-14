/** @file
 * @brief Own a nine-node initialized tree, an unrelated busy referent and exact preflight service snapshots.
 */
static AmaHandle protected_nodes[9], unrelated;
static AmaServiceState previous_services[9];
/** Resolve stable metadata only while the test retains the published object. */
static AmaPoolCell *preflight_cell(AmaHandle handle) {
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_object_lookup(&world, handle, false, &type, &cell);
    assert(status.code == AMA_OK);
    return cell;
}
/** Compare all logical fields, avoiding C padding or allocation-address equality as an oracle. */
static void assert_service_snapshot(const AmaServiceState *actual, const AmaServiceState *expected) {
    assert(actual->activity == expected->activity);
    assert(actual->return_link.attached == expected->return_link.attached && actual->return_link.host == expected->return_link.host);
    assert(ama_handle_equal(actual->return_link.caller, expected->return_link.caller));
    assert(actual->return_link.continuation == expected->return_link.continuation && actual->return_link.call_site == expected->return_link.call_site);
    assert(actual->resume.valid == expected->resume.valid && actual->resume.selector == expected->resume.selector);
    assert(actual->current_operation == expected->current_operation);
    assert(actual->last_refusal.code == expected->last_refusal.code && actual->last_refusal.message == expected->last_refusal.message);
    assert(actual->last_refusal.message_length == expected->last_refusal.message_length);
    assert(ama_handle_equal(actual->outgoing, expected->outgoing) && actual->outgoing_previous == expected->outgoing_previous);
}
/** Mix Ready/Yielded owned states and a Running external reference target; ownership, not references, defines the protection boundary. */
static void setup_preflight(void) {
    setup_objects();
    AmaFieldRecipe fields[6] = {0};
    fields[2].count = 2;
    AmaRecipe recipe = {.fields = fields, .field_count = 6};
    AmaStatus status = ama_object_create(&world, 2, &recipe, &protected_nodes[0]);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(protected_nodes[0]);
    RootValue *first = object_payload((*ama_handle_list_element(&root->branches, 0)));
    RootValue *second = object_payload((*ama_handle_list_element(&root->branches, 1)));
    AmaHandle nodes[] = {protected_nodes[0], root->left, root->right, (*ama_handle_list_element(&root->branches, 0)),
        first->left, first->right, (*ama_handle_list_element(&root->branches, 1)), second->left, second->right};
    memcpy(protected_nodes, nodes, sizeof(nodes));
    status = ama_object_create(&world, 1, NULL, &unrelated);
    assert(status.code == AMA_OK && constructed == 10);
    root->selected = unrelated;
    preflight_cell(unrelated)->metadata.service.activity = AMA_INSTANCE_RUNNING;
    for (size_t index = 0; index < 9; index++) {
        AmaServiceState *service = &preflight_cell(protected_nodes[index])->metadata.service;
        service->current_operation = index + 20;
        service->last_refusal = ama_status_make(AMA_BUSY, "earlier refusal", 15);
        if (index % 2 == 0) {
            service->activity = AMA_INSTANCE_YIELDED;
            service->resume = (AmaResumePosition){true, index + 30};
        }
        previous_services[index] = *service;
    }
}
/** Verify complete restoration and no native/ownership effect, then finalize the two independent roots. */
static void finish_preflight(void) {
    assert(!world.failed && !world.operation_active && cloned == 0 && destroyed == 0 && constructed == 10);
    for (size_t index = 0; index < 9; index++) {
        assert_service_snapshot(&preflight_cell(protected_nodes[index])->metadata.service, &previous_services[index]);
    }
    assert(preflight_cell(unrelated)->metadata.service.activity == AMA_INSTANCE_RUNNING);
    preflight_cell(unrelated)->metadata.service.activity = AMA_INSTANCE_READY;
    AmaStatus status = ama_object_destroy(&world, protected_nodes[0]);
    assert(status.code == AMA_OK && destroyed == 9);
    status = ama_object_destroy(&world, unrelated);
    assert(status.code == AMA_OK && destroyed == 10);
    destroy_objects();
}
/** Initialize one independently owned protection arena; the enclosing test supplies the guarded synchronous operation lifetime. */
static void start_protection(AmaObjectOperation *operation, const AmaObjectProtection *authority) {
    *operation = (AmaObjectOperation){.world = &world, .source_authority = authority};
    source_arena_init(&operation->arena, world.allocator);
}
/** Restore only owned protections before destroying their borrowed-record arena. */
static void end_protection(AmaObjectOperation *operation) {
    ama_object_unprotect(operation);
    source_arena_destroy(&operation->arena);
}
