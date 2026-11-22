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
    char *arguments[] = {"ama", "--emit", "first", "/test/fixture/v1.0.0/src/main", "--root", "second", "--root", "second",
        "--exclude", "build", "--exclude", "build", "--out", "old", "--out", "new", "--sdk", "sdk",
        "--cc", "first-cc", "--cc", "last-cc", "--c-flag", "-O1", "--c-flag", "-O2",
        "--compile", "--compile", "--overwrite", "--overwrite",
        "--mode", "executable", "--component", "test.options"};
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
    char *arguments[] = {"ama", "--check", "project", "/test/fixture/v1.0.0/src/main", "--standard-root", "library",
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
    char *missing[] = {"ama", "--check", "project", "/test/fixture/v1.0.0/src/main", "--standard-root"};
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
    char *missing[] = {"ama", "--check", "root", "/test/fixture/v1.0.0/src/main", "--root"};
    invalid_arguments(5, missing, "Command option needs a value");
    char *unknown[] = {"ama", "--check", "root", "/test/fixture/v1.0.0/src/main", "--unknown"};
    invalid_arguments(5, unknown, "Command option needs a value");
    char *inapplicable[] = {"ama", "--check", "root", "/test/fixture/v1.0.0/src/main", "--out", "output"};
    invalid_arguments(6, inapplicable, "Unknown command option");
    char *emit[] = {"ama", "--emit", "root", "/test/fixture/v1.0.0/src/main", "--out", "output"};
    invalid_arguments(6, emit, "Project emission needs --out and --sdk");
    char *native[] = {"ama", "--native", "root", "/test/fixture/v1.0.0/src/main", "--out", "output", "--sdk", "sdk", "--overwrite"};
    invalid_arguments(9, native, "Native generation needs --out/--sdk; --overwrite/--protocols require --create");
    char *protocols[] = {"ama", "--native", "root", "/test/fixture/v1.0.0/src/main", "--out", "output", "--sdk", "sdk", "--protocols", "?"};
    invalid_arguments(10, protocols, "Native generation needs --out/--sdk; --overwrite/--protocols require --create");
}

/** Validate native relations/defaults without invoking a source loader/toolchain. */
static void it_assembles_native_creation_and_check_defaults(void) {
    char *arguments[] = {"ama", "--native", "root", "/test/fixture/v1.0.0/src/main", "--out", "output", "--sdk", "sdk",
        "--protocols", "?=[]", "--create", "--overwrite"};
    setup_test();
    DriverOptions options;
    driver_options_init(&options, sources.allocator);
    bool valid = driver_options_parse(&options, 12, arguments);
    assert(valid && options.create && options.overwrite && !options.compile);
    assert(options.protocols == arguments[9] && strcmp(options.compiler, "gcc") == 0);
    driver_options_destroy(&options);
    char *check[] = {"ama", "--check", "root", "/test/fixture/v1.0.0/src/main"};
    driver_options_init(&options, sources.allocator);
    valid = driver_options_parse(&options, 4, check);
    assert(valid && options.root_count == 1 && options.output == NULL);
    assert(options.roots[0].excluded_count == 0 && strcmp(options.compiler, "gcc") == 0);
    driver_options_destroy(&options);
    destroy_test();
}

/** Enumerate every allocation failure and prove partial arrays are reclaimed. */
static void it_cleans_every_partial_allocation_and_rejects_unrepresentable_storage(void) {
    char *arguments[] = {"ama", "--check", "root", "/test/fixture/v1.0.0/src/main"};
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

/** Source selectors use exact logical names and canonical original byte offsets, with no duplicate precedence rule. */
static void it_validates_source_tool_selectors_and_required_rename_operands(void) {
    char *valid[] = {"ama","--editor","project","/test/fixture/v1.0.0/src/main","--module","/test/fixture/v1.0.0/src/worker","--offset","0"};
    setup_test();
    DriverOptions options;
    driver_options_init(&options,sources.allocator);
    assert(driver_options_parse(&options,8,valid));
    assert(options.request == DRIVER_REQUEST_EDITOR && options.has_offset && options.offset == 0);
    assert(strcmp(options.query_module,"/test/fixture/v1.0.0/src/worker") == 0 && options.root_count == 1 && !options.compile);
    driver_options_destroy(&options);
    destroy_test();
    const char *bad_offsets[] = {"", "00", "+1", "-1", "1 ", "184467440737095516160"};
    for (size_t index = 0; index < sizeof(bad_offsets)/sizeof(*bad_offsets); index++) {
        valid[7] = (char *)bad_offsets[index];
        invalid_arguments(8,valid,"Expected one canonical nonnegative --offset in the supported byte range");
    }
    char *duplicate[] = {"ama","--editor","project","/test/fixture/v1.0.0/src/main","--offset","0","--offset","1"};
    invalid_arguments(8,duplicate,"Expected one canonical nonnegative --offset in the supported byte range");
    char *rename[] = {"ama","--rename-plan","project","/test/fixture/v1.0.0/src/main","--offset","3","--name","new name"};
    setup_test();
    driver_options_init(&options,sources.allocator);
    assert(driver_options_parse(&options,8,rename));
    assert(options.request == DRIVER_REQUEST_RENAME_PLAN && options.offset == 3);
    assert(strcmp(options.query_module,"/test/fixture/v1.0.0/src/main") == 0 && strcmp(options.rename_name,"new name") == 0);
    driver_options_destroy(&options);
    destroy_test();
    invalid_arguments(4,rename,"Rename planning requires --offset and --name");
    invalid_arguments(6,rename,"Rename planning requires --offset and --name");
}
/** Exact final directories and explicit root kinds/settings cannot be inferred from a command's positional source directory. */
static void it_validates_relocation_authority_and_standalone_recovery(void) {
    setup_test();
    DriverOptions options;
    driver_options_init(&options,sources.allocator);
    char *arguments[] = {"ama","--relocate-preview","project","old directory","--destination","new directory",
        "--destination-standard-root","library","--entry","/test/fixture/v1.0.0/src/old/main","--configuration","component.library",
        "--invalidate","out","--exclude","ignored","--root","importers"};
    assert(driver_options_parse(&options,18,arguments));
    assert(options.valid && options.request == DRIVER_REQUEST_RELOCATE_PREVIEW);
    assert(strcmp(options.relocation_source,"old directory") == 0 && strcmp(options.destination,"new directory") == 0);
    assert(options.root_count == 2 && options.destination_root.kind == MODULE_ROOT_STANDARD);
    assert(options.destination_root.excluded_count == 1 && strcmp(options.destination_root.excluded[0],"ignored") == 0);
    assert(options.configuration_count == 1 && options.invalidation_count == 1 && strcmp(options.entry,"/test/fixture/v1.0.0/src/old/main") == 0);
    driver_options_destroy(&options);
    destroy_test();
    invalid_arguments(4,arguments,"Relocation requires --destination; apply additionally requires --recovery");
    arguments[1] = "--relocate-apply";
    invalid_arguments(6,arguments,"Relocation requires --destination; apply additionally requires --recovery");
    char *recover[] = {"ama","--relocate-recover","journal","--root","project"};
    invalid_arguments(5,recover,"Recovery requires exactly one explicit journal directory");
    setup_test();
    driver_options_init(&options,sources.allocator);
    assert(driver_options_parse(&options,3,recover));
    assert(options.valid && options.root_count == 0 && options.entry == NULL && strcmp(options.journal,"journal") == 0);
    driver_options_destroy(&options);
    destroy_test();
    char *duplicate[] = {"ama","--relocate-preview","project","old","--destination","new","--destination-root","one","--destination-standard-root","two"};
    invalid_arguments(10,duplicate,"Relocation accepts one explicit destination root kind/path");
}
/** Optional relocation arrays retain every failed prefix without increasing ordinary command storage. */
static void it_unwinds_every_relocation_option_array_allocation(void) {
    size_t requests = 0;
    for (size_t failure = 0; failure <= requests; failure++) {
        setup_test();
        DriverOptions options; driver_options_init(&options,sources.allocator);
        char *arguments[] = {"ama","--relocate-preview","project","old","--destination","new","--configuration","one.library","--invalidate","cache"};
        allocation_count = 0; failure_at = failure;
        bool valid = driver_options_parse(&options,10,arguments);
        failure_at = 0;
        if (failure == 0) { assert(valid); requests = allocation_count; assert(requests != 0); }
        else { assert(!valid && !options.valid && options.error.code == DIAGNOSTIC_OUT_OF_MEMORY); }
        driver_options_destroy(&options); destroy_test();
    }
}
/** Explicit compiler delivery values retain exact bytes and reject ambiguous authority. */
static void it_recognizes_exact_delivery_modes_and_component_authority(void) {
    for (int mode = 0; mode < 2; mode++) {
        setup_test(); DriverOptions options; driver_options_init(&options, sources.allocator);
        char *arguments[] = {"ama", "--emit", "root", "/test/fixture/v1.0.0/src/main", "--out", "output", "--sdk", "sdk",
            "--mode", mode ? "source-inclusion" : "executable", "--component", "компонент / name  "};
        bool valid = driver_options_parse(&options, 12, arguments);
        assert(valid && options.has_delivery_mode && options.delivery_mode == (mode ? DELIVERY_SOURCE_INCLUSION : DELIVERY_EXECUTABLE));
        assert(!strcmp(options.component, "компонент / name  "));
        driver_options_destroy(&options); destroy_test();
    }
    char *unknown[] = {"ama", "--emit", "root", "/test/fixture/v1.0.0/src/main", "--out", "output", "--sdk", "sdk", "--mode", "other"};
    invalid_arguments(10, unknown, "Unknown delivery mode; expected executable or source-inclusion");
    char *repeated[] = {"ama", "--emit", "root", "/test/fixture/v1.0.0/src/main", "--out", "output", "--sdk", "sdk", "--mode", "executable", "--mode", "source-inclusion"};
    invalid_arguments(12, repeated, "Delivery requires exactly one --mode value");
    char *component[] = {"ama", "--emit", "root", "/test/fixture/v1.0.0/src/main", "--out", "output", "--sdk", "sdk", "--component", "first", "--component", "second"};
    invalid_arguments(12, component, "Delivery requires one nonempty UTF-8 --component value without repetition");
    component[9] = "";
    invalid_arguments(10, component, "Delivery requires one nonempty UTF-8 --component value without repetition");
    char *missing[] = {"ama", "--emit", "root", "/test/fixture/v1.0.0/src/main", "--out", "output", "--sdk", "sdk",
        "--mode", "executable", "--component", "application"};
    invalid_arguments(8, missing, "Project emission requires explicit --mode and --component");
    invalid_arguments(10, missing, "Project emission requires explicit --mode and --component");
    missing[9] = "source-inclusion";
    char *compile[] = {"ama", "--emit", "root", "/test/fixture/v1.0.0/src/main", "--out", "output", "--sdk", "sdk",
        "--mode", "source-inclusion", "--component", "application", "--compile"};
    invalid_arguments(13, compile, "Native executable compilation requires --mode executable");
    assert(!driver_option_recognize(DRIVER_REQUEST_NATIVE, "--mode"));
    assert(!driver_option_recognize(DRIVER_REQUEST_LIBRARY_BUILD, "--component"));
}
/** Entry-free domain options start immediately after ROOT, retain explicit root
 * kinds/exclusions and never accept output, execution or executable-entry flags. */
static void it_parses_source_domain_roots_without_an_entry_or_mutation_options(void) {
    char *arguments[] = {"ama", "--source-domain", "root", "--root", "other",
        "--standard-root", "standard", "--exclude", "excluded"};
    size_t requests = 0;
    for (size_t failure = 0; !failure || failure <= requests; failure++) {
        setup_test(); DriverOptions options; driver_options_init(&options, sources.allocator);
        failure_at = failure;
        bool valid = driver_options_parse(&options, 9, arguments);
        if (!failure) {
            assert(valid && options.valid && !options.entry && !options.compile && !options.output);
            assert(options.root_count == 3 && options.roots[2].kind == MODULE_ROOT_STANDARD);
            assert(!strcmp(options.roots[1].path, "other"));
            assert(options.roots[0].excluded_count == 1 && !strcmp(options.roots[0].excluded[0], "excluded"));
            requests = allocation_count; assert(requests);
        } else { assert(!valid && !options.valid && options.error.code == DIAGNOSTIC_OUT_OF_MEMORY); }
        driver_options_destroy(&options); destroy_test();
    }
    invalid_arguments(2, arguments, "Source-domain inspection requires an explicit source root");
    const char *forbidden[] = {"--out", "--entry", "--compile", "--mode", "--apply", "--create"};
    for (size_t index = 0; index < sizeof(forbidden) / sizeof(*forbidden); index++) {
        setup_test(); DriverOptions options; driver_options_init(&options, sources.allocator);
        char *invalid[] = {"ama", "--source-domain", "root", (char *)forbidden[index], "value"};
        bool valid = driver_options_parse(&options, 5, invalid);
        assert(!valid && !options.valid);
        driver_options_destroy(&options); destroy_test();
    }
}
/** Captured inspection has one explicit file and no implicit roots, exclusions,
 * output directory, toolchain or option-allocation side effects. */
static void it_selects_exact_source_snapshot_input_without_root_fallback(void) {
    char *arguments[] = {"ama", "--source-domain", "--snapshot", "prepared.json", "--root", "other"};
    setup_test(); DriverOptions options; driver_options_init(&options, sources.allocator);
    bool valid = driver_options_parse(&options, 4, arguments);
    assert(valid && options.valid && !strcmp(options.source_snapshot, "prepared.json"));
    assert(!options.roots && !options.root_count && !options.entry && !options.compiler && !allocation_count);
    driver_options_destroy(&options); destroy_test();
    invalid_arguments(3, arguments, "Captured source inspection requires --snapshot FILE and optional --probe-entry CANONICAL_MODULE");
    invalid_arguments(6, arguments, "Captured source inspection requires --snapshot FILE and optional --probe-entry CANONICAL_MODULE");
    arguments[3] = "";
    invalid_arguments(4, arguments, "Captured source inspection requires --snapshot FILE and optional --probe-entry CANONICAL_MODULE");
}

/** Captured entry probes use exact canonical identity and allocate no option storage. */
static void it_selects_exact_captured_entry_probes_without_allocating_or_accepting_aliases(void) {
    char *arguments[] = {"ama", "--source-domain", "--snapshot", "prepared.json", "--probe-entry", "/vendor/pkg/v1.0.0/src/main", "--probe-entry", "//other"};
    setup_test(); DriverOptions options; driver_options_init(&options, sources.allocator);
    assert(driver_options_parse(&options, 6, arguments) && options.valid && options.source_probe == arguments[5]);
    assert(options.source_snapshot == arguments[3] && !options.roots && !options.root_count && !options.entry && !allocation_count);
    driver_options_destroy(&options); destroy_test();
    const char *invalid[] = {"", "./relative", "~alias", ".T", "//", "/vendor/pkg/v1.0.0/src/main/"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        arguments[5] = (char *)invalid[index];
        invalid_arguments(6, arguments, "Captured source inspection requires --snapshot FILE and optional --probe-entry CANONICAL_MODULE");
    }
    arguments[5] = "//main";
    invalid_arguments(8, arguments, "Captured source inspection requires --snapshot FILE and optional --probe-entry CANONICAL_MODULE");
}

/** Exercise independent recognition/storage and their real request assembly. */
void run(void) {
    it_selects_exact_source_snapshot_input_without_root_fallback();
    it_selects_exact_captured_entry_probes_without_allocating_or_accepting_aliases();
    it_parses_source_domain_roots_without_an_entry_or_mutation_options();
    it_recognizes_exact_delivery_modes_and_component_authority();
    it_unwinds_every_relocation_option_array_allocation();
    it_validates_relocation_authority_and_standalone_recovery();
    it_validates_source_tool_selectors_and_required_rename_operands();
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
