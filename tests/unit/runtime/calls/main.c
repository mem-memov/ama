#include "common.c"

static void it_constructs_literals_once_per_call_and_initializes_results_after_parameters(void) {
    setup_call();
    for (size_t iteration = 0; iteration < 3; iteration++) {
        bool refused = false;
        AmaStatus status = ama_program_acquire(&program, caller, &invocation, &refused);
        assert(status.code == AMA_OK && !refused);
        size_t before = constructed;
        status = ama_program_prepare(&program, caller, &invocation);
        assert(status.code == AMA_OK && constructed == before + 2);
        RootValue *value = object_payload(target);
        assert(((LeafValue *)object_payload(value->left))->native.number == 73);
        assert(((LeafValue *)object_payload(value->right))->native.number == 73);
        status = ama_program_release(&program, caller);
        assert(status.code == AMA_OK);
    }
    finish_call();
}

static void it_protects_supplied_values_while_defaults_observe_new_parameters(void) {
    setup_call();
    supplied.source = default_source;
    RootValue *source = object_payload(caller);
    ((LeafValue *)object_payload(source->right))->native.number = 95;
    bool refused = false;
    AmaStatus status = ama_program_acquire(&program, caller, &invocation, &refused);
    assert(status.code == AMA_OK);
    status = ama_program_prepare(&program, caller, &invocation);
    assert(status.code == AMA_OK);
    RootValue *value = object_payload(target);
    assert(((LeafValue *)object_payload(value->left))->native.number == 95);
    assert(((LeafValue *)object_payload(value->right))->native.number == 95);
    finish_call();
}

static void it_releases_acquisition_after_constructor_failure_and_preserves_previous_value(void) {
    setup_call();
    bool refused = false;
    AmaStatus status = ama_program_acquire(&program, caller, &invocation, &refused);
    assert(status.code == AMA_OK);
    failed_constructor = constructed + 1;
    RootValue *value = object_payload(target);
    void *previous = object_payload(value->left);
    status = ama_program_prepare(&program, caller, &invocation);
    assert(status.code == AMA_INVALID_LITERAL && object_payload(value->left) == previous);
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, target, &cell);
    assert(status.code == AMA_OK && cell->metadata.service.activity == AMA_INSTANCE_READY);
    finish_call();
}

static void it_refuses_a_busy_input_before_any_constructor(void) {
    setup_call();
    RootValue *value = object_payload(caller);
    AmaPoolCell *source = NULL;
    AmaStatus status = ama_program_cell(&program, value->right, &source);
    assert(status.code == AMA_OK);
    source->metadata.service.activity = AMA_INSTANCE_RUNNING;
    supplied.source = default_source;
    bool refused = false;
    status = ama_program_acquire(&program, caller, &invocation, &refused);
    assert(status.code == AMA_OK);
    size_t before = constructed;
    status = ama_program_prepare(&program, caller, &invocation);
    assert(status.code == AMA_BUSY && constructed == before);
    finish_call();
}

static void it_releases_all_leases_after_each_preparation_allocation_failure(void) {
    size_t count = 0;
    for (size_t failure = 0; failure <= count; failure++) {
        setup_call();
        bool refused = false;
        AmaStatus status = ama_program_acquire(&program, caller, &invocation, &refused);
        assert(status.code == AMA_OK);
        size_t start = allocation_count;
        if (failure != 0) { failure_at = start + failure; }
        status = ama_program_prepare(&program, caller, &invocation);
        if (failure == 0) { assert(status.code == AMA_OK); count = allocation_count - start; }
        else {
            assert(status.code == AMA_OUT_OF_MEMORY);
            AmaPoolCell *cell = NULL;
            AmaStatus found = ama_program_cell(&program, caller, &cell);
            assert(found.code == AMA_OK && cell->metadata.service.outgoing.generation == 0);
        }
        failure_at = 0;
        finish_call();
    }
}

static void it_keeps_earlier_committed_parameters_when_a_later_constructor_fails(void) {
    setup_call();
    root_fields[1].role=AMA_PROGRAM_PARAMETER;
    RootValue *value=object_payload(target);
    void *old_right=object_payload(value->right);
    failed_constructor=constructed+2;
    bool refused=false;
    AmaStatus status=ama_program_acquire(&program,caller,&invocation,&refused);
    assert(status.code==AMA_OK);
    status=ama_program_prepare(&program,caller,&invocation);
    assert(status.code==AMA_INVALID_LITERAL);
    assert(((LeafValue *)object_payload(value->left))->native.number==73);
    assert(object_payload(value->right)==old_right);
    finish_call();
}

static void it_skips_interface_preparation_for_reads_and_resume_acquisitions(void) {
    setup_call();
    RootValue *value = object_payload(target);
    NativeValue *previous = &((LeafValue *)object_payload(value->left))->native;
    void *resource = previous->resource;
    size_t before = constructed;
    invocation.mode = AMA_ACCESS_READ;
    bool refused = false;
    AmaStatus status = ama_program_acquire(&program, caller, &invocation, &refused);
    assert(status.code == AMA_OK);
    status = ama_program_prepare(&program, caller, &invocation);
    assert(status.code == AMA_OK && constructed == before && previous->resource == resource);
    status = ama_program_release(&program, caller);
    assert(status.code == AMA_OK);
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, target, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_YIELDED;
    cell->metadata.service.resume = (AmaResumePosition){.valid = true, .selector = 42};
    invocation.mode = AMA_ACCESS_RESUME;
    status = ama_program_acquire(&program, caller, &invocation, &refused);
    assert(status.code == AMA_OK && !refused);
    status = ama_program_prepare(&program, caller, &invocation);
    assert(status.code == AMA_OK && constructed == before && previous->resource == resource);
    status = ama_program_release(&program, caller);
    assert(status.code == AMA_OK && cell->metadata.service.resume.selector == 42);
    assert(cell->metadata.service.activity == AMA_INSTANCE_YIELDED);
    finish_call();
}

void run(void) {
    it_constructs_literals_once_per_call_and_initializes_results_after_parameters();
    it_protects_supplied_values_while_defaults_observe_new_parameters();
    it_releases_acquisition_after_constructor_failure_and_preserves_previous_value();
    it_refuses_a_busy_input_before_any_constructor();
    it_releases_all_leases_after_each_preparation_allocation_failure();
    it_keeps_earlier_committed_parameters_when_a_later_constructor_fails();
    it_skips_interface_preparation_for_reads_and_resume_acquisitions();
}
int main(void) { run(); return EXIT_SUCCESS; }
