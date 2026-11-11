#include "../common.c"

static void it_refreshes_owned_outputs_and_preserves_handwritten_companions(void) {
    file("generated/project/main.c", "/* handwritten, deliberately retained */\n");
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(published);
    const SourceText *text = NULL;
    SourceStatus loaded = source_manager_load(&sources, "generated/project/main.c", &text);
    assert(loaded == SOURCE_OK);
    assert(source_length(text) == strlen("/* handwritten, deliberately retained */\n"));
    end_scaffold();
}

static void it_rejects_edited_owned_outputs_before_publishing_other_files(void) {
    file("generated/native/ama-native-types.h", "/* author edit */\n");
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(!published && scaffold.error.code == DIAGNOSTIC_INVALID_PATH);
    const SourceText *text = NULL;
    SourceStatus loaded = source_manager_load(&sources, "generated/native/ama-native-types.h", &text);
    assert(loaded == SOURCE_OK && source_length(text) == strlen("/* author edit */\n"));
    end_scaffold();
    begin_scaffold();
    built = build_scaffold(true);
    assert(built);
    published = native_scaffold_publish(&scaffold);
    assert(published);
    end_scaffold();
}

static void it_cleans_staging_and_locks_after_every_publication_allocation_failure(void) {
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    size_t start = allocation_count;
    bool published = native_scaffold_publish(&scaffold);
    assert(published);
    size_t count = allocation_count - start;
    end_scaffold();
    for (size_t failure = 1; failure <= count; failure++) {
        begin_scaffold();
        built = build_scaffold(false);
        assert(built);
        failure_at = allocation_count + failure;
        published = native_scaffold_publish(&scaffold);
        assert(!published && scaffold.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        end_scaffold();
    }
    begin_scaffold();
    built = build_scaffold(false);
    assert(built);
    published = native_scaffold_publish(&scaffold);
    assert(published);
    end_scaffold();
}

static void it_rejects_a_retained_lock_without_guessing_its_age(void) {
    file("generated/native/.ama-native.lock", "retained");
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(!published && scaffold.error.code == DIAGNOSTIC_IO_FAILURE);
    end_scaffold();
    int removed = unlink("generated/native/.ama-native.lock");
    assert(removed == 0);
}

static void it_refuses_to_refresh_a_different_entry_into_the_same_scope(void) {
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0};
    bool checked = compiler_frontend_check(&frontend, &root, 1, ".value", (SemanticLibraries){0});
    assert(checked);
    native_scaffold_init(&scaffold, &frontend, sources.allocator);
    bool built = build_scaffold(false);
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(!published && scaffold.error.code == DIAGNOSTIC_INVALID_PATH);
    end_scaffold();
}

static void it_never_claims_an_unknown_existing_file_as_generated(void) {
    directory("generated/unowned");
    file("generated/unowned/ama-native-types.h", "/* preexisting unknown file */\n");
    begin_scaffold();
    bool built = native_scaffold_build(&scaffold, (ScaffoldOptions){
        .directory = "generated/unowned", .sdk = TEST_SDK, .compiler = "gcc"});
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(!published && scaffold.error.code == DIAGNOSTIC_INVALID_PATH);
    struct stat metadata;
    int found = stat("generated/unowned/.ama-native-outputs", &metadata);
    assert(found == -1 && errno == ENOENT);
    end_scaffold();
}

static void it_rejects_symlinks_and_malformed_manifests_even_with_overwrite(void) {
    int removed = unlink("generated/native/ama-native-types.h");
    assert(removed == 0);
    int linked = symlink("../project/main.c", "generated/native/ama-native-types.h");
    assert(linked == 0);
    begin_scaffold();
    bool built = build_scaffold(true);
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(!published && scaffold.error.code == DIAGNOSTIC_UNSUPPORTED_PATH);
    end_scaffold();
    removed = unlink("generated/native/ama-native-types.h");
    assert(removed == 0);
    file("generated/native/.ama-native-outputs", "AMA-NATIVE-OUTPUTS 1\ninvalid\n");
    begin_scaffold();
    built = build_scaffold(true);
    assert(built);
    published = native_scaffold_publish(&scaffold);
    assert(!published && scaffold.error.code == DIAGNOSTIC_INVALID_PATH);
    end_scaffold();
}

void run(void) {
    prepare_native();
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(published);
    end_scaffold();
    it_refreshes_owned_outputs_and_preserves_handwritten_companions();
    it_rejects_edited_owned_outputs_before_publishing_other_files();
    it_cleans_staging_and_locks_after_every_publication_allocation_failure();
    it_rejects_a_retained_lock_without_guessing_its_age();
    it_refuses_to_refresh_a_different_entry_into_the_same_scope();
    it_never_claims_an_unknown_existing_file_as_generated();
    it_rejects_symlinks_and_malformed_manifests_even_with_overwrite();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
