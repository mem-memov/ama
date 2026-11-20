/** @file
 * @brief Check report ownership after source snapshots expire and exhaust every allocation prefix.
 */
#include "common.c"
#include "tools/relocation/report/report.h"

/** Select one report path using its exact physical suffix, not a module-name guess. */
static const RelocationReportedFile *reported(const RelocationReport *report, const char *suffix) {
    size_t length = strlen(suffix);
    for (const RelocationReportedFile *row = report->files; row != NULL; row = row->next) {
        size_t total = strlen(row->before);
        if (total >= length && strcmp(row->before+total-length,suffix) == 0) { return row; }
    }
    return NULL;
}
/** Prepared source/configuration roles are copied before the compiler-side plan is destroyed. */
static void it_owns_complete_report_scope_after_the_plan_expires(void) {
    prepare("owned-report");
    file("project/packages/test/fixture/v1.0.0/src/old/draft","unfinished\n.old.a\n");
    file("project/packages/test/fixture/v1.0.0/src/old/helper.c","/* Opaque .old.a text */\n");
    folder("artifacts");
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    reset_preview();
    const char *excluded[] = {"out"};
    roots[0].excluded = excluded;
    roots[0].excluded_count = 1;
    assert(relocation_preview_build(&preview,&move_request));
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    relocation_staging_init(&staging,sources.allocator);
    const char *outputs[] = {path("artifacts/missing.bin")};
    assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references,.entry="/test/fixture/v1.0.0/src/old/a",
        .generated_outputs=outputs,.generated_output_count=1}));
    RelocationReport report;
    relocation_report_init(&report,sources.allocator);
    assert(relocation_report_build(&report,&staging));
    retire_plan();
    assert(report.ready && report.rebuild_required && strcmp(report.entry,"/test/fixture/v1.0.0/src/new/a") == 0);
    assert(report.before_root_count == 2 && report.after_root_count == 2);
    assert(report.before_roots[0].kind == MODULE_ROOT_PROJECT);
    assert(report.before_roots[0].excluded_count == 1 && strcmp(report.before_roots[0].excluded[0],"out") == 0);
    const RelocationReportedFile *draft = reported(&report,"/draft"), *opaque = reported(&report,"/helper.c");
    const RelocationReportedFile *importer = reported(&report,"/unused"), *output = reported(&report,"/missing.bin");
    assert(draft != NULL && draft->draft && !draft->rewritten && !draft->opaque && draft->existed);
    assert(opaque != NULL && opaque->opaque && !opaque->rewritten && !opaque->invalidated);
    assert(importer != NULL && importer->rewritten && !importer->opaque);
    assert(output != NULL && output->invalidated && !output->existed);
    size_t count = 0;
    for (const RelocationReportedModule *row = report.modules; row != NULL; row = row->next) {
        if (strcmp(row->before_identity,"/test/fixture/v1.0.0/src/old/draft") == 0) { assert(!row->active && strcmp(row->after_identity,"/test/fixture/v1.0.0/src/new/draft") == 0); }
        count++;
    }
    assert(count == 3);
    relocation_report_destroy(&report);
    finish();
    assert(live_allocations == 0);
}
/** Every failed deep-copy prefix publishes no partial report and releases all independent strings on destruction. */
static void it_releases_every_failed_report_copy_prefix(void) {
    prepare("report-allocations");
    size_t baseline = live_allocations;
    RelocationReport report;
    relocation_report_init(&report,sources.allocator);
    allocation_count = 0;
    assert(relocation_report_build(&report,&staging));
    size_t measured = allocation_count;
    relocation_report_destroy(&report);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        relocation_report_init(&report,sources.allocator);
        allocation_count = 0;
        failure_at = ordinal;
        assert(!relocation_report_build(&report,&staging));
        assert(!report.ready && report.status == RELOCATION_OUT_OF_MEMORY);
        failure_at = 0;
        relocation_report_destroy(&report);
        assert(live_allocations == baseline);
    }
    done();
}
/** Exercise the actual report/source ownership boundary and failed allocation cleanup. */
void run(void) {
    it_owns_complete_report_scope_after_the_plan_expires();
    it_releases_every_failed_report_copy_prefix();
}
/** Report owners can outlive their input plans but never their allocator authority. */
int main(void) { run(); return EXIT_SUCCESS; }
