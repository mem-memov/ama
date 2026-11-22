/** @file
 * @brief Verify exact module-as-data promotion, cycles, dependency-policy separation and allocation refusal.
 * @see [T224 closure contract](../../../../../../../docs/implementation/source-operations.md)
 */
static unsigned invalid_declared_module;
/** Certify explicit data-main membership using real catalog handles and captures;
 * no outgoing semantic edge is used to infer this separate requirement.
 */
static AcquisitionClosureStatus expand_declared_modules(void *context, const char *identity,
    AcquisitionExpansion *output, SourceIssue *issue) {
    AcquisitionClosureStatus status = expand(context, identity, output, issue);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    bool child = !strcmp(identity, child_identity), parent = !strcmp(identity, parent_identity);
    if (!child && !parent) { return status; }
    const char *target = child ? parent_identity : child_identity;
    const ModuleSource *source = module_source(target);
    AcquisitionDeclaredInputs *input = source_arena_allocate(&facts, sizeof(*input)); assert(input);
    *input = (AcquisitionDeclaredInputs){.source = origin};
    assert(capture_unit(source, ACQUISITION_ACTIVE, &input->files) == SOURCE_OK);
    output->declared_inputs = input;
    AcquisitionDeclaredModule *first = source_arena_allocate(&facts, sizeof(*first)); assert(first);
    AcquisitionDeclaredModule *repeat = source_arena_allocate(&facts, sizeof(*repeat)); assert(repeat);
    *repeat = (AcquisitionDeclaredModule){.identity = target};
    *first = (AcquisitionDeclaredModule){.next = repeat, .identity = target};
    if (child && invalid_declared_module) {
        if (invalid_declared_module == 1) { first->identity = ""; }
        if (invalid_declared_module == 2) { first->identity = "/test/fixture/v1.0.0"; }
        if (invalid_declared_module == 3) { first->identity = NULL; }
        if (invalid_declared_module == 4) { first->origin = (SourceSpan){source->text, source_length(source->text) + 1, source_length(source->text) + 1}; }
        if (invalid_declared_module == 5) { first->origin = (SourceSpan){NULL, 1, 1}; }
    }
    output->declared_modules = first;
    return ACQUISITION_CLOSURE_OK;
}
/** Structural context promotes for declared data even when every incoming semantic edge is skipped. */
static void it_promotes_declared_module_cycles_once_independently_of_semantic_dependency_policy(void) {
    prepare(true, false, false, false); invalid_declared_module = 0; dependency_choices = 0;
    AcquisitionClosureProvider provider = {.expand = expand_declared_modules, .admit_test = admit,
        .follow_dependency = keep_parent_external};
    assert(acquisition_closure_compute(&closure, &child_identity, 1, &provider) == ACQUISITION_CLOSURE_OK);
    assert(closure.valid && expansions == 5 && admissions == 2 && dependency_choices);
    assert(retained("src/parent+/extra.data") && retained("tests/parent/check") && retained("tests/parent/helper"));
    const AcquisitionRoleRecord *parent = closure.roles.records;
    while (parent && strcmp(parent->identity, parent_identity)) { parent = parent->next; }
    assert(parent && parent->role == ACQUISITION_ACTIVE && parent->expanded);
    size_t declared = 0, topology = 0;
    for (const AcquisitionReason *reason = parent->reasons; reason; reason = reason->next) {
        assert(reason->kind != ACQUISITION_DEPENDENCY);
        if (reason->kind == ACQUISITION_DECLARED_MODULE) { declared++; }
        if (reason->kind == ACQUISITION_TOPOLOGY) { topology++; }
    }
    assert(declared == 1 && topology == 1 && !acquisition_roles_take(&closure.roles));
    finish();
}
/** Invalid exact identities/provenance cannot publish an active promotion or a valid closure. */
static void it_refuses_invalid_declared_module_identities_and_provenance_without_promotion(void) {
    for (unsigned invalid = 1; invalid <= 5; invalid++) {
        prepare(true, false, false, false); invalid_declared_module = invalid;
        AcquisitionClosureProvider provider = {.expand = expand_declared_modules, .admit_test = admit,
            .follow_dependency = keep_parent_external};
        assert(acquisition_closure_compute(&closure, &child_identity, 1, &provider) == ACQUISITION_CLOSURE_INVALID);
        assert(!closure.valid && expansions == 1 && admissions == 0);
        const AcquisitionRoleRecord *parent = closure.roles.records;
        while (parent && strcmp(parent->identity, parent_identity)) { parent = parent->next; }
        assert(parent && parent->role == ACQUISITION_CONTEXT && !parent->expanded);
        finish();
    }
}
/** The independent healthy provider certifies real facts while every controller allocation is faulted. */
static void it_retires_every_controller_allocation_in_declared_data_promotion_and_cycles(void) {
    size_t count = 0;
    for (size_t failure = 0; !failure || failure <= count; failure++) {
        prepare(true, false, false, false); invalid_declared_module = 0;
        acquisition_closure_destroy(&closure);
        PlanFaults faults = {.failure = failure};
        SourceAllocator allocator = {&faults, plan_allocate, plan_release};
        acquisition_closure_init(&closure, allocator);
        AcquisitionClosureProvider provider = {.expand = expand_declared_modules, .admit_test = admit,
            .follow_dependency = keep_parent_external};
        AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &child_identity, 1, &provider);
        if (!failure) { assert(status == ACQUISITION_CLOSURE_OK && closure.valid); count = faults.attempts; assert(count); }
        else { assert(status == ACQUISITION_CLOSURE_OUT_OF_MEMORY && !closure.valid); }
        finish(); assert(!faults.live);
    }
}
