/** @file
 * @brief Check full-key model reuse/order and every concrete project generation allocation before publication.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** Two captured owners have disjoint C view/member identities; equal-key fields share only their code/type descriptor, not instance slots. */
static void inspect_captured_model(NativeScaffold *model) {
    const ProjectModule *definition = project_find(&frontend.project, ".container");
    const ScaffoldType *previous = NULL;
    size_t count = 0;
    for (const ScaffoldType *type = model->types; type; type = type->next) {
        bool native = type->plan->type->module->syntax.sections.kind == MODULE_NATIVE;
        assert(type->native_prefix->category == (native ? IDENTITY_NATIVE_NAMESPACE : IDENTITY_TYPE));
        if (type->plan->type->module != definition) { continue; }
        if (previous) {
            const IdentityName *a = previous->native_prefix, *b = type->native_prefix;
            size_t length = a->encoding_length < b->encoding_length ? a->encoding_length : b->encoding_length;
            int order = memcmp(a->encoding, b->encoding, length);
            assert(order < 0 || (order == 0 && a->encoding_length < b->encoding_length));
            for (size_t role = 0; role < SCAFFOLD_ROLE_COUNT; role++) {
                assert(strcmp(previous->names[role]->internal_name, type->names[role]->internal_name) != 0);
            }
            assert(previous->members && type->members && previous->members->identity != type->members->identity);
        }
        previous = type;
        count++;
    }
    assert(count == 2);
    const StorageField *first = NULL, *again = NULL;
    for (const ScaffoldMember *member = model->selected->members; member; member = member->next) {
        SourceSpan name = member->field->symbol->name;
        const unsigned char *bytes = source_bytes(name.source) + name.start;
        if (name.end - name.start == 5 && memcmp(bytes, "first", 5) == 0) { first = member->field; }
        if (name.end - name.start == 5 && memcmp(bytes, "again", 5) == 0) { again = member->field; }
    }
    assert(first && again && first != again && first->slot != again->slot && first->type == again->type);
}

/** Build only in-memory artifacts against immutable admitted input; each refused allocation must preserve a destroyable owner and prevent ready publication. */
static void it_unwinds_every_captured_project_generation_allocation_before_publication(void) {
    prepare_captured_definitions();
    prepare_project(captured_program);
    native_scaffold_destroy(&assembly);
    size_t baseline = live_allocations, total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        native_scaffold_init(&assembly, &frontend, sources.allocator);
        ScaffoldOptions options = {.directory = "generated/output", .sdk = TEST_SDK};
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        bool built = project_codegen_build(&assembly, options);
        if (!cut) { assert(built && assembly.ready); total = allocation_count - before; assert(total); inspect_captured_model(&assembly); }
        else {
            assert(!built && !assembly.ready && assembly.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
            assert(diagnostic_valid(&assembly.error));
        }
        failure_at = 0;
        native_scaffold_destroy(&assembly);
        assert(live_allocations == baseline && frontend.semantic.constants_valid && frontend.plan.valid);
    }
    printf("captured project generation allocation refusal positions: %zu\n", total);
    destroy_frontend();
}
