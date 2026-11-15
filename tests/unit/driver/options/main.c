/**
 * @file
 * @brief Verify exact option recognition, request assembly and partial-owner cleanup.
 */
#include <stdint.h>
#include "../../source/common.c"
#include "driver/options/options.h"
#include "driver/options/recognition/option.h"
#include "driver/options/storage/storage.h"

/** Construct/destroy one invalid request and require its precise failure category. */
static void invalid_arguments(int count, char *const arguments[], const char *message) {
    setup_test();
    DriverOptions options;
    driver_options_init(&options, sources.allocator);
    bool valid = driver_options_parse(&options, count, arguments);
    assert(!valid && !options.valid && options.error.code == DIAGNOSTIC_INVALID_PATH);
    assert(strcmp(options.error.message, message) == 0);
    driver_options_destroy(&options);
    destroy_test();
}

/** A library configuration is a physical input path, with no implicit entry or launcher flags. */
static void it_keeps_library_configuration_separate_from_executable_options(void) {
    setup_test();
    DriverOptions options;
    driver_options_init(&options, sources.allocator);
    char *arguments[] = {"ama", "--library-check", "project", "config with spaces.exports", "--standard-root", "standard"};
    bool valid = driver_options_parse(&options, 6, arguments);
    assert(valid && options.entry == NULL && !options.compile);
    assert(strcmp(options.library_configuration, "config with spaces.exports") == 0);
    assert(options.root_count == 2 && options.roots[1].kind == MODULE_ROOT_STANDARD);
    assert(driver_option_recognize(DRIVER_REQUEST_LIBRARY_CHECK, "--compile") == NULL);
    assert(driver_option_recognize(DRIVER_REQUEST_LIBRARY_CHECK, "--out") == NULL);
    assert(driver_option_recognize(DRIVER_REQUEST_LIBRARY_HEADER, "--root") != NULL);
    assert(driver_option_recognize(DRIVER_REQUEST_LIBRARY_HEADER, "--compile") == NULL);
    driver_options_destroy(&options);
    destroy_test();
}
/** Artifact building requires explicit output/SDK and keeps ABI-affecting flags in its configuration. */
static void it_requires_explicit_library_build_destinations_and_manifest_flags(void) {
    setup_test();
    DriverOptions options; driver_options_init(&options,sources.allocator);
    char *arguments[]={"ama","--library-build","project","component.exports","--out","library output",
        "--sdk","compiler sdk","--cc","chosen-cc"};
    bool valid=driver_options_parse(&options,10,arguments);
    assert(valid && options.request==DRIVER_REQUEST_LIBRARY_BUILD && options.entry==NULL);
    assert(strcmp(options.library_configuration,"component.exports")==0);
    assert(strcmp(options.output,"library output")==0 && strcmp(options.sdk,"compiler sdk")==0);
    assert(strcmp(options.compiler,"chosen-cc")==0 && !options.compile && options.c_flag_count==0);
    assert(driver_option_recognize(DRIVER_REQUEST_LIBRARY_BUILD,"--c-flag")==NULL);
    assert(driver_option_recognize(DRIVER_REQUEST_LIBRARY_BUILD,"--compile")==NULL);
    driver_options_destroy(&options); destroy_test();
    invalid_arguments(4,arguments,"Project emission needs --out and --sdk");
}

/** Compare the complete fixed spelling/request/arity relation independently. */
static void it_recognizes_only_exact_applicable_options(void) {
    const char *names[] = {"--root", "--exclude", "--out", "--sdk", "--cc", "--protocols",
        "--c-flag", "--compile", "--create", "--overwrite"};
    const bool applicable[4][10] = {
        {true, true, false, false, false, false, false, false, false, false},
        {true, true, false, false, false, false, false, false, false, false},
        {true, true, true, true, true, true, true, false, true, true},
        {true, true, true, true, true, false, true, true, false, true}
    };
    for (size_t request = 0; request < 4; request++) {
        for (size_t name = 0; name < 10; name++) {
            const DriverOption *option = driver_option_recognize((DriverRequest)(DRIVER_REQUEST_CHECK + request), names[name]);
            assert((option != NULL) == applicable[request][name]);
            if (option != NULL) {
                assert(option->takes_value == (name < 7));
                assert(strcmp(option->name, names[name]) == 0);
            }
        }
        assert(driver_option_recognize((DriverRequest)(DRIVER_REQUEST_CHECK + request), "--roo") == NULL);
        assert(driver_option_recognize((DriverRequest)(DRIVER_REQUEST_CHECK + request), "--root=value") == NULL);
    }
    assert(driver_option_recognize(DRIVER_REQUEST_HELP, "--root") == NULL);
    assert(driver_option_recognize(DRIVER_REQUEST_UNAVAILABLE, "--root") == NULL);
}

/** Repeats preserve current append/last-value/idempotent behavior and borrowed bytes. */
static void it_preserves_repeated_values_flags_and_shared_root_exclusions(void) {
    setup_test();
    char *arguments[] = {"ama", "--emit", "first", ".main", "--root", "second", "--root", "second",
        "--exclude", "build", "--exclude", "build", "--out", "old", "--out", "new", "--sdk", "sdk",
        "--cc", "first-cc", "--cc", "last-cc", "--c-flag", "-O1", "--c-flag", "-O2",
        "--compile", "--compile", "--overwrite", "--overwrite"};
    DriverOptions options;
    driver_options_init(&options, sources.allocator);
    bool valid = driver_options_parse(&options, (int)(sizeof(arguments) / sizeof(*arguments)), arguments);
    assert(valid && options.valid && options.request == DRIVER_REQUEST_EMIT);
    assert(options.root_count == 3 && options.roots[0].path == arguments[2]);
    assert(options.roots[1].path == arguments[5] && options.roots[2].path == arguments[7]);
    assert(options.output == arguments[15] && options.compiler == arguments[21]);
    assert(options.entry == arguments[3] && options.sdk == arguments[17]);
    assert(options.compile && options.overwrite && !options.create);
    assert(options.c_flag_count == 2 && options.c_flags[0] == arguments[23] && options.c_flags[1] == arguments[25]);
    for (size_t index = 0; index < options.root_count; index++) {
        assert(options.roots[index].kind == MODULE_ROOT_PROJECT);
        assert(options.roots[index].excluded == options.roots[0].excluded);
        assert(options.roots[index].excluded_count == 2);
        assert(options.roots[index].excluded[0] == arguments[9]);
        assert(options.roots[index].excluded[1] == arguments[11]);
    }
    driver_options_destroy(&options);
    assert(!options.valid && options.roots == NULL && options.c_flags == NULL);
    destroy_test();
}

/** Explicit standard roots retain argv order and share the request exclusions. */
static void it_selects_standard_root_authority_only_by_an_explicit_option(void) {
    setup_test();
    DriverOptions options;
    driver_options_init(&options, sources.allocator);
    char *arguments[] = {"ama", "--check", "project", ".main", "--standard-root", "library",
        "--root", "imports", "--exclude", "generated"};
    bool valid = driver_options_parse(&options, 10, arguments);
    assert(valid && options.valid && options.root_count == 3);
    assert(options.roots[0].kind == MODULE_ROOT_PROJECT && options.roots[1].kind == MODULE_ROOT_STANDARD
        && options.roots[2].kind == MODULE_ROOT_PROJECT && options.roots[1].path == arguments[5]);
    assert(options.roots[1].excluded_count == 1 && options.roots[1].excluded[0] == arguments[9]);
    driver_options_destroy(&options);
    destroy_test();
    for (DriverRequest request = DRIVER_REQUEST_CHECK; request <= DRIVER_REQUEST_EMIT; request++) {
        const DriverOption *option = driver_option_recognize(request, "--standard-root");
        assert(option != NULL && option->takes_value && option->kind == DRIVER_OPTION_STANDARD_ROOT);
    }
    char *missing[] = {"ama", "--check", "project", ".main", "--standard-root"};
    invalid_arguments(5, missing, "Command option needs a value");
}

/** All operand/missing-value/unknown partitions preserve existing diagnostics. */
static void it_rejects_invalid_missing_and_wrong_request_options(void) {
    const char *operands = "Expected a source root and an exact logical module path";
    invalid_arguments(0, NULL, operands);
    char *short_arguments[] = {"ama", "--check", "root"};
    invalid_arguments(3, short_arguments, operands);
    char *path[] = {"ama", "--check", "root", "main"};
    invalid_arguments(4, path, operands);
    char *missing[] = {"ama", "--check", "root", ".main", "--root"};
    invalid_arguments(5, missing, "Command option needs a value");
    char *unknown[] = {"ama", "--check", "root", ".main", "--unknown"};
    invalid_arguments(5, unknown, "Command option needs a value");
    char *inapplicable[] = {"ama", "--check", "root", ".main", "--out", "output"};
    invalid_arguments(6, inapplicable, "Unknown command option");
    char *emit[] = {"ama", "--emit", "root", ".main", "--out", "output"};
    invalid_arguments(6, emit, "Project emission needs --out and --sdk");
    char *native[] = {"ama", "--native", "root", ".main", "--out", "output", "--sdk", "sdk", "--overwrite"};
    invalid_arguments(9, native, "Native generation needs --out/--sdk; --overwrite/--protocols require --create");
    char *protocols[] = {"ama", "--native", "root", ".main", "--out", "output", "--sdk", "sdk", "--protocols", "?"};
    invalid_arguments(10, protocols, "Native generation needs --out/--sdk; --overwrite/--protocols require --create");
}

/** Validate native relations/defaults without invoking a source loader/toolchain. */
static void it_assembles_native_creation_and_check_defaults(void) {
    char *arguments[] = {"ama", "--native", "root", ".main", "--out", "output", "--sdk", "sdk",
        "--protocols", "?=[]", "--create", "--overwrite"};
    setup_test();
    DriverOptions options;
    driver_options_init(&options, sources.allocator);
    bool valid = driver_options_parse(&options, 12, arguments);
    assert(valid && options.create && options.overwrite && !options.compile);
    assert(options.protocols == arguments[9] && strcmp(options.compiler, "gcc") == 0);
    driver_options_destroy(&options);
    char *check[] = {"ama", "--check", "root", ".main"};
    driver_options_init(&options, sources.allocator);
    valid = driver_options_parse(&options, 4, check);
    assert(valid && options.root_count == 1 && options.output == NULL);
    assert(options.roots[0].excluded_count == 0 && strcmp(options.compiler, "gcc") == 0);
    driver_options_destroy(&options);
    destroy_test();
}

/** Enumerate every allocation failure and prove partial arrays are reclaimed. */
static void it_cleans_every_partial_allocation_and_rejects_unrepresentable_storage(void) {
    char *arguments[] = {"ama", "--check", "root", ".main"};
    size_t requests = 0;
    for (size_t failure = 0; failure <= requests; failure++) {
        setup_test();
        DriverOptions options;
        driver_options_init(&options, sources.allocator);
        failure_at = failure;
        bool valid = driver_options_parse(&options, 4, arguments);
        if (failure == 0) {
            assert(valid);
            requests = allocation_count;
            assert(requests != 0);
        } else {
            assert(!valid && !options.valid && options.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        }
        driver_options_destroy(&options);
        destroy_test();
    }
    setup_test();
    DriverOptions options;
    DriverOptionStorage storage = {0};
    driver_options_init(&options, sources.allocator);
    bool allocated = driver_options_allocate(&options, SIZE_MAX, &storage);
    assert(!allocated && options.error.code == DIAGNOSTIC_INVALID_PATH && allocation_count == 0);
    driver_options_destroy(&options);
    destroy_test();
}

/** Exercise independent recognition/storage and their real request assembly. */
void run(void) {
    it_requires_explicit_library_build_destinations_and_manifest_flags();
    it_keeps_library_configuration_separate_from_executable_options();
    it_recognizes_only_exact_applicable_options();
    it_preserves_repeated_values_flags_and_shared_root_exclusions();
    it_selects_standard_root_authority_only_by_an_explicit_option();
    it_rejects_invalid_missing_and_wrong_request_options();
    it_assembles_native_creation_and_check_defaults();
    it_cleans_every_partial_allocation_and_rejects_unrepresentable_storage();
}
/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
