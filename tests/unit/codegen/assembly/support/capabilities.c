/** @file
 * @brief Require artifact generation for complete checked foreach control graphs.
 */
/** Empty and nonempty foreach bodies both have complete executable adapters. */
static void it_builds_complete_traversal_adapters_after_valid_ir(void) {
    const char *cases[] = {
        "@!\nitems[]\n.value\nitems[item&]\n{\n}\n",
        "@!\nitems[]\n.value\nitems[item&]\n{\n item& !\n}\n"
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        prepare_assembly();
        file("generated/project/main", cases[index]);
        begin_assembly();
        assert(frontend.valid && frontend.plan.valid && frontend.plan.storage_ready);
        bool built = build_assembly();
        assert(built && assembly.ready && !assembly.published);
        assert(assembly.first != NULL && assembly.types != NULL);
        end_assembly();
    }
}
