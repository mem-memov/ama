/**
 * @file
 * @brief Verify call preparation phases without constructing a compiler or project.
 */
#include "../calls/common.c"
#include "runtime/program/calls/preparation/internal.h"

static AmaObjectOperation protection;
static size_t index_count, index_order[2];

/** Retain scratch and source protections until every selected installation ends. */
static void begin_preparation(void) {
    protection = (AmaObjectOperation){.world = &world};
    source_arena_init(&protection.arena, world.allocator);
    world.operation_active = true;
}

/** Source-state restoration belongs to this outer owner, not a field replacement. */
static void end_preparation(void) {
    ama_object_unprotect(&protection);
    source_arena_destroy(&protection.arena);
    world.operation_active = false;
}

/** Record actual index protocol order and reject any unexpected reevaluation. */
static AmaStatus read_index(const void *payload, size_t *index) {
    const NativeValue *value = payload;
    assert(index_count < 2 && value->number < 2);
    index_order[index_count++] = value->number;
    *index = value->number;
    return ama_status_success();
}

/** Index decoding follows binding text order once, before any source lease exists. */
static void it_selects_indexed_sources_once_in_text_order_before_acquisition(void) {
    setup_call();
    RootValue *value = object_payload(caller);
    ((LeafValue *)object_payload(value->left))->native.number = 0;
    ((LeafValue *)object_payload(value->right))->native.number = 1;
    for (size_t index = 0; index < 2; index++) {
        AmaMembership identity;
        AmaStatus inserted = ama_membership_insert(&value->references, index, (AmaHandle){0}, &identity);
        assert(inserted.code == AMA_OK);
    }
    (*ama_handle_list_element(&value->references, 0)) = value->left;
    (*ama_handle_list_element(&value->references, 1)) = value->right;
    program_types[0].index = read_index;
    const AmaProgramAccess first = {.field = 0}, second = {.field = 1};
    const AmaProgramAccess one = {.field = 5, .index = &second};
    const AmaProgramAccess zero = {.field = 5, .index = &first};
    AmaProgramInput bindings[] = {
        {.field = 1, .operation = 41, .source = {.kind = AMA_SOURCE_LOCAL, .access = &one, .type_id = 1}},
        {.field = 0, .operation = 42, .source = {.kind = AMA_SOURCE_LOCAL, .access = &zero, .type_id = 1}}
    };
    invocation.inputs = bindings;
    invocation.input_count = 2;
    AmaPreparedInput inputs[2] = {0};
    index_count = 0;
    begin_preparation();
    size_t before = constructed;
    AmaStatus status = ama_call_select_sources(&program, caller, &invocation, inputs);
    assert(status.code == AMA_OK && index_count == 2 && index_order[0] == 1 && index_order[1] == 0);
    assert(ama_handle_equal(inputs[0].value.handle, value->right));
    assert(ama_handle_equal(inputs[1].value.handle, value->left));
    assert(protection.protected_first == NULL && constructed == before);
    AmaPoolCell *context = NULL;
    status = ama_program_cell(&program, caller, &context);
    assert(status.code == AMA_OK && context->metadata.service.current_operation == 42);
    end_preparation();
    finish_call();
}

/** Repeated owned sources share authority, but each destination gets its own clone. */
static void it_keeps_repeated_sources_protected_across_distinct_field_commits(void) {
    setup_call();
    AmaProgramInput bindings[] = {{.field = 0, .source = default_source}, {.field = 1, .source = default_source}};
    invocation.inputs = bindings;
    invocation.input_count = 2;
    RootValue *original = object_payload(caller);
    ((LeafValue *)object_payload(original->right))->native.number = 95;
    bool refused;
    AmaStatus status = ama_program_acquire(&program, caller, &invocation, &refused);
    assert(status.code == AMA_OK && !refused);
    AmaPreparedInput inputs[2] = {0};
    begin_preparation();
    status = ama_call_select_sources(&program, caller, &invocation, inputs);
    assert(status.code == AMA_OK);
    status = ama_call_protect_sources(&protection, inputs, 2);
    assert(status.code == AMA_OK && protection.protected_first != NULL && protection.protected_first->next == NULL);
    AmaPoolCell *source = protection.protected_first->cell;
    assert(source->metadata.service.activity == AMA_INSTANCE_ACQUIRED);
    status = ama_call_check_fields(&program, target, &program_types[1], inputs, 2);
    assert(status.code == AMA_OK && source->metadata.service.activity == AMA_INSTANCE_ACQUIRED);
    size_t before = cloned;
    uint64_t current_operation = 0;
    status = ama_call_install_fields(&program, target, &program_types[1], &invocation, inputs,
        protection.protected_first, &current_operation);
    assert(status.code == AMA_OK && cloned == before + 2 && source->metadata.service.activity == AMA_INSTANCE_ACQUIRED);
    RootValue *result = object_payload(target);
    NativeValue *left = &((LeafValue *)object_payload(result->left))->native;
    NativeValue *right = &((LeafValue *)object_payload(result->right))->native;
    assert(left->number == 95 && right->number == 95 && left->resource != right->resource);
    end_preparation();
    assert(source->metadata.service.activity == AMA_INSTANCE_READY);
    status = ama_program_release(&program, caller);
    assert(status.code == AMA_OK);
    finish_call();
}

/** Empty storage still conflicts with an identical reset or an owning ancestor reset. */
static void it_rejects_same_list_and_empty_nested_list_container_overlap_before_construction(void) {
    for (size_t nested = 0; nested < 2; nested++) {
        setup_call();
        if (nested) {
            AmaStatus status = ama_object_destroy(&world, target);
            assert(status.code == AMA_OK);
            AmaFieldRecipe fields[6] = {0};
            fields[2].count = 1;
            fields[2].supplied = true;
            AmaRecipe recipe = {.fields = fields, .field_count = 6};
            status = ama_object_create(&world, 2, &recipe, &target);
            assert(status.code == AMA_OK);
            ((RootValue *)object_payload(caller))->self = target;
        }
        root_fields[2].role = AMA_PROGRAM_PARAMETER;
        RootValue *value = object_payload(target);
        AmaHandle container = nested ? (*ama_handle_list_element(&value->branches, 0)) : target;
        RootValue *owner = object_payload(container);
        AmaPreparedInput input = {.input = &supplied,
            .value = {.form = AMA_FIELD_OWNING_LIST, .type_id = 2, .container = container, .list = &owner->branches}};
        supplied.source = (AmaProgramSource){.kind = AMA_SOURCE_LOCAL, .type_id = 2};
        assert(input.value.list->count == 0);
        begin_preparation();
        size_t before = constructed;
        AmaStatus status = ama_call_check_fields(&program, target, &program_types[1], &input, 1);
        assert(status.code == AMA_BUSY && constructed == before);
        assert(leaf_type.pool.live_count != 0 && !world.failed);
        end_preparation();
        finish_call();
    }
}

/** Defaults observe committed parameters; a later failure keeps the earlier commit. */
static void it_installs_one_constructor_per_field_and_retains_the_successful_prefix(void) {
    for (size_t failure = 0; failure < 2; failure++) {
        setup_call();
        uint64_t operations[6] = {11, 22};
        invocation.installation_operations = operations;
        bool refused;
        AmaStatus status = ama_program_acquire(&program, caller, &invocation, &refused);
        assert(status.code == AMA_OK && !refused);
        RootValue *value = object_payload(target);
        void *old_right = ((LeafValue *)object_payload(value->right))->native.resource;
        void *body_resource = value->native.resource;
        size_t before = constructed;
        if (failure) { failed_constructor = before + 2; }
        AmaPreparedInput input = {.input = &supplied};
        begin_preparation();
        uint64_t operation = 0;
        status = ama_call_install_fields(&program, target, &program_types[1], &invocation, &input, NULL, &operation);
        assert(status.code == (failure ? AMA_INVALID_LITERAL : AMA_OK) && constructed == before + 2 && operation == 22);
        assert(((LeafValue *)object_payload(value->left))->native.number == 73 && value->native.resource == body_resource);
        if (failure) { assert(((LeafValue *)object_payload(value->right))->native.resource == old_right); }
        else { assert(((LeafValue *)object_payload(value->right))->native.number == 73); }
        end_preparation();
        status = ama_program_release(&program, caller);
        assert(status.code == AMA_OK);
        finish_call();
    }
}

/** Input installation preserves a selected snapshot, while ordinary inputs and ordinary result defaults clear positions. */
static void it_preserves_explicit_continuation_inputs_without_preserving_ordinary_defaults(void) {
    for (size_t continuation = 0; continuation < 2; continuation++) {
        setup_call();
        RootValue *original = object_payload(caller);
        AmaPoolCell *source = NULL;
        AmaStatus status = ama_program_cell(&program, original->right, &source);
        assert(status.code == AMA_OK);
        source->metadata.service.activity = AMA_INSTANCE_YIELDED;
        source->metadata.service.resume = (AmaResumePosition){true, 57};
        supplied = (AmaProgramInput){.field = 0, .source = default_source};
        supplied.source.continuation = continuation != 0;
        bool refused = true;
        status = ama_program_acquire(&program, caller, &invocation, &refused);
        assert(status.code == AMA_OK && !refused);
        status = ama_program_prepare(&program, caller, &invocation);
        assert(status.code == AMA_OK);
        RootValue *result = object_payload(target);
        AmaPoolCell *parameter = NULL, *ordinary_result = NULL;
        status = ama_program_cell(&program, result->left, &parameter);
        assert(status.code == AMA_OK);
        status = ama_program_cell(&program, result->right, &ordinary_result);
        assert(status.code == AMA_OK);
        assert(parameter->metadata.service.resume.valid == (continuation != 0));
        assert(parameter->metadata.service.resume.selector == (continuation ? 57 : 0));
        assert(!ordinary_result->metadata.service.resume.valid);
        assert(!parameter->metadata.service.return_link.attached);
        assert(source->metadata.service.activity == AMA_INSTANCE_YIELDED && source->metadata.service.resume.selector == 57);
        status = ama_program_release(&program, caller);
        assert(status.code == AMA_OK);
        finish_call();
    }
}

/** Prefix % on a reference protects and snapshots its target rather than copying a reference handle into an owned parameter. */
static void it_installs_a_referenced_continuation_as_an_owned_input_snapshot(void) {
    setup_call();
    RootValue *original = object_payload(caller);
    original->selected = original->right;
    AmaPoolCell *source = NULL;
    AmaStatus status = ama_program_cell(&program, original->right, &source);
    assert(status.code == AMA_OK);
    source->metadata.service.activity = AMA_INSTANCE_YIELDED;
    source->metadata.service.resume = (AmaResumePosition){true, 91};
    const AmaProgramAccess reference = {.field = 3};
    supplied = (AmaProgramInput){.field = 0, .source = {.kind = AMA_SOURCE_LOCAL,
        .access = &reference, .type_id = 1, .continuation = true}};
    bool refused = true;
    status = ama_program_acquire(&program, caller, &invocation, &refused);
    assert(status.code == AMA_OK && !refused);
    status = ama_program_prepare(&program, caller, &invocation);
    assert(status.code == AMA_OK);
    RootValue *result = object_payload(target);
    AmaPoolCell *parameter = NULL;
    status = ama_program_cell(&program, result->left, &parameter);
    assert(status.code == AMA_OK && parameter->metadata.service.resume.valid && parameter->metadata.service.resume.selector == 91);
    assert(!ama_handle_equal(result->left, original->right) && ama_handle_equal(original->selected, original->right));
    assert(source->metadata.service.activity == AMA_INSTANCE_YIELDED && source->metadata.service.resume.selector == 91);
    status = ama_program_release(&program, caller);
    assert(status.code == AMA_OK);
    finish_call();
}

/** Count exact control validation, independently of any native clone hook or replacement. */
static AmaStatus reject_snapshot_controls(AmaWorld *owner, AmaHandle source, const void *payload,
    const AmaServiceState *service, void *resource) {
    (void)owner; (void)source; (void)payload; (void)service;
    (*(size_t *)resource)++;
    return ama_status_make(AMA_INVALID_STATE, NULL, 0);
}

/** Repeated ordinary/snapshot sources must validate the stronger mode before committing even the first parameter. */
static void it_validates_snapshot_controls_before_any_input_field_replacement(void) {
    setup_call();
    size_t validated = 0;
    const AmaObjectControlHooks controls = {.validate_continuation = reject_snapshot_controls};
    leaf_type.controls = &controls;
    leaf_type.control_context = &validated;
    AmaProgramInput bindings[] = {{.field = 0, .source = default_source}, {.field = 1, .source = default_source}};
    bindings[1].source.continuation = true;
    root_fields[1].role = AMA_PROGRAM_PARAMETER;
    invocation.inputs = bindings;
    invocation.input_count = 2;
    RootValue *before = object_payload(target);
    void *old_left = ((LeafValue *)object_payload(before->left))->native.resource;
    void *old_right = ((LeafValue *)object_payload(before->right))->native.resource;
    size_t copies = cloned;
    bool refused = true;
    AmaStatus status = ama_program_acquire(&program, caller, &invocation, &refused);
    assert(status.code == AMA_OK && !refused);
    status = ama_program_prepare(&program, caller, &invocation);
    assert(status.code == AMA_INVALID_STATE && validated == 1 && cloned == copies);
    assert(((LeafValue *)object_payload(before->left))->native.resource == old_left);
    assert(((LeafValue *)object_payload(before->right))->native.resource == old_right);
    leaf_type.controls = NULL;
    finish_call();
}

/**
 * Execute every named assertion case selected by this leaf; the recursive harness owns process
 * status and success-only cleanup.
 */
#include "transport.c"

void run(void) {
    it_selects_indexed_sources_once_in_text_order_before_acquisition();
    it_keeps_repeated_sources_protected_across_distinct_field_commits();
    it_preserves_explicit_continuation_inputs_without_preserving_ordinary_defaults();
    it_validates_snapshot_controls_before_any_input_field_replacement();
    it_installs_a_referenced_continuation_as_an_owned_input_snapshot();
    it_rejects_same_list_and_empty_nested_list_container_overlap_before_construction();
    it_installs_one_constructor_per_field_and_retains_the_successful_prefix();
    it_constructs_prepared_transport_scalars_and_lists_without_default_overwrite();
    it_preserves_old_field_when_a_prepared_transport_decoder_fails();
}

/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return 0;
}
