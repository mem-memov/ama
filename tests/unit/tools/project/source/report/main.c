/** @file
 * @brief Verify source-domain JSON with real checked inputs and independent Python decoding.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include <sys/wait.h>
#include <unistd.h>
#include "../../../../compiler/common.c"
#include "delivery/source/domain/domain.h"
#include "delivery/source/report/report.h"
static SourceArena exports;
static const DeliverySourceDefinition *definitions;
/** Invoke only the independent decoder, never a shell or native/Ama program. */
static void verify(const char *mode) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", "verify.py", mode, (char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Capture source/configuration bytes without any physical source files. */
static void prepare(bool populated) {
    setup_frontend(); source_arena_init(&exports, sources.allocator); definitions = NULL;
    const char *names[] = {"//main", "//palette", "//element", "//helper", "//marker", "//native", "//draft"};
    const char *texts[] = {"@!\nitems[]\n ~chosen[]\n", "@=\nvalues[]\n//element\n", "@!\n",
        "@<\nvalue\n.T\n@!\n", "@...\n", "@C\n", "unfinished !\n"};
    ModuleSnapshotEntry entries[7]; size_t count = populated ? 7 : 0;
    for (size_t index = 0; index < count; index++) {
        const SourceText *text = NULL, *configuration = NULL;
        const char *path = index == 0 ? "physical/\xffsource" : names[index];
        SourceStatus added = source_manager_add(&sources, path, (const unsigned char *)texts[index], strlen(texts[index]), &text);
        assert(added == SOURCE_OK);
        if (!index) {
            const char *bytes = "~chosen = //palette/values\n";
            added = source_manager_add(&sources, "captured-module.ini", (const unsigned char *)bytes, strlen(bytes), &configuration);
            assert(added == SOURCE_OK);
        }
        entries[index] = (ModuleSnapshotEntry){names[index], text, 1, index + 1, configuration};
    }
    ModuleStatus captured = module_catalog_snapshot(&frontend.project.catalog, entries, count);
    assert(captured == MODULE_OK);
    if (populated) {
        const SourceText *configuration = NULL, *abi = NULL;
        SourceStatus added = source_manager_add(&sources, "captured-package.ini", (const unsigned char *)"# metadata\n", 11, &configuration);
        assert(added == SOURCE_OK);
        captured = module_catalog_capture_version(&frontend.project.catalog, "/vendor/pkg/v1.0.0", "physical/version", configuration);
        assert(captured == MODULE_OK);
        const ModuleSource *native = NULL;
        captured = module_catalog_find(&frontend.project.catalog, "//native", &native); assert(captured == MODULE_OK);
        const char *bytes = "+ initialize\n- destroy\n";
        added = source_manager_add(&sources, "captured-native.abi", (const unsigned char *)bytes, strlen(bytes), &abi);
        assert(added == SOURCE_OK);
        captured = module_catalog_snapshot_native(&frontend.project.catalog, native, "missing-body.c", "missing-header.h", abi);
        assert(captured == MODULE_OK);
    }
    bool checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
    if (!checked) { fprintf(stderr, "Report input refused: %s\n", frontend.error.message); }
    assert(checked);
}
/** Retire exported spans before their compiler snapshot owners. */
static void finish(void) { source_arena_destroy(&exports); destroy_frontend(); }
/** The independent decoder verifies topology, categories, scopes and lossless
 * physical paths without parsing C output or rerunning compiler algorithms. */
static void it_round_trips_real_checked_domain_configuration_native_and_edge_facts(void) {
    prepare(true);
    FILE *output = fopen("generated/domain.json", "wb"); assert(output);
    bool written = delivery_source_report(output, &frontend.project.catalog, definitions);
    assert(written && fclose(output) == 0);
    verify("populated");
    finish();
}
/** No implicit entry or source record is manufactured for an empty domain. */
static void it_reports_empty_domains_as_complete_empty_arrays(void) {
    prepare(false);
    FILE *output = fopen("generated/domain.json", "wb"); assert(output);
    bool written = delivery_source_report(output, &frontend.project.catalog, definitions);
    assert(written && fclose(output) == 0);
    verify("empty");
    finish();
}
/** Both immediate output refusal and final flush failure return failure while
 * retaining stream ownership at the caller. */
static void it_refuses_write_flush_invalid_owner_and_invalid_category_results(void) {
    prepare(false);
    FILE *output = fopen("main.c", "rb"); assert(output);
    bool written = delivery_source_report(output, &frontend.project.catalog, definitions);
    assert(!written && fseek(output, 0, SEEK_SET) == 0 && fclose(output) == 0);
    output = fopen("/dev/full", "wb"); assert(output);
    written = delivery_source_report(output, &frontend.project.catalog, definitions);
    assert(!written); (void)fclose(output);
    output = tmpfile(); assert(output);
    ModuleCatalog absent = {0};
    written = delivery_source_report(output, &absent, NULL); assert(!written && ftell(output) == 0);
    written = delivery_source_report(NULL, &frontend.project.catalog, NULL); assert(!written);
    written = delivery_source_report(output, NULL, NULL); assert(!written && ftell(output) == 0);
    DeliverySourceDefinition invalid = {.identity = "//bad", .kind = (DeliverySourceKind)99};
    written = delivery_source_report(output, &frontend.project.catalog, &invalid); assert(!written);
    assert(fclose(output) == 0);
    finish();
}
/** Explicitly execute semantic transport, empty and real output refusal cases. */
void run(void) {
    it_round_trips_real_checked_domain_configuration_native_and_edge_facts();
    it_reports_empty_domains_as_complete_empty_arrays();
    it_refuses_write_flush_invalid_owner_and_invalid_category_results();
}
/** Keep the C assertion harness authoritative; Python checks decoded observations. */
int main(void) { run(); return 0; }
