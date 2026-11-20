/** @file
 * @brief Verify copy diagnostics against declared ownership edges and native contracts.
 */
/** Scalar/list ownership reports its first declared field and the exact native ABI
 * seed, even through another owned module. References are deliberately excluded.
 */
static void it_retains_the_owned_copy_witness_and_native_seed(void) {
    const char *fields[] = {"resource\n/test/fixture/v1.0.0/src/resource", "resources[]\n/test/fixture/v1.0.0/src/resource", "child\n/test/fixture/v1.0.0/src/child"};
    for (size_t shape = 0; shape < sizeof(fields) / sizeof(*fields); shape++) {
        prepare_types();
        file("generated/program/packages/test/fixture/v1.0.0/src/child", "@!\nresource\n/test/fixture/v1.0.0/src/resource\n");
        char wrapper[256];
        int length = snprintf(wrapper, sizeof(wrapper), "@!\n%s\n", fields[shape]);
        assert(length > 0 && (size_t)length < sizeof(wrapper));
        file("generated/program/packages/test/fixture/v1.0.0/src/wrapper", wrapper);
        SemanticStatus status = resolve_text("@!\nsource\n/test/fixture/v1.0.0/src/wrapper\ncopy\n/test/fixture/v1.0.0/src/wrapper\n % source\n");
        assert(status == SEMANTIC_INVALID && diagnostic_valid(&semantic.error));
        assert(semantic.error.related_count == 3);
        assert(strcmp(semantic.error.subject, "/test/fixture/v1.0.0/src/wrapper") == 0);
        assert(layout_text_is(semantic.error.related[0].location.span, "copy\n/test/fixture/v1.0.0/src/wrapper\n % source"));
        assert(layout_text_is(semantic.error.related[1].location.span, fields[shape]));
        assert(strcmp(semantic.error.related[1].role, "owned field requiring independent copying") == 0);
        assert(strcmp(semantic.error.related[1].name, "/test/fixture/v1.0.0/src/wrapper") == 0);
        assert(strcmp(semantic.error.related[2].name, "/test/fixture/v1.0.0/src/resource") == 0);
        assert(strstr(source_path(semantic.error.related[2].location.span.source), "resource.abi") != NULL);
        assert(semantic.error.related[2].location.span.start == source_length(semantic.error.related[2].location.span.source));
        assert(strstr(semantic.error.cause, "no declared =") != NULL);
        assert(strstr(semantic.error.help, "references do not copy") != NULL);
        destroy_semantic();
    }
    prepare_types();
    SemanticStatus status = resolve_text("@!\nsource\n/test/fixture/v1.0.0/src/resource\ncopy\n/test/fixture/v1.0.0/src/resource\n source\n");
    assert(status == SEMANTIC_INVALID && semantic.error.related_count == 2);
    assert(strcmp(semantic.error.related[1].name, "/test/fixture/v1.0.0/src/resource") == 0);
    destroy_semantic();
    file("generated/program/packages/test/fixture/v1.0.0/src/wrapper", "@!\nresource&\n/test/fixture/v1.0.0/src/resource\n");
    expect_resolved("@!\nsource\n/test/fixture/v1.0.0/src/wrapper\ncopy\n/test/fixture/v1.0.0/src/wrapper\n % source\n", true);
}
