/** @file
 * @brief Own leaf-local original snapshots, real compiler admission and structural fixture owners.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#ifndef AMA_SOURCE_EXPANSION_TEST_COMMON
#define AMA_SOURCE_EXPANSION_TEST_COMMON

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/expansion/expansion.h"
#include "delivery/source/snapshot/write.h"
static SourceRootCapture capture;
static SourceAnalysis analysis;
static SourceExpansion expansion;
static AcquisitionClosure closure;
static const char *main_identity = "/vendor/pkg/v1.0.0/src/main";
static const char *child_identity = "/vendor/pkg/v1.0.0/src/parent/child";
/** Keep fixture directories exact; never forgive unrelated filesystem failure. */
static void directory(const char *path) { assert(mkdir(path, 0700) == 0 || errno == EEXIST); }
/** Write exact authored bytes without invoking a shell or source generator. */
static void file(const char *path, const char *bytes) {
    FILE *output = fopen(path, "wb"); assert(output);
    assert(fwrite(bytes, 1, strlen(bytes), output) == strlen(bytes));
    assert(fclose(output) == 0);
}
/** Independent small source cycle, draft ancestor and opaque structural material. */
static void fixtures(void) {
    const char *directories[] = {"generated/project", "generated/project/packages", "generated/project/packages/vendor",
        "generated/project/packages/vendor/pkg", "generated/project/packages/vendor/pkg/v1.0.0",
        "generated/project/packages/vendor/pkg/v1.0.0/src", "generated/project/packages/vendor/pkg/v1.0.0/tests",
        "generated/project/packages/vendor/pkg/v1.0.0/src/parent+",
        "generated/project/packages/vendor/pkg/v1.0.0/tests/parent",
        "generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child",
        "generated/project/packages/vendor/pkg/v1.0.0/tests/main", "generated/project/packages/vendor/pkg/v1.0.0/tests/other",
        "generated/standard", "generated/project/packages/vendor/pkg/v2.0.0",
        "generated/project/packages/vendor/pkg/v2.0.0/src", "generated/project/packages/vendor/pkg/v2.0.0/tests"};
    for (size_t index = 0; index < sizeof(directories) / sizeof(*directories); index++) { directory(directories[index]); }
    file("generated/project/packages/vendor/pkg/v1.0.0/package.ini", "; original\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/README.md", "optional guidance\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent", "unfinished draft\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent.extra", "opaque parent companion\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/child", "@!\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/opaque.data", "opaque tree bytes\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/main", "@!\nother&\n./other\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/other", "@!\nmain&\n./main\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/main/check", "@!\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/other/check", "@!\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/check", "@!\n");
    file("generated/standard/plain", "@!\n");
    file("generated/project/packages/vendor/pkg/v2.0.0/package.ini", "; empty selected version\n");
}
/** Freeze all compiler inputs once; real child processes read only the serialized request. */
static void capture_fixtures(void) {
    source_root_capture_init(&capture, sources.allocator);
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    assert(source_root_capture_build(&capture, roots, 2) == SOURCE_CAPTURE_OK);
    FILE *request = fopen("generated/request.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &capture.view));
    assert(fclose(request) == 0);
    source_analysis_init(&analysis, sources.allocator);
    SourceAnalysisStatus status = source_analysis_check(&analysis, &capture.view, "../../../../../../ama", "generated/request.json");
    if (status != SOURCE_ANALYSIS_OK && analysis.original.diagnostics) {
        fwrite(source_bytes(analysis.original.diagnostics), 1, source_length(analysis.original.diagnostics), stderr);
    }
    assert(status == SOURCE_ANALYSIS_OK);
    source_expansion_init(&expansion, sources.allocator, &capture, &analysis, NULL);
    acquisition_closure_init(&closure, sources.allocator);
}
/** Prepare fixture variants before freezing their shared original domain. */
static void prepare_profile(bool standard_dependency, bool open_main_test, bool missing_main_test) {
    assert(!(open_main_test && missing_main_test));
    setup_test(); fixtures();
    if (standard_dependency) { file("generated/project/packages/vendor/pkg/v1.0.0/src/main", "@!\nother&\n./other\nstandard&\n//plain\n"); }
    if (open_main_test) { file("generated/project/packages/vendor/pkg/v1.0.0/tests/main/check", "@<\nvalue\n.T\n@!\n"); }
    if (missing_main_test) {
        assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/main/check") == 0);
        assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/tests/main") == 0);
    }
    capture_fixtures();
}

/** Preserve the ordinary fixture profile for structural component consumers. */
static void prepare(void) { prepare_profile(false, false, false); }
/** Retire closure borrowers first, then structural and original compiler owners. */
static void finish(void) {
    acquisition_closure_destroy(&closure); source_expansion_destroy(&expansion);
    source_analysis_destroy(&analysis); source_root_capture_destroy(&capture); destroy_test();
}
#endif
