/** @file
 * @brief Fail every complete current-source namespace/declaration allocation before any filesystem publication.
 */
/** Provider planning and later artifact construction retain one safely destructible allocation prefix, including empty/failed provider tables. */
static void it_unwinds_every_current_source_provider_and_scaffold_allocation(void) {
    prepare_providers();
    begin_contract("/test/fixture/v1.0.0/src/main", false);
    size_t before = allocation_count;
    bool built = build_contract(false, false);
    assert(built);
    size_t count = allocation_count - before;
    end_contract();
    bool provider_failure = false, emitted_failure = false;
    for (size_t failure = 1; failure <= count; failure++) {
        begin_contract("/test/fixture/v1.0.0/src/main", false);
        failure_at = allocation_count + failure;
        built = build_contract(false, false);
        assert(!built && !scaffold.ready && !scaffold.published && scaffold.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        provider_failure |= scaffold.providers.attempted && !scaffold.providers.ready;
        emitted_failure |= scaffold.providers.ready;
        end_contract();
    }
    assert(provider_failure && emitted_failure);
}

/** The newly emitted error macro's SDK dependency must exist before generation, rather than failing only after unusable headers were published. */
static void it_requires_the_library_status_sdk_header_before_model_or_artifact_emission(void) {
    prepare_providers();
    directory("generated/partial-sdk");
    directory("generated/partial-sdk/native");
    directory("generated/partial-sdk/native/status");
    file("generated/partial-sdk/native/status/status.h", "/* Only this initial required path exists. */\n");
    begin_contract("/test/fixture/v1.0.0/src/main", false);
    bool built = native_scaffold_build(&scaffold, (ScaffoldOptions){.directory = "generated/partial-sdk-output", .sdk = "generated/partial-sdk"});
    assert(!built && scaffold.error.code == DIAGNOSTIC_INVALID_PATH);
    assert(strstr(scaffold.error.primary.path, "native/status/library.h") != NULL);
    assert(scaffold.types == NULL && scaffold.first == NULL && !scaffold.providers.attempted);
    end_contract();
}
