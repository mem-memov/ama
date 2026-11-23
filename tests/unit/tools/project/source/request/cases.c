/** @file
 * @brief Verify exact command/mode/policy matrices and explicit root/exclusion provenance.
 * @see [T277 entry review](../../../../../../docs/implementation/source-operations.md#t277-entry-review-after-accepted-t276)
 */
/** All operation/mode/dependency/replacement combinations are determined by
 * fixed command intent, independently of filesystem names or their existence. */
static void it_recognizes_explicit_source_operation_modes_and_policies_without_effects(void) {
    setup_test(); const char *verbs[] = {"copy", "delete", "move"};
    SourceOperationKind kinds[] = {SOURCE_OPERATION_COPY, SOURCE_OPERATION_DELETE, SOURCE_OPERATION_MOVE};
    for (size_t verb = 0; verb < 3; verb++) {
        for (unsigned apply = 0; apply < 2; apply++) {
            for (unsigned policy = 0; policy < 3; policy++) {
                for (unsigned replace = 0; replace < 2; replace++) {
                    const char *arguments[12] = {"ama-project", verbs[verb], "source with spaces; $(literal)"};
                    size_t count = 3;
                    if (verb != 1) { arguments[count++] = "destination\nwith a literal newline"; }
                    arguments[count++] = apply ? "--apply" : "--preview";
                    arguments[count++] = "--root"; arguments[count++] = "project directory";
                    if (policy) { arguments[count++] = policy == 1 ? "--extend-package" : "--copy-dependencies"; }
                    if (replace) { arguments[count++] = "--replace"; }
                    ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
                    ProjectSourceRequestStatus expected = verb == 1 && (policy || replace) ? PROJECT_SOURCE_REQUEST_SCOPE : PROJECT_SOURCE_REQUEST_OK;
                    assert(project_source_request_parse(&request, (int)count, arguments) == expected && request.valid == (expected == PROJECT_SOURCE_REQUEST_OK));
                    if (request.valid) {
                        assert(request.kind == kinds[verb] && request.source == arguments[2] && request.replace == (replace != 0));
                        assert(request.mode == (apply ? PROJECT_SOURCE_APPLY : PROJECT_SOURCE_PREVIEW));
                        assert(request.dependencies == (policy == 1 ? SOURCE_COPY_EXTEND_PACKAGE : policy == 2 ? SOURCE_COPY_CLONE_DEPENDENCIES : SOURCE_COPY_KEEP_EXTERNAL));
                        assert(!strcmp(request.compiler, "ama") && request.root_count == 1 && !request.destination_root_count);
                        assert(request.roots[0].value.kind == MODULE_ROOT_PROJECT && !request.roots[0].value.excluded_count);
                        assert(request.arguments == arguments && request.argument_count == count);
                        assert(verb == 1 ? request.destination == NULL : request.destination == arguments[3]);
                        size_t calls = allocation_count;
                        assert(project_source_request_parse(&request, 0, NULL) == PROJECT_SOURCE_REQUEST_INVALID && request.valid && allocation_count == calls);
                    }
                    project_source_request_destroy(&request); assert(!live_allocations);
                }
            }
        }
    }
    destroy_test();
}
/** Forward exclusions refer to exact roots and remain independent for O/F lists;
 * none of these physical names is normalized or interpreted as a module name. */
static void it_preserves_root_families_exact_argument_indices_and_order_independent_exclusions(void) {
    setup_test();
    const char *arguments[] = {"ama-project", "copy", "source", "destination", "--preview",
        "--exclude", "project", "output with spaces", "--destination-exclude", "project", "cache/output",
        "--root", "project", "--standard-root", "not named stdlib", "--root", "./project",
        "--exclude", "project", "notes.ini", "--destination-root", "project",
        "--destination-standard-root", "another standard", "--compiler", "compiler with literal;characters"};
    ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
    assert(project_source_request_parse(&request, (int)(sizeof(arguments)/sizeof(*arguments)), arguments) == PROJECT_SOURCE_REQUEST_OK);
    assert(request.root_count == 3 && request.destination_root_count == 2 && request.compiler == arguments[25]);
    assert(request.roots[0].argument == 12 && request.roots[0].value.path == arguments[12]);
    assert(request.roots[1].argument == 14 && request.roots[1].value.kind == MODULE_ROOT_STANDARD);
    assert(request.roots[2].argument == 16 && request.roots[2].value.path == arguments[16]);
    assert(request.roots[0].value.excluded_count == 2 && request.roots[0].value.excluded[0] == arguments[7]
        && request.roots[0].value.excluded[1] == arguments[19]);
    assert(!request.roots[1].value.excluded_count && !request.roots[2].value.excluded_count);
    assert(request.destination_roots[0].value.excluded_count == 1 && request.destination_roots[0].value.excluded[0] == arguments[10]);
    assert(request.destination_roots[1].value.kind == MODULE_ROOT_STANDARD && request.destination_roots[1].argument == 23);
    project_source_request_destroy(&request); destroy_test();
}
