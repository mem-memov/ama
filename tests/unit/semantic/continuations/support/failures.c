/** @file
 * @brief Fail every semantic allocation for successful and statically rejected positive reference-origin chains.
 */
/** Every proof record remains arena-owned if allocation or alias validation fails; success never publishes partial facts. */
static void it_unwinds_every_positive_reference_origin_allocation(void) {
    prepare_types();
    const char *texts[] = {
        "@!\nfirst\n/test/fixture/v1.0.0/src/value\nsecond\n/test/fixture/v1.0.0/src/value\nalias&\n & first\ncopy&\n alias&\nsecond\n % copy&\n",
        "@!\nfirst\n/test/fixture/v1.0.0/src/value\nalias&\n & first\ncopy&\n alias&\nfirst\n % copy&\n"
    };
    for (size_t shape = 0; shape < 2; shape++) {
        SemanticStatus status = resolve_text(texts[shape]);
        assert(status == (shape == 0 ? SEMANTIC_OK : SEMANTIC_INVALID));
        size_t count = allocation_count;
        destroy_semantic();
        for (size_t failure = 1; failure <= count; failure++) {
            file("generated/program/packages/test/fixture/v1.0.0/src/main", texts[shape]);
            setup_semantic();
            status = load_semantic("generated/program");
            assert(status == SEMANTIC_OK);
            status = semantic_native(&semantic);
            assert(status == SEMANTIC_OK);
            if (failure <= allocation_count) { destroy_semantic(); continue; }
            failure_at = failure;
            status = semantic_resolve(&semantic);
            assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.resolved && diagnostic_valid(&semantic.error));
            destroy_semantic();
        }
    }
}
