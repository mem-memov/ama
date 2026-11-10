#include "../common.c"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#define TYPE_FIXTURES PROJECT_ROOT "/tests/integration/57-typed-layouts/project/"

static void it_compiles_two_projects_in_one_process(void) {
    setup_test();
    bool first = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project", "main", "generated/first.c", &error);
    bool second = compile_project(PROJECT_ROOT "/tests/integration/06-values/project", "main", "generated/second.c", &error);
    if (!second) error_print(stderr, &error);
    assert(first && second && error.error_code == ERROR_NONE);
    destroy_test();
}

static void it_can_compile_after_a_failed_compilation(void) {
    setup_test();
    bool rejected = compile_project(PROJECT_ROOT "/tests/integration/21-reject-cycle/project", "main", "generated/rejected.c", &error);
    assert(!rejected && error.error_code == ERROR_CYCLE);
    bool compiled = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project", "main", "generated/recovered.c", &error);
    assert(compiled && error.error_code == ERROR_NONE);
    destroy_test();
}

static void it_returns_file_errors_to_its_caller(void) {
    setup_test();
    bool compiled = compile_project("generated/missing-input", "main", "generated/unused.c", &error);
    assert(!compiled && error.error_code == ERROR_IO);
    compiled = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project", "main", "generated/missing-output/program.c", &error);
    assert(!compiled && error.error_code == ERROR_IO);
    destroy_test();
}

static void it_compiles_native_and_ama_projects_with_fresh_state(void) {
    setup_test();
    bool native = compile_project(PROJECT_ROOT "/tests/integration/26-native-calls/project",
        "main", "generated/native.c", &error);
    if (!native) error_print(stderr, &error);
    assert(native && error.error_code == ERROR_NONE);
    bool ama = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project",
        "main", "generated/after-native.c", &error);
    assert(ama && error.error_code == ERROR_NONE);
    destroy_test();
}

static void it_accepts_an_explicitly_disabled_library_and_reports_missing_library_paths(void) {
    setup_test();
    bool empty = compile_project_with_library(PROJECT_ROOT "/tests/integration/22-code-only/project",
        "main", "generated/without-library.c", "", &error);
    assert(empty && error.error_code == ERROR_NONE);
    bool needs_library = compile_project_with_library(PROJECT_ROOT "/tests/integration/01-hello/project",
        "main", "generated/missing-system-module.c", NULL, &error);
    assert(!needs_library && strstr(error.message, "sys/") != NULL);
    bool missing = compile_project_with_library(PROJECT_ROOT "/tests/integration/22-code-only/project",
        "main", "generated/invalid-library.c", "generated/no-such-library", &error);
    assert(!missing && error.error_code == ERROR_IO);
    destroy_test();
}

static void it_preserves_output_and_can_recover_after_body_validation_fails(void) {
    setup_test();
    FILE* preserved = fopen("generated/untouched.c", "wb");
    assert(preserved != NULL);
    int written = fputs("keep existing output\n", preserved);
    assert(written >= 0);
    close_file(preserved);
    bool rejected = compile_project(PROJECT_ROOT "/tests/integration/17-reject-unknown-param/project",
        "main", "generated/untouched.c", &error);
    assert(!rejected && error.error_code == ERROR_INVALID);
    preserved = fopen("generated/untouched.c", "rb");
    assert(preserved != NULL);
    assert_stream_equals(preserved, "keep existing output\n");
    close_file(preserved);
    bool compiled = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project",
        "main", "generated/after-body-error.c", &error);
    assert(compiled && error.error_code == ERROR_NONE);
    destroy_test();
}

static void write_fixture(const char* path, const char* text) {
    FILE* file = fopen(path, "wb");
    assert(file && fputs(text, file) >= 0);
    close_file(file);
}

static void expect_fixture(const char* path, const char* text) {
    FILE* file = fopen(path, "rb");
    assert(file);
    assert_stream_equals(file, text);
    close_file(file);
}

static void create_directory(const char* path) {
    int status = mkdir(path, 0777);
    assert(status == 0 || errno == EEXIST);
}

static void it_exports_typed_projects_repeatedly_and_recovers_after_failure(void) {
    setup_test();
    bool first = generate_type_layouts(TYPE_FIXTURES "valid", "generated/types", &error);
    if (!first) error_print(stderr, &error);
    assert(first && error.error_code == ERROR_NONE);
    bool rejected = generate_type_layouts(TYPE_FIXTURES "unknown", "generated/types", &error);
    assert(!rejected && error.error_code == ERROR_INVALID && error.line_number == 3);
    assert(strcmp(error.module, "main") == 0);
    bool recovered = generate_type_layouts(TYPE_FIXTURES "empty", "generated/types", &error);
    assert(recovered && error.error_code == ERROR_NONE);
    bool legacy = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project",
        "main", "generated/after-types.c", &error);
    assert(legacy && error.error_code == ERROR_NONE);
    destroy_test();
}

static void it_preserves_existing_type_output_on_parse_and_layout_errors(void) {
    setup_test();
    create_directory("generated/preserved-types");
    write_fixture("generated/preserved-types/ama_types.h", "old header\n");
    write_fixture("generated/preserved-types/ama_types.c", "old implementation\n");
    const char* projects[] = {TYPE_FIXTURES "unknown", TYPE_FIXTURES "cycle",
        TYPE_FIXTURES "bad-syntax", TYPE_FIXTURES "draft-type", TYPE_FIXTURES "references"};
    for (size_t index = 0; index < sizeof(projects) / sizeof(projects[0]); index++) {
        assert(!generate_type_layouts(projects[index], "generated/preserved-types", &error));
        assert(error.error_code != ERROR_NONE);
        expect_fixture("generated/preserved-types/ama_types.h", "old header\n");
        expect_fixture("generated/preserved-types/ama_types.c", "old implementation\n");
    }
    destroy_test();
}

static void it_loads_only_an_explicit_typed_library(void) {
    setup_test();
    bool without = generate_type_layouts(TYPE_FIXTURES "library-client", "generated/library-types", &error);
    assert(!without && error.error_code == ERROR_INVALID);
    bool with = generate_type_layouts_with_library(TYPE_FIXTURES "library-client",
        "generated/library-types", TYPE_FIXTURES "library", &error);
    assert(with && error.error_code == ERROR_NONE);
    bool duplicate = generate_type_layouts_with_library(TYPE_FIXTURES "valid",
        "generated/library-types", TYPE_FIXTURES "valid", &error);
    assert(!duplicate && strstr(error.message, "duplicate module path"));
    bool missing = generate_type_layouts_with_library(TYPE_FIXTURES "empty",
        "generated/library-types", "generated/no-such-library", &error);
    assert(!missing && error.error_code == ERROR_IO);
    destroy_test();
}

static void it_refuses_to_replace_native_sources_with_generated_type_files(void) {
    setup_test();
    create_directory("generated/protected-project");
    write_fixture("generated/protected-project/ama_types", "@C\n");
    write_fixture("generated/protected-project/ama_types.h", "native header\n");
    write_fixture("generated/protected-project/ama_types.c", "native implementation\n");
    bool rejected = generate_type_layouts("generated/protected-project", "generated/protected-project", &error);
    assert(!rejected && strstr(error.message, "overwrite source"));
    expect_fixture("generated/protected-project/ama_types.h", "native header\n");
    expect_fixture("generated/protected-project/ama_types.c", "native implementation\n");
    create_directory("generated/linked-output");
    assert(link("generated/protected-project/ama_types.h", "generated/linked-output/ama_types.h") == 0);
    rejected = generate_type_layouts("generated/protected-project", "generated/linked-output", &error);
    assert(!rejected && strstr(error.message, "overwrite source"));
    assert(unlink("generated/linked-output/ama_types.h") == 0);
    assert(symlink("../protected-project/ama_types.h", "generated/linked-output/ama_types.h") == 0);
    rejected = generate_type_layouts("generated/protected-project", "generated/linked-output", &error);
    assert(!rejected && strstr(error.message, "overwrite source"));
    expect_fixture("generated/protected-project/ama_types.h", "native header\n");
    destroy_test();
}

static void it_rejects_aliased_and_invalid_type_outputs(void) {
    setup_test();
    create_directory("generated/aliased-output");
    write_fixture("generated/aliased-output/ama_types.h", "keep\n");
    assert(link("generated/aliased-output/ama_types.h", "generated/aliased-output/ama_types.c") == 0);
    assert(!generate_type_layouts(TYPE_FIXTURES "empty", "generated/aliased-output", &error));
    assert(strstr(error.message, "different files"));
    expect_fixture("generated/aliased-output/ama_types.h", "keep\n");
    create_directory("generated/invalid-output");
    create_directory("generated/invalid-output/ama_types.c");
    write_fixture("generated/invalid-output/ama_types.h", "keep header\n");
    assert(!generate_type_layouts(TYPE_FIXTURES "empty", "generated/invalid-output", &error));
    assert(strstr(error.message, "not a regular file"));
    expect_fixture("generated/invalid-output/ama_types.h", "keep header\n");
    create_directory("generated/dangling-output");
    assert(symlink("ama_types.h", "generated/dangling-output/ama_types.c") == 0);
    assert(!generate_type_layouts(TYPE_FIXTURES "empty", "generated/dangling-output", &error));
    assert(strstr(error.message, "dangling symbolic link"));
    assert(access("generated/dangling-output/ama_types.h", F_OK) != 0);
    destroy_test();
}

void run(void) {
    it_exports_typed_projects_repeatedly_and_recovers_after_failure();
    it_preserves_existing_type_output_on_parse_and_layout_errors();
    it_loads_only_an_explicit_typed_library();
    it_refuses_to_replace_native_sources_with_generated_type_files();
    it_rejects_aliased_and_invalid_type_outputs();
    it_preserves_output_and_can_recover_after_body_validation_fails();
    it_accepts_an_explicitly_disabled_library_and_reports_missing_library_paths();
    it_compiles_native_and_ama_projects_with_fresh_state();
    it_compiles_two_projects_in_one_process();
    it_can_compile_after_a_failed_compilation();
    it_returns_file_errors_to_its_caller();
}

int main(void) {
    run();
    printf("unit/project/repeat: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
