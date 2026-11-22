/** @file
 * @brief Verify exact original-domain ownership, typed cached test admission and unchanged graph obligations.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/analysis/analysis.h"
static SourceAnalysis analysis;
static DeliverySourceSnapshot snapshot;
static DeliverySourceSnapshotView view;
static const char *executable;
/** Generate immutable leaf-local data without shared output directories. */
static void generate(void) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", "generate.py", (char *)NULL); _exit(127); }
    int status; assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Decode independent captured data before any analysis allocation fault. */
static void prepare(const char *name) {
    setup_test(); delivery_source_snapshot_init(&snapshot, sources.allocator);
    const SourceText *input = NULL; assert(source_manager_load(&sources, name, &input) == SOURCE_OK);
    assert(delivery_source_snapshot_read(&snapshot, input) == SOURCE_JSON_OK);
    view = (DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, snapshot.companions};
    source_analysis_init(&analysis, sources.allocator);
}
/** Keep captured sources alive until all borrowed candidate/probe metadata retires. */
static void finish(void) {
    source_analysis_destroy(&analysis); delivery_source_snapshot_destroy(&snapshot); destroy_test();
}
/** Original checked export, never a guessed name or copied candidate object. */
static const DeliverySourceDefinition *definition(const char *identity) {
    for (const DeliverySourceDefinition *item = analysis.original.response.definitions; item; item = item->next) {
        if (!strcmp(item->identity, identity)) { return item; }
    }
    assert(false); return NULL;
}
/** Controlled child has coherent source bytes but changes a graph category only on probe. */
static int producer(bool probe) {
    FILE *input = fopen(probe ? "generated/changed-graph.json" : "generated/captured-response.json", "rb"); assert(input);
    int byte; while ((byte = fgetc(input)) != EOF) { assert(fputc(byte, stdout) != EOF); }
    assert(!ferror(input) && fclose(input) == 0); return 0;
}
/** T235 direct roots include grounded original formals while exact Open stays support. */
static void it_admits_real_direct_test_candidates_and_reuses_owned_cached_proofs(void) {
    prepare("generated/request.json");
    char compiler[] = "../../../../../../ama", request[] = "generated/request.json";
    SourceAnalysisStatus checked = source_analysis_check(&analysis, &view, compiler, request);
    if (checked != SOURCE_ANALYSIS_OK) {
        fprintf(stderr, "Original analysis stage %d, client %d, exit %d\n", checked, analysis.original.status, analysis.original.process.exit_code);
        if (analysis.original.diagnostics) {
            fwrite(source_bytes(analysis.original.diagnostics), 1, source_length(analysis.original.diagnostics), stderr);
        }
        if (analysis.original.response.issue.message) { fprintf(stderr, "%s\n", analysis.original.response.issue.message); }
    }
    assert(checked == SOURCE_ANALYSIS_OK && analysis.valid);
    compiler[0] = '!'; request[0] = '!';
    SourceArena selected; source_arena_init(&selected, sources.allocator);
    const AcquisitionTestRoot *roots = NULL;
    assert(acquisition_test_roots(&selected, analysis.original.response.definitions,
        "/vendor/pkg/v1.0.0/tests/sort", &roots) == ACQUISITION_TEST_ROOTS_OK);
    AcquisitionClosureProvider provider = {.context = &analysis, .admit_test = source_analysis_admit_test};
    size_t executable_count = 0, open_count = 0;
    for (const AcquisitionTestRoot *root = roots; root; root = root->next) {
        SourceIssue issue = {0}; AcquisitionClosureStatus status = provider.admit_test(provider.context, root->definition, &issue);
        assert(status == ACQUISITION_CLOSURE_OK || status == ACQUISITION_CLOSURE_OPEN_TEST);
        assert(!issue.message && analysis.last_probe);
        if (status == ACQUISITION_CLOSURE_OK) { executable_count++; } else { open_count++; }
        size_t before = allocation_count; const SourceAnalysisProbe *retained = analysis.last_probe;
        assert(provider.admit_test(provider.context, root->definition, &issue) == status);
        assert(analysis.last_probe == retained && allocation_count == before);
    }
    assert(executable_count == 2 && open_count == 1);
    assert(source_analysis_check(&analysis, &view, "invalid", "invalid") == SOURCE_ANALYSIS_INVALID && analysis.valid);
    source_arena_destroy(&selected); finish();
}
/** Exact handle ownership and source category refuse before spawning or allocating proof state. */
static void it_refuses_foreign_static_and_unprepared_candidate_handles(void) {
    prepare("generated/request.json"); SourceIssue issue = {0};
    assert(source_analysis_admit_test(&analysis, NULL, &issue) == ACQUISITION_CLOSURE_PREPARATION_FAILED);
    assert(source_analysis_check(&analysis, &view, "../../../../../../ama", "generated/request.json") == SOURCE_ANALYSIS_OK);
    DeliverySourceDefinition foreign = *definition("/vendor/pkg/v1.0.0/tests/sort/check");
    size_t before = allocation_count;
    assert(source_analysis_admit_test(&analysis, &foreign, &issue) == ACQUISITION_CLOSURE_INVALID);
    assert(source_analysis_admit_test(&analysis, definition("/vendor/pkg/v1.0.0/tests/sort/marker"), &issue) == ACQUISITION_CLOSURE_INVALID);
    assert(allocation_count == before && !analysis.probes && !analysis.last_probe && issue.message); finish();
}
/** Matching bytes/schema do not authorize a changed original checked graph; failed cache entries remain failures. */
static void it_rejects_changed_probe_graph_and_retains_failure_without_retry(void) {
    prepare("generated/base.json"); assert(source_analysis_check(&analysis, &view, executable, "generated/base.json") == SOURCE_ANALYSIS_OK);
    SourceIssue issue = {0}; const DeliverySourceDefinition *candidate = definition("//native");
    assert(source_analysis_admit_test(&analysis, candidate, &issue) == ACQUISITION_CLOSURE_PREPARATION_FAILED);
    assert(issue.message && issue.span.source && analysis.last_probe && analysis.last_probe->compiler.valid);
    const SourceAnalysisProbe *retained = analysis.last_probe; size_t before = allocation_count;
    assert(source_analysis_admit_test(&analysis, candidate, &issue) == ACQUISITION_CLOSURE_PREPARATION_FAILED);
    assert(analysis.last_probe == retained && allocation_count == before); finish();
}
/** Actual compiler refusal preserves original raw diagnostics and never yields provider readiness. */
static void it_retains_failed_original_compiler_diagnostics_without_exposing_a_domain(void) {
    prepare("generated/base.json");
    assert(source_analysis_check(&analysis, &view, "../../../../../../ama", "generated/bad-source.json") == SOURCE_ANALYSIS_COMPILER);
    assert(!analysis.valid && analysis.original.diagnostics && source_length(analysis.original.diagnostics));
    SourceIssue issue = {0}; assert(source_analysis_admit_test(&analysis, NULL, &issue) == ACQUISITION_CLOSURE_PREPARATION_FAILED); finish();
}
/** Every original/probe result allocation refuses without false readiness or healthy Open. */
static void it_retires_every_original_and_probe_allocation_failure(void) {
    prepare("generated/base.json"); size_t before = allocation_count;
    assert(source_analysis_check(&analysis, &view, "../../../../../../ama", "generated/base.json") == SOURCE_ANALYSIS_OK);
    size_t original_count = allocation_count - before; finish(); assert(original_count);
    for (size_t index = 1; index <= original_count; index++) {
        prepare("generated/base.json"); failure_at = allocation_count + index;
        assert(source_analysis_check(&analysis, &view, "../../../../../../ama", "generated/base.json") != SOURCE_ANALYSIS_OK && !analysis.valid);
        failure_at = 0; finish();
    }
    prepare("generated/base.json");
    assert(source_analysis_check(&analysis, &view, "../../../../../../ama", "generated/base.json") == SOURCE_ANALYSIS_OK);
    const DeliverySourceDefinition *candidate = definition("//native"); SourceIssue issue = {0}; before = allocation_count;
    assert(source_analysis_admit_test(&analysis, candidate, &issue) == ACQUISITION_CLOSURE_OK);
    size_t probe_count = allocation_count - before; finish(); assert(probe_count);
    for (size_t index = 1; index <= probe_count; index++) {
        prepare("generated/base.json");
        assert(source_analysis_check(&analysis, &view, "../../../../../../ama", "generated/base.json") == SOURCE_ANALYSIS_OK);
        candidate = definition("//native"); failure_at = allocation_count + index;
        AcquisitionClosureStatus status = source_analysis_admit_test(&analysis, candidate, &issue);
        assert(status != ACQUISITION_CLOSURE_OK && status != ACQUISITION_CLOSURE_OPEN_TEST);
        failure_at = 0; finish();
    }
}
/** Execute original-domain, T235 admission, graph-drift and failure ownership contracts. */
void run(void) {
    generate();
    it_admits_real_direct_test_candidates_and_reuses_owned_cached_proofs();
    it_refuses_foreign_static_and_unprepared_candidate_handles();
    it_rejects_changed_probe_graph_and_retains_failure_without_retry();
    it_retains_failed_original_compiler_diagnostics_without_exposing_a_domain();
    it_retires_every_original_and_probe_allocation_failure();
}
/** Also supply a controlled shell-free compiler producer for graph-drift acceptance. */
int main(int argc, char **argv) {
    if (argc == 4 || argc == 6) {
        assert(!strcmp(argv[1], "--source-domain") && !strcmp(argv[2], "--snapshot"));
        if (argc == 6) { assert(!strcmp(argv[4], "--probe-entry")); }
        return producer(argc == 6);
    }
    executable = argv[0]; run(); return 0;
}
