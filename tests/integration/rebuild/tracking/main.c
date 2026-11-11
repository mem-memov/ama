#include "../common.c"

static void it_discovers_nested_sources_and_leaves_unchanged_builds_alone(void) {
    setup_test();
    copy_fixture();
    run_build("generated/project", "all", NULL);
    expect_invocations(3, 1);
    expect_program_value(7);
    reset_trace();
    run_build("generated/project", "all", NULL);
    expect_invocations(0, 0);
    destroy_test();
}

static void it_rebuilds_only_consumers_of_a_transitive_header(void) {
    prepare_project();
    write_file("generated/project/src/nested/detail/constant.h", "#define FIXTURE_VALUE 9\n");
    run_build("generated/project", "all", NULL);
    expect_invocations(1, 1);
    expect_program_value(9);
    destroy_test();
}

static void it_compiles_new_deep_sources_and_relinks(void) {
    prepare_project();
    make_directory("generated/project/src/extra");
    make_directory("generated/project/src/extra/deep");
    write_file("generated/project/src/extra/deep/added.c", "int added(void) { return 1; }\n");
    run_build("generated/project", "all", NULL);
    expect_invocations(1, 1);
    expect_program_value(7);
    destroy_test();
}

static void it_relinks_after_removing_a_source_without_recompiling_others(void) {
    prepare_project();
    int result = unlink("generated/project/src/unrelated.c");
    assert(result == 0);
    run_build("generated/project", "all", NULL);
    expect_invocations(0, 1);
    expect_program_value(7);
    destroy_test();
}

static void it_rebuilds_for_each_changed_compilation_setting(void) {
    const char *settings[] = {
        "C_COMPILER_FLAGS=-O0 -std=c11 -DEXTRA_VALUE=1",
        "CPPFLAGS=-DEXTRA_VALUE=1", "LTO_FLAGS=-DEXTRA_VALUE=1"
    };
    for (size_t index = 0; index < sizeof(settings) / sizeof(settings[0]); index++) {
        prepare_project();
        run_build("generated/project", "all", settings[index]);
        expect_invocations(3, 1);
        expect_program_value(8);
        destroy_test();
    }
}

static void it_only_relinks_when_linker_flags_change(void) {
    prepare_project();
    run_build("generated/project", "all", "LINKER_FLAGS=-g");
    expect_invocations(0, 1);
    expect_program_value(7);
    destroy_test();
}

static void it_rebuilds_when_the_compiler_command_changes(void) {
    prepare_project();
    char compiler_path[4096];
    char option[4096];
    int length = snprintf(compiler_path, sizeof(compiler_path),
                            "%s/generated/compiler", working_directory);
    assert(length > 0 && (size_t) length < sizeof(compiler_path));
    int result = symlink(compiler_path, "generated/project/other-compiler");
    assert(result == 0);
    length = snprintf(option, sizeof(option), "C_COMPILER=%s/generated/project/other-compiler",
                        working_directory);
    assert(length > 0 && (size_t) length < sizeof(option));
    run_build("generated/project", "all", option);
    expect_invocations(3, 1);
    destroy_test();
}

static void it_preserves_unchanged_test_objects_and_tracks_parent_fixtures(void) {
    prepare_build("generated/project/tests/check");
    run_build("generated/project/tests/check", "build", NULL);
    expect_invocations(0, 0);
    write_file("generated/project/tests/common.c", "#include <assert.h>\n#include \"nested/value.h\"\n");
    run_build("generated/project/tests/check", "build", NULL);
    expect_invocations(1, 1);
    destroy_test();
}

static void it_tracks_transitive_headers_for_test_and_production_objects(void) {
    prepare_build("generated/project/tests/check");
    write_file("generated/project/src/nested/detail/constant.h", "#define FIXTURE_VALUE 9\n");
    run_build("generated/project/tests/check", "run", NULL);
    expect_invocations(2, 1);
    destroy_test();
}

static void it_recompiles_tests_after_test_definitions_change(void) {
    prepare_build("generated/project/tests/check");
    run_build("generated/project/tests/check", "run", "TEST_DEFINES=-DTEST_SWITCH=1");
    expect_invocations(3, 1);
    destroy_test();
}

static void it_tracks_test_compilation_flags_as_well_as_definitions(void) {
    const char *settings[] = {
        "C_COMPILER_FLAGS=-O1 -std=c11", "CPPFLAGS=-DTEST_SWITCH=1",
        "LTO_FLAGS=-DTEST_SWITCH=1"
    };
    for (size_t index = 0; index < sizeof(settings) / sizeof(settings[0]); index++) {
        prepare_build("generated/project/tests/check");
        run_build("generated/project/tests/check", "run", settings[index]);
        expect_invocations(3, 1);
        destroy_test();
    }
}

static void it_relinks_tests_when_their_source_list_changes(void) {
    prepare_build("generated/project/tests/check");
    run_build("generated/project/tests/check", "run", "SOURCES=nested/value.c");
    expect_invocations(0, 1);
    destroy_test();
}

static void it_does_not_recompile_tests_for_linker_only_changes(void) {
    prepare_build("generated/project/tests/check");
    run_build("generated/project/tests/check", "run", "LINKER_FLAGS=-g");
    expect_invocations(0, 1);
    destroy_test();
}

void run(void) {
    it_discovers_nested_sources_and_leaves_unchanged_builds_alone();
    it_rebuilds_only_consumers_of_a_transitive_header();
    it_compiles_new_deep_sources_and_relinks();
    it_relinks_after_removing_a_source_without_recompiling_others();
    it_rebuilds_for_each_changed_compilation_setting();
    it_only_relinks_when_linker_flags_change();
    it_rebuilds_when_the_compiler_command_changes();
    it_preserves_unchanged_test_objects_and_tracks_parent_fixtures();
    it_tracks_transitive_headers_for_test_and_production_objects();
    it_recompiles_tests_after_test_definitions_change();
    it_tracks_test_compilation_flags_as_well_as_definitions();
    it_relinks_tests_when_their_source_list_changes();
    it_does_not_recompile_tests_for_linker_only_changes();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
