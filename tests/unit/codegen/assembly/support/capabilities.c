/** @file
 * @brief Require complete static IR but reject missing execution adapters before allocating artifact/model state.
 */
/** Foreach cursor frames remain statically checkable before their runtime adapter. */
static void it_rejects_unimplemented_execution_adapters_after_valid_ir_before_artifact_staging(void) {
    const char *cases[] = {
        "@!\nitems[]\n.value\nitems[item&]\n{\n}\n"
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        prepare_assembly();
        file("generated/project/main", cases[index]);
        begin_assembly();
        assert(frontend.valid && frontend.plan.valid && frontend.plan.storage_ready);
        size_t before = allocation_count;
        bool built = build_assembly();
        assert(!built && !assembly.ready && !assembly.published && assembly.first == NULL);
        assert(assembly.types == NULL && assembly.selected == NULL && allocation_count == before);
        assert(diagnostic_valid(&assembly.error) && assembly.error.code == DIAGNOSTIC_UNSUPPORTED_FEATURE);
        assert(source_span_valid(assembly.error.primary.span));
        assert(assembly.error.primary.span.source == frontend.project.entry->source->text);
        end_assembly();
    }
}
