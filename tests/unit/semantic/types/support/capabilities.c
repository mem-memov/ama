/** @file
 * @brief Verify copy diagnostics against declared ownership edges and native contracts.
 */
/** Scalar/list ownership reports its first declared field and the exact native ABI
 * seed, even through another owned module. References are deliberately excluded.
 */
static void it_retains_the_owned_copy_witness_and_native_seed(void) {
    const char *fields[] = {"resource\n.resource", "resources[]\n.resource", "child\n.child"};
    for (size_t shape = 0; shape < sizeof(fields) / sizeof(*fields); shape++) {
        prepare_types();
        file("generated/program/child", "@!\nresource\n.resource\n");
        char wrapper[256];
        int length = snprintf(wrapper, sizeof(wrapper), "@!\n%s\n", fields[shape]);
        assert(length > 0 && (size_t)length < sizeof(wrapper));
        file("generated/program/wrapper", wrapper);
        SemanticStatus status = resolve_text("@!\nsource\n.wrapper\ncopy\n.wrapper\n % source\n");
        assert(status == SEMANTIC_INVALID && diagnostic_valid(&semantic.error));
        assert(semantic.error.related_count == 3);
        assert(strcmp(semantic.error.subject, ".wrapper") == 0);
        assert(layout_text_is(semantic.error.related[0].location.span, "copy\n.wrapper\n % source"));
        assert(layout_text_is(semantic.error.related[1].location.span, fields[shape]));
        assert(strcmp(semantic.error.related[1].role, "owned field requiring independent copying") == 0);
        assert(strcmp(semantic.error.related[1].name, ".wrapper") == 0);
        assert(strcmp(semantic.error.related[2].name, ".resource") == 0);
        assert(strstr(source_path(semantic.error.related[2].location.span.source), "resource.abi") != NULL);
        assert(semantic.error.related[2].location.span.start == source_length(semantic.error.related[2].location.span.source));
        assert(strstr(semantic.error.cause, "no declared =") != NULL);
        assert(strstr(semantic.error.help, "references do not copy") != NULL);
        destroy_semantic();
    }
    prepare_types();
    SemanticStatus status = resolve_text("@!\nsource\n.resource\ncopy\n.resource\n source\n");
    assert(status == SEMANTIC_INVALID && semantic.error.related_count == 2);
    assert(strcmp(semantic.error.related[1].name, ".resource") == 0);
    destroy_semantic();
    file("generated/program/wrapper", "@!\nresource&\n.resource\n");
    expect_resolved("@!\nsource\n.wrapper\ncopy\n.wrapper\n % source\n", true);
}
